//===- MMIXObjectFile.cpp - MMIX object file binding ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the MMIXObjectFile class, which binds the MMIXObject
// class to the generic ObjectFile wrapper.
//
//===----------------------------------------------------------------------===//

#include "llvm/Object/MMIXObjectFile.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/Endian.h"

using namespace std;
using namespace llvm;
using namespace object;
using namespace support::endian;

namespace {
inline const char *ToCharP(const unsigned char *p) {
  return reinterpret_cast<const char *>(p);
}
} // namespace

bool MMIXObjectFile::classof(Binary const *v) { return v->isMMO(); }

// constructor
Expected<std::unique_ptr<MMIXObjectFile>>
MMIXObjectFile::create(MemoryBufferRef Object) {
  std::unique_ptr<MMIXObjectFile> Obj(new MMIXObjectFile(Object));
  assert(Obj->getData().size() % 4 == 0 && "Invalid MMO size");
  auto ObjDataIter = Obj->getData().bytes_begin();
  Error Status = Obj->initPreamble(ObjDataIter);
  if (Status) {
    return std::move(Status);
  }
  Status = Obj->initContent(ObjDataIter);
  if (Status) {
    return std::move(Status);
  }
  Status = Obj->initPostamble(ObjDataIter);
  if (Status) {
    return std::move(Status);
  }
  Status = Obj->initSymbolTable(ObjDataIter);
  if (Status) {
    return std::move(Status);
  }
  Obj->initSymbolTrie();
  return std::move(Obj);
}

Error MMIXObjectFile::initPreamble(const unsigned char *&Iter) {
  auto Start = Iter;
  assert(*Iter == MMO::MM && "Invalid MM");
  assert(Iter[1] == MMO::LOP_PRE && "Invalid file header");
  Preamble.Version = Iter[2];
  uint8_t TetraCount = Iter[3];
  Iter += 4;
  if (TetraCount > 0) {
    Preamble.CreatedTime = support::endian::read32be(Iter);
    Iter += 4;
    if (TetraCount > 1) {
      Preamble.ExtraData = ArrayRef<std::uint8_t>(Iter, (TetraCount - 1) * 4);
    }
    Iter += (TetraCount - 1) * 4;
  }
  return Error::success();
}

Error MMIXObjectFile::initContent(const unsigned char *&Iter) {
  const unsigned char *CurrentPos = Iter;
  bool ReachPostamble = false;
  Error Err = Error::success();
  if (Err) return Err; // check first, even if it is success.
  while (!ReachPostamble && Iter != getData().bytes_end()) {
    if (Iter[0] == MMO::MM) {
      if (auto Len = Iter - CurrentPos; Len != 0) {
        if (Len % 4 != 0) {
          return createError("Binary data should  be 4 bytes aligned.");
        } else {
          Content.emplace_back(ArrayRef<uint8_t>(CurrentPos, Len));
        }
      }

      // handle lop_xxx
      switch (Iter[1]) {
      case MMO::LOP_QUOTE:
        Content.emplace_back(MMO::Quote(Iter));
        break;
      case MMO::LOP_LOC:
        Content.emplace_back(MMO::Loc(Iter, Err));
        break;
      case MMO::LOP_SKIP:
        Content.emplace_back(MMO::Skip(Iter));
        break;
      case MMO::LOP_FIXO:
        Content.emplace_back(MMO::Fixo(Iter, Err));
        break;
      case MMO::LOP_FIXR:
        Content.emplace_back(MMO::Fixr(Iter));
        break;
      case MMO::LOP_FIXRX:
        Content.emplace_back(MMO::Fixrx(Iter, Err));
        break;
      case MMO::LOP_FILE:
        Content.emplace_back(MMO::File(Iter));
        break;
      case MMO::LOP_LINE:
        Content.emplace_back(MMO::Line(Iter));
        break;
      case MMO::LOP_SPEC:
        Content.emplace_back(MMO::Spec(Iter));
        break;
      case MMO::LOP_POST:
        ReachPostamble = true;
        break;
      default:
        break;
      }

      // CurrentPos now should point to normal binary content;
      CurrentPos = Iter;
    } else {
      Iter += 4;
    }
  }
  if (Err)
    return Err;

  if (Iter[1] != MMO::LOP_POST) {
    return createError("Invalid mmo file, can't find lop_post.");
  } else {
    return Error::success();
  }
}

