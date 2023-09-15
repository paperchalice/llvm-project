#include "MMIXAsmPrinter.h"
#include "MMIXInstrInfo.h"
#include "TargetInfo/MMIXTargetInfo.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/TargetRegistry.h"

namespace llvm {

MMIXAsmPrinter::MMIXAsmPrinter(TargetMachine &TM,
                               std::unique_ptr<MCStreamer> Streamer)
    : AsmPrinter(TM, std::move(Streamer)), InstLowering(OutContext, *this) {}

void MMIXAsmPrinter::emitInstruction(const MachineInstr *MI) {
  MCInst I;
  InstLowering.lower(MI, I);
  EmitToStreamer(*OutStreamer, I);
}

} // namespace llvm

using namespace llvm;

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeMMIXAsmPrinter() {
  RegisterAsmPrinter<MMIXAsmPrinter> TheMMIXAsmPrinter(getTheMMIXTarget());
}
