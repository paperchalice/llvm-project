#ifndef LLVM_TOOLS_MMIXWARE_LLVM_MMIX_TREAP_H
#define LLVM_TOOLS_MMIXWARE_LLVM_MMIX_TREAP_H

#include <cstdint>
#include <memory>

namespace mmix {

class MemTetra {
public:
  std::uint32_t Tetra;  ///< the tetrabyte of simulated memory
  std::uint32_t Freq;   ///< the number of times it was obeyed as an instruction
  std::uint8_t Bkpt;    ///< breakpoint information for this tetrabyte
  std::uint8_t FileNo;  ///< source  le number, if known
  std::uint16_t LineNo; ///< source line number, if known
};

class MemNode {
public:
  static constexpr std::size_t DAT_LEN = 512;
  static std::uint32_t Priority;
  std::uint64_t Loc;
  std::uint32_t Stamp;
  MemNode *Left = nullptr;
  MemNode *Right = nullptr;
  MemTetra Dat[DAT_LEN] = {};

public:
  MemNode(std::uint64_t Loc);

public:
};

class MemTreap {
public:
private:
  MemNode *Root;
  MemNode *LastMem;

public:
  MemTreap();
  ~MemTreap();

public:
  MemTetra *find(std::uint64_t Addr);

private:
  void destroy(MemNode *Node) noexcept;
};
} // namespace mmix
#endif // LLVM_TOOLS_MMIXWARE_LLVM_MMIX_TREAP_H