Error MMIXObjectFile::initSymbolTable(const unsigned char *&Iter) {
  Iter += 4;
  return decodeSymbolTable(Iter);
}

void MMIXObjectFile::initSymbolTrie() {
  if (auto Root = get_if<MMOTrieUTF8>(&TrieRoot)) {
    for (const auto &S : SymbTab) {
      Root->insert(S);
    }
    Root->prune();
  } else if (auto Root = get_if<MMOTrieUTF16>(&TrieRoot)) {
    for (const auto &S : SymbTab) {
      Root->insert(S);
    }
    Root->prune();
  }
}

Error MMIXObjectFile::initPostamble(const unsigned char *&Iter) {
  assert(Iter[0] == MMO::MM && Iter[1] == MMO::LOP_POST);
  if (Iter[2] != 0)
    return createError("invalid postamble!");
  Postamble.G = Iter[3];
  if (Postamble.G < 32)
    return createError("invalid 'G' in postamble, must >= 32!");
  Iter += 4;
  for (int i = 0; i != 256 - Postamble.G; ++i) {
    Postamble.Values.push_back(read64be(Iter));
    Iter += 8;
  }
  return Error::success();
}

Error MMIXObjectFile::decodeSymbolTable(const unsigned char *&Iter) {
  SmallVector<UTF16, 32> SymbolName;
  Error E = Error::success();
  decodeSymbolTable(Iter, SymbolName, E);
  std::sort(SymbTab.begin(), SymbTab.end(),
            [](const MMO::Symbol &S1, const MMO::Symbol &S2) {
              return S1.Serial < S2.Serial;
            });
  return std::move(E);
}

void MMIXObjectFile::decodeSymbolTable(const uint8_t *&Start,
                                       SmallVector<UTF16, 32> &Name, Error &E) {
  if (E) return;
  const uint8_t M = *Start++; /* the master control byte */

  if (M & 0x40) {
    decodeSymbolTable(Start, Name,
                      E); // traverse the left subtrie, if it is nonempty
  }

  if (M & 0x2F) {
    UTF16 C = 0;
    if (M & 0x80) { // 16-bit character
      if (holds_alternative<monostate>(TrieRoot)) {
        TrieRoot = MMOTrieUTF16();
      } else if (holds_alternative<MMOTrieUTF8>(TrieRoot)) {
        E = createError("All symbol names must have the same encoding.");
        return;
      }
      C = support::endian::read16be(Start);
      Start += 2;
    } else {
      if (holds_alternative<monostate>(TrieRoot)) {
        TrieRoot = MMOTrieUTF8();
      } else if (holds_alternative<MMOTrieUTF16>(TrieRoot)) {
        E = createError("All symbol names must have the same encoding.");
        return;
      }
      C = *Start++;
    }
    Name.push_back(C);
    if (M & 0xF) { // complete
      auto readBytes = [&Start](const std::size_t &K) {
        uint64_t V = 0;
        for (std::size_t i = 0; i != K; ++i) {
          V <<= 8;
          V += *Start++;
        }
        return V;
      };
      MMO::Symbol S;
      // convert to UTF-8 encode
      std::string UTF8Name;
      if (holds_alternative<MMOTrieUTF16>(TrieRoot)) {
        if (!convertUTF16ToUTF8String(Name, UTF8Name)) {
          E = createError("invalid symbol name");
          return;
        }
      } else {
        for (const auto &C : Name) {
          UTF8Name += static_cast<char>(C);
        }
      }
      S.Name = UTF8Name;
      uint8_t J = M & 0xF;
      if (J == 0xF) { // register
        S.Type = MMO::REGISTER;
        S.Equiv = *Start++;
      } else if (J <= 8) { // symbol
        S.Equiv = readBytes(J);
        if (J == 2 && S.Equiv == 0) {
          S.Type = MMO::UNDEFINED;
        }
      } else {
        S.Equiv = readBytes(J - 8);
        S.Equiv += 0x20'00'00'00'00'00'00'00;
      }
      // set print position
      auto ObjBegin = getData().bytes_begin();
      S.PrintPos = Start + (4 - ((Start - ObjBegin) % 4));
      assert((S.PrintPos - ObjBegin) % 4 == 0);
      // serial number
      uint32_t SN = *Start++;
      S.Serial = SN;
      while (!(S.Serial & 0x80)) {
        S.Serial <<= 7;
        S.Serial += SN;
      }
      S.Serial -= 128;
      SymbTab.push_back(S);
    }

    if (M & 0x20) {
      decodeSymbolTable(Start, Name, E); // traverse the middle subtrie
    }
    Name.pop_back();
  }

  if (M & 0x10) {
    decodeSymbolTable(Start, Name,
                      E); // traverse the right subtrie, if it is nonempty
  }
}

