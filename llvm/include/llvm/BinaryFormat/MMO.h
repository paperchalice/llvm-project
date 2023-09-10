//===- llvm/BinaryFormat/MMO.h - The MMIX object format ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_BINARYFORMAT_MMO_H
#define LLVM_BINARYFORMAT_MMO_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/Error.h"
#include <cstdint>
#include <ctime>
#include <forward_list>
#include <deque>
#include <optional>
#include <variant>

namespace llvm {
class MCSymbol;
namespace MMO {

// Loader instructions are distinguished from tetrabytes of data by their first
// (most significant) byte, which has the special escape-code value 0x98, called
// MM in the program below.
constexpr std::uint8_t MM = 0x98;          ///< the lop start
constexpr std::uint8_t CurrentVersion = 1; ///< mmo format version

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

struct FixupInfo {
  const std::uint64_t Addr;
  enum class FixupKind {
    FIXUP_WYDE,
    FIXUP_OCTA,
    FIXUP_JUMP,
  } Kind;
  const MCSymbol *Symbol;
};

enum FixrxType {
  FIXRX_OTHERWISE = 16,
  FIXRX_JMP = 24,
};

struct SymNode {
  std::uint32_t Serial;
  std::uint64_t Equiv;
  bool IsRegister = false;
  void encodeEquivLen(int &M);
  std::size_t writeBinEquivLen(raw_ostream &OS, int &M);
  std::size_t writeBinSerial(raw_ostream &OS, int &M);
};

struct Symbol {
  std::string Name;
  std::uint32_t Serial;
  std::uint64_t Equiv;
  SymbolType Type = MMO::NORMAL;
  const std::uint8_t *PrintPos; //< for mmotype to determine when output tetra
};

struct MMOTrieNode {
public:
  std::uint16_t Ch;
  std::shared_ptr<MMOTrieNode> Left, Mid, Right;
  std::optional<MMO::SymNode> SymNode;
  MMOTrieNode(const uint8_t &C);
};

class MMOTrie {
  std::shared_ptr<MMOTrieNode> Root;
  mutable std::size_t OutCnt = 0;

public:
  MMOTrie();

  void insert(const MMO::Symbol &S);

  std::shared_ptr<MMOTrieNode> search(StringRef S);

public:
  std::size_t writeBin(raw_ostream &OS) const;

private:
  void writeBin(raw_ostream &OS, std::shared_ptr<MMOTrieNode> N) const;
};

struct AsmSharedInfo {
  std::uint64_t PC = 0;
  std::uint64_t MMOLoc = 0;
  std::uint16_t MMOLine = 1;
  std::size_t CurrentLine = 1;
  std::forward_list<FixupInfo> FixupList = {};
  std::deque<std::uint64_t> GregList = {0};
  bool Expand = false;
};
} // namespace MMO
} // namespace llvm

#endif // LLVM_BINARYFORMAT_MMO_H
