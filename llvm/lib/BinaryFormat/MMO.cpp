#include "llvm/BinaryFormat/MMO.h"
#include "llvm/Support/Endian.h"

using namespace llvm;
using namespace llvm::support::endian;
using namespace llvm::MMO;

void SymNode::encodeEquivLen(int &M) {
  std::uint32_t X = Hi_32(Equiv);
  if ((X & 0xFFFF'0000) == 0x2000'0000) {
    M += 8;
    X -= 0x2000'0000;
  }
  if (X) {
    M += 4;
  } else {
    X = Equiv & 0xFFFF'FFFF;
  }
  int j = 1;
  for (; j < 4; ++j) {
    if (X < static_cast<uint32_t>(1 << (8 * j)))
      break;
  }
  M += j;
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
  Tt->SymNode = {S.Serial, S.Equiv, S.Type == MMO::SymbolType::REGISTER};
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
    if (N->SymNode->IsRegister) {
      M += 0xF;
    } else {
      N->SymNode->encodeEquivLen(M);
    }
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
