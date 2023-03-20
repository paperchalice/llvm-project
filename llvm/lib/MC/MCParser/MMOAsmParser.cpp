#include "llvm/MC/MCParser/MCAsmParserExtension.h"

namespace llvm {

namespace {
class MMOAsmParser : public MCAsmParserExtension {

public:
  MMOAsmParser() = default;
};

} // namespace

MCAsmParserExtension *createMMOAsmParser() { return new MMOAsmParser; }

} // namespace llvm
