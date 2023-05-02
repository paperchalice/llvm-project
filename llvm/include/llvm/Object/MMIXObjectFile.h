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
#include <type_traits>
#include <variant>

namespace llvm {

namespace object {

namespace MMO {

using ByteArrayRef = ArrayRef<std::uint8_t>;

struct Segment {
  StringRef Data;
};

struct Quote {
  ByteArrayRef RawData;
  ByteArrayRef Value;
};
struct Loc {
  ByteArrayRef RawData;
  std::uint8_t HighByte;
  std::uint64_t Offset;
};
struct Skip {
  ByteArrayRef RawData;
  std::uint16_t Delta;
};
struct Fixo {
  ByteArrayRef RawData;
  std::uint8_t HighByte;
  std::uint64_t Offset;
};
struct Fixr {
  ByteArrayRef RawData;
  std::uint16_t Delta;
};
struct Fixrx {
  ByteArrayRef RawData;
  std::uint8_t FixType;
  std::int32_t Delta;
};
struct File {
  ByteArrayRef RawData;
  std::uint8_t Number;
  std::optional<StringRef> Name;
};
struct Line {
  ByteArrayRef RawData;
  std::uint16_t Number;
};
struct Spec {
  ByteArrayRef RawData;
  std::uint16_t Type;
};

struct Pre {
  ByteArrayRef RawData;
  std::uint8_t Version = llvm::MMO::CurrentVersion;
  std::optional<std::time_t> CreatedTime;
  std::optional<ArrayRef<std::uint8_t>> ExtraData;
};

struct Post {
  ByteArrayRef RawData;
  std::uint8_t G;
  std::vector<std::uint64_t> Values;
};

} // namespace MMO

class MMIXObjectFile : public ObjectFile {
public:
  // constructor
  static Expected<std::unique_ptr<MMIXObjectFile>>
  create(MemoryBufferRef Object, bool InitContent);

public:
  // RTTI
  static bool classof(Binary const *v);

public:
  /// The number of bytes used to represent an address in this object
  ///        file format.
  uint8_t getBytesInAddress() const override;

  StringRef getFileFormatName() const override;
  Triple::ArchType getArch() const override;
  Expected<SubtargetFeatures> getFeatures() const override;
  std::optional<StringRef> tryGetCPUName() const override;

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
  bool is64Bit() const override;
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
  MMO::Pre Preamble;
  ArrayRef<std::uint8_t> ContentRef;
  std::vector<std::variant<ArrayRef<std::uint8_t>, MMO::Quote, MMO::Loc,
                           MMO::Skip, MMO::Fixo, MMO::Fixr, MMO::Fixrx,
                           MMO::File, MMO::Line, MMO::Spec>>
      Content;
  MMO::Post Postamble;
  SmallVector<llvm::MMO::Symbol, 32> SymbTab;
  using SymbItT = decltype(SymbTab.begin());

private:
  MMIXObjectFile(MemoryBufferRef Object);
  Error initMMIXObjectFile();
  Error initPreamble(const unsigned char *&Iter);
  Error initContent(const unsigned char *&Iter);
  Error initSymbolTable(const unsigned char *&Iter);
  Error initPostamble(const unsigned char *&Iter);
  void decodeSymbolTable(const uint8_t *&Start, SmallVector<UTF16, 32> &Name,
                         Error &E);
  Error decodeSymbolTable(const unsigned char *&Iter);

public:
public:
  using ContentT = decltype(Content);
  const llvm::MMO::Symbol &getMMOSymbol(const SymbolRef &Symb) const;
  const llvm::MMO::Symbol &getMMOSymbol(const DataRefImpl &Symb) const;
  const MMO::Pre &getMMOPreamble() const;
  const MMO::Post &getMMOPostamble() const;
  const ContentT &getMMOContent() const;
  const auto &getSTab() const { return SymbTab; }
  bool isSymbolNameUTF16() const;
};

} // namespace object
} // namespace llvm

#endif // LLVM_OBJECT_MMIXOBJECTFILE_H
