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

#include "llvm/ObjectYAML/MMOYAML.h"
#include "llvm/ObjectYAML/yaml2obj.h"

using namespace llvm;
using namespace std;
using support::endian::write16be;
using support::endian::write32be;
using support::endian::write64be;

namespace {
inline void writeLop(raw_ostream &OS, const MMO::Lopcode &Op) {
  OS << MMO::MM << Op;
}

inline bool is64Bit(const yaml::Hex64 &H) {
  return (0xFFFF'FFFFUL << 32) & H.value;
}
} // namespace
namespace {
class MMOWriter {
public:
  MMOWriter(MMOYAML::Object &O);
  Error write(raw_ostream &OS);

private:
  void writePreamble(raw_ostream &OS);
  void writeContent(raw_ostream &OS);
  void writePostamble(raw_ostream &OS);
  uint16_t writeSymbolTable(raw_ostream &OS);

private:
  MMOYAML::Object &Obj;
};

MMOWriter::MMOWriter(MMOYAML::Object &O) : Obj(O) {}

void MMOWriter::writePreamble(raw_ostream &OS) {
  const auto &Pre = Obj.Pre;
  OS << MMO::MM << MMO::LOP_PRE << Pre.Version;
  uint8_t Cnt = 0;
  if (Pre.CreatedTime.has_value()) {
    Cnt += 1;
  }
  if (Pre.Content.has_value()) {
    Cnt += Pre.Content->binary_size() / 4;
  }
  OS << Cnt;

  if (Pre.CreatedTime.has_value()) {
    char Buf[4];
    write32be(Buf, *Pre.CreatedTime);
    OS.write(Buf, sizeof(Buf));
  }
  if (Pre.Content.has_value()) {
    Pre.Content->writeAsBinary(OS);
  }
}

void MMOWriter::writeContent(raw_ostream &OS) {
  const auto &Content = Obj.Segments;
  for (const auto &S : Content) {
    if (auto BinRef = get_if<yaml::BinaryRef>(&S)) {
      BinRef->writeAsBinary(OS);
    } else {
      // it is lop
      const auto &Lop = get<MMOYAML::Lop>(S);
      if (holds_alternative<MMOYAML::Quote>(Lop)) {
        auto Q = get<MMOYAML::Quote>(Lop);
        writeLop(OS, MMO::LOP_QUOTE);
        OS.write(0);
        OS.write(1);
        Q.Value.writeAsBinary(OS);
      } else if (auto pL = get_if<MMOYAML::Loc>(&Lop)) {
        writeLop(OS, MMO::LOP_LOC);
        OS << pL->HighByte;
        OS << (is64Bit(pL->Offset) ? '\x2' : '\x1');
        if (is64Bit(pL->Offset)) {
          char Buf[8];
          write64be(Buf, pL->Offset.value);
          OS.write(Buf, sizeof(Buf));
        } else {
          char Buf[4];
          write32be(Buf, pL->Offset.value);
          OS.write(Buf, sizeof(Buf));
        }
      } else if (auto pS = get_if<MMOYAML::Skip>(&Lop)) {
        writeLop(OS, MMO::LOP_SKIP);
        char Buf[2];
        write16be(Buf, pS->Delta);
        OS.write(Buf, sizeof(Buf));
      } else if (auto pF = get_if<MMOYAML::Fixo>(&Lop)) {
        OS << MMO::MM << MMO::LOP_FIXO;
        OS << pF->HighByte;
        if (is64Bit(pF->Offset)) {
          OS << 2;
          char Buf[8];
          write64be(Buf, pF->Offset.value);
          OS.write(Buf, sizeof(Buf));
        } else {
          OS << 1;
          char Buf[4];
          write32be(Buf, pF->Offset.value);
          OS.write(Buf, sizeof(Buf));
        }
      } else if (auto pF = get_if<MMOYAML::Fixr>(&Lop)) {
        writeLop(OS, MMO::LOP_FIXR);
        char Buf[2];
        write16be(Buf, pF->Delta.value);
        OS.write(Buf, sizeof(Buf));
      } else if (auto pF = get_if<MMOYAML::Fixrx>(&Lop)) {
        writeLop(OS, MMO::LOP_FIXRX);
        OS << pF->Z;
        char Buf[8];
        write64be(Buf, pF->Delta.value);
        OS.write(Buf, sizeof(Buf));
      } else if (auto pF = get_if<MMOYAML::File>(&Lop)) {
        writeLop(OS, MMO::LOP_FILE);
        OS << pF->Number;
        if (pF->Name) {
          OS << static_cast<uint8_t>(pF->Name->size() / 4);
          OS << *pF->Name;
        } else {
          OS << 0;
        }
      } else if (auto pL = get_if<MMOYAML::Line>(&Lop)) {
        writeLop(OS, MMO::LOP_LINE);
        char Buf[2];
        write16be(Buf, pL->LineNumber);
        OS.write(Buf, sizeof(Buf));
      } else if (auto pS = get_if<MMOYAML::Spec>(&Lop)) {
        writeLop(OS, MMO::LOP_SPEC);
        char Buf[2];
        write16be(Buf, pS->Type);
        OS.write(Buf, sizeof(Buf));
      }
    }
  }
}

void MMOWriter::writePostamble(raw_ostream &OS) {
  writeLop(OS, MMO::LOP_POST); OS.write(0);
  OS.write(Obj.Post.G);
  char Buf[8];
  for(const auto & R : Obj.Post.Items) {
    write64be(Buf, R.value);
    OS.write(Buf, sizeof(Buf));
  }
}

uint16_t MMOWriter::writeSymbolTable(raw_ostream &OS) {
  writeLop(OS, MMO::LOP_STAB); OS.write_zeros(2);
  const auto &Symbols = Obj.Symbols;
  return 1;
}

Error MMOWriter::write(raw_ostream &OS) {
  writePreamble(OS);
  writeContent(OS);
  writePostamble(OS);
  uint16_t TetraCount = writeSymbolTable(OS);
  writeLop(OS, MMO::LOP_END);
  char Buf[2]; write16be(Buf, TetraCount);
  OS.write(Buf, sizeof(Buf));
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
