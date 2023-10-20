#include "RegisterStack.h"
#include "Simulator.h"
#include "Treap.h"

#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>

using namespace llvm;
using namespace mmix;

RegisterStack::RegisterStack(std::size_t LRingSize, Simulator &S)
    : LRingSize(std::max(LRingSize, 0x100UL)), Sim(S), CurRound(ROUND_NEAR) {
  constexpr std::time_t abstime = 0;
  G[rK] = UINT64_MAX;
  G[rN] = (VERSION_MAJOR << 56) | (VERSION_MINOR << 48) |
          (VERSION_PATCH << 40) | static_cast<std::uint32_t>(abstime);
  G[rT] = 0x8000'0005'0000'0000;
  G[rTT] = 0x8000'0006'0000'0000;
  G[rV] = 0x369c'2004'0000'0000;

  L.resize(this->LRingSize);
}

void RegisterStack::testStoreBkpt(MemTetra *LL) {
  if (LL->Bkpt & write_bit)
    Sim.Ctx.Tracing = Sim.Ctx.Breakpoint = true;
}

void RegisterStack::stackStore() {
  auto LL = Sim.Mem.find(G[rS]);
  auto K = getS() & getLRingMask();
  LL->Tetra = Hi_32(L[K]);
  testStoreBkpt(LL);
  (LL + 1)->Tetra = Lo_32(L[K]);
  testStoreBkpt(LL + 1);

  if (Sim.Ctx.StackTracing) {
    Sim.Ctx.Tracing = true;
    outs() << "to be done\n";
  }
  G[rS] += 8;
}

void RegisterStack::stackLoad() {
  G[rS] -= 8;
  auto LL = Sim.Mem.find(G[rS]);
  auto K = getS() & getLRingMask();
  L[K] = LL->Tetra;
  testLoadBkpt(LL);
  L[K] |= (LL+1)->Tetra;
  testLoadBkpt(LL+1);

  if (Sim.Ctx.StackTracing) {
    Sim.Ctx.Tracing = true;
    outs() << "to be done\n";
  }
}
