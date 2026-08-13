//===-- MMIXTargetStreamer.h - MMIX Target Streamer -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXTARGETSTREAMER_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXTARGETSTREAMER_H

#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/FormattedStream.h"

namespace llvm {

class MMIXTargetStreamer : public MCTargetStreamer {
public:
  using MCTargetStreamer::MCTargetStreamer;

public:
  StringRef getPrefix() const { return Prefix; }
  void setPrefix(StringRef NewPrefix) { Prefix = NewPrefix; }
  void emitGREG(const MCExpr &Expr, StringRef Name) {
    GlobalRegs.push_back(&Expr);
    emitGREG(Name);
  }
  virtual void emitGREG(StringRef Name);
  void emitPREFIX(StringRef NewPrefix) {
    Prefix = NewPrefix;
    emitPREFIX();
  }
  virtual void emitPREFIX() {}

  bool isOSBinFormatELF();

protected:
  int getCurrentGlobalReg() const { return 254 - GlobalRegs.size(); }
  int getLastGlobalReg() const { return 255 - GlobalRegs.size(); }

protected:
  std::string Prefix;
  std::vector<const MCExpr *> GlobalRegs;
};

class MMIXTargetAsmStreamer : public MMIXTargetStreamer {
public:
  MMIXTargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS)
      : MMIXTargetStreamer(S), OS(OS) {}

public:
  void emitPREFIX() override;

private:
  formatted_raw_ostream &OS;
};

class MMIXTargetELFStreamer : public MMIXTargetStreamer {
public:
  using MMIXTargetStreamer::MMIXTargetStreamer;
};

MCTargetStreamer *createMMIXObjectTargetStreamer(MCStreamer &S,
                                                 const MCSubtargetInfo &STI);

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXTARGETSTREAMER_H
