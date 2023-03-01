#ifndef LLVM_MC_MCMMOSTREAMER_H
#define LLVM_MC_MCMMOSTREAMER_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCObjectStreamer.h"

namespace llvm {

class MCMMOStreamer : public MCObjectStreamer {
public:
  MCMMOStreamer(MCContext &Context, std::unique_ptr<MCAsmBackend> MAB,
                std::unique_ptr<MCObjectWriter> OW,
                std::unique_ptr<MCCodeEmitter> Emitter);

public:
  void emitInstToData(const MCInst &Inst, const MCSubtargetInfo &) override;

public:
  // MCStreamer
  bool emitSymbolAttribute(MCSymbol *Symbol, MCSymbolAttr Attribute) override;
  void emitCommonSymbol(MCSymbol *Symbol, uint64_t Size, Align ByteAlignment) override;
  void emitZerofill(MCSection *Section, MCSymbol *Symbol = nullptr,
                            uint64_t Size = 0, Align ByteAlignment = Align(1),
                            SMLoc Loc = SMLoc()) override;
};

} // namespace llvm

#endif // LLVM_MC_MCMMOSTREAMER_H
