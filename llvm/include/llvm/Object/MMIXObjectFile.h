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
  struct File {
    std::uint8_t FileNumber;
    Optional<StringRef> FileName;
  };
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

template <typename T> struct MMOTrieNode {
public:
  T Ch;
  std::shared_ptr<MMOTrieNode> Left, Mid, Right;
  std::optional<MMOSymNode> SymNode;
  MMOTrieNode(const T &C) : Ch(C) {}
};

template <typename T> struct MMOTrie {
  static_assert(std::is_same<T, std::uint8_t>::value ||
                    std::is_same<T, std::uint16_t>::value,
                "T must be uint8_t or uint_16_t");
  std::shared_ptr<MMOTrieNode<T>> Root;

private:
  static constexpr const char *SpecialName[] = {
      "rB", "rD", "rE", "rH",  "rJ", "rM", "rR",  "rBB", "rC",  "rN", "rO",
      "rS", "rI", "rT", "rTT", "rK", "rQ", "rU",  "rV",  "rG",  "rL", "rA",
      "rF", "rP", "rW", "rX",  "rY", "rZ", "rWW", "rXX", "rYY", "rZZ"};
  static constexpr const char *Predefs[] = {
      "ROUND_CURRENT",   "ROUND_OFF", "ROUND_UP",     "ROUND_DOWN",
      "ROUND_NEAR",      "Inf",       "Data_Segment", "Pool_Segment",
      "Stack_Segment",   "D_BIT",     "V_BIT",        "W_BIT",
      "I_BIT",           "O_BIT",     "U_BIT",        "Z_BIT",
      "X_BIT",           "D_Handler", "V_Handler",    "W_Handler",
      "I_Handler",       "O_Handler", "U_Handler",    "Z_Handler",
      "X_Handler",       "StdIn",     "StdOut",       "StdErr",
      "TextRead",        "TextWrite", "BinaryRead",   "BinaryWrite",
      "BinaryReadWrite", "Halt",      "Fopen",        "Fclose",
      "Fread",           "Fgets",     "Fgetws",       "Fwrite",
      "Fputs",           "Fputws",    "Fseek",        "Ftell",
  };

public:
  MMOTrie() : Root(std::make_shared<MMOTrieNode<T>>(':')) {
    Root->Mid = std::make_shared<MMOTrieNode<T>>('^');
    for (const auto &N : SpecialName) {
      search(N);
    }
    for (const auto &P : Predefs) {
      search(P);
    }
  }

  void insert(const MMOSymbol &S) {
    auto Tt = search(S.Name);
    Tt->SymNode = {S.SerialNumber, S.Address,
                   S.Type == MMO::SymbolType::REGISTER};
  }

  auto search(const StringRef &S) {
    auto Tt = Root;
    //
    typename std::conditional<std::is_same<T, std::uint8_t>::value, StringRef,
                              SmallVector<UTF16, 32>>::type Str;
    if constexpr (std::is_same<T, std::uint8_t>::value) {
      Str = S;
    } else {
      convertUTF8ToUTF16String(S, Str);
    }
    auto Sz = Str.size();
    for (std::size_t i = 0; i != Sz; ++i) {
      if (Tt->Mid) {
        Tt = Tt->Mid;
        while (Str[i] != Tt->Ch) {
          if (Str[i] < Tt->Ch) {
            if (Tt->Left)
              Tt = Tt->Left;
            else {
              Tt->Left = std::make_shared<MMOTrieNode<T>>(Str[i]);
              Tt = Tt->Left;
              break;
            }
          } else {
            if (Tt->Right)
              Tt = Tt->Right;
            else {
              Tt->Right = std::make_shared<MMOTrieNode<T>>(Str[i]);
              Tt = Tt->Right;
              break;
            }
          }
        }
      } else {
        Tt->Mid = std::make_shared<MMOTrieNode<T>>(Str[i]);
        Tt = Tt->Mid;
      }
    }
    return Tt;
  }

private:
  std::shared_ptr<MMOTrieNode<T>> prune(std::shared_ptr<MMOTrieNode<T>> t) {
    bool Useful = false;
    if (t->SymNode) {
      Useful = true;
    }
    if (t->Left) {
      t->Left = prune(t->Left);
      if (t->Left)
        Useful = true;
    }
    if (t->Mid) {
      t->Mid = prune(t->Mid);
      if (t->Mid)
        Useful = true;
    }
    if (t->Right) {
      t->Right = prune(t->Right);
      if (t->Right)
        Useful = true;
    }
    if (Useful)
      return t;
    else
      return nullptr;
  }

public:
  void prune() { prune(Root); }
  void writeBin(raw_ostream &OS) { writeBin(OS, Root); }

private:
  void writeBin(raw_ostream &OS, std::shared_ptr<MMOTrieNode<T>> N) {
    int M = std::is_same<T, std::uint8_t>::value ? 0 : 0x80;

    if (N->Left)
      M += 0x40;
    if (N->Mid)
      M += 0x20;
    if (N->Right)
      M += 0x10;

    if (N->SymNode) {
      const auto S = N->SymNode.value();
      if (S.IsRegister)
        M += 0xF;
      else {
        std::uint32_t X = 0;
        if ((S.Equiv & (0xFFFFUL << (32 + 16))) == (0x20000000UL << 32)) {
          M += 8;
          X = ((S.Equiv - (0x20000000UL << 32)) >> 32);
        } else {
          X = (S.Equiv >> 32);
        }
        if (X)
          M += 4;
        else
          X = S.Equiv & 0xFFFF'FFFF;
        int j = 1;
        for (; j < 4; ++j) {
          if (X < (unsigned int)(1 << (8 * j)))
            break;
        }
        M += j;
      }
    }
    OS << static_cast<std::uint8_t>(M);

    if (N->Left)
      writeBin(OS, N->Left);

    if (M & 0x2F) {
      if constexpr (std::is_same<T, std::uint16_t>::value) {
        char Buf[2];
        support::endian::write16be(Buf, N->Ch);
        OS.write(Buf, sizeof(Buf));
      } else {
        OS << N->Ch;
      }
      M &= 0xF;
      if (M && N->SymNode) {
        if (M == 0xF)
          M = 1;
        else if (M > 8)
          M -= 8;
        for (; M != 0; --M) {
          if (M > 4) {
            OS << static_cast<std::uint8_t>(
                ((N->SymNode->Equiv >> 32) >> (8 * (M - 5))) & 0xFF);
          } else {
            OS << static_cast<std::uint8_t>(
                static_cast<std::uint32_t>((N->SymNode->Equiv & 0xFFFF'FFFF)) >>
                    (8 * (M - 1)) &
                0xFF);
          }
        }
        for (M = 0; M < 4; ++M) {
          if (N->SymNode->Serial <
              static_cast<std::uint32_t>(1 << (7 * (M + 1))))
            break;
        }
        for (; M >= 0; --M) {
          OS << static_cast<std::uint8_t>(
              (N->SymNode->Serial >> (7 * (M)) & 0x7F) + (M ? 0 : 0x80));
        }
      }
      if (N->Mid)
        writeBin(OS, N->Mid);
    }

    if (N->Right)
      writeBin(OS, N->Right);
  }
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
  using MMOTrieUTF8 = MMOTrie<std::uint8_t>;
  using MMOTrieUTF16 = MMOTrie<std::uint16_t>;
  std::variant<std::monostate, MMOTrieUTF8, MMOTrieUTF16> TrieRoot;
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
  bool isSymbolNameUTF16() const;
};

} // namespace object
} // namespace llvm

#endif // LLVM_OBJECT_MMIXOBJECTFILE_H
