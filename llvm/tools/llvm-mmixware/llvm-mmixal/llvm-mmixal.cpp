#include "MMIXALParser.h"
#include "MMIXMCAsmInfoMMIXAL.h"
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
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileUtilities.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/WithColor.h"
#include "llvm/TargetParser/Host.h"
#include <ctime>
#include <filesystem>
#include <optional>

using namespace llvm;

namespace {

cl::OptionCategory MMIXALCategory("MMIXAL Options");

cl::opt<bool> Expand(
    "x",
    cl::desc("Expand memory-oriented commands that cannot be assembled as "
             "single instructions, by assembling auxiliary instructions "
             "that make temporary use of global register $255."),
    cl::cat(MMIXALCategory));
cl::opt<std::string>
    ListingName("l",
                cl::desc("Output a listing of the assembled input and "
                         "output to a text file called <listingname>."),
                cl::value_desc("listingname"), cl::cat(MMIXALCategory));
cl::opt<std::size_t> BufferSize(
    "b", cl::desc("Allow up to <buffersize> characters per line of input."),
    cl::value_desc("buffersize"), cl::cat(MMIXALCategory));
cl::opt<std::string> InputFilename(cl::Positional, cl::desc("<input file>"),
                                   cl::Required, cl::desc("sourcefilename"),
                                   cl::cat(MMIXALCategory));
cl::opt<std::string> ObjectFileName(
    "o", cl::desc("Send the output to a binary file called <objectfilename>."),
    cl::value_desc("objectfilename"), cl::cat(MMIXALCategory));

cl::OptionCategory LLVMMMIXALExtraCategory("LLVM MMIXAL extra options");
cl::opt<bool> StrictMode("strict-mode", cl::desc("Mimic MMIXAL"),
                         cl::init(false), cl::cat(LLVMMMIXALExtraCategory));
// llvm-mc options

enum class ActionType {
  AC_AsLex,
  AC_Assemble,
  AC_Disassemble,
  AC_MDisassemble,
};

cl::OptionCategory MCCategory("MC Options");

static cl::opt<ActionType> Action(
    cl::desc("Action to perform:"), cl::init(ActionType::AC_Assemble),
    cl::values(clEnumValN(ActionType::AC_AsLex, "as-lex",
                          "Lex tokens from a .s file"),
               clEnumValN(ActionType::AC_Assemble, "assemble",
                          "Assemble a .s file (default)"),
               clEnumValN(ActionType::AC_Disassemble, "disassemble",
                          "Disassemble strings of hex bytes"),
               clEnumValN(ActionType::AC_MDisassemble, "mdis",
                          "Marked up disassembly of strings of hex bytes")),
    cl::cat(MCCategory));
} // namespace

static std::unique_ptr<ToolOutputFile> GetOutputStream(std::string Path) {
  // convert to output file name
  StringRef OutputFileNameRef = Path;
  if (OutputFileNameRef.ends_with(".mms")) {
    *Path.rbegin() = 'o';
  } else if (OutputFileNameRef.ends_with(".MMS")) {
    *Path.rbegin() = 'O';
  } else {
    Path += ".mmo";
  }
  if (!ObjectFileName.empty()) {
    Path = ObjectFileName;
  }

  std::error_code EC;
  auto Out = std::make_unique<ToolOutputFile>(Path, EC, sys::fs::OF_None);
  if (EC) {
    WithColor::error() << EC.message() << '\n';
    exit(1);
  }
  return Out;
}

static int AsLexInput(const std::unique_ptr<MCAsmParser> &Parser,
                      raw_ostream &OS) {

  bool Error = false;
  while (Parser->Lex().isNot(AsmToken::Eof)) {
    auto Tok = Parser->getTok();
    Tok.dump(OS);
    if (Tok.is(AsmToken::Integer)) {
      OS << " [ " << Tok.getIntVal() << " ]";
    }
    OS << "\n";
    if (Parser->getTok().getKind() == AsmToken::Error)
      Error = true;
  }
  return Error;
}

extern "C" {
void LLVMInitializeMMIXTargetInfo();
void LLVMInitializeMMIXTargetMC();
void LLVMInitializeMMIXDisassembler();
}
namespace llvm::MMIXAL {
MCTargetAsmParser *LLVMInitializeMMIXALTargetParser(
    const MCSubtargetInfo &STI, MCAsmParser &Parser, const MCInstrInfo &MII,
    const MCTargetOptions &Options, llvm::MMO::AsmSharedInfo &SI);
}

