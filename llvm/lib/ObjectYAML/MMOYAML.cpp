//===- MMOYAML.cpp - MMO YAMLIO implementation ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines classes for handling the YAML representation of MMO.
//
//===----------------------------------------------------------------------===//

#include "llvm/ObjectYAML/MMOYAML.h"

using std::get;
using std::holds_alternative;

namespace llvm {

namespace MMOYAML {

Object::Object(const object::MMIXObjectFile &O)
    : Pre(O.getMMOPreamble()), Post(O.getMMOPostamble()) {}

Preamble::Preamble(const object::MMOPreamble &P)
    : Version(P.Version), CreatedTime(P.CreatedTime) {
      if(P.ExtraData.has_value()) {
        Content = yaml::BinaryRef(*P.ExtraData);
      }
    }

Preamble::Preamble() {}

Postamble::Postamble(const object::MMOPostamble &P) : G(P.G) {
  for (const auto &V : P.Values) {
    Items.push_back(V);
  }
}

Postamble::Postamble() : G(255) {

}

} // namespace MMOYAML

namespace yaml {

// lops
void CustomMappingTraits<MMOYAML::Lop>::inputOne(IO &IO, StringRef key, MMOYAML::Lop &L) {
  MMOYAML::MMO_LOP_TYPE OpCode;
  IO.mapRequired("OpCode", OpCode);
  switch(OpCode) {
    case MMO::LOP_QUOTE: {
      MMOYAML::Quote Q;
      IO.mapRequired("Value", Q.Value);
      L = Q;
    } break;
    case MMO::LOP_LOC: {
      MMOYAML::Loc Loc;
      IO.mapRequired("HighByte", Loc.HighByte);
      IO.mapRequired("Offset", Loc.Offset);
      L = Loc;
    } break;
    case MMO::LOP_SKIP: {
      MMOYAML::Skip S;
      IO.mapRequired("Delta", S.Delta);
      L = S;
    } break;
    case MMO::LOP_FIXO: {
      MMOYAML::Fixo F;
      IO.mapRequired("HighByte", F.HighByte);
      IO.mapRequired("Offset", F.Offset);
      L = F;
    } break;
    case MMO::LOP_FIXR: {
      MMOYAML::Fixr F;
      IO.mapRequired("Delta", F.Delta);
      L = F;
    } break;
    case MMO::LOP_FIXRX: {
      MMOYAML::Fixrx F;
      IO.mapRequired("Delta", F.Delta);
      IO.mapRequired("Z", F.Z);
      L = F;
    } break;
    case MMO::LOP_FILE: {
      MMOYAML::File F;
      IO.mapRequired("FileNumber", F.Number);
      IO.mapOptional("FileName", F.Name);
      L = F;
    } break;
    case MMO::LOP_LINE: {
      MMOYAML::Line LN;
      IO.mapRequired("Line", LN.LineNumber);
      L = LN;
    } break;
    case MMO::LOP_SPEC: {
      MMOYAML::Spec S;
      IO.mapRequired("Type", S.Type);
      L = S;
    } break;
    default:
    break;
  }
}

void CustomMappingTraits<MMOYAML::Lop>::output(IO &IO, MMOYAML::Lop &L) {
  if (holds_alternative<MMOYAML::Quote>(L)) {
    auto &YQ = get<MMOYAML::Quote>(L);
    MMOYAML::MMO_LOP_TYPE LopType = MMO::LOP_QUOTE;
    IO.mapRequired("OpCode", LopType);
    IO.mapRequired("Value", YQ.Value);
  } else if (holds_alternative<MMOYAML::Loc>(L)) {
    auto &YQ = get<MMOYAML::Loc>(L);
    MMOYAML::MMO_LOP_TYPE LopType = MMO::LOP_LOC;
    IO.mapRequired("OpCode", LopType);
    IO.mapRequired("HighByte", YQ.HighByte);
    IO.mapRequired("Offset", YQ.Offset);
  } else if (holds_alternative<MMOYAML::Skip>(L)) {
    auto &YS = get<MMOYAML::Skip>(L);
    MMOYAML::MMO_LOP_TYPE LopType = MMO::LOP_SKIP;
    IO.mapRequired("OpCode", LopType);
    IO.mapRequired("Delta", YS.Delta);
  } else if (holds_alternative<MMOYAML::Fixo>(L)) {
    auto &YF = get<MMOYAML::Fixo>(L);
    MMOYAML::MMO_LOP_TYPE LopType = MMO::LOP_FIXO;
    IO.mapRequired("OpCode", LopType);
    IO.mapRequired("HighByte", YF.HighByte);
    IO.mapRequired("Offset", YF.Offset);
  } else if (holds_alternative<MMOYAML::Fixr>(L)) {
    auto &YF = get<MMOYAML::Fixr>(L);
    MMOYAML::MMO_LOP_TYPE LopType = MMO::LOP_FIXR;
    IO.mapRequired("OpCode", LopType);
    IO.mapRequired("Delta", YF.Delta);
  } else if (holds_alternative<MMOYAML::Fixrx>(L)) {
    auto &YF = get<MMOYAML::Fixrx>(L);
    MMOYAML::MMO_LOP_TYPE LopType = MMO::LOP_FIXRX;
    IO.mapRequired("OpCode", LopType);
    IO.mapRequired("Z", YF.Z);
    IO.mapRequired("Delta", YF.Delta);
  } else if (holds_alternative<MMOYAML::File>(L)) {
    auto &YF = get<MMOYAML::File>(L);
    MMOYAML::MMO_LOP_TYPE LopType = MMO::LOP_FILE;
    IO.mapRequired("OpCode", LopType);
    IO.mapRequired("FileNumber", YF.Number);
    IO.mapOptional("FileName", YF.Name);
  } else if (holds_alternative<MMOYAML::Line>(L)) {
    auto &YL = get<MMOYAML::Line>(L);
    MMOYAML::MMO_LOP_TYPE LopType = MMO::LOP_LINE;
    IO.mapRequired("OpCode", LopType);
    IO.mapRequired("Line", YL.LineNumber);
  } else if (holds_alternative<MMOYAML::Spec>(L)) {
    auto &YS = get<MMOYAML::Spec>(L);
    MMOYAML::MMO_LOP_TYPE LopType = MMO::LOP_SPEC;
    IO.mapRequired("OpCode", LopType);
    IO.mapRequired("Type", YS.Type);
  }
}

void MappingTraits<MMOYAML::Preamble>::mapping(IO &IO, MMOYAML::Preamble &P) {
  IO.mapRequired("Version", P.Version);
  IO.mapOptional("CreatedTime", P.CreatedTime);
  IO.mapOptional("Content", P.Content);
}

void CustomMappingTraits<MMOYAML::Segment>::inputOne(IO &IO, StringRef key, MMOYAML::Segment &Seg) {
  if (key == "Bin") {
    IO.mapRequired("Bin", get<BinaryRef>(Seg));
  } else if (key == "LOP") {
    Seg = MMOYAML::Lop();
    IO.mapRequired("LOP", get<MMOYAML::Lop>(Seg));
  }
}

void CustomMappingTraits<MMOYAML::Segment>::output(IO &IO, MMOYAML::Segment &Seg) {
  if (holds_alternative<BinaryRef>(Seg)) {
    IO.mapRequired("Bin", get<BinaryRef>(Seg));
  } else if (holds_alternative<MMOYAML::Lop>(Seg)) {
    auto &YLop = get<MMOYAML::Lop>(Seg);
    IO.mapRequired("LOP", YLop);
  }
}

void MappingTraits<MMOYAML::Symbol>::mapping(IO &IO, MMOYAML::Symbol &S) {
  IO.mapRequired("Name", S.Name);
  IO.mapRequired("SerialNumber", S.SerialNumber);
  IO.mapRequired("Address", S.Address);
  IO.mapRequired("Type", S.Type);
}

void MappingTraits<MMOYAML::Object>::mapping(IO &IO, MMOYAML::Object &O) {
  IO.mapTag("!MMO", true);
  IO.mapRequired("Preamble", O.Pre);
  IO.mapRequired("Segments", O.Segments);
  IO.mapRequired("Postamble", O.Post);
  IO.mapRequired("Symbols", O.Symbols);
}

void MappingTraits<MMOYAML::Postamble>::mapping(IO &IO, MMOYAML::Postamble &P) {
  IO.mapRequired("G", P.G);
  IO.mapRequired("Values", P.Items);
}

void ScalarEnumerationTraits<MMOYAML::MMO_SYMBOL_TYPE>::enumeration(
    IO &IO, MMOYAML::MMO_SYMBOL_TYPE &Value) {
#define ECase(X) IO.enumCase(Value, #X, MMO::X)
  ECase(NORMAL);
  ECase(REGISTER);
  ECase(UNDEFINED);
#undef ECase
  IO.enumFallback<Hex8>(Value);
}

void ScalarEnumerationTraits<MMOYAML::MMO_SEGMENT_TYPE>::enumeration(
    IO &IO, MMOYAML::MMO_SEGMENT_TYPE &Value) {
#define ECase(X) IO.enumCase(Value, #X, MMO::X##_SEGMENT)
  ECase(INSTRUCTION);
  ECase(DATA);
  ECase(POOL);
  ECase(STACK);
#undef ECase
  IO.enumFallback<Hex32>(Value);
}

void ScalarEnumerationTraits<MMOYAML::MMO_LOP_TYPE>::enumeration(
    IO &IO, MMOYAML::MMO_LOP_TYPE &Value) {
#define ECase(X) IO.enumCase(Value, #X, MMO::LOP_##X)
  ECase(QUOTE);
  ECase(LOC);
  ECase(SKIP);
  ECase(FIXO);
  ECase(FIXR);
  ECase(FIXRX);
  ECase(FILE);
  ECase(LINE);
  ECase(SPEC);
  ECase(PRE);
  ECase(POST);
  ECase(STAB);
  ECase(END);
#undef ECase
  IO.enumFallback<Hex8>(Value);
}

} // namespace yaml
} // namespace llvm