add_llvm_component_group(MMIX)

set(LLVM_TARGET_DEFINITIONS MMIX.td)
tablegen(LLVM MMIXGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM MMIXGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM MMIXGenCallingConv.inc -gen-callingconv)
tablegen(LLVM MMIXGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM MMIXGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM MMIXGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM MMIXGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM MMIXGenSubtargetInfo.inc -gen-subtarget)
add_public_tablegen_target(MMIXCommonTableGen)

add_llvm_target(MMIXCodeGen
  MMIXCallingConvention.cpp
  MMIXCallLowering.cpp
  MMIXFrameLowering.cpp
  MMIXInstrInfo.cpp
  MMIXRegisterInfo.cpp
  MMIXSubtarget.cpp
  MMIXTargetLowering.cpp
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

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
add_subdirectory(Utils)
