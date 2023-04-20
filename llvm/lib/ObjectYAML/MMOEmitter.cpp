//===- yaml2elf - Convert YAML to a ELF object file -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// The MMO component of yaml2obj.
///
//===----------------------------------------------------------------------===//

#include "llvm/Object/MMIXObjectFile.h"
#include "llvm/ObjectYAML/MMOYAML.h"
#include "llvm/ObjectYAML/yaml2obj.h"
#include "llvm/Support/EndianStream.h"

using namespace llvm;
using namespace std;
using namespace object;
using support::endian::write32be;

namespace {
inline void writeLop(raw_ostream &OS, const MMO::Lopcode &Op) {
  OS << MMO::MM << Op;
}

inline bool is64Bit(const yaml::Hex64 &H) {
  return (0xFFFF'FFFFUL << 32) & H.value;
}
} // namespace

namespace llvm::MMOYAML {

void Quote::writeBin(raw_ostream &OS) const {
  writeLop(OS, MMO::LOP_QUOTE);
  OS.write(0);
  OS.write(1);
  Value.writeAsBinary(OS);
}

void Loc::writeBin(raw_ostream &OS) const {
  writeLop(OS, MMO::LOP_LOC);
  support::endian::Writer Writer(OS, support::endianness::big);
  OS << HighByte;
  OS << (is64Bit(Offset) ? '\x2' : '\x1');
  if (is64Bit(Offset)) {
    Writer.write(Offset.value);
  } else {
    Writer.write<uint32_t>(Offset.value);
  }
}

void Skip::writeBin(raw_ostream &OS) const {
  writeLop(OS, MMO::LOP_SKIP);
  support::endian::Writer Writer(OS, support::endianness::big);
  Writer.write<uint16_t>(Delta.value);
}

void Fixo::writeBin(raw_ostream &OS) const {
  writeLop(OS, MMO::LOP_FIXO);
  OS << HighByte;
  support::endian::Writer Writer(OS, support::endianness::big);
  if (is64Bit(Offset)) {
    OS << '\x2';
    Writer.write<uint64_t>(Offset.value);
  } else {
    OS << '\x1';
    Writer.write<uint32_t>(Offset.value);
  }
}

void Fixr::writeBin(raw_ostream &OS) const {
  writeLop(OS, MMO::LOP_FIXR);
  support::endian::Writer Writer(OS, support::endianness::big);
  Writer.write<uint16_t>(Delta.value);
}

void Fixrx::writeBin(raw_ostream &OS) const {
  writeLop(OS, MMO::LOP_FIXRX);
  OS.write('\x0');
  OS << Z;
  char Buf[4];
  if (Delta < 0) {
    write32be(Buf, Delta + (1 << Z));
    Buf[0] = 1;
  } else {
    write32be(Buf, Delta);
    Buf[0] = 0;
  }
  OS.write(Buf, sizeof(Buf));
}

void File::writeBin(raw_ostream &OS) const {
  writeLop(OS, MMO::LOP_FILE);
  OS << Number;
  if (Name) {
    OS << static_cast<uint8_t>(Name->size() / 4);
    OS << *Name;
  } else {
    OS << 0;
  }
}

void Line::writeBin(raw_ostream &OS) const {
  writeLop(OS, MMO::LOP_LINE);
  support::endian::Writer Writer(OS, support::endianness::big);
  Writer.write<uint16_t>(Number);
}

void Spec::writeBin(raw_ostream &OS) const {
  writeLop(OS, MMO::LOP_SPEC);
  support::endian::Writer Writer(OS, support::endianness::big);
  Writer.write<uint16_t>(Type);
}

void Pre::writeBin(raw_ostream &OS) const {
  OS << MMO::MM << MMO::LOP_PRE << Version;
  support::endian::Writer Writer(OS, support::endianness::big);
  uint8_t Cnt = 0;
  if (CreatedTime.has_value()) {
    Cnt += 1;
  }
  if (ExtraData.has_value()) {
    Cnt += ExtraData->binary_size() / 4;
  }
  OS << Cnt;

  if (CreatedTime.has_value()) {
    Writer.write<uint32_t>(*CreatedTime);
  }
  if (ExtraData.has_value()) {
    ExtraData->writeAsBinary(OS);
  }
}

void Post::writeBin(raw_ostream &OS) const {
  writeLop(OS, MMO::LOP_POST);
  support::endian::Writer Writer(OS, support::endianness::big);
  OS.write(0);
  OS.write(G);
  for (const auto &V : Values) {
    Writer.write<uint64_t>(V.value);
  }
}

} // namespace llvm::MMOYAML

namespace {
class MMOWriter {
public:
  MMOWriter(MMOYAML::Object &O);
  Error write(raw_ostream &OS);

private:
  void writePreamble(raw_ostream &OS);
  void writeContent(raw_ostream &OS);
  void writePostamble(raw_ostream &OS);
  size_t writeSymbolTable(raw_ostream &OS);

private:
  MMOYAML::Object &Obj;
};

MMOWriter::MMOWriter(MMOYAML::Object &O) : Obj(O) {}

void MMOWriter::writePreamble(raw_ostream &OS) { Obj.Preamble.writeBin(OS); }

void MMOWriter::writeContent(raw_ostream &OS) {
  const auto &Content = Obj.Segments;
  for (const auto &S : Content) {
    if (auto BinRef = get_if<yaml::BinaryRef>(&S)) {
      BinRef->writeAsBinary(OS);
    } else if (auto pLop = get_if<MMOYAML::ContentLop>(&S)) {
      visit([&](const auto &L) { L.writeBin(OS); }, *pLop);
    }
  }
}

void MMOWriter::writePostamble(raw_ostream &OS) { Obj.Postamble.writeBin(OS); }

size_t MMOWriter::writeSymbolTable(raw_ostream &OS) {
  writeLop(OS, MMO::LOP_STAB);
  OS.write_zeros(2);
  MMO::MMOTrie Root;
  for (const auto &S : Obj.SymTab.Symbols) {
    MMO::Symbol Symb;
    Symb.Serial = S.Serial;
    Symb.Name = S.Name.drop_front();
    Symb.Equiv = S.Equiv;
    switch (S.Type) {
    case MMO::SymbolType::NORMAL:
      Symb.Type = MMO::SymbolType::NORMAL;
      break;
    case MMO::SymbolType::REGISTER:
      Symb.Type = MMO::SymbolType::REGISTER;
      break;
    default:
      Symb.Type = MMO::SymbolType::UNDEFINED;
      break;
    }
    Root.insert(Symb);
  }
  return Root.writeBin(OS);
}

Error MMOWriter::write(raw_ostream &OS) {
  support::endian::Writer Writer(OS, support::endianness::big);
  writePreamble(OS);
  writeContent(OS);
  writePostamble(OS);
  assert(OS.tell() % 4 == 0);
  auto Cnt = writeSymbolTable(OS);
  assert(Cnt % 4 == 0);
  writeLop(OS, MMO::LOP_END);
  Writer.write<uint16_t>(Cnt / 4);
  return Error::success();
}
} // namespace

namespace llvm {
namespace yaml {
bool yaml2mmo(MMOYAML::Object &Doc, raw_ostream &Out, ErrorHandler EH) {
  MMOWriter Writer(Doc);
  if (Error Err = Writer.write(Out)) {
    handleAllErrors(std::move(Err),
                    [&](const ErrorInfoBase &Err) { EH(Err.message()); });
    return false;
  }
  return true;
}
} // end namespace yaml
} // end namespace llvm
