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
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/TargetParser/SubtargetFeature.h"

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <variant>

using llvm::support::endian::read16be;
using llvm::support::endian::read32be;
using llvm::support::endian::read64be;
using namespace llvm;
using namespace object;

static std::uint8_t getInst(const unsigned char *A) { return A[0]; }
static std::uint8_t getX(const unsigned char *A) { return A[1]; }
static std::uint8_t getY(const unsigned char *A) { return A[2]; }
static std::uint8_t getZ(const unsigned char *A) { return A[3]; }
static std::uint16_t getYZ(const unsigned char *A) { return read16be(A + 2); }

bool MMIXObjectFile::classof(Binary const *v) { return v->isMMO(); }

// constructor
Expected<std::unique_ptr<MMIXObjectFile>>
MMIXObjectFile::create(MemoryBufferRef Object, bool InitContent) {
  std::unique_ptr<MMIXObjectFile> Obj(new MMIXObjectFile(Object));
  if (InitContent) {
    Error Status = Obj->initMMIXObjectFile();
    if (Status) {
      return std::move(Status);
    }
  }
  return std::move(Obj);
}

Error MMIXObjectFile::initMMIXObjectFile() {
  auto ObjDataIter = getData().bytes_begin();
  Error Status = initPreamble(ObjDataIter);
  if (Status) {
    return std::move(Status);
  }
  Status = initContent(ObjDataIter);
  if (Status) {
    return std::move(Status);
  }
  Status = initPostamble(ObjDataIter);
  if (Status) {
    return std::move(Status);
  }
  if (getInst(ObjDataIter) == MMO::MM && getX(ObjDataIter) == MMO::LOP_STAB) {
    Status = initSymbolTable(ObjDataIter);
    if (Status) {
      return std::move(Status);
    }
  }
  return Error::success();
}

Error MMIXObjectFile::initPreamble(const unsigned char *&Iter) {
  assert(getInst(Iter) == MMO::MM && "Invalid MM");
  assert(getX(Iter) == MMO::LOP_PRE && "Invalid file header");
  auto PreambleBegin = Iter;
  Preamble.Version = getY(Iter);
  std::int32_t TetraCount = getZ(Iter);
  Iter += 4;
  if (TetraCount > 0) {
    time_t Time = support::endian::read32be(Iter);
    Preamble.CreatedTime = Time;
    Iter += 4;
    --TetraCount;
    if (TetraCount) {
      Preamble.ExtraData = ArrayRef<std::uint8_t>(Iter, TetraCount * 4);
    }
    Iter += 4 * TetraCount;
  }
  Preamble.RawData = {PreambleBegin, Iter};
  return Error::success();
}

