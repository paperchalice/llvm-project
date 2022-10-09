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
}

MMOTrie::MMOTrie(): Root(new MMOTrieNode()) {}
void MMOTrie::insertSymbol(const MMOSymbol &S) {
  
}

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
  for (; Iter != getData().bytes_end(); Iter += 4) {
    if (Iter[0] == MMO::MM) {
      if (Iter - CurrentPos != 0) {
        assert((Iter - CurrentPos)%4 == 0);
        Content.emplace_back(ArrayRef<uint8_t>(
            CurrentPos, Iter - CurrentPos));
      }
      switch (Iter[1]) {
      case MMO::LOP_QUOTE: {
        Iter += 4;
        MMOLOp::Quote QuotedVal = {
            ArrayRef<uint8_t>(Iter, 4)};
        MMOLOp OpQuote = {QuotedVal};
        Content.emplace_back(OpQuote);
      } break;
      case MMO::LOP_LOC: {
        MMOLOp::Loc LocVal;
        uint8_t TetraCount = Iter[3];
        LocVal.HighByte = Iter[2];
        Iter += 4;
        switch (TetraCount) {
        case 1:
          LocVal.Offset = read32be(Iter);
          break;
        case 2:
          LocVal.Offset = read64be(Iter);
          Iter += 4;
          break;
        default:
          return createError("Z field of lop_loc must be 1 or 2!");
        }
        Content.emplace_back(MMOLOp{LocVal});
      } break;
      case MMO::LOP_SKIP: {
        MMOLOp::Skip S = {read16be(Iter + 2)};
        Content.emplace_back(MMOLOp{S});
      } break;
      case MMO::LOP_FIXO: {
        MMOLOp::Fixo F;
        F.HighByte = Iter[1];
        uint8_t TetraCount = Iter[3];
        Iter += 4;
        switch (TetraCount) {
        case 1:
          F.Offset = read32be(Iter);
          break;
        case 2:
          F.Offset = read64be(Iter);
          Iter += 4;
          break;
        default:
          return createError("Z field of lop_loc must be 1 or 2!");
        }
        Content.emplace_back(MMOLOp{F});
      } break;
      case MMO::LOP_FIXR: {
        MMOLOp::Fixr F = {read16be(Iter + 2)};
        Content.emplace_back(MMOLOp{F});
      } break;
      case MMO::LOP_FIXRX: {
        uint8_t Z = Iter[3];
        if (!(Z == 16 || Z == 24)) {
          return createError("Z field of lop_fixrx must be 16 or 24!");
        }
        Iter += 4;
        MMOLOp::Fixrx F = {Z, read32be(Iter)};
        Content.emplace_back(MMOLOp{F});
      } break;
      case MMO::LOP_FILE: {
        MMOLOp::File F = {Iter[2]};
        uint8_t TetraCount = Iter[3];
        if (TetraCount != 0) {
          F.FileName =
              StringRef(ToCharP(Iter + 4), 4 * TetraCount);
          Iter += 4 * TetraCount;
        }
        Content.emplace_back(MMOLOp{F});
      } break;
      case MMO::LOP_LINE: {
        MMOLOp::Line L = {read16be(Iter + 2)};
        Content.emplace_back(MMOLOp{L});
      } break;
      case MMO::LOP_SPEC: {
        MMOLOp::Spec S = {read16be(Iter + 2)};
        Content.emplace_back(MMOLOp{S});
      } break;
      case MMO::LOP_POST:
        ReachPostamble = true;
      default:
        break;
      }
      if (ReachPostamble) {
        break;
      }
      // Set current content position to the tetra after the lop
      CurrentPos = Iter + 4;
    }
  }
  assert(Iter[1] == MMO::LOP_POST && "invalid postamble must follow instructions and data");
  return Error::success();
}

Error MMIXObjectFile::initSymbolTable(const unsigned char *&Iter) {
  Iter+=4;
  return decodeSymbolTable(Iter);
}

void MMIXObjectFile::initSymbolTrie() {

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
  // std::sort(SymbTab.begin(), SymbTab.end(),
  //           [](const MMOSymbol &S1, const MMOSymbol &S2) {
  //             return S1.SerialNumber < S2.SerialNumber;
  //           });
  return std::move(E);
}

void MMIXObjectFile::decodeSymbolTable(const uint8_t *&Start,
                                       SmallVector<UTF16, 32> &Name, Error &E) {
  const uint8_t M = *Start++; /* the master control byte */

  if (M & 0x40) {
    decodeSymbolTable(Start, Name,
                      E); // traverse the left subtrie, if it is nonempty
  }

  if (M & 0x2F) {
    UTF16 C = 0;
    if (M & 0x80) { // 16-bit character
      C = support::endian::read16be(Start);
      Start += 2;
    } else {
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
      MMOSymbol S;
      // convert to UTF-8 encode
      std::string UTF8Name;
      if (!convertUTF16ToUTF8String(Name, UTF8Name)) {
        E = createError("invalid symbol name");
      }
      S.Name = UTF8Name;
      uint8_t J = M & 0xF;
      if (J == 0xF) { // register
        S.Type = MMO::REGISTER;
        S.Address = *Start++;
      } else if (J <= 8) { // symbol
        S.Address = readBytes(J);
        if (J == 2 && S.Address == 0) {
          S.Type = MMO::UNDEFINED;
        }
      } else {
        S.Address = readBytes(J - 8);
        S.Address += 0x20'00'00'00'00'00'00'00;
      }
      // set print position
      auto ObjBegin = getData().bytes_begin();
      S.PrintPos = ObjBegin + ((Start - ObjBegin) & ~3);
      assert((S.PrintPos - ObjBegin) % 4 == 0);
      // serial number
      uint32_t SN = *Start++;
      S.SerialNumber = SN;
      while (!(S.SerialNumber & 0x80)) {
        S.SerialNumber <<= 7;
        S.SerialNumber += SN;
      }
      S.SerialNumber -= 128;
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
      std::find_if(SymbTab.begin(), SymbTab.end(), [](const MMOSymbol &S) {
        return S.Name.compare(":main") == 0;
      });
  if (It == SymbTab.end()) {
    return 0;
  } else {
    return It->Address;
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
  return S->Address;
}
uint64_t MMIXObjectFile::getSymbolValueImpl(DataRefImpl Symb) const {
  auto S = reinterpret_cast<SymbItT>(Symb.p);
  return S->Address;
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

const MMOSymbol &MMIXObjectFile::getMMOSymbol(const DataRefImpl &Symb) const {
  auto S = reinterpret_cast<SymbItT>(Symb.p);
  return *S;
}

const MMOSymbol &MMIXObjectFile::getMMOSymbol(const SymbolRef &Symb) const {
  return getMMOSymbol(Symb.getRawDataRefImpl());
}

const MMOPreamble &MMIXObjectFile::getMMOPreamble() const { return Preamble; }

const MMOPostamble &MMIXObjectFile::getMMOPostamble() const {
  return Postamble;
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
