//===- llvm/BinaryFormat/MMO.h - The MMIX object format ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_BINARYFORMAT_MMO_H
#define LLVM_BINARYFORMAT_MMO_H

#include "llvm/ADT/SmallString.h"
#include <cstdint>

namespace llvm {
namespace MMO {

// Loader instructions are distinguished from tetrabytes of data by their first
// (most significant) byte, which has the special escape-code value 0x98, called
// MM in the program below.
constexpr std::uint8_t MM = 0x98; ///< the lop start

// When a tetrabyte does begin with the escape code, its next byte is the
// lopcode defining a loader instruction. There are thirteen lopcodes:
enum Lopcode : std::uint8_t {
  LOP_QUOTE = 0x0, ///< the quotation lopcode
  LOP_LOC = 0x1,   ///< the location lopcode
  LOP_SKIP = 0x2,  ///< the skip lopcode
  LOP_FIXO = 0x3,  ///< the octabyte-fix lopcode
  LOP_FIXR = 0x4,  ///< the relative-fix lopcode
  LOP_FIXRX = 0x5, ///< extended relative-fix lopcode
  LOP_FILE = 0x6,  ///< the file name lopcode
  LOP_LINE = 0x7,  ///< the file position lopcode
  LOP_SPEC = 0x8,  ///< the special hook lopcode
  LOP_PRE = 0x9,   ///< the preamble lopcode
  LOP_POST = 0xA,  ///< the postamble lopcode
  LOP_STAB = 0xB,  ///< the symbol table lopcode
  LOP_END = 0xC,   ///< the end-it-all lopcode
};

enum SymbolType : std::uint8_t {
  REGISTER,
  NORMAL,
  UNDEFINED,
};

enum SegmentBaseAddr : std::uint8_t {
  INSTRUCTION_SEGMENT = 0x00,
  DATA_SEGMENT = 0x20,
  POOL_SEGMENT = 0x40,
  STACK_SEGMENT = 0x60,
};

} // namespace MMO
} // namespace llvm

#endif // LLVM_BINARYFORMAT_MMO_H
