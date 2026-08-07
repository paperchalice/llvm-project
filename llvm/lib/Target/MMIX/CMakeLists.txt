add_llvm_component_group(MMIX)

set(LLVM_TARGET_DEFINITIONS MMIX.td)
tablegen(LLVM MMIXGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM MMIXGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM MMIXGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM MMIXGenSubtargetInfo.inc -gen-subtarget)
add_public_tablegen_target(MMIXCommonTableGen)

add_llvm_target(MMIXCodeGen
  MMIXFrameLowering.cpp
  MMIXInstrInfo.cpp
  MMIXRegisterInfo.cpp
  MMIXSubtarget.cpp
  MMIXTargetMachine.cpp

  LINK_COMPONENTS
  Analysis
  CodeGen
  CodeGenTypes
  Core
  MC
  SelectionDAG
  Support
  Target
  TargetParser

  MMIXDesc
  MMIXInfo
)

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
