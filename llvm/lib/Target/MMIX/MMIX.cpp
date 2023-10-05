#include "MMIX.h"

using namespace llvm;

static cl::OptionCategory MMIXCategory("MMIX specific options",
                                       "These control some MMIX parameters");

cl::opt<std::uint8_t> MMIX::backend::InitialGCounter(
    "mmix-initial-g", cl::desc("X86: Set G counter value"), cl::init(231),
    cl::Hidden, cl::cat(MMIXCategory));

cl::opt<std::uint8_t> MMIX::backend::InitialLCounter(
    "mmix-initial-l", cl::desc("MMIX: Set L counter initial value"),
    cl::init(32), cl::Hidden, cl::cat(MMIXCategory));
