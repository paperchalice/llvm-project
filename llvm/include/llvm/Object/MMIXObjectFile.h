//===- MMIXObjectFile.h - MMIX object file format --------------------------*-
// C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the MMIX object file format class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_OBJECT_MMIXOBJECTFILE_H
#define LLVM_OBJECT_MMIXOBJECTFILE_H

#include "llvm/BinaryFormat/MMO.h"
#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Object/SymbolicFile.h"
#include "llvm/Support/ConvertUTF.h"
#include <chrono>
#include <ctime>
#include <optional>

// TODO: use std::optional

namespace llvm {

namespace object {

struct MMOLOp {
  struct Quote {
    ArrayRef<std::uint8_t> Value;
  };
  struct Loc {
    std::uint8_t HighByte;
    std::uint64_t Offset;
  };
  struct Skip {
    std::uint16_t Delta;
  };
  struct Fixo {
    std::uint8_t HighByte;
    std::uint64_t Offset;
  };
  struct Fixr {
    std::uint16_t Delta;
  };
  struct Fixrx {
    std::uint8_t Z;
    std::int64_t Delta;
  };
  struct File {std::uint8_t FileNumber;
    Optional<StringRef> FileName;};
  struct Line {
    std::uint16_t LineNumber;
  };
  struct Spec {
    std::uint16_t Type;
  };
  std::variant<Quote, Loc, Skip, Fixo, Fixr, Fixrx, File, Line, Spec> Content;
};

struct MMOPreamble {
  std::uint8_t Version = 1;
  Optional<std::time_t> CreatedTime;
  Optional<ArrayRef<std::uint8_t>> ExtraData;
};

struct MMOPostamble {
  std::uint8_t G;
  std::vector<std::uint64_t> Values;
};

struct MMOSymbol {
  SmallString<32> Name;
  MMO::SymbolType Type = MMO::SymbolType::NORMAL;
  std::uint64_t Address = 0;
  std::uint32_t SerialNumber;
  const std::uint8_t *PrintPos; //< for mmotype to determine when output tetra
};

struct MMOSymNode {
std::uint32_t Serial;
std::uint64_t Equiv;
bool IsRegister = false;
};

struct MMOTrieNode {
public:
  std::uint8_t Ch = ':';
  std::shared_ptr<MMOTrieNode> left, mid, right;
  std::optional<MMOSymNode> SymNode;

};

struct MMOTrie {
  std::shared_ptr<MMOTrieNode> Root;
  public:
  MMOTrie();
  void insertSymbol(const MMOSymbol &S);
};

class MMIXObjectFile : public ObjectFile {
public:
  // constructor
  static Expected<std::unique_ptr<MMIXObjectFile>>
  create(MemoryBufferRef Object);

public: // RTTI
  static bool classof(Binary const *v);

public:
  /// The number of bytes used to represent an address in this object
  ///        file format.
  uint8_t getBytesInAddress() const override;

  StringRef getFileFormatName() const override;
  Triple::ArchType getArch() const override;
  SubtargetFeatures getFeatures() const override;
  Optional<StringRef> tryGetCPUName() const override;

  /***
   * A program should begin at the special symbolic location Main (more
   * precisely, at the address corresponding to the fully qualified symbol
   * :Main). This symbol always has serial number 1, and it must always be
   * defined.
   */
  Expected<uint64_t> getStartAddress() const override;

  /** True if this is a relocatable object (.o/.obj).
   *
   * Many readers will have noticed that MMIXAL has no facilities for
   * relocatable output, nor does mmo format support such features. The author’s
   * first drafts of MMIXAL and mmo did allow relocatable objects, with external
   * linkages, but the rules were substantially more complicated and therefore
   * inconsistent with the goals of The Art of Computer Programming. So it is
   * always false.
   */
  bool isRelocatableObject() const override;

  // Interface from SymbolicFile
public:
  void moveSymbolNext(DataRefImpl &Symb) const override;
  Expected<uint32_t> getSymbolFlags(DataRefImpl Symb) const override;
  basic_symbol_iterator symbol_begin() const override;
  basic_symbol_iterator symbol_end() const override;

  section_iterator section_begin() const override;
  section_iterator section_end() const override;

  // Interface from ObjectFile
protected:
  // SymbolRef
  Expected<StringRef> getSymbolName(DataRefImpl Symb) const override;
  Expected<uint64_t> getSymbolAddress(DataRefImpl Symb) const override;
  uint64_t getSymbolValueImpl(DataRefImpl Symb) const override;
  uint64_t getCommonSymbolSizeImpl(DataRefImpl Symb) const override;
  Expected<SymbolRef::Type> getSymbolType(DataRefImpl Symb) const override;
  Expected<section_iterator> getSymbolSection(DataRefImpl Symb) const override;

  // SectionRef
  void moveSectionNext(DataRefImpl &Sec) const override;
  Expected<StringRef> getSectionName(DataRefImpl Sec) const override;
  uint64_t getSectionAddress(DataRefImpl Sec) const override;
  uint64_t getSectionIndex(DataRefImpl Sec) const override;
  uint64_t getSectionSize(DataRefImpl Sec) const override;
  Expected<ArrayRef<uint8_t>>
  getSectionContents(DataRefImpl Sec) const override;
  uint64_t getSectionAlignment(DataRefImpl Sec) const override;
  bool isSectionCompressed(DataRefImpl Sec) const override;
  bool isSectionText(DataRefImpl Sec) const override;
  bool isSectionData(DataRefImpl Sec) const override;
  bool isSectionBSS(DataRefImpl Sec) const override;
  // A section is 'virtual' if its contents aren't present in the object image.
  bool isSectionVirtual(DataRefImpl Sec) const override;

  relocation_iterator section_rel_begin(DataRefImpl Sec) const override;
  relocation_iterator section_rel_end(DataRefImpl Sec) const override;

  // RelocationRef
  void moveRelocationNext(DataRefImpl &Rel) const override;
  uint64_t getRelocationOffset(DataRefImpl Rel) const override;
  symbol_iterator getRelocationSymbol(DataRefImpl Rel) const override;
  uint64_t getRelocationType(DataRefImpl Rel) const override;
  void getRelocationTypeName(DataRefImpl Rel,
                             SmallVectorImpl<char> &Result) const override;

private:
  MMOPreamble Preamble;
  ArrayRef<std::uint8_t> ContentRef;
  std::vector<std::variant<ArrayRef<std::uint8_t>, MMOLOp>> Content;
  MMOPostamble Postamble;
  SmallVector<MMOSymbol, 32> SymbTab;
  MMOTrieNode TrieRoot;
  using SymbItT = decltype(SymbTab.begin());

private:
  MMIXObjectFile(MemoryBufferRef Object);
  Error initPreamble(const unsigned char *&Iter);
  Error initContent(const unsigned char *&Iter);
  Error initSymbolTable(const unsigned char *&Iter);
  void initSymbolTrie();
  Error initPostamble(const unsigned char *&Iter);
  void decodeSymbolTable(const unsigned char *&Start,
                         SmallVector<UTF16, 32> &Name, Error &E);
  Error decodeSymbolTable(const unsigned char *&Iter);

public:
public:
using ContentT = decltype(Content);
  const MMOSymbol &getMMOSymbol(const SymbolRef &Symb) const;
  const MMOSymbol &getMMOSymbol(const DataRefImpl &Symb) const;
  const MMOPreamble &getMMOPreamble() const;
  const MMOPostamble &getMMOPostamble() const;
  const ContentT &getMMOContent() const;
};

} // namespace object
} // namespace llvm

#endif // LLVM_OBJECT_MMIXOBJECTFILE_H
