#include "llvm/MC/MCMMOStreamer.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCObjectStreamer.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/TargetRegistry.h"

namespace llvm {

MCMMOStreamer::MCMMOStreamer(MCContext &Context,
                             std::unique_ptr<MCAsmBackend> MAB,
                             std::unique_ptr<MCObjectWriter> OW,
                             std::unique_ptr<MCCodeEmitter> Emitter)
    : MCObjectStreamer(Context, std::move(MAB), std::move(OW),
                       std::move(Emitter)) {
  // MMIXAL always requires everything is relaxed
  getAssembler().setRelaxAll(true);
}

void MCMMOStreamer::emitInstToData(const MCInst &Inst,
                                   const MCSubtargetInfo &STI) {
  MCDataFragment *DF = getOrCreateDataFragment();
  SmallVector<MCFixup, 4> Fixups;
  SmallString<256> Code;
  getAssembler().getEmitter().encodeInstruction(Inst, Code, Fixups, STI);
  DF->getFixups().append(Fixups);
  DF->getContents().append(Code.begin(), Code.end());
}

bool MCMMOStreamer::emitSymbolAttribute(MCSymbol *Symbol,
                                        MCSymbolAttr Attribute) {
  switch (Attribute) {
  default:
    return false;
  case MCSA_Global:
    Symbol->setExternal(true);
    Symbol->setRedefinable(false);
    getAssembler().registerSymbol(*Symbol);
    return true;
  case MCSA_Local:
    Symbol->setExternal(false);
    return true;
  case MCSA_Internal:
    Symbol->setExternal(false);
    Symbol->setRedefinable(true);
    return true;
  }
  return false;
}

void MCMMOStreamer::emitCommonSymbol(MCSymbol *Symbol, uint64_t Size,
                                     Align ByteAlignment) {}

void MCMMOStreamer::emitZerofill(MCSection *Section, MCSymbol *Symbol,
                                 uint64_t Size, Align ByteAlignment,
                                 SMLoc Loc) {}

MCStreamer *createMMOStreamer(MCContext &Ctx,
                              std::unique_ptr<MCAsmBackend> &&TAB,
                              std::unique_ptr<MCObjectWriter> &&OW,
                              std::unique_ptr<MCCodeEmitter> &&CE) {
  return new MCMMOStreamer(Ctx, std::move(TAB), std::move(OW), std::move(CE));
}

} // namespace llvm