#ifndef LLVM_TOOLS_LLVM_MMIXWARE_LLVM_MMIX_REGISTERSTACK_H
#define LLVM_TOOLS_LLVM_MMIXWARE_LLVM_MMIX_REGISTERSTACK_H

#include "Constant.h"

#include <cstdint>
#include <vector>

namespace mmix {

class Simulator;
class MemTreap;
class MemTetra;

class RegisterStack {
public:
  RegisterStack(std::size_t LRingSize, Simulator &S);
  std::uint64_t getO() const { return G[rO] / 8 % LRingSize; }
  std::uint64_t getS() const { return G[rS] / 8 % LRingSize; }

private:
  std::uint64_t G[256];
  Simulator &Sim;
  std::size_t LRingSize; //< the number of local registers (a power of 2)
  std::uint8_t CurRound;
  std::vector<std::uint64_t> L; //< local registers
  std::size_t getLRingMask() const { return LRingSize - 1; }
  void stackStore();
  void stackLoad();
  void testStoreBkpt(MemTetra *LL);
  void testLoadBkpt(MemTetra *LL);
};

} // namespace mmix

#endif // LLVM_TOOLS_LLVM_MMIXWARE_LLVM_MMIX_REGISTERSTACK_H
