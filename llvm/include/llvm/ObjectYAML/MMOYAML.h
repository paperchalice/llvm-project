//===- MMOYAML.h - MMO YAMLIO implementation --------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file declares classes for handling the YAML representation
/// of MMO.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_OBJECTYAML_MMOYAML_H
#define LLVM_OBJECTYAML_MMOYAML_H

#include "llvm/ADT/StringRef.h"
#include "llvm/BinaryFormat/MMO.h"
#include "llvm/Object/MMIXObjectFile.h"
#include "llvm/ObjectYAML/DWARFYAML.h"
#include "llvm/ObjectYAML/YAML.h"
#include "llvm/Support/YAMLTraits.h"
#include <cstdint>
#include <memory>
#include <optional>
#include <variant>
#include <vector>
namespace llvm {
namespace MMOYAML {

LLVM_YAML_STRONG_TYPEDEF(std::uint8_t, MMO_SYMBOL_TYPE)
LLVM_YAML_STRONG_TYPEDEF(std::uint8_t, MMO_SEGMENT_TYPE)
LLVM_YAML_STRONG_TYPEDEF(std::uint8_t, MMO_LOP_TYPE)
LLVM_YAML_STRONG_TYPEDEF(std::uint8_t, MMO_FIXRX_TYPE)

struct Quote {
  static inline MMO_LOP_TYPE OpCode = MMO::LOP_QUOTE;
  yaml::BinaryRef Value;
  Quote() = default;
  Quote(const MMO::Quote &Q);
  void writeAsBinary(raw_ostream &OS) const;
};

struct Loc {
  static inline MMO_LOP_TYPE OpCode = MMO::LOP_LOC;
  MMO_SEGMENT_TYPE HighByte;
  yaml::Hex64 Offset;
  Loc() = default;
  Loc(const MMO::Loc &L);
  void writeAsBinary(raw_ostream &OS) const;
};

struct Skip {
  static inline MMO_LOP_TYPE OpCode = MMO::LOP_SKIP;
  yaml::Hex16 Delta;
  Skip() = default;
  
  Skip(const MMO::Skip &S);
  void writeAsBinary(raw_ostream &OS) const;
};

struct Fixo {
  static inline MMO_LOP_TYPE OpCode = MMO::LOP_FIXO;
  yaml::Hex8 HighByte;
  yaml::Hex64 Offset;
  Fixo() = default;
  Fixo(const MMO::Fixo &F);
  void writeAsBinary(raw_ostream &OS) const;
};

struct Fixr {
  static inline MMO_LOP_TYPE OpCode = MMO::LOP_FIXR;
  yaml::Hex16 Delta;
  Fixr(const MMO::Fixr &F);
  Fixr() = default;
  void writeAsBinary(raw_ostream &OS) const;
};

struct Fixrx {
  static inline MMO_LOP_TYPE OpCode = MMO::LOP_FIXRX;
  MMO_FIXRX_TYPE FixType;
  std::int32_t Delta;
  Fixrx() = default;
  Fixrx(const MMO::Fixrx &F);
  void writeAsBinary(raw_ostream &OS) const;
};

struct File {
  static inline MMO_LOP_TYPE OpCode = MMO::LOP_FILE;
  std::optional<StringRef> Name;
  std::uint8_t Number;
  File() = default;
  File(const MMO::File &F);
  void output(yaml::IO &IO);
  void writeAsBinary(raw_ostream &OS) const;
};

struct Line {
  static inline MMO_LOP_TYPE OpCode = MMO::LOP_LINE;
  std::uint16_t Number;
  Line() = default;
  Line(const MMO::Line &L);
  void output(yaml::IO &IO);
  void writeAsBinary(raw_ostream &OS) const;
};

struct Spec {
  static inline MMO_LOP_TYPE OpCode = MMO::LOP_SPEC;
  std::uint16_t Type;
  Spec() = default;
  Spec(const MMO::Spec &S);
  void writeAsBinary(raw_ostream &OS) const;
};

struct Pre {
  static inline MMO_LOP_TYPE OpCode = MMO::LOP_PRE;
  std::uint8_t Version;
  std::optional<std::time_t> CreatedTime;
  std::optional<yaml::BinaryRef> ExtraData;

  Pre() = default;
  Pre(const MMO::Pre &P);
  void writeAsBinary(raw_ostream &OS) const;
};

struct Post {
  static inline MMO_LOP_TYPE OpCode = MMO::LOP_POST;
  std::uint8_t G;
  std::vector<yaml::Hex64> Values;

