#include "llvm/BinaryFormat/MMO.h"
#include "llvm/MC/MCAsmLayout.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCMMIXObjectWriter.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCSymbolMMO.h"
#include "llvm/MC/MCFragment.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Object/MMIXObjectFile.h"
#include "llvm/Support/Alignment.h"
#include "llvm/Support/Casting.h"
#include <ctime>

namespace {

using namespace llvm;

// copy from MCAssembler.cpp
static void writeFragment(raw_ostream &OS, const MCAssembler &Asm,
                          const MCAsmLayout &Layout, const MCFragment &F) {
  // FIXME: Embed in fragments instead?
  uint64_t FragmentSize = Asm.computeFragmentSize(Layout, F);

  support::endianness Endian = support::endianness::big;

  if (const MCEncodedFragment *EF = dyn_cast<MCEncodedFragment>(&F))
    Asm.writeFragmentPadding(OS, *EF, FragmentSize);

  // This variable (and its dummy usage) is to participate in the assert at
  // the end of the function.
  uint64_t Start = OS.tell();
  (void)Start;

  switch (F.getKind()) {
  case MCFragment::FT_Align: {
    const MCAlignFragment &AF = cast<MCAlignFragment>(F);
    assert(AF.getValueSize() && "Invalid virtual align in concrete fragment!");

    uint64_t Count = FragmentSize / AF.getValueSize();

    // FIXME: This error shouldn't actually occur (the front end should emit
    // multiple .align directives to enforce the semantics it wants), but is
    // severe enough that we want to report it. How to handle this?
    if (Count * AF.getValueSize() != FragmentSize)
      report_fatal_error("undefined .align directive, value size '" +
                         Twine(AF.getValueSize()) +
                         "' is not a divisor of padding size '" +
                         Twine(FragmentSize) + "'");

    // See if we are aligning with nops, and if so do that first to try to fill
    // the Count bytes.  Then if that did not fill any bytes or there are any
    // bytes left to fill use the Value and ValueSize to fill the rest.
    // If we are aligning with nops, ask that target to emit the right data.
    if (AF.hasEmitNops()) {
      if (!Asm.getBackend().writeNopData(OS, Count, AF.getSubtargetInfo()))
        report_fatal_error("unable to write nop sequence of " + Twine(Count) +
                           " bytes");
      break;
    }

    // Otherwise, write out in multiples of the value size.
    for (uint64_t i = 0; i != Count; ++i) {
      switch (AF.getValueSize()) {
      default:
        llvm_unreachable("Invalid size!");
      case 1:
        OS << char(AF.getValue());
        break;
      case 2:
        support::endian::write<uint16_t>(OS, AF.getValue(), Endian);
        break;
      case 4:
        support::endian::write<uint32_t>(OS, AF.getValue(), Endian);
        break;
      case 8:
        support::endian::write<uint64_t>(OS, AF.getValue(), Endian);
        break;
      }
    }
    break;
  }

  case MCFragment::FT_Data:
    OS << cast<MCDataFragment>(F).getContents();
    break;

  case MCFragment::FT_Relaxable:
    OS << cast<MCRelaxableFragment>(F).getContents();
    break;

  case MCFragment::FT_CompactEncodedInst:
    OS << cast<MCCompactEncodedInstFragment>(F).getContents();
    break;

  case MCFragment::FT_Fill: {
    const MCFillFragment &FF = cast<MCFillFragment>(F);
    uint64_t V = FF.getValue();
    unsigned VSize = FF.getValueSize();
    const unsigned MaxChunkSize = 16;
    char Data[MaxChunkSize];
    assert(0 < VSize && VSize <= MaxChunkSize && "Illegal fragment fill size");
    // Duplicate V into Data as byte vector to reduce number of
    // writes done. As such, do endian conversion here.
    for (unsigned I = 0; I != VSize; ++I) {
      unsigned index = Endian == support::little ? I : (VSize - I - 1);
      Data[I] = uint8_t(V >> (index * 8));
    }
    for (unsigned I = VSize; I < MaxChunkSize; ++I)
      Data[I] = Data[I - VSize];

    // Set to largest multiple of VSize in Data.
    const unsigned NumPerChunk = MaxChunkSize / VSize;
    // Set ChunkSize to largest multiple of VSize in Data
    const unsigned ChunkSize = VSize * NumPerChunk;

    // Do copies by chunk.
    StringRef Ref(Data, ChunkSize);
    for (uint64_t I = 0, E = FragmentSize / ChunkSize; I != E; ++I)
      OS << Ref;

    // do remainder if needed.
    unsigned TrailingCount = FragmentSize % ChunkSize;
    if (TrailingCount)
      OS.write(Data, TrailingCount);
    break;
  }

  case MCFragment::FT_Nops: {
    const MCNopsFragment &NF = cast<MCNopsFragment>(F);

    int64_t NumBytes = NF.getNumBytes();
    int64_t ControlledNopLength = NF.getControlledNopLength();
    int64_t MaximumNopLength =
        Asm.getBackend().getMaximumNopSize(*NF.getSubtargetInfo());

    assert(NumBytes > 0 && "Expected positive NOPs fragment size");
    assert(ControlledNopLength >= 0 && "Expected non-negative NOP size");

    if (ControlledNopLength > MaximumNopLength) {
      Asm.getContext().reportError(NF.getLoc(),
                                   "illegal NOP size " +
                                       std::to_string(ControlledNopLength) +
                                       ". (expected within [0, " +
                                       std::to_string(MaximumNopLength) + "])");
      // Clamp the NOP length as reportError does not stop the execution
      // immediately.
      ControlledNopLength = MaximumNopLength;
    }

    // Use maximum value if the size of each NOP is not specified
    if (!ControlledNopLength)
      ControlledNopLength = MaximumNopLength;

    while (NumBytes) {
      uint64_t NumBytesToEmit =
          (uint64_t)std::min(NumBytes, ControlledNopLength);
      assert(NumBytesToEmit && "try to emit empty NOP instruction");
      if (!Asm.getBackend().writeNopData(OS, NumBytesToEmit,
                                         NF.getSubtargetInfo())) {
        report_fatal_error("unable to write nop sequence of the remaining " +
                           Twine(NumBytesToEmit) + " bytes");
        break;
      }
      NumBytes -= NumBytesToEmit;
    }
    break;
  }

  case MCFragment::FT_LEB: {
    const MCLEBFragment &LF = cast<MCLEBFragment>(F);
    OS << LF.getContents();
    break;
  }

  case MCFragment::FT_BoundaryAlign: {
    const MCBoundaryAlignFragment &BF = cast<MCBoundaryAlignFragment>(F);
    if (!Asm.getBackend().writeNopData(OS, FragmentSize, BF.getSubtargetInfo()))
      report_fatal_error("unable to write nop sequence of " +
                         Twine(FragmentSize) + " bytes");
    break;
  }

  case MCFragment::FT_SymbolId: {
    const MCSymbolIdFragment &SF = cast<MCSymbolIdFragment>(F);
    support::endian::write<uint32_t>(OS, SF.getSymbol()->getIndex(), Endian);
    break;
  }

  case MCFragment::FT_Org: {
    const MCOrgFragment &OF = cast<MCOrgFragment>(F);

    for (uint64_t i = 0, e = FragmentSize; i != e; ++i)
      OS << char(OF.getValue());

    break;
  }

  case MCFragment::FT_Dwarf: {
    const MCDwarfLineAddrFragment &OF = cast<MCDwarfLineAddrFragment>(F);
    OS << OF.getContents();
    break;
  }
  case MCFragment::FT_DwarfFrame: {
    const MCDwarfCallFrameFragment &CF = cast<MCDwarfCallFrameFragment>(F);
    OS << CF.getContents();
    break;
  }
  case MCFragment::FT_CVInlineLines: {
    const auto &OF = cast<MCCVInlineLineTableFragment>(F);
    OS << OF.getContents();
    break;
  }
  case MCFragment::FT_CVDefRange: {
    const auto &DRF = cast<MCCVDefRangeFragment>(F);
    OS << DRF.getContents();
    break;
  }
  case MCFragment::FT_PseudoProbe: {
    const MCPseudoProbeAddrFragment &PF = cast<MCPseudoProbeAddrFragment>(F);
    OS << PF.getContents();
    break;
  }
  case MCFragment::FT_Dummy:
    llvm_unreachable("Should not have been added");
  }

  assert(OS.tell() - Start == FragmentSize &&
         "The stream should advance by fragment size");
}

class MMIXObjectWriter : public MCObjectWriter {
  ::support::endian::Writer W;

