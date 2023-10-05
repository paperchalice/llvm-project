#include "llvm/Object/Binary.h"
#include "llvm/Object/MMIXObjectFile.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/WithColor.h"
#include "MMIXInstrInfo.h"

using namespace llvm;
using namespace llvm::object;

namespace {
static StringRef Overview =
    "This program simulates a simplied version of the MMIX computer.";
cl::opt<std::size_t> TraceThreshold(
    "t", cl::desc("Trace each instruction the first <n> times it is executed."),
    cl::value_desc("n"));
cl::opt<std::size_t>
    TracingExceptions("e",
           cl::desc("trace each instruction with an exception matching <x>"),
           cl::value_desc("x"));
cl::opt<bool> StackTracing("r",
                     cl::desc("trace hidden details of the register stack"));
cl::opt<std::size_t>
    ShowingSource("l", cl::desc("list source lines when tracing, filling gaps <= <n>"),
         cl::value_desc("n"));
cl::opt<bool> ShowingStats("s", cl::desc("show statistics after each traced instruction"));
cl::opt<bool> Profiling("P", cl::desc("print a profile when simulation ends"));
cl::opt<std::size_t>
    ListWithProfile("L", cl::desc("list source lines with the profile"),
         cl::value_desc("n"));
cl::opt<bool> Verbose("v", cl::desc("be verbose: show almost everything"));
cl::opt<bool> Quiet("q", cl::desc("be quiet: show only the simulated standard output"));
cl::opt<bool> Interacting("i", cl::desc("run interactively (prompt for online commands)"));
cl::opt<bool> InteractAfterBreak("I", cl::desc("interact, but only after the program halts"));
cl::opt<std::size_t>
    BufferSize("b", cl::desc("change the buffer size for source lines"),
         cl::value_desc("n"));
cl::opt<std::size_t>
    LringSize("c", cl::desc("change the cyclic local register ring size"),
         cl::value_desc("n"));
cl::opt<std::string>
    StdinFileName("f", cl::desc("use given file to simulate standard input"),
         cl::value_desc("filename"));
cl::opt<std::string>
    DumpFileName("D", cl::desc("dump a file for use by other simulators"),
         cl::value_desc("filename"));
cl::opt<std::string> InputFilename(cl::Positional, cl::desc("<progfile>"));
} // namespace

int main(int argc, char *argv[]) {
  InitLLVM X(argc, argv);
  cl::ParseCommandLineOptions(argc, argv, Overview, nullptr, nullptr, true);
  // auto CreateResult = createBinary(InputFilename, nullptr, true);
  // ExitOnError ExitOnErr;
  // ExitOnErr.setBanner(std::string(argv[0]) + ": ");
  // if (auto E = CreateResult.takeError()) {
  //   ExitOnErr(std::move(E));
  // }
  // OwningBinary<Binary> OBinary = std::move(*CreateResult);
  // Binary &Binary = *OBinary.getBinary();
  // if (MMIXObjectFile *MMO = dyn_cast<MMIXObjectFile>(&Binary)) {

  // } else {
  //   outs().flush();
  //   WithColor::error(errs(), "llvm-mmotype")
  //       << "Input is not an MMO file (first two bytes are wrong)!\n";
  //   return -5;
  // }
  return 0;
}
