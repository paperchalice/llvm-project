#include "MMODump.h"

#include "llvm/Object/MMIXObjectFile.h"

using namespace llvm;
using namespace llvm::object;
using namespace llvm::objdump;

MMODumper::MMODumper(const object::MMIXObjectFile &O) : Dumper(O), Obj(O) {}

std::unique_ptr<Dumper> objdump::createMMODumper(const MMIXObjectFile &Obj) {
  return std::make_unique<MMODumper>(Obj);
}
