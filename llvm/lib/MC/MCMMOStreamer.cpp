#include "llvm/MC/MCObjectStreamer.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

namespace {

class MMOStreamer : public MCObjectStreamer {
  
};

}

namespace llvm {
MCStreamer *createMMOStreamer(MCContext &Ctx,
                              std::unique_ptr<MCAsmBackend> &&TAB,
                              std::unique_ptr<MCObjectWriter> &&OW,
                              std::unique_ptr<MCCodeEmitter> &&CE) {
  return nullptr;
}

} // namespace llvm