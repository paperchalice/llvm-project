//===-- MMIXALStreamer.h - MMIXAL Streamer --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Stream to MMIXAL as much as possible.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXALSTREAMER_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXALSTREAMER_H

#include "llvm/MC/MCAsmStreamer.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/Support/FormattedStream.h"

namespace llvm {

class MMIXALStreamer : public MCAsmBaseStreamer {
public:
  MMIXALStreamer(MCContext &Context, std::unique_ptr<formatted_raw_ostream> OS,
                 std::unique_ptr<MCInstPrinter> Printer,
                 std::unique_ptr<MCCodeEmitter> Emitter,
                 std::unique_ptr<MCAsmBackend> AsmBackend);

public:
  void addBlankLine() override { OS << '\n'; }
  void emitAssignment(MCSymbol *Symbol, const MCExpr *Value) override;
  void emitLabel(MCSymbol *Symbol, SMLoc Loc) override;
  void AddComment(const Twine &T, bool EOL) override;
  void emitInstruction(const MCInst &Inst, const MCSubtargetInfo &STI) override;
  bool emitSymbolAttribute(MCSymbol *Symbol, MCSymbolAttr Attribute) override;
  void emitCommonSymbol(MCSymbol *Symbol, uint64_t Size,
                        Align ByteAlignment) override;

private:
  std::unique_ptr<formatted_raw_ostream> OSOwner;
  formatted_raw_ostream &OS;
  std::unique_ptr<MCInstPrinter> InstPrinter;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXALSTREAMER_H
