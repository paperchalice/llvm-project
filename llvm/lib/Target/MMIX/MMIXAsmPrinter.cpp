#include "MMIXAsmPrinter.h"
#include "TargetInfo/MMIXTargetInfo.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/Support/TargetRegistry.h"
#include "MMIXInstrInfo.h"

namespace llvm {

MMIXAsmPrinter::MMIXAsmPrinter(TargetMachine &TM,
                               std::unique_ptr<MCStreamer> Streamer)
    : AsmPrinter(TM, std::move(Streamer)) {}


void MMIXAsmPrinter::emitInstruction(const MachineInstr *MI) {
  auto I = MCInstBuilder(MMIX::Inst).addReg(MMIX::RA).addReg(MMIX::RA).addReg(MMIX::RA);
  OutStreamer->emitInstruction(I, getSubtargetInfo());
}

} // namespace llvm

using namespace llvm;

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeMMIXAsmPrinter() {
  RegisterAsmPrinter<MMIXAsmPrinter> AP0(getTheMMIXTarget());
}
