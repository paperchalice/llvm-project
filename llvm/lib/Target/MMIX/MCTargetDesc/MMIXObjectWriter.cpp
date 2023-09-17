#include "MMIXObjectWriter.h"
#include "llvm/BinaryFormat/COFF.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/BinaryFormat/MachO.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

MMIXELFObjectWriter::MMIXELFObjectWriter(bool Is64Bit, uint8_t OSABI)
    : MCELFObjectTargetWriter(Is64Bit, OSABI, ELF::EM_NONE, true) {}

unsigned MMIXELFObjectWriter::getRelocType(MCContext &Ctx,
                                           const MCValue &Target,
                                           const MCFixup &Fixup,
                                           bool IsPCRel) const {
  return 0;
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createMMIXELFObjectWriter(bool Is64Bit, uint8_t OSABI) {
  return ::std::unique_ptr<MCObjectTargetWriter>(
      new MMIXELFObjectWriter(Is64Bit, OSABI));
}
