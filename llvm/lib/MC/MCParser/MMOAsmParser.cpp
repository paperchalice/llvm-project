//===- MMOAsmParser.cpp - MMO Assembly Parser ---------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/MCParser/MCAsmParserExtension.h"

namespace {

using namespace llvm;

class MMOAsmParser : public MCAsmParserExtension {

public:
  MMOAsmParser() = default;
};

}

namespace llvm {

MCAsmParserExtension *createMMOAsmParser() { return new MMOAsmParser; }

} // namespace llvm
