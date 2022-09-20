#ifndef LLVM_LIB_TARGET_MMIX_ASM_MMIXASMPRINTER_H
#define LLVM_LIB_TARGET_MMIX_ASM_MMIXASMPRINTER_H

#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"

namespace llvm {

class MMIXAsmPrinter : public AsmPrinter {
public:
  MMIXAsmPrinter(TargetMachine &TM, std::unique_ptr<MCStreamer> Streamer);
  void emitInstruction(const MachineInstr *MI) override;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_ASM_MMIXASMPRINTER_H
