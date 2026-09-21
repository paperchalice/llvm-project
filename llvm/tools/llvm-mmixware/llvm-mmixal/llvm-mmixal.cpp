//===-- llvm-mmixal.cpp - MMIX assembler for LLVM -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "AsmParser/MMIXALParser.h"

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
#include "llvm/Support/LLVMDriver.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/WithColor.h"
#include "llvm/TargetParser/Host.h"

using namespace llvm;
using namespace MMIX;

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
cl::opt<std::size_t>
    BufferSize("b",
               cl::desc("Allow up to <buffersize> characters per line of "
                        "input. (This option has no effect)"),
               cl::value_desc("buffersize"), cl::Hidden,
               cl::cat(MMIXALCategory));
cl::opt<std::string> InputFilename(cl::Positional, cl::desc("<input file>"),
                                   cl::Required, cl::desc("sourcefilename"),
                                   cl::cat(MMIXALCategory));
cl::opt<std::string> ObjectFileName(
    "o", cl::desc("Send the output to a binary file called <objectfilename>."),
    cl::value_desc("objectfilename"), cl::cat(MMIXALCategory));
cl::opt<bool> RelaxAll(
    "relax-all",
    cl::desc("Relax all instructions and no fixup lops will be emitted"));

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

static int AsLexInput(ALParser &Parser, raw_ostream &OS) {
  bool Error = false;
  while (Parser.lex().isNot(ALToken::Eof)) {
    auto Tok = Parser.getTok();
    Tok.dump(OS);
    if (Tok.is(ALToken::Integer)) {
      OS << " [ " << Tok.getUIntVal() << " ]";
    }
    OS << "\n";
    if (Parser.getTok().getKind() == ALToken::Error)
      Error = true;
  }
  return Error;
}

int llvm_mmixal_main(int argc, char *argv[], const llvm::ToolContext &) {
  // Initialize targets and assembly printers/parsers.
  LLVMInitializeMMIXTargetInfo();
  LLVMInitializeMMIXTargetMC();
  LLVMInitializeMMIXAsmParser();

  cl::ParseCommandLineOptions(argc, argv, "llvm mmixal\n");

  ErrorOr<std::unique_ptr<MemoryBuffer>> BufferPtr =
      MemoryBuffer::getFileOrSTDIN(InputFilename, /*IsText=*/true);

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

  auto MMIXTriple = Triple(Triple::mmix);
  std::string Error;
  const Target *TheTarget = TargetRegistry::lookupTarget(MMIXTriple, Error);
  std::unique_ptr<MCRegisterInfo> MRI(TheTarget->createMCRegInfo(MMIXTriple));
  assert(MRI && "Unable to create MMIX register info!");
  MCTargetOptions MCOptions;
  MCOptions.AssemblyLanguage = "mmixal";
  MCOptions.MCRelaxAll = RelaxAll;
  std::unique_ptr<MCAsmInfo> MAI(
      TheTarget->createMCAsmInfo(*MRI, MMIXTriple, MCOptions));
  std::unique_ptr<MCSubtargetInfo> STI(
      TheTarget->createMCSubtargetInfo(MMIXTriple, "generic", ""));
  std::unique_ptr<MCInstrInfo> MCII(TheTarget->createMCInstrInfo());
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
  MCContext Ctx(MMIXTriple, *MAI, *MRI, *STI, &SrcMgr, &MCOptions);
  std::unique_ptr<MCObjectFileInfo> MOFI(TheTarget->createMCObjectFileInfo(
      Ctx, /*PIC=*/false, /*LargeCodeModel=*/true));
  Ctx.setObjectFileInfo(MOFI.get());

  // Set up the Object streamer
  auto *CE = TheTarget->createMCCodeEmitter(*MCII, Ctx);
  std::unique_ptr<MCStreamer> Str;
  auto FOut = std::make_unique<formatted_raw_ostream>(*OS);
  Str.reset(TheTarget->createMCObjectStreamer(
      MMIXTriple, Ctx, std::unique_ptr<MCAsmBackend>(MAB),
      MAB->createObjectWriter(*OS), std::unique_ptr<MCCodeEmitter>(CE), *STI));

  std::unique_ptr<ALParser> Parser(
      createMCMMIXALParser(SrcMgr, Ctx, *Str, *MAI));

  int Res = 1;
  switch (Action) {
  case ActionType::AC_AsLex:
    Res = AsLexInput(*Parser, dbgs());
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

  return 0;
}
