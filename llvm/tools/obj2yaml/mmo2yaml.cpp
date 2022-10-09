//===------ utils/mmo2yaml.cpp - obj2yaml conversion tool -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "obj2yaml.h"
#include "llvm/ADT/Twine.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/Object/MMIXObjectFile.h"
#include "llvm/ObjectYAML/DWARFYAML.h"
#include "llvm/ObjectYAML/MMOYAML.h"
#include "llvm/Support/DataExtractor.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/YAMLTraits.h"
#include <variant>

using namespace llvm;
using namespace object;
using std::get;
using std::holds_alternative;
using std::uint8_t;

namespace {

class MMODumper {
public:
  MMODumper(const object::MMIXObjectFile &Obj);

public:
  MMOYAML::Object &getYAMLObj();
  Error dump();
  void dumpContent();
  void dumpSymbols();

private:
  const object::MMIXObjectFile &Obj;
  MMOYAML::Object YAMLObj;
};

MMODumper::MMODumper(const object::MMIXObjectFile &Obj)
    : Obj(Obj), YAMLObj(Obj) {}

MMOYAML::Object &MMODumper::getYAMLObj() { return YAMLObj; }

Error MMODumper::dump() {
  dumpContent();
  dumpSymbols();
  return Error::success();
}

void MMODumper::dumpContent() {
  const auto Content = Obj.getMMOContent();
  for (const auto S : Content) {
    if (holds_alternative<ArrayRef<uint8_t>>(S)) {
      yaml::BinaryRef BinRef(get<ArrayRef<uint8_t>>(S));
      YAMLObj.Segments.emplace_back(BinRef);
    } else if (holds_alternative<MMOLOp>(S)) {
      MMOYAML::Lop YLop;
      const auto &Op = get<MMOLOp>(S);
      switch (Op.Content.index()) {
      case MMO::LOP_QUOTE: {
        const auto &Q = get<MMOLOp::Quote>(Op.Content);
        MMOYAML::Quote YQ = {Q.Value};
        YLop = YQ;
      } break;
      case MMO::LOP_LOC: {
        const auto &L = get<MMOLOp::Loc>(Op.Content);
        MMOYAML::Loc YL = {L.HighByte, L.Offset};
        YLop = YL;
      } break;
      case MMO::LOP_SKIP: {
        const auto &S = get<MMOLOp::Skip>(Op.Content);
        MMOYAML::Skip YS = {S.Delta};
        YLop = YS;
      } break;
      case MMO::LOP_FIXO: {
        const auto &F = get<MMOLOp::Fixo>(Op.Content);
        MMOYAML::Fixo YF = {F.HighByte, F.Offset};
        YLop = YF;
      } break;
      case MMO::LOP_FIXR: {
        const auto &F = get<MMOLOp::Fixr>(Op.Content);
        MMOYAML::Fixo YF = {F.Delta};
        YLop = YF;
      } break;
      case MMO::LOP_FIXRX: {
        const auto &F = get<MMOLOp::Fixrx>(Op.Content);
        MMOYAML::Fixo YF = {F.Z, F.Delta};
        YLop = YF;
      } break;
      case MMO::LOP_FILE: {
        const auto &F = get<MMOLOp::File>(Op.Content);
        MMOYAML::File YF = {F.FileNumber, F.FileName};
        YLop = YF;
      } break;
      case MMO::LOP_LINE: {
        const auto &L = get<MMOLOp::Line>(Op.Content);
        MMOYAML::Line YL = {L.LineNumber};
        YLop = YL;
      } break;
      case MMO::LOP_SPEC: {
        const auto &S = get<MMOLOp::Spec>(Op.Content);
        MMOYAML::Line YS = {S.Type};
        YLop = YS;
      }
      default:
        break;
      }
      YAMLObj.Segments.emplace_back(YLop);
    }
  }
}

void MMODumper::dumpSymbols() {
  ExitOnError Err("invalid symbol table");
  for (const auto &S : Obj.symbols()) {
    MMOYAML::Symbol YS;
    const auto &MMOS = Obj.getMMOSymbol(S);
    YS.Name = MMOS.Name;
    YS.Address = MMOS.Address;
    YS.SerialNumber = MMOS.SerialNumber;
    YS.Type = MMOS.Type;
    YAMLObj.Symbols.emplace_back(YS);
  }
}

} // end namespace

Error mmo2yaml(raw_ostream &Out, const object::MMIXObjectFile &Obj) {
  MMODumper Dumper(Obj);
  Error Err = Dumper.dump();
  yaml::Output Yout(Out);
  Yout << Dumper.getYAMLObj();

  return Err;
}
