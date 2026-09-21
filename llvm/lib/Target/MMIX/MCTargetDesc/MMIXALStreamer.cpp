//===-- MMIXALStreamer.cpp - MMIXAL Streamer ------------------------------===//
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

#include "MMIXALStreamer.h"
#include "MMIXInstPrinter.h"

#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/Support/Regex.h"

using namespace llvm;

MMIXALStreamer::MMIXALStreamer(MCContext &Context,
                               std::unique_ptr<formatted_raw_ostream> os,
                               std::unique_ptr<MCInstPrinter> Printer,
                               std::unique_ptr<MCCodeEmitter> Emitter,
                               std::unique_ptr<MCAsmBackend> AsmBackend)
    : MCAsmBaseStreamer(Context, std::move(Emitter), std::move(AsmBackend)),
      OSOwner(std::move(os)), OS(*OSOwner), InstPrinter(std::move(Printer)) {}

void MMIXALStreamer::emitInstruction(const MCInst &Inst,
                                     const MCSubtargetInfo &STI) {
  // Show the encoding in a comment if we have a code emitter.
  addEncodingComment(Inst, STI);

  if (getTargetStreamer())
    getTargetStreamer()->prettyPrintAsm(*InstPrinter, 0, Inst, STI, OS);
  else
    InstPrinter->printInst(&Inst, 0, "", STI, OS);

  StringRef Comments = CommentToEmit;
  if (Comments.size() && Comments.back() != '\n')
    getCommentOS() << '\n';

  OS << '\n';
};

void MMIXALStreamer::AddComment(const Twine &T, bool EOL) {
  raw_ostream &COS = getCommentOS();
  COS << T;
  if (EOL)
    COS << '\n';
}

void MMIXALStreamer::emitAssignment(MCSymbol *Symbol, const MCExpr *Value) {
  Symbol->print(OS, getContext().getAsmInfo());
  OS.PadToColumn(1);
  OS << "IS";
  OS.PadToColumn(2);
  getContext().getAsmInfo().printExpr(OS, *Value);
  OS << '\n';
  MCStreamer::emitAssignment(Symbol, Value);
}

void MMIXALStreamer::emitLabel(MCSymbol *Symbol, SMLoc Loc) {
  MCStreamer::emitLabel(Symbol, Loc);
  Symbol->print(OS, getContext().getAsmInfo());
}

bool MMIXALStreamer::emitSymbolAttribute(MCSymbol *Symbol,
                                         MCSymbolAttr Attribute) {
  return false;
}

void MMIXALStreamer::emitCommonSymbol(MCSymbol *Symbol, uint64_t Size,
                                      Align ByteAlignment) {}
