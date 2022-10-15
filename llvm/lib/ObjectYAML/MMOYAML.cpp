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
using std::visit;

namespace llvm {

namespace MMOYAML {

Object::Object(const object::MMIXObjectFile &O)
    : Preamble(O.getMMOPreamble()), Postamble(O.getMMOPostamble()) {}

Quote::Quote(const MMO::Quote &Q) : Value(Q.Value) {}
Quote::Quote(yaml::IO &IO) { IO.mapRequired("Value", Value); }
Loc::Loc(const MMO::Loc &L) : HighByte(L.HighByte), Offset(L.Offset) {}
Loc::Loc(yaml::IO &IO) {
  IO.mapRequired("HighByte", HighByte);
  IO.mapRequired("Offset", Offset);
}

Skip::Skip(const MMO::Skip &S) : Delta(S.Delta) {}
Skip::Skip(yaml::IO &IO) { IO.mapRequired("Delta", Delta); }
Fixo::Fixo(const MMO::Fixo &F) : HighByte(F.HighByte), Offset(F.Offset) {}
Fixo::Fixo(yaml::IO &IO) {
  IO.mapRequired("HighByte", HighByte);
  IO.mapRequired("Offset", Offset);
}
Fixr::Fixr(const MMO::Fixr &F) : Delta(F.Delta) {}
Fixr::Fixr(yaml::IO &IO) { IO.mapRequired("Delta", Delta); }
Fixrx::Fixrx(const MMO::Fixrx &F) : Z(F.Z), Delta(F.Delta) {}
Fixrx::Fixrx(yaml::IO &IO) {
  IO.mapRequired("Z", Z);
  IO.mapRequired("Delta", Delta);
}
File::File(const MMO::File &F) : Name(F.Name), Number(F.Number) {}
File::File(yaml::IO &IO) {
  IO.mapOptional("Name", Name);
  IO.mapRequired("Number", Number);
}
Line::Line(const MMO::Line &L) : Number(L.Number) {}
Line::Line(yaml::IO &IO) { IO.mapRequired("Number", Number); }
Spec::Spec(const MMO::Spec &S) : Type(S.Type) {}
Spec::Spec(yaml::IO &IO) { IO.mapRequired("Type", Type); }
Pre::Pre(const MMO::Pre &P) : Version(P.Version), CreatedTime(P.CreatedTime) {
  if (P.ExtraData.has_value()) {
    ExtraData = yaml::BinaryRef(*P.ExtraData);
  }
}

Post::Post(const MMO::Post &P) : G(P.G) {
  for (const auto &V : P.Values) {
    Values.push_back(V);
  }
}

} // namespace MMOYAML

namespace yaml {

// lops
void CustomMappingTraits<MMOYAML::ContentLop>::inputOne(
    IO &IO, StringRef key, MMOYAML::ContentLop &L) {
  MMOYAML::MMO_LOP_TYPE OpCode;
  IO.mapRequired("OpCode", OpCode);
  switch (OpCode) {
  case MMO::LOP_QUOTE:
    L = MMOYAML::Quote(IO);
    break;
  case MMO::LOP_LOC:
    L = MMOYAML::Loc(IO);
    break;
  case MMO::LOP_SKIP:
    L = MMOYAML::Skip(IO);
    break;
  case MMO::LOP_FIXO:
    L = MMOYAML::Fixo(IO);
    break;
  case MMO::LOP_FIXR:
    L = MMOYAML::Fixr(IO);
    break;
  case MMO::LOP_FIXRX:
    L = MMOYAML::Fixrx(IO);
    break;
  case MMO::LOP_FILE:
    L = MMOYAML::File(IO);
    break;
  case MMO::LOP_LINE:
    L = MMOYAML::Line(IO);
    break;
  case MMO::LOP_SPEC:
    L = MMOYAML::Spec(IO);
    break;
  default:
    break;
  }
}

void CustomMappingTraits<MMOYAML::ContentLop>::output(IO &IO,
                                                      MMOYAML::ContentLop &L) {
  // visit([&](const auto &Op) { L.output(IO); }, L);
  if (holds_alternative<MMOYAML::Quote>(L)) {
    auto &YQ = get<MMOYAML::Quote>(L);
    // YQ.output(IO);
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
    IO.mapRequired("Number", YF.Number);
    IO.mapOptional("Name", YF.Name);
  } else if (holds_alternative<MMOYAML::Line>(L)) {
    auto &YL = get<MMOYAML::Line>(L);
    MMOYAML::MMO_LOP_TYPE LopType = MMO::LOP_LINE;
    IO.mapRequired("OpCode", LopType);
    IO.mapRequired("Number", YL.Number);
  } else if (holds_alternative<MMOYAML::Spec>(L)) {
    auto &YS = get<MMOYAML::Spec>(L);
    MMOYAML::MMO_LOP_TYPE LopType = MMO::LOP_SPEC;
    IO.mapRequired("OpCode", LopType);
    IO.mapRequired("Type", YS.Type);
  }
}

void MappingTraits<MMOYAML::Pre>::mapping(IO &IO, MMOYAML::Pre &P) {
  IO.mapRequired("Version", P.Version);
  IO.mapOptional("CreatedTime", P.CreatedTime);
  IO.mapOptional("ExtraData", P.ExtraData);
}

void CustomMappingTraits<MMOYAML::Segment>::inputOne(IO &IO, StringRef key,
                                                     MMOYAML::Segment &Seg) {
  if (key == "Bin") {
    BinaryRef Bin;
    IO.mapRequired("Bin", Bin);
    Seg = Bin;
  } else if (key == "LOP") {
    MMOYAML::ContentLop Lop;
    IO.mapRequired("LOP", Lop);
    Seg = Lop;
  }
}

void CustomMappingTraits<MMOYAML::Segment>::output(IO &IO,
                                                   MMOYAML::Segment &Seg) {
  
  if (holds_alternative<BinaryRef>(Seg)) {
    IO.mapRequired("Bin", get<BinaryRef>(Seg));
  } else if (holds_alternative<MMOYAML::ContentLop>(Seg)) {
    auto &YLop = get<MMOYAML::ContentLop>(Seg);
    IO.mapRequired("LOP", YLop);
  }
}

void MappingTraits<MMOYAML::Symbol>::mapping(IO &IO, MMOYAML::Symbol &S) {
  IO.mapRequired("Name", S.Name);
  IO.mapRequired("SerialNumber", S.Serial);
  IO.mapRequired("Equiv", S.Equiv);
  IO.mapRequired("Type", S.Type);
}

void MappingTraits<MMOYAML::SymbolTable>::mapping(IO &IO,
                                                  MMOYAML::SymbolTable &S) {
  IO.mapRequired("IsUTF16", S.IsUTF16);
  IO.mapRequired("Symbol", S.Symbols);
}

void MappingTraits<MMOYAML::Object>::mapping(IO &IO, MMOYAML::Object &O) {
  IO.mapTag("!MMO", true);
  IO.mapRequired("Preamble", O.Preamble);
  IO.mapRequired("Segments", O.Segments);
  IO.mapRequired("Postamble", O.Postamble);
  IO.mapRequired("SymbolTable", O.SymTab);
}

void MappingTraits<MMOYAML::Post>::mapping(IO &IO, MMOYAML::Post &P) {
  IO.mapRequired("G", P.G);
  IO.mapRequired("Values", P.Values);
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