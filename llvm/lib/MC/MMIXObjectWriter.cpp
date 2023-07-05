#include "llvm/BinaryFormat/MMO.h"
#include "llvm/MC/MCAsmLayout.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCMMIXObjectWriter.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCSymbolMMO.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Object/MMIXObjectFile.h"
#include "llvm/Support/Alignment.h"
#include "llvm/Support/Casting.h"
#include <ctime>

namespace {

using namespace llvm;

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
    Asm.writeSectionData(W.OS, &Sec, Layout);
  }

  writeSymbolTable(Asm);
  return Cnt;
}

void MMIXObjectWriter::writeSymbolTable(MCAssembler &Asm) {
  W.write(ArrayRef("\x98\x0b\x00\x00", 4));
  auto SymbolTable = Asm.symbols();
  for (auto &Symbol : SymbolTable) {
    auto MMOSymbol = cast<MCSymbolMMO>(&Symbol);
    if(MMOSymbol->getIndex() == 0) {
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