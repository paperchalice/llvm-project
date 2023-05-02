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
  for (const auto &S : Content) {
    std::visit([&](auto &&C){
      YAMLObj.Segments.emplace_back(C);
    }, S);
  }
}

void MMODumper::dumpSymbols() {
  ExitOnError Err("invalid symbol table");
  for (const auto &S : Obj.symbols()) {
    MMOYAML::Symbol YS;
    const auto &MMOS = Obj.getMMOSymbol(S);
    YS.Name = MMOS.Name;
    YS.Equiv = MMOS.Equiv;
    YS.Serial = MMOS.Serial;
    YS.Type = MMOS.Type;
    YAMLObj.SymTab.IsUTF16 = Obj.isSymbolNameUTF16();
    YAMLObj.SymTab.Symbols.emplace_back(YS);
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