MMIXObjectFile::MMIXObjectFile(MemoryBufferRef Object)
    : ObjectFile(ID_MMO, Object) {}

// The number of bytes used to represent an address in this object
// file format.
uint8_t MMIXObjectFile::getBytesInAddress() const { return 8; }

StringRef MMIXObjectFile::getFileFormatName() const { return "MMO-mmix"; }

Triple::ArchType MMIXObjectFile::getArch() const {
  return Triple::ArchType::mmix;
}

SubtargetFeatures MMIXObjectFile::getFeatures() const {
  SubtargetFeatures Features;
  return Features;
}

Optional<StringRef> MMIXObjectFile::tryGetCPUName() const {
  return StringRef("MMIX");
}

Expected<uint64_t> MMIXObjectFile::getStartAddress() const {
  // return the address of symbol `Main` otherwise 0
  const auto &It =
      std::find_if(SymbTab.begin(), SymbTab.end(), [](const MMO::Symbol &S) {
        return S.Name.compare(":Main") == 0;
      });
  if (It == SymbTab.end()) {
    return 0;
  } else {
    return It->Equiv;
  }
}

bool MMIXObjectFile::isRelocatableObject() const { return false; }

// Interface from SymbolicFile
void MMIXObjectFile::moveSymbolNext(DataRefImpl &Symb) const {
  auto Next = reinterpret_cast<decltype(SymbTab.begin())>(Symb.p);
  Symb.p = reinterpret_cast<uintptr_t>(Next + 1);
}

Expected<uint32_t> MMIXObjectFile::getSymbolFlags(DataRefImpl Symb) const {
  return 0;
}

basic_symbol_iterator MMIXObjectFile::symbol_begin() const {
  DataRefImpl DataRef;
  DataRef.p = reinterpret_cast<uintptr_t>(SymbTab.begin());
  BasicSymbolRef BSR(DataRef, this);
  return BSR;
}
basic_symbol_iterator MMIXObjectFile::symbol_end() const {
  DataRefImpl DataRef;
  DataRef.p = reinterpret_cast<uintptr_t>(SymbTab.end());
  BasicSymbolRef BSR(DataRef, this);
  return BSR;
}

section_iterator MMIXObjectFile::section_begin() const {
  SectionRef SR;
  section_iterator I(SR);
  return I;
}
section_iterator MMIXObjectFile::section_end() const {
  SectionRef SR;
  section_iterator I(SR);
  return I;
}

// Interface from ObjectFile
// SymbolRef
Expected<StringRef> MMIXObjectFile::getSymbolName(DataRefImpl Symb) const {
  auto S = reinterpret_cast<SymbItT>(Symb.p);
  return S->Name;
}
Expected<uint64_t> MMIXObjectFile::getSymbolAddress(DataRefImpl Symb) const {
  auto S = reinterpret_cast<SymbItT>(Symb.p);
  return S->Equiv;
}
uint64_t MMIXObjectFile::getSymbolValueImpl(DataRefImpl Symb) const {
  auto S = reinterpret_cast<SymbItT>(Symb.p);
  return S->Equiv;
}
uint64_t MMIXObjectFile::getCommonSymbolSizeImpl(DataRefImpl Symb) const {
  return 4;
}
Expected<SymbolRef::Type>
MMIXObjectFile::getSymbolType(DataRefImpl Symb) const {
  return SymbolRef::Type::ST_Unknown;
}
Expected<section_iterator>
MMIXObjectFile::getSymbolSection(DataRefImpl Symb) const {
  SectionRef SR;
  section_iterator I(SR);
  return I;
}

