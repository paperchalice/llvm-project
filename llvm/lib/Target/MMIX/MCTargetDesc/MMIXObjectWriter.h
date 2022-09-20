#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXOBJECTWRITER_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXOBJECTWRITER_H

#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCMachObjectWriter.h"
#include "llvm/MC/MCWinCOFFObjectWriter.h"
#include <memory>

namespace llvm {




class MMIXELFObjectWriter : public MCELFObjectTargetWriter {
public:
  MMIXELFObjectWriter(bool Is64Bit, uint8_t OSABI);
  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override;

protected:
private:
};

std::unique_ptr<MCObjectTargetWriter>
createMMIXELFObjectWriter(bool Is64Bit, uint8_t OSABI);




} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXOBJECTWRITER_H