int main(int argc, char *argv[]) {
  InitLLVM X(argc, argv);
  cl::ParseCommandLineOptions(argc, argv);
  auto ProgName = StringRef(std::filesystem::path(argv[0]).stem());
  if (ProgName.lower() == "mmixal") {
    if (!StrictMode)
      StrictMode = true;
  }
  // Initialize targets and assembly printers/parsers.
  LLVMInitializeMMIXTargetInfo();
  LLVMInitializeMMIXTargetMC();
  LLVMInitializeMMIXDisassembler();

  ErrorOr<std::unique_ptr<MemoryBuffer>> BufferPtr =
      MemoryBuffer::getFileOrSTDIN(InputFilename, /*IsText=*/true);
  raw_ostream &ListingStream = [&]() -> raw_ostream & {
    if (ListingName.getNumOccurrences() == 0) {
      return nulls();
    } else {
      std::error_code EC;
      std::string Name = ListingName.empty() ? std::string("-") : ListingName;
      static raw_fd_ostream Ret(Name, EC);
      if (EC) {
        WithColor::error(errs(), "mmixal") << "unable to open " << Name << '\n';
        std::exit(254);
      }
      return Ret;
    }
  }();

  if (std::error_code EC = BufferPtr.getError()) {
    WithColor::error(errs(), "mmixal")
        << InputFilename << ": " << EC.message() << '\n';
    return 254;
  }

  std::unique_ptr<ToolOutputFile> Out = GetOutputStream(InputFilename);
  raw_pwrite_stream *OS = &Out->os();

  SourceMgr SrcMgr;

  // Tell SrcMgr about this buffer, which is what the parser will pick up.
  SrcMgr.AddNewSourceBuffer(std::move(*BufferPtr), SMLoc());

  constexpr auto TripleStr = "mmix-none-mmixware-mmo";
  auto MMIXTriple = Triple(TripleStr);

  std::string Error;
  const Target *TheTarget =
      TargetRegistry::lookupTarget("mmix", MMIXTriple, Error);
  std::unique_ptr<MCRegisterInfo> MRI(TheTarget->createMCRegInfo(TripleStr));
  assert(MRI && "Unable to create MMIX register info!");

  MCTargetOptions MCOptions;
  MCOptions.AssemblyLanguage = "mmixal";

  std::unique_ptr<MMIXMCAsmInfoMMIXAL> MAI(
      createMMIXMCAsmInfoMMIXAL(MCOptions));
  std::unique_ptr<MCInstrInfo> MCII(TheTarget->createMCInstrInfo());
  MAI->StrictMode = StrictMode;

  std::unique_ptr<MCSubtargetInfo> STI(
      TheTarget->createMCSubtargetInfo(TripleStr, "generic", ""));
  auto IP = TheTarget->createMCInstPrinter(MMIXTriple, 0, *MAI, *MCII, *MRI);
  if (!IP) {
    WithColor::error()
        << "unable to create instruction printer for target triple '"
        << MMIXTriple.normalize() << "' with assembly variant " << 0 << ".\n";
    exit(1);
  }

  auto *MAB = TheTarget->createMCAsmBackend(*STI, *MRI, MCOptions);

  // FIXME: This is not pretty. MCContext has a ptr to MCObjectFileInfo and
  // MCObjectFileInfo needs a MCContext reference in order to initialize itself.
  MCContext Ctx(MMIXTriple, MAI.get(), MRI.get(), STI.get(), &SrcMgr,
                &MCOptions);
  std::unique_ptr<MCObjectFileInfo> MOFI(TheTarget->createMCObjectFileInfo(
      Ctx, /*PIC=*/false, /*LargeCodeModel=*/true));
  Ctx.setObjectFileInfo(MOFI.get());

  // Set up the Object streamer
  auto *CE = TheTarget->createMCCodeEmitter(*MCII, Ctx);
  std::unique_ptr<MCStreamer> Str;
  auto FOut = std::make_unique<formatted_raw_ostream>(*OS);
  Str.reset(TheTarget->createMCObjectStreamer(
      MMIXTriple, Ctx, std::unique_ptr<MCAsmBackend>(MAB),
      MAB->createObjectWriter(*OS), std::unique_ptr<MCCodeEmitter>(CE), *STI,
      MCOptions.MCRelaxAll, MCOptions.MCIncrementalLinkerCompatible,
      /*DWARFMustBeAtTheEnd*/ false));

  auto ParserPtr = createMCMMIXALParser(SrcMgr, Ctx, *Str, *MAI, ListingStream);
  ParserPtr->SharedInfo.Expand = Expand;
  std::unique_ptr<MCAsmParser> Parser(ParserPtr);
  std::unique_ptr<MCTargetAsmParser> TAP(
      MMIXAL::LLVMInitializeMMIXALTargetParser(*STI, *Parser, *MCII, MCOptions,
                                               ParserPtr->SharedInfo));
  Parser->setTargetParser(*TAP);

  int Res = 1;
  switch (Action) {
  case ActionType::AC_AsLex:
    Res = AsLexInput(Parser, *OS);
    break;
  case ActionType::AC_Assemble:
    Res = Parser->Run(false);
    break;
  default:
    break;
  }

  if (Res == 0) {
    Out->keep();
  }
  return ParserPtr->getErrorCount();
}
