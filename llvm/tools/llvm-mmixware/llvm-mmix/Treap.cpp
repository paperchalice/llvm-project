#include "Treap.h"

using namespace mmix;

std::uint32_t MemNode::Priority = 314159265; //< pseudorandom time stamp counter

MemNode::MemNode(std::uint64_t Loc) : Loc(Loc) {
  Stamp = Priority;
  Priority += 0x9e3779b9; // \(\lfloor 2^32(\phi - 1) \rfloor\)
}

void MemTreap::destroy(MemNode *Node) noexcept {
  if (!Node)
    return;
  destroy(Node->Left);
  destroy(Node->Right);
  delete Node;
}

MemTreap::MemTreap() {
  Root = new MemNode(0x4000'0000'0000'0000);
  LastMem = Root;
}

MemTreap::~MemTreap() { destroy(Root); }

MemTetra *MemTreap::find(std::uint64_t Addr) {
  auto Offset = Addr & 0x7FC;
  auto Key = Addr & 0xFFFF'F800;
  auto *P = LastMem;
  MemNode **Q = &Root;
  if (P->Loc != Key) {
    for (P = Root; P;) {
      if (Key == P->Loc)
        goto found;
      if (Key <= P->Loc)
        P = P->Left;
      else
        P = P->Right;
    }

    for (P = Root; P && P->Stamp < MemNode::Priority; P = *Q) {
      if (Key <= P->Loc)
        Q = &P->Left;
      else
        Q = &P->Right;
    }
    *Q = new MemNode(Key);
    {
      auto L = &(*Q)->Left, R = &(*Q)->Right;
      while (P) {
        if (Key <= P->Loc)
          *R = P, R = &P->Left, P = *R;
        else
          *L = P, L = &P->Right, P = *L;
      }
      L = R = nullptr;
    }
    P = *Q;
  found:
    LastMem = P;
  }
  return &LastMem->Dat[Offset >> 2];
}
