#ifndef LLVM_TOOLS_LLVM_MMIXWARE_LLVM_MMIX_VERSION_H
#define LLVM_TOOLS_LLVM_MMIXWARE_LLVM_MMIX_VERSION_H

#include <cstdint>

namespace mmix {

constexpr inline std::uint64_t VERSION_MAJOR = 1;
constexpr inline std::uint64_t VERSION_MINOR = 0;
constexpr inline std::uint64_t VERSION_PATCH = 3;

enum {
  trace_bit = 1 << 3,
  read_bit = 1 << 2,
  write_bit = 1 << 1,
  exec_bit = 1 << 0,
};

} // namespace mmix

#endif // LLVM_TOOLS_LLVM_MMIXWARE_LLVM_MMIX_VERSION_H
