#ifndef LLVM_TOOLS_LLVM_MMIXWARE_LLVM_MMIX_SIMULATOR_H
#define LLVM_TOOLS_LLVM_MMIXWARE_LLVM_MMIX_SIMULATOR_H

#include "Treap.h"
#include "RegisterStack.h"

#include <cstdint>

namespace mmix {

struct SimulatorContext {
std::uint32_t CurLoc = 0;
int ShownLine = 0;
int CurLine = 0;
int ShownFile = -1;
int CurFile = -1;
std::uint64_t Tmp; // an octabyte of temporary interest

bool Tracing;
bool StackTracing;
bool Breakpoint;

void showLine() const;
};

class Simulator {
friend RegisterStack;

private:
SimulatorContext Ctx;

MemTreap Mem;
RegisterStack RegStack;
std::uint64_t Clock;
SimulatorContext Ctx;

};

}

#endif // LLVM_TOOLS_LLVM_MMIXWARE_LLVM_MMIX_SIMULATOR_H
