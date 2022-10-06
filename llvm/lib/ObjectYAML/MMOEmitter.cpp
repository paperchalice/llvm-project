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

namespace llvm {
namespace yaml {
bool yaml2mmo(MMOYAML::Object &Doc, raw_ostream &Out, ErrorHandler EH) {
  return true;
}
} // end namespace yaml
} // end namespace llvm
