#ifndef LLVM_TOOLS_MMODUMP_H
#define LLVM_TOOLS_MMODUMP_H

#include "llvm-objdump.h"

namespace llvm {
namespace objdump {

class MMODumper : public Dumper {
  const object::MMIXObjectFile &Obj;

public:
  MMODumper(const object::MMIXObjectFile &O);
};

} // namespace objdump
} // namespace llvm

#endif // LLVM_TOOLS_MMODUMP_H
