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

struct Preamble {
  std::uint8_t Version;
  Optional<std::time_t> CreatedTime;
  Optional<yaml::BinaryRef> Content;

  Preamble(const object::MMOPreamble &P);
  Preamble();
};

struct Postamble {
  std::uint8_t G;
  std::vector<yaml::Hex64> Items;

  Postamble(const object::MMOPostamble &P);
  Postamble();
};

struct Quote {
  yaml::BinaryRef Value;
};

struct Loc {
  MMO_SEGMENT_TYPE HighByte;
  yaml::Hex64 Offset;
};

struct Skip {
  yaml::Hex16 Delta;
};

struct Fixo {
  yaml::Hex8 HighByte;
  yaml::Hex64 Offset;
};

struct Fixr {
  yaml::Hex16 Delta;
};

struct Fixrx {
  yaml::Hex8 Z;
  yaml::Hex32 Delta;
};

struct File {
  std::uint8_t Number;
  Optional<StringRef> Name;
};

struct Line {
  std::uint16_t LineNumber;
};

struct Spec {
  std::uint16_t Type;
};

using Lop = std::variant<Quote, Loc, Skip, Fixo, Fixr, Fixrx, File, Line, Spec>;

struct Symbol {
  StringRef Name;
  yaml::Hex64 Address;
  uint32_t SerialNumber;
  MMO_SYMBOL_TYPE Type;
};

struct SymbolTable {
  bool IsUTF16;
  std::vector<Symbol> Symbols;
};

using Segment = std::variant<yaml::BinaryRef, Lop>;

struct Object {
  Preamble Pre;
  std::vector<Segment> Segments;
  Postamble Post;
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

// lops
template <>
struct CustomMappingTraits<MMOYAML::Lop> {
  static void inputOne(IO &io, StringRef key, MMOYAML::Lop &L);
  static void output(IO &io, MMOYAML::Lop &L);
};

template <> struct MappingTraits<MMOYAML::Preamble> {
  static void mapping(IO &IO, MMOYAML::Preamble &P);
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

template <> struct MappingTraits<MMOYAML::Postamble> {
  static void mapping(IO &IO, MMOYAML::Postamble &P);
};

} // end namespace yaml
} // end namespace llvm

#endif // LLVM_OBJECTYAML_MMOYAML_H
