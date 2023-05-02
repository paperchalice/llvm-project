#ifndef LLVM_MC_MCMMIXOBJECTWRITER_H
#define LLVM_MC_MCMMIXOBJECTWRITER_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/StringTableBuilder.h"
#include "llvm/Support/EndianStream.h"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace llvm {

class MCMMIXObjectTargetWriter : public MCObjectTargetWriter {
public:
  static bool classof(const MCObjectTargetWriter *W) {
    return W->getFormat() == Triple::MMO;
  }

public:
  Triple::ObjectFormatType getFormat() const override { return Triple::MMO; }

protected:
private:
};

/// Construct a new MMIX Object writer instance.
///
/// This routine takes ownership of the target writer subclass.
///
/// \param MOTW - The target specific MMO writer subclass.
/// \param OS - The stream to write to.
/// \returns The constructed object writer.
std::unique_ptr<MCObjectWriter>
createMMIXObjectWriter(std::unique_ptr<MCMMIXObjectTargetWriter> MOTW,
                       raw_pwrite_stream &OS);

} // namespace llvm
#endif // LLVM_MC_MCMMIXOBJECTWRITER_H
