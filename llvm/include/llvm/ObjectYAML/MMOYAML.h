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

// TODO: use std::optional
namespace llvm {
namespace MMOYAML {

LLVM_YAML_STRONG_TYPEDEF(std::uint8_t, MMO_SYMBOL_TYPE)
LLVM_YAML_STRONG_TYPEDEF(std::uint8_t, MMO_SEGMENT_TYPE)
LLVM_YAML_STRONG_TYPEDEF(std::uint8_t, MMO_LOP_TYPE)
LLVM_YAML_STRONG_TYPEDEF(std::uint8_t, MMO_FIXRX_TYPE)

struct Pre {
  std::uint8_t Version;
  Optional<std::time_t> CreatedTime;
  Optional<yaml::BinaryRef> ExtraData;

  Pre() = default;
  Pre(raw_ostream &OS);
  Pre(const MMO::Pre &P);
  void writeBin(raw_ostream &OS) const;
};

struct Post {
  std::uint8_t G;
  std::vector<yaml::Hex64> Values;
  Post() = default;
  Post(const MMO::Post &P);
  void writeBin(raw_ostream &OS) const;
};

struct Quote {
  yaml::BinaryRef Value;
  Quote() = default;
  Quote(yaml::IO &IO);
  Quote(const MMO::Quote &Q);
  void output(yaml::IO &IO);
  void writeBin(raw_ostream &OS) const;
};

struct Loc {
  MMO_SEGMENT_TYPE HighByte;
  yaml::Hex64 Offset;
  Loc() = default;
  Loc(yaml::IO &IO);
  Loc(const MMO::Loc &L);
  void writeBin(raw_ostream &OS) const;
};

struct Skip {
  yaml::Hex16 Delta;
  Skip() = default;
  Skip(const MMO::Skip &S);
  Skip(yaml::IO &IO);
  void writeBin(raw_ostream &OS) const;
};

struct Fixo {
  yaml::Hex8 HighByte;
  yaml::Hex64 Offset;
  Fixo() = default;
  Fixo(yaml::IO &IO);
  Fixo(const MMO::Fixo &F);
  void writeBin(raw_ostream &OS) const;
};

struct Fixr {
  yaml::Hex16 Delta;
  Fixr(const MMO::Fixr &F);
  Fixr() = default;
  Fixr(yaml::IO &IO);
  void writeBin(raw_ostream &OS) const;
};

struct Fixrx {
  MMO_FIXRX_TYPE Z;
  std::int32_t Delta;
  Fixrx() = default;
  Fixrx(yaml::IO &IO);
  Fixrx(const MMO::Fixrx &F);
  void writeBin(raw_ostream &OS) const;
};

struct File {
  Optional<StringRef> Name;
  std::uint8_t Number;
  File() = default;
  File(yaml::IO &IO);
  File(const MMO::File &F);
  void writeBin(raw_ostream &OS) const;
};

struct Line {
  std::uint16_t Number;
  Line() = default;
  Line(yaml::IO &IO);
  Line(const MMO::Line &L);
  void output(yaml::IO &IO);
  void writeBin(raw_ostream &OS) const;
};

struct Spec {
  std::uint16_t Type;
  Spec() = default;
  Spec(yaml::IO &IO);
  Spec(const MMO::Spec &S);
  void writeBin(raw_ostream &OS) const;
};

using ContentLop = std::variant<Quote, Loc, Skip, Fixo, Fixr, Fixrx, File, Line, Spec>;

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

using Segment = std::variant<yaml::BinaryRef, ContentLop>;

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
template <>
struct CustomMappingTraits<MMOYAML::ContentLop> {
  static void inputOne(IO &io, StringRef key, MMOYAML::ContentLop &L);
  static void output(IO &io, MMOYAML::ContentLop &L);
};

template <> struct MappingTraits<MMOYAML::Pre> {
  static void mapping(IO &IO, MMOYAML::Pre &P);
};

template <>
struct CustomMappingTraits<MMOYAML::Segment> {
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

template <> struct MappingTraits<MMOYAML::Post> {
  static void mapping(IO &IO, MMOYAML::Post &P);
};

} // end namespace yaml
} // end namespace llvm

#endif // LLVM_OBJECTYAML_MMOYAML_H