// SectionRef
void MMIXObjectFile::moveSectionNext(DataRefImpl &Sec) const {}
Expected<StringRef> MMIXObjectFile::getSectionName(DataRefImpl Sec) const {
  return StringRef("");
}
uint64_t MMIXObjectFile::getSectionAddress(DataRefImpl Sec) const { return 0; }
uint64_t MMIXObjectFile::getSectionIndex(DataRefImpl Sec) const { return 0; }
uint64_t MMIXObjectFile::getSectionSize(DataRefImpl Sec) const { return 0; }
Expected<ArrayRef<uint8_t>>
MMIXObjectFile::getSectionContents(DataRefImpl Sec) const {
  ArrayRef<uint8_t> AR;
  return AR;
}
uint64_t MMIXObjectFile::getSectionAlignment(DataRefImpl Sec) const {
  return 4;
}
bool MMIXObjectFile::isSectionCompressed(DataRefImpl Sec) const {
  return false;
}
bool MMIXObjectFile::isSectionText(DataRefImpl Sec) const { return false; }
bool MMIXObjectFile::isSectionData(DataRefImpl Sec) const { return false; }
bool MMIXObjectFile::isSectionBSS(DataRefImpl Sec) const {
  return false;
} // no bss concept
// A section is 'virtual' if its contents aren't present in the object image.
bool MMIXObjectFile::isSectionVirtual(DataRefImpl Sec) const {
  return false;
} // no virtual concept

relocation_iterator MMIXObjectFile::section_rel_begin(DataRefImpl Sec) const {
  RelocationRef RR;
  relocation_iterator RI(RR);
  return RI;
}
relocation_iterator MMIXObjectFile::section_rel_end(DataRefImpl Sec) const {
  RelocationRef RR;
  relocation_iterator RI(RR);
  return RI;
}

// RelocationRef
void MMIXObjectFile::moveRelocationNext(DataRefImpl &Rel) const {}
uint64_t MMIXObjectFile::getRelocationOffset(DataRefImpl Rel) const {
  return 0;
}
symbol_iterator MMIXObjectFile::getRelocationSymbol(DataRefImpl Rel) const {
  SymbolRef SR;
  symbol_iterator SI(SR);
  return SI;
}
uint64_t MMIXObjectFile::getRelocationType(DataRefImpl Rel) const { return 0; }
void MMIXObjectFile::getRelocationTypeName(
    DataRefImpl Rel, SmallVectorImpl<char> &Result) const {}

const MMIXObjectFile::ContentT &MMIXObjectFile::getMMOContent() const {
  return Content;
}

const MMO::Symbol &MMIXObjectFile::getMMOSymbol(const DataRefImpl &Symb) const {
  auto S = reinterpret_cast<SymbItT>(Symb.p);
  return *S;
}

const MMO::Symbol &MMIXObjectFile::getMMOSymbol(const SymbolRef &Symb) const {
  return getMMOSymbol(Symb.getRawDataRefImpl());
}

const MMO::Pre &MMIXObjectFile::getMMOPreamble() const { return Preamble; }

const MMO::Post &MMIXObjectFile::getMMOPostamble() const {
  return Postamble;
}

bool MMIXObjectFile::isSymbolNameUTF16() const {
  return holds_alternative<MMOTrieUTF16>(TrieRoot);
}

//////////////////////////////////////////////////////////////

static Expected<std::unique_ptr<MMIXObjectFile>>
createPtr(MemoryBufferRef Object) {
  auto ObjOrErr = MMIXObjectFile::create(Object);
  if (Error E = ObjOrErr.takeError()) {
    return std::move(E);
  }
  return std::move(*ObjOrErr);
}

Expected<std::unique_ptr<MMIXObjectFile>>
ObjectFile::createMMIXObjectFile(MemoryBufferRef Object) {
  if (Object.getBufferSize() % 4 != 0) {
    return make_error<GenericBinaryError>("Invalid MMO file alignment",
                                          object_error::invalid_file_type);
  }
  return createPtr(Object);
}