Error MMIXObjectFile::initContent(const unsigned char *&Iter) {
  bool ReachPostamble = false;
  bool LastIsLOP = true;
  const std::uint8_t *BinRefStart = Iter;
  std::uint64_t PC = 0;
  Error Err = Error::success();
  while (!ReachPostamble && Iter < DataEnd) {
    // handle LOP
    if (getInst(Iter) == MMO::MM) {
      if (!LastIsLOP && Iter != BinRefStart) {
        if (getX(Iter) == MMO::LOP_QUOTE) {
          PC += 4;
          Iter += 8;
          continue;
        }

        MMOData D{PC, ArrayRef<std::uint8_t>(BinRefStart, Iter)};
        D.StartAddress -= D.Data.size();
        Content.emplace_back(D);
        LastIsLOP = true;
      }
      // handle lop_xxx
      switch (getX(Iter)) {
      case MMO::LOP_QUOTE:
        LastIsLOP = false;
        PC += 4;
        Iter += 8;
        break;
      case MMO::LOP_LOC: {
        auto TetraCount = getZ(Iter);
        MMOLoc Loc;
        auto LocBegin = Iter;
        Loc.HighByte = getY(Iter);
        uint64_t Offset = 0;
        Iter += 4;
        switch (TetraCount) {
        case 1:
          Offset = read32be(Iter);
          Iter += 4;
          break;
        case 2:
          Offset = read64be(Iter);
          Iter += 8;
          break;
        default:
          return createStringError(std::errc::invalid_argument,
                                   "Z field of lop_loc must be 1 or 2!");
        }
        Loc.Offset = Offset;
        Loc.RawData = {LocBegin, Iter};
        PC = ((std::uint64_t)Loc.HighByte << 54) | Offset;
        Content.emplace_back(Loc);
      } break;
      case MMO::LOP_SKIP:
        Content.emplace_back(MMOSkip{{Iter, 4}, getYZ(Iter)});
        PC += getYZ(Iter);
        Iter += 4;
        break;
      case MMO::LOP_FIXO: {
        MMOFixo Fix;
        auto FixoBegin = Iter;
        Fix.HighByte = getY(Iter);
        auto TetraCount = getZ(Iter);
        Iter += 4;
        uint64_t Offset = 0;
        switch (TetraCount) {
        case 1:
          Offset = read32be(Iter);
          Iter += 4;
          break;
        case 2:
          Offset = read64be(Iter);
          Iter += 8;
          break;
        default:
          return createStringError(std::errc::invalid_argument,
                                   "`Z` field of `lop_fixo` must be 1 or 2!");
        }
        Fix.Offset = Offset;
        Fix.RawData = {FixoBegin, Iter};
        Content.emplace_back(Fix);
      } break;
      case MMO::LOP_FIXR:
        Content.emplace_back(MMOFixr{{Iter, 4}, getYZ(Iter)});
        Iter += 4;
        break;
      case MMO::LOP_FIXRX: {
        auto FixrxBegin = Iter;
        MMOFixrx Fix;
        Fix.FixType = getZ(Iter);
        if (!(Fix.FixType == MMO::FIXRX_JMP ||
              Fix.FixType == MMO::FIXRX_OTHERWISE))
          return createStringError(std::errc::invalid_argument,
                                   "Z field in lop_fixrx must be 16 or 24!");
        Iter += 4;
        std::uint32_t Tet = read32be(Iter) & 0x00FF'FFFF;
        Fix.Delta = Iter[0] ? (Tet - (1 << Fix.FixType)) : Tet;
        Iter += 4;
        Fix.RawData = {FixrxBegin, Iter};
        Content.emplace_back(Fix);
      } break;
      case MMO::LOP_FILE: {
        auto FileBegin = Iter;
        MMOFile F;
        F.Number = getY(Iter);
        auto TetraCount = Iter[3];
        Iter += 4;
        StringRef Name;
        if (TetraCount != 0) {
          Name =
              StringRef(reinterpret_cast<const char *>(Iter), 4 * TetraCount);
        }
        F.Name = Name.trim('\0');
        Iter += 4 * TetraCount;
        F.RawData = {FileBegin, Iter};
        Content.emplace_back(F);
      }

      break;
      case MMO::LOP_LINE:
        Content.emplace_back(MMOLine{{Iter, 4}, getYZ(Iter)});
        Iter += 4;
        break;
      case MMO::LOP_SPEC: {
        ArrayRef<std::uint8_t> RawData(Iter, 4);
        auto Type = getYZ(Iter);
        Iter += 4;
        auto SpecialDataBegin = Iter;
        while (Iter < DataEnd) {
          if (getInst(Iter) == MMO::MM) {
            if (getX(Iter) == MMO::LOP_QUOTE)
              Iter += 4;
            else
              break;
          }
          Iter += 4;
        }
        MMOSpec SP = {RawData, Type, {SpecialDataBegin, Iter}};
        Content.emplace_back(SP);
        BinRefStart = Iter;
        break;
      }
      case MMO::LOP_POST:
        ReachPostamble = true;
        break;
      default:
        return createError("Unknown LOP");
      }
    } else {
      if (LastIsLOP) {
        BinRefStart = Iter;
        LastIsLOP = false;
      }

      PC += 4;
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

Error MMIXObjectFile::initPostamble(const unsigned char *&Iter) {
  assert(Iter[0] == MMO::MM && Iter[1] == MMO::LOP_POST);
  auto PostBegin = Iter;
  if (getY(Iter) != 0)
    return createError("invalid postamble!");
  Postamble.G = getZ(Iter);
  if (Postamble.G < 32)
    return createError("invalid 'G' in postamble, must >= 32!");
  Iter += 4;

  for (int i = 0; i != 256 - Postamble.G; ++i) {
    auto RegVal = read64be(Iter);
    Postamble.Values.push_back(RegVal);
    Iter += 8;
  }
  Postamble.RawData = {PostBegin, Iter};
  return Error::success();
}

Error MMIXObjectFile::decodeSymbolTable(const unsigned char *&Iter,
                                        bool SortSymbolTable) {
  SmallVector<UTF16, 32> SymbolName;
  Error E = Error::success();
  decodeSymbolTable(Iter, SymbolName, E);
  if (SortSymbolTable) {
    std::sort(SymbTab.begin(), SymbTab.end(),
              [](const MMO::Symbol &S1, const MMO::Symbol &S2) {
                return S1.Serial < S2.Serial;
              });
  }
  if (Iter >= DataEnd) {
    if (E) {
      return std::move(E);
    } else {
      return createError("malformed symbol table");
    }
  }
  return std::move(E);
}

void MMIXObjectFile::decodeSymbolTable(const unsigned char *&Start,
                                       SmallVector<UTF16, 32> &Name, Error &E) {
  if (E)
    return;

  const auto M = *Start++; // the master control byte
  if (M & 0x40) {
    decodeSymbolTable(Start, Name,
                      E); // traverse the left subtrie, if it is nonempty
  }

  if (M & 0x2F) {
    UTF16 C = 0;
    if (M & 0x80) { // 16-bit character
      // TODO: handle UTF16
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
      MMO::Symbol S;
      // convert to UTF-8 encode
      std::string UTF8Name;
      for (const auto &C : Name) {
        UTF8Name += static_cast<char>(C);
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
      S.PrintPos = ObjBegin + alignTo<4>(Start - ObjBegin);
      // serial number
      uint32_t SN = *Start++;
      S.Serial = SN;
      while (!(S.Serial & 0x80)) {
        S.Serial <<= 7;
        S.Serial += SN;
      }
      S.Serial -= 128;
      S.Name = S.Name.substr(1); // drop start colon
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
    : ObjectFile(ID_MMO, Object) {
  DataEnd = getData().bytes_end() - getData().size() % 4;
}

// The number of bytes used to represent an address in this object
// file format.
uint8_t MMIXObjectFile::getBytesInAddress() const { return 1; }

StringRef MMIXObjectFile::getFileFormatName() const {
  return "MMIX Object File";
}

Triple::ArchType MMIXObjectFile::getArch() const {
  return Triple::ArchType::mmix;
}

Expected<SubtargetFeatures> MMIXObjectFile::getFeatures() const {
  SubtargetFeatures Features;
  return Features;
}

std::optional<StringRef> MMIXObjectFile::tryGetCPUName() const {
  return StringRef("MMIX");
}

Expected<uint64_t> MMIXObjectFile::getStartAddress() const {
  // return the address of symbol `Main` otherwise 0
  const auto &It =
      std::find_if(SymbTab.begin(), SymbTab.end(), [](const MMO::Symbol &S) {
        return S.Name.compare(":Main") == 0;
      });
  if (It == SymbTab.end()) {
    return createError("Symbol Main is undefined!");
  } else {
    return It->Equiv;
  }
}

bool MMIXObjectFile::isRelocatableObject() const { return false; }

// Interface from SymbolicFile

bool MMIXObjectFile::is64Bit() const { return true; }

void MMIXObjectFile::moveSymbolNext(DataRefImpl &Symb) const {
  auto Next = reinterpret_cast<decltype(SymbTab)::value_type*>(Symb.p);
  Symb.p = reinterpret_cast<std::uintptr_t>(Next + 1);
}

Expected<uint32_t> MMIXObjectFile::getSymbolFlags(DataRefImpl Symb) const {
  auto P = reinterpret_cast<decltype(SymbTab.data())>(Symb.p);
  switch(P->Type) {
  case MMO::NORMAL:
    return SymbolRef::SF_Global | SymbolRef::SF_Absolute;
  case MMO::REGISTER:
    return SymbolRef::SF_Global | SymbolRef::SF_Common;// FIXME: no register...
  default:
    return createError("unknown flag");
  }
}

basic_symbol_iterator MMIXObjectFile::symbol_begin() const {
  DataRefImpl DataRef;
  DataRef.p = reinterpret_cast<uintptr_t>(SymbTab.data());
  BasicSymbolRef BSR(DataRef, this);
  return BSR;
}
basic_symbol_iterator MMIXObjectFile::symbol_end() const {
  DataRefImpl DataRef;
  DataRef.p = reinterpret_cast<uintptr_t>(SymbTab.data() + SymbTab.size());
  BasicSymbolRef BSR(DataRef, this);
  return BSR;
}

section_iterator MMIXObjectFile::section_begin() const {
  auto Iter = std::find_if(Content.begin(), Content.end(), [](const auto &S) {
    return std::holds_alternative<MMOData>(S);
  });
  auto P = Content.data() + (Iter - Content.begin());
  DataRefImpl DR;
  DR.p = reinterpret_cast<std::uintptr_t>(P);
  SectionRef SR(DR, this);
  section_iterator I(SR);
  return I;
}

section_iterator MMIXObjectFile::section_end() const {
  auto P = Content.data() + Content.size();
  DataRefImpl DR;
  DR.p = reinterpret_cast<std::uintptr_t>(P);
  SectionRef SR(DR, this);
  section_iterator I(SR);
  return I;
}

// Interface from ObjectFile
// SymbolRef
Expected<StringRef> MMIXObjectFile::getSymbolName(DataRefImpl Symb) const {
  auto S = reinterpret_cast<decltype(SymbTab.data())>(Symb.p);
  return S->Name;
}
Expected<uint64_t> MMIXObjectFile::getSymbolAddress(DataRefImpl Symb) const {
  auto S = reinterpret_cast<decltype(SymbTab.data())>(Symb.p);
  return S->Equiv;
}
uint64_t MMIXObjectFile::getSymbolValueImpl(DataRefImpl Symb) const {
  auto S = reinterpret_cast<decltype(SymbTab.data())>(Symb.p);
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
  DataRefImpl DR;
  SectionRef SR(DR, this);
  section_iterator I(SR);
  return section_begin();
}

// SectionRef
void MMIXObjectFile::moveSectionNext(DataRefImpl &Sec) const {
  auto P = reinterpret_cast<decltype(Content)::value_type *>(Sec.p);
  std::ptrdiff_t Index = P - Content.data();
  auto Start = Content.begin() + Index + 1;
  auto Iter = std::find_if(Start, Content.end(), [](const auto &S) {
    return std::holds_alternative<MMOData>(S);
  });
  auto Ptr = Content.data() + (Iter - Content.begin());
  Sec.p = reinterpret_cast<std::uintptr_t>(Ptr);
}

Expected<StringRef> MMIXObjectFile::getSectionName(DataRefImpl Sec) const {
  auto P = reinterpret_cast<decltype(Content)::value_type *>(Sec.p);
  auto DataP = std::get_if<MMOData>(P);
  if (!DataP)
    return createError("invalid section");

  std::uint8_t FistByte = DataP->StartAddress >> 54;
  switch (FistByte) {
  case MMO::INSTRUCTION_SEGMENT:
    return ".text";
  case MMO::DATA_SEGMENT:
    return ".data";
  case MMO::POOL_SEGMENT:
    return ".pool";
  case MMO::STACK_SEGMENT:
    return ".stack";
  default:
    return "UNKNOWN";
  }
}
uint64_t MMIXObjectFile::getSectionAddress(DataRefImpl Sec) const {
  auto P = reinterpret_cast<decltype(Content)::value_type *>(Sec.p);
  return std::get<MMOData>(*P).StartAddress;
}
uint64_t MMIXObjectFile::getSectionIndex(DataRefImpl Sec) const { return 0; }
uint64_t MMIXObjectFile::getSectionSize(DataRefImpl Sec) const {
  auto P = reinterpret_cast<decltype(Content)::value_type *>(Sec.p);
  return std::get<MMOData>(*P).Data.size();
}
Expected<ArrayRef<uint8_t>>
MMIXObjectFile::getSectionContents(DataRefImpl Sec) const {
  auto P = reinterpret_cast<decltype(Content)::value_type *>(Sec.p);
  auto LastPtr = Content.data() + Content.size();
  if (P >= LastPtr && P < Content.data())
    return createError("invalid section iterator");
  if (auto R = std::get_if<MMOData>(P))
    return R->Data;
  else
    return createError("invalid section type");
}

uint64_t MMIXObjectFile::getSectionAlignment(DataRefImpl Sec) const {
  return 4;
}
bool MMIXObjectFile::isSectionCompressed(DataRefImpl Sec) const {
  return false;
}

bool MMIXObjectFile::isSectionText(DataRefImpl Sec) const {
  auto P = reinterpret_cast<decltype(Content)::value_type *>(Sec.p);
  return (std::get<MMOData>(*P).StartAddress >> 54) == 0;
}

bool MMIXObjectFile::isSectionData(DataRefImpl Sec) const {
  auto P = reinterpret_cast<decltype(Content)::value_type *>(Sec.p);
  return (std::get<MMOData>(*P).StartAddress >> 54) == 0x20;
}

bool MMIXObjectFile::isSectionBSS(DataRefImpl Sec) const {
  return false;
} // no bss concept

// A section is 'virtual' if its contents aren't present in the object image.
bool MMIXObjectFile::isSectionVirtual(DataRefImpl Sec) const {
  return false;
} // no virtual concept

relocation_iterator MMIXObjectFile::section_rel_begin(DataRefImpl Sec) const {
  RelocationRef RR(Sec, this);
  auto RI = relocation_iterator(RR);
  return RI;
}
relocation_iterator MMIXObjectFile::section_rel_end(DataRefImpl Sec) const {
  RelocationRef RR(Sec, this);
  auto RI = relocation_iterator(RR);
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
  auto S = reinterpret_cast<decltype(SymbTab.data())>(Symb.p);
  return *S;
}

const MMO::Symbol &MMIXObjectFile::getMMOSymbol(const SymbolRef &Symb) const {
  return getMMOSymbol(Symb.getRawDataRefImpl());
}

const MMOPre &MMIXObjectFile::getMMOPreamble() const { return Preamble; }

const MMOPost &MMIXObjectFile::getMMOPostamble() const { return Postamble; }

bool MMIXObjectFile::isSymbolNameUTF16() const { return false; }

//////////////////////////////////////////////////////////////

static Expected<std::unique_ptr<MMIXObjectFile>>
createPtr(MemoryBufferRef Object, bool InitContent) {
  auto ObjOrErr = MMIXObjectFile::create(Object, InitContent);
  if (Error E = ObjOrErr.takeError()) {
    return std::move(E);
  }
  return std::move(*ObjOrErr);
}

Expected<std::unique_ptr<MMIXObjectFile>>
ObjectFile::createMMIXObjectFile(MemoryBufferRef Object, bool InitContent) {
  return createPtr(Object, InitContent);
}