  /// The target specific MMO writer instance.
  std::unique_ptr<MCMMIXObjectTargetWriter> TargetObjectWriter;

private:
  MMO::MMOTrie SymbolTrie;

public:
  MMIXObjectWriter(std::unique_ptr<MCMMIXObjectTargetWriter> MOTW,
                   raw_pwrite_stream &OS)
      : W(OS, support::big), TargetObjectWriter(std::move(MOTW)) {}
  ~MMIXObjectWriter() override {}

public:
  void recordRelocation(MCAssembler &Asm, const MCAsmLayout &Layout,
                        const MCFragment *Fragment, const MCFixup &Fixup,
                        MCValue Target, uint64_t &FixedValue) override {}

  void executePostLayoutBinding(MCAssembler &Asm,
                                const MCAsmLayout &Layout) override {}

  uint64_t writeObject(MCAssembler &Asm, const MCAsmLayout &Layout) override;

private:
  void writeSymbolTable(MCAssembler &Asm);
};

uint64_t MMIXObjectWriter::writeObject(MCAssembler &Asm,
                                       const MCAsmLayout &Layout) {
  uint64_t Cnt = 0;
  // write preamble
  W.write<uint8_t>({MMO::MM, MMO::LOP_PRE, MMO::CurrentVersion, '\x01'});
  W.write<uint32_t>(static_cast<uint32_t>(time(nullptr)));
  Cnt += 4;

  for (const MCSection &Sec : Asm) {
    // FIXME: Shoud be below
    // Asm.writeSectionData(W.OS, &Sec, Layout);
    // but it would trigger assertion when fix lop_fixo
    for(const auto &F : Sec.getFragmentList())
      writeFragment(W.OS, Asm, Layout, F);
  }

  writeSymbolTable(Asm);
  return Cnt;
}

void MMIXObjectWriter::writeSymbolTable(MCAssembler &Asm) {
  W.write(ArrayRef("\x98\x0b\x00\x00", 4));
  auto SymbolTable = Asm.symbols();
  for (auto &Symbol : SymbolTable) {
    auto MMOSymbol = cast<MCSymbolMMO>(&Symbol);
    if (MMOSymbol->getIndex() == 0) {
      continue; // skip directional symbol
    }
    MMOSymbol->isReg();
    MMO::Symbol S = {Symbol.getName().drop_front().str(), Symbol.getIndex(),
                     MMOSymbol->getEquivalent(),
                     MMOSymbol->isReg() ? MMO::REGISTER : MMO::NORMAL};
    SymbolTrie.insert(S);
  }
  auto Cnt = SymbolTrie.writeBin(W.OS);
  W.OS.write("\x98\x0c", 2);
  char Data[2];
  support::endian::write16be(Data, Cnt / 4);
  W.OS.write(Data, 2);
}

} // namespace

namespace llvm {

std::unique_ptr<MCObjectWriter>
createMMIXObjectWriter(std::unique_ptr<MCMMIXObjectTargetWriter> MOTW,
                       raw_pwrite_stream &OS) {
  return std::make_unique<MMIXObjectWriter>(std::move(MOTW), OS);
}
} // namespace llvm