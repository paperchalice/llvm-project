#include "llvm/ADT/StringSwitch.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCParser/AsmLexer.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCTargetOptionsCommandFlags.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Option/Arg.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Option/Option.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Compression.h"
#include "llvm/Support/FileUtilities.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/WithColor.h"
#include <ctime>
#include <optional>

using namespace llvm;

namespace {

cl::opt<bool> Expand(
    "x", cl::desc("Expand memory-oriented commands that cannot be assembled as "
                  "single instructions, by assembling auxiliary instructions "
                  "that make temporary use of global register $255."));
cl::opt<std::string>
    ListingName("l",
                cl::desc("Output a listing of the assembled input and "
                         "output to a text file called <listingname>."),
                cl::value_desc("listingname"));
cl::opt<std::size_t>
    BufferSize("b",
               cl::desc("Allow up to <bufsize> characters per line of input."),
               cl::value_desc("bufsize"));
cl::opt<std::string> InputFilename(cl::Positional, cl::desc("<input file>"),
                                   cl::init("-"), cl::desc("sourcefilename"));
} // namespace

int main(int argc, char *argv[]) {
  InitLLVM X(argc, argv);
  cl::ParseCommandLineOptions(argc, argv);

  // Initialize targets and assembly printers/parsers.
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllDisassemblers();

  ErrorOr<std::unique_ptr<MemoryBuffer>> BufferPtr =
      MemoryBuffer::getFileOrSTDIN(InputFilename, /*IsText=*/true);
  if (std::error_code EC = BufferPtr.getError()) {
    WithColor::error(errs(), "mmixal")
        << InputFilename << ": " << EC.message() << '\n';
    return 1;
  }

  SourceMgr SrcMgr;

  // Tell SrcMgr about this buffer, which is what the parser will pick up.
  SrcMgr.AddNewSourceBuffer(std::move(*BufferPtr), SMLoc());

  // FIXME: This is not pretty. MCContext has a ptr to MCObjectFileInfo and
  // MCObjectFileInfo needs a MCContext reference in order to initialize itself.
  constexpr auto TripleStr = "mmix-mmo";
  auto MMIXTriple = Triple(TripleStr);

  std::string Error;
  const Target *TheTarget =
      TargetRegistry::lookupTarget("mmix", MMIXTriple, Error);
  std::unique_ptr<MCRegisterInfo> MRI(TheTarget->createMCRegInfo(TripleStr));
  assert(MRI && "Unable to create MMIX register info!");

  MCTargetOptions MCOptions;
  MCOptions.AssemblyLanguage = "mmixal";

  std::unique_ptr<MCAsmInfo> MAI(
      TheTarget->createMCAsmInfo(*MRI, TripleStr, MCOptions));
  assert(MAI && "Unable to create MMIX asm info!");

  std::unique_ptr<MCInstrInfo> MCII(TheTarget->createMCInstrInfo());
  assert(MCII && "Unable to create instruction info!");

  std::unique_ptr<MCSubtargetInfo> STI(
      TheTarget->createMCSubtargetInfo(TripleStr, "generic", ""));
  assert(STI && "Unable to create subtarget info!");

  auto IP = TheTarget->createMCInstPrinter(MMIXTriple, 0, *MAI, *MCII, *MRI);
  if (!IP) {
    WithColor::error()
        << "unable to create instruction printer for target triple '"
        << MMIXTriple.normalize() << "' with assembly variant " << 0 << ".\n";
    return 1;
  }

  std::unique_ptr<MCAsmBackend> MAB(
      TheTarget->createMCAsmBackend(*STI, *MRI, MCOptions));

  MCContext Ctx(MMIXTriple, MAI.get(), MRI.get(), STI.get(), &SrcMgr,
                &MCOptions);

  // Set up the AsmStreamer.
  std::unique_ptr<MCCodeEmitter> CE(TheTarget->createMCCodeEmitter(*MCII, Ctx));

  std::unique_ptr<MCStreamer> Str;
  auto FOut = std::make_unique<formatted_raw_ostream>();
  Str.reset(TheTarget->createAsmStreamer(
      Ctx, std::move(FOut), /*asmverbose*/ true,
      /*useDwarfDirectory*/ false, IP, std::move(CE), std::move(MAB), true));

  class MMIXMCAsmParser;
  std::unique_ptr<MCAsmParser> Parser(createMCMMIXALParser(SrcMgr, Ctx, *Str, *MAI));
  std::unique_ptr<MCTargetAsmParser> TAP(
      TheTarget->createMCAsmParser(*STI, *Parser, *MCII, MCOptions));
  if (!TAP) {
    WithColor::error(errs(), "mmixal")
        << "this target does not support assembly parsing.\n";
    return 1;
  }
  Parser->setTargetParser(*TAP);
  Parser->Run(false);
  return 0;
}
