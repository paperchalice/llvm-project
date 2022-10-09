#include "llvm/Object/Binary.h"
#include "llvm/Object/MMIXObjectFile.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/raw_ostream.h"
#include <string>
using namespace llvm;
using namespace llvm::object;

namespace {
cl::opt<bool> ListSymbolTableOnly("s", cl::desc("List only the symbol table"));
cl::opt<bool> Verbose("v", cl::desc("List tetrabytes of input"));
cl::opt<std::string> InputFilename(cl::Positional, cl::desc("<input file>"),
                                   cl::init("-"));

void dumpAddress(const uint64_t Addr, uint64_t Value) {
  outs() << format_hex_no_prefix(Addr, 16) << ": " << format_hex_no_prefix(Value, 16) << "\n";
}
void dumpAddress(const uint64_t Addr, uint32_t Value) {
  outs() << format_hex_no_prefix(Addr, 16) << ": " << format_hex_no_prefix(Value, 8) << "\n";
}

[[noreturn]] void reportError(StringRef File, const Twine &Message,
                              int ExitCode) {
  outs().flush();
  WithColor::error(errs(), "llvm-mmotype")
      << "'" << File << "':" << Message << "\n";
  exit(ExitCode);
}

void reportWarning(StringRef File, const Twine &Message) {
  outs().flush();
  WithColor::warning(errs(), "llvm-mmotype")
      << "'" << File << "': " << Message << "\n";
}

void outputTetra(const unsigned char *Addr) {
  if (!Verbose)
    return;
  outs() << "  ";
  for (int i = 0; i != 4; ++i) {
    outs() << format_hex_no_prefix(Addr[i], 2);
  }
  outs() << "\n";
}

int dumpMMO(MMIXObjectFile *Obj) {
  auto CurrentTetraBegin = Obj->getData().bytes_begin();
  uint64_t CurrentLoc = 0;
  outputTetra(CurrentTetraBegin);

  for (auto CurrentTetraBegin = Obj->getData().bytes_begin();
       CurrentTetraBegin < Obj->getData().bytes_end(); CurrentTetraBegin += 4) {
    if (CurrentTetraBegin[0] == MMO::MM) {
      switch (CurrentTetraBegin[1]) {
      case MMO::LOP_QUOTE:
        CurrentTetraBegin += 4;
        outputTetra(CurrentTetraBegin);
        break;
      case MMO::LOP_LOC: {
        uint64_t Addr = CurrentTetraBegin[2] << 56;
        if (CurrentTetraBegin[3] == 1) {
          CurrentTetraBegin += 4;
          outputTetra(CurrentTetraBegin);
          Addr += support::endian::read32be(CurrentTetraBegin);
        } else if (CurrentTetraBegin[3] == 2) {
          CurrentTetraBegin += 4;
          outputTetra(CurrentTetraBegin);
          Addr += support::endian::read64be(CurrentTetraBegin);
          CurrentTetraBegin += 4;
          outputTetra(CurrentTetraBegin);
        } else {
          reportWarning(Obj->getFileName(),
                        "Z field of lop_loc should be 1 or 2");
        }
      } break;
      case MMO::LOP_SKIP:
        CurrentLoc += support::endian::read16be(CurrentTetraBegin + 2);
        break;
      case MMO::LOP_FIXO: {
        uint64_t Addr = CurrentTetraBegin[1] << 56;
        if (CurrentTetraBegin[2] == 1) {
          CurrentTetraBegin += 4;
          outputTetra(CurrentTetraBegin);
          Addr += support::endian::read32be(CurrentTetraBegin);
          dumpAddress(Addr, CurrentLoc);
        }
      } break;
      default:
        break;
      }
    }
  }
  return 0;
}
} // namespace

int main(int argc, char *argv[]) {
  InitLLVM X(argc, argv);
  cl::ParseCommandLineOptions(argc, argv);
  auto CreateResult = createBinary(InputFilename);
  if (auto E = CreateResult.takeError()) {
    reportError(InputFilename, "Can't open file.", -2);
  }
  OwningBinary<Binary> OBinary = std::move(*CreateResult);
  Binary &Binary = *OBinary.getBinary();

  if (MMIXObjectFile *MMO = dyn_cast<MMIXObjectFile>(&Binary)) {
    return dumpMMO(MMO);
  } else {
    outs().flush();
    WithColor::error(errs(), "llvm-mmotype")
        << "Input is not an MMO file (first two bytes are wrong)!\n";
    return -5;
  }
}
