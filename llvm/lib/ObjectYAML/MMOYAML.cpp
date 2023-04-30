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

Quote::Quote(const object::MMO::Quote &Q) : Value(Q.Value) {}
Loc::Loc(const object::MMO::Loc &L) : HighByte(L.HighByte), Offset(L.Offset) {}
Skip::Skip(const object::MMO::Skip &S) : Delta(S.Delta) {}
Fixo::Fixo(const object::MMO::Fixo &F)
    : HighByte(F.HighByte), Offset(F.Offset) {}
Fixr::Fixr(const object::MMO::Fixr &F) : Delta(F.Delta) {}
Fixrx::Fixrx(const object::MMO::Fixrx &F)
    : FixType(F.FixType), Delta(F.Delta) {}
File::File(const object::MMO::File &F) : Name(F.Name), Number(F.Number) {}
Line::Line(const object::MMO::Line &L) : Number(L.Number) {}
Spec::Spec(const object::MMO::Spec &S) : Type(S.Type) {}
Pre::Pre(const object::MMO::Pre &P)
    : Version(P.Version), CreatedTime(P.CreatedTime) {
  if (P.ExtraData.has_value()) {
    ExtraData = yaml::BinaryRef(*P.ExtraData);
  }
}

Post::Post(const object::MMO::Post &P) : G(P.G) {
  for (const auto &V : P.Values) {
    Values.push_back(V);
  }
}

} // namespace MMOYAML

namespace yaml {

// lops

void MappingTraits<MMOYAML::Quote>::mapping(IO &IO, MMOYAML::Quote &Q) {
  IO.mapRequired("Value", Q.Value);
}

void MappingTraits<MMOYAML::Loc>::mapping(IO &IO, MMOYAML::Loc &L) {
  IO.mapRequired("HighByte", L.HighByte);
  IO.mapRequired("Offset", L.Offset);
}

void MappingTraits<MMOYAML::Skip>::mapping(IO &IO, MMOYAML::Skip &S) {
  IO.mapRequired("Delta", S.Delta);
}

void MappingTraits<MMOYAML::Fixo>::mapping(IO &IO, MMOYAML::Fixo &F) {
  IO.mapRequired("HighByte", F.HighByte);
  IO.mapRequired("Offset", F.Offset);
}

void MappingTraits<MMOYAML::Fixr>::mapping(IO &IO, MMOYAML::Fixr &F) {
  IO.mapRequired("Delta", F.Delta);
}

void MappingTraits<MMOYAML::Fixrx>::mapping(IO &IO, MMOYAML::Fixrx &F) {
  IO.mapRequired("FixType", F.FixType);
  IO.mapRequired("Delta", F.Delta);
}

void MappingTraits<MMOYAML::File>::mapping(IO &IO, MMOYAML::File &F) {
  IO.mapOptional("Name", F.Name);
  IO.mapRequired("Number", F.Number);
}

void MappingTraits<MMOYAML::Line>::mapping(IO &IO, MMOYAML::Line &L) {
  IO.mapRequired("Number", L.Number);
}

void MappingTraits<MMOYAML::Spec>::mapping(IO &IO, MMOYAML::Spec &S) {
  IO.mapRequired("Type", S.Type);
}

void MappingTraits<MMOYAML::Pre>::mapping(IO &IO, MMOYAML::Pre &P) {
  IO.mapRequired("Version", P.Version);
  IO.mapOptional("CreatedTime", P.CreatedTime);
  IO.mapOptional("ExtraData", P.ExtraData);
}

void MappingTraits<MMOYAML::Post>::mapping(IO &IO, MMOYAML::Post &P) {
  IO.mapRequired("G", P.G);
  IO.mapRequired("Values", P.Values);
}

void CustomMappingTraits<MMOYAML::Segment>::inputOne(IO &IO, StringRef key,
                                                     MMOYAML::Segment &Seg) {
  using namespace MMOYAML;
  if (key == "Bin") {
    BinaryRef Bin;
    IO.mapRequired("Bin", Bin);
    Seg = Bin;
  } else if (key == "OpCode") {
    MMOYAML::MMO_LOP_TYPE OpCode;
    IO.mapRequired("OpCode", OpCode);
    switch (OpCode) {
#define ECase(Up, Low)                                                         \
  case MMO::LOP_##Up: {                                                        \
    Low Op;                                                                    \
    MappingTraits<Low>::mapping(IO, Op);                                       \
    Seg = Op;                                                                  \
  }                                                                            \
    return
      ECase(QUOTE, Quote);
      ECase(LOC, Loc);
      ECase(SKIP, Skip);
      ECase(FIXO, Fixo);
      ECase(FIXR, Fixr);
      ECase(FIXRX, Fixrx);
      ECase(FILE, File);
      ECase(LINE, Line);
      ECase(SPEC, Spec);
    default:
      return;
    }
#undef ECase
  }
}

void MappingTraits<MMOYAML::Symbol>::mapping(IO &IO, MMOYAML::Symbol &S) {
  IO.mapRequired("Name", S.Name);
  IO.mapRequired("Serial", S.Serial);
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

void ScalarEnumerationTraits<MMOYAML::MMO_FIXRX_TYPE>::enumeration(
    IO &IO, MMOYAML::MMO_FIXRX_TYPE &Value) {
#define ECase(X) IO.enumCase(Value, #X, MMO::FIXRX_##X)
  ECase(OTHERWISE);
  ECase(JMP);
#undef ECase
  IO.enumFallback<Hex8>(Value);
}

template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

void CustomMappingTraits<MMOYAML::Segment>::output(IO &IO,
                                                   MMOYAML::Segment &Seg) {
  using namespace MMOYAML;
  std::visit(overloaded{[&](BinaryRef &Bin) { IO.mapRequired("Bin", Bin); },
                        [&](auto &Op) {
                          using OpType = std::remove_reference_t<decltype(Op)>;
                          IO.mapRequired("OpCode", OpType::OpCode);
                          MappingTraits<OpType>::mapping(IO, Op);
                        }},
             Seg);
}

} // namespace yaml
} // namespace llvm