  Post() = default;
  Post(const MMO::Post &P);
  void writeAsBinary(raw_ostream &OS) const;
};

using ContentLop =
    std::variant<Quote, Loc, Skip, Fixo, Fixr, Fixrx, File, Line, Spec>;

struct Symbol {
  StringRef Name;
  yaml::Hex64 Equiv;
  uint32_t Serial;
  MMO_SYMBOL_TYPE Type;
};

struct SymbolTable {
  bool IsUTF16;
  std::vector<Symbol> Symbols;
};

using Segment = std::variant<yaml::BinaryRef, Quote, Loc, Skip, Fixo, Fixr, Fixrx, File, Line, Spec>;

struct Object {
  Pre Preamble;
  std::vector<Segment> Segments;
  Post Postamble;
  SymbolTable SymTab;

  Object(const object::MMIXObjectFile &O);
  Object() = default;
};
} // end namespace MMOYAML
} // end namespace llvm

LLVM_YAML_IS_SEQUENCE_VECTOR(MMOYAML::Symbol)
LLVM_YAML_IS_SEQUENCE_VECTOR(MMOYAML::Segment)

namespace llvm {
namespace yaml {

template <> struct ScalarEnumerationTraits<MMOYAML::MMO_SYMBOL_TYPE> {
  static void enumeration(IO &IO, MMOYAML::MMO_SYMBOL_TYPE &Value);
};

template <> struct ScalarEnumerationTraits<MMOYAML::MMO_SEGMENT_TYPE> {
  static void enumeration(IO &IO, MMOYAML::MMO_SEGMENT_TYPE &Value);
};

template <> struct ScalarEnumerationTraits<MMOYAML::MMO_LOP_TYPE> {
  static void enumeration(IO &IO, MMOYAML::MMO_LOP_TYPE &Value);
};

template <> struct ScalarEnumerationTraits<MMOYAML::MMO_FIXRX_TYPE> {
  static void enumeration(IO &IO, MMOYAML::MMO_FIXRX_TYPE &Value);
};

// lops

template <> struct MappingTraits<MMOYAML::Quote> {
  static void mapping(IO &IO, MMOYAML::Quote &Q);
};

template <> struct MappingTraits<MMOYAML::Loc> {
  static void mapping(IO &IO, MMOYAML::Loc &L);
};

template <> struct MappingTraits<MMOYAML::Skip> {
  static void mapping(IO &IO, MMOYAML::Skip &S);
};

template <> struct MappingTraits<MMOYAML::Fixo> {
  static void mapping(IO &IO, MMOYAML::Fixo &F);
};

template <> struct MappingTraits<MMOYAML::Fixr> {
  static void mapping(IO &IO, MMOYAML::Fixr &F);
};

template <> struct MappingTraits<MMOYAML::Fixrx> {
  static void mapping(IO &IO, MMOYAML::Fixrx &F);
};

template <> struct MappingTraits<MMOYAML::File> {
  static void mapping(IO &IO, MMOYAML::File &F);
};

template <> struct MappingTraits<MMOYAML::Line> {
  static void mapping(IO &IO, MMOYAML::Line &L);
};

template <> struct MappingTraits<MMOYAML::Spec> {
  static void mapping(IO &IO, MMOYAML::Spec &P);
};

template <> struct MappingTraits<MMOYAML::Pre> {
  static void mapping(IO &IO, MMOYAML::Pre &P);
};

template <> struct MappingTraits<MMOYAML::Post> {
  static void mapping(IO &IO, MMOYAML::Post &P);
};

template <> struct CustomMappingTraits<MMOYAML::Segment> {
  static void inputOne(IO &io, StringRef key, MMOYAML::Segment &elem);
  static void output(IO &io, MMOYAML::Segment &elem);
};

template <> struct MappingTraits<MMOYAML::Symbol> {
  static void mapping(IO &IO, MMOYAML::Symbol &O);
};

template <> struct MappingTraits<MMOYAML::SymbolTable> {
  static void mapping(IO &IO, MMOYAML::SymbolTable &O);
};

template <> struct MappingTraits<MMOYAML::Object> {
  static void mapping(IO &IO, MMOYAML::Object &O);
};

} // end namespace yaml
} // end namespace llvm

#endif // LLVM_OBJECTYAML_MMOYAML_H
