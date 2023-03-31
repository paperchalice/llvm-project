#include "llvm/BinaryFormat/MMO.h"
#include "llvm/Support/Endian.h"

using namespace llvm;
using namespace llvm::support::endian;
using namespace llvm::MMO;

namespace {
inline const char *ToCharP(const unsigned char *p) {
  return reinterpret_cast<const char *>(p);
}
} // namespace

Quote::Quote(const std::uint8_t *&Iter)
    : Value(ArrayRef<uint8_t>(Iter + 4, 4)) {
  Iter += 8;
}

Loc::Loc(const std::uint8_t *&Iter, Error &E) : HighByte(Iter[2]) {
  uint8_t TetraCount = Iter[3];
  Iter += 4;
  switch (TetraCount) {
  case 1:
    Offset = read32be(Iter);
    Iter += 4;
    break;
  case 2:
    Offset = read64be(Iter);
    Iter += 8;
    break;
  default:
    E = createStringError(std::errc::invalid_argument,
                          "Z field of lop_loc must be 1 or 2!");
    break;
  }
}

Skip::Skip(const std::uint8_t *&Iter) : Delta(read16be(Iter + 2)) { Iter += 4; }

Fixo::Fixo(const std::uint8_t *&Iter, Error &E) : HighByte(Iter[2]) {
  uint8_t TetraCount = Iter[3];
  Iter += 4;
  switch (TetraCount) {
  case 1:
    Offset = read32be(Iter);
    Iter += 4;
    break;
  case 2:
    Offset = read64be(Iter);
    Iter += 8;
    break;
  default:
    E = createStringError(std::errc::invalid_argument,
                          "`Z` field of `lop_fixo` must be 1 or 2!");
    break;
  }
}

Fixr::Fixr(const std::uint8_t *&Iter) : Delta(read16be(Iter + 2)) { Iter += 4; }

Fixrx::Fixrx(const std::uint8_t *&Iter, Error &E) : Z(Iter[3]) {
  if (!(Z == FIXRX_JMP || Z == FIXRX_OTHERWISE))
    E = createStringError(std::errc::invalid_argument,
                          "Z field in lop_fixrx must be 16 or 24!");
  Iter += 4;
  uint32_t Tet = read32be(Iter) & 0x00FF'FFFF;
  Delta = Iter[0] ? (Tet - (1 << Z)) : Tet;
  Iter += 4;
}

File::File(const std::uint8_t *&Iter) : Number(Iter[2]) {
  uint8_t TetraCount = Iter[3];
  Iter += 4;
  if (TetraCount != 0) {
    Name = StringRef(ToCharP(Iter), 4 * TetraCount);
  }
  Iter += 4 * TetraCount;
}

Line::Line(const std::uint8_t *&Iter) : Number(read16be(Iter + 2)) {
  Iter += 4;
}

Spec::Spec(const std::uint8_t *&Iter) : Type(read16be(Iter + 2)) { Iter += 4; }

void SymNode::encodeEquivLen(int &M) {
  if (IsRegister) {
    M += 0xF;
  } else {
    std::uint32_t X = Equiv >> 32;
    if ((Equiv & (0xFFFFUL << (32 + 16))) == (0x2000UL << (32 + 16))) {
      M += 8;
      X -= 0x2000'0000;
    }
    if (X)
      M += 4;
    else
      X = Equiv & 0xFFFF'FFFF;
    int j = 1;
    for (; j < 4; ++j) {
      if (X < static_cast<uint32_t>(1 << (8 * j)))
        break;
    }
    M += j;
  }
}

std::size_t SymNode::writeBinEquivLen(raw_ostream &OS, int &M) {
  std::size_t Cnt = 0;
  if (M == 0xF)
    M = 1;
  else if (M > 8)
    M -= 8;
  for (; M != 0; --M) {
    if (M > 4) {
      OS << static_cast<std::uint8_t>(((Equiv >> 32) >> (8 * (M - 5))) & 0xFF);
      ++Cnt;
    } else {
      OS << static_cast<std::uint8_t>(
          static_cast<std::uint32_t>((Equiv & 0xFFFF'FFFF)) >> (8 * (M - 1)) &
          0xFF);
      ++Cnt;
    }
  }
  return Cnt;
}

std::size_t SymNode::writeBinSerial(raw_ostream &OS, int &M) {
  size_t Cnt = 0;
  for (M = 0; M < 4; ++M) {
    if (Serial < static_cast<std::uint32_t>(1 << (7 * (M + 1))))
      break;
  }
  for (; M >= 0; --M) {
    OS << static_cast<std::uint8_t>((Serial >> (7 * (M)) & 0x7F) +
                                    (M ? 0 : 0x80));
    ++Cnt;
  }
  return Cnt;
}

MMOTrieNode::MMOTrieNode(const uint8_t &C) : Ch(C) {}

MMOTrie::MMOTrie() : Root(std::make_shared<MMOTrieNode>(':')) {
  Root->Mid = std::make_shared<MMOTrieNode>('^');
}

void MMOTrie::insert(const MMO::Symbol &S) {
  auto Tt = search(S.Name);
  Tt->SymNode = {S.Serial, S.Equiv, MMO::SymbolType::REGISTER};
}

std::shared_ptr<MMOTrieNode> MMOTrie::search(StringRef S) {
  auto Tt = Root;
  auto Str = S;
  auto Sz = S.size();
  for (std::size_t i = 0; i != Sz; ++i) {
    if (Tt->Mid) {
      Tt = Tt->Mid;
      while (Str[i] != Tt->Ch) {
        if (Str[i] < Tt->Ch) {
          if (Tt->Left)
            Tt = Tt->Left;
          else {
            Tt->Left = std::make_shared<MMOTrieNode>(Str[i]);
            Tt = Tt->Left;
            break;
          }
        } else {
          if (Tt->Right)
            Tt = Tt->Right;
          else {
            Tt->Right = std::make_shared<MMOTrieNode>(Str[i]);
            Tt = Tt->Right;
            break;
          }
        }
      }
    } else {
      Tt->Mid = std::make_shared<MMOTrieNode>(Str[i]);
      Tt = Tt->Mid;
    }
  }
  return Tt;
}

std::size_t MMOTrie::writeBin(raw_ostream &OS) const {
  writeBin(OS, Root);
  auto New = alignTo<4>(OutCnt);
  OS.write_zeros(New - OutCnt);
  return New;
}

void MMOTrie::writeBin(raw_ostream &OS, std::shared_ptr<MMOTrieNode> N) const {
  int M = 0;
  if (N->Left)
    M += 0x40;
  if (N->Mid)
    M += 0x20;
  if (N->Right)
    M += 0x10;

  if (N->SymNode) {
    if (N->SymNode->IsRegister)
      M += 0xF;
    N->SymNode->encodeEquivLen(M);
  }
  OS.write(static_cast<std::uint8_t>(M));
  ++OutCnt;

  if (N->Left)
    writeBin(OS, N->Left);

  if (M & 0x2F) {
    // TODO: handle UTF16be
    if (M & 0x80) {
      OS.write(N->Ch >> 8);
    }
    OS.write(N->Ch);
    ++OutCnt;
    M &= 0xF;
    if (M && N->SymNode) {
      OutCnt += N->SymNode->writeBinEquivLen(OS, M);
      OutCnt += N->SymNode->writeBinSerial(OS, M);
    }
    if (N->Mid)
      writeBin(OS, N->Mid);
  }

  if (N->Right)
    writeBin(OS, N->Right);
}
