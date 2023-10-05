#include "llvm/Object/Binary.h"
#include "llvm/Object/MMIXObjectFile.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/WithColor.h"

using namespace llvm;

int main(int argc, char *argv[]) {
  InitLLVM X(argc, argv);
  cl::ParseCommandLineOptions(argc, argv);
}
