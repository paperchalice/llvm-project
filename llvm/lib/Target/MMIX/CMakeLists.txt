add_llvm_component_group(MMIX)

set(LLVM_TARGET_DEFINITIONS MMIX.td)

tablegen(LLVM MMIXGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM MMIXGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM MMIXGenCallingConv.inc -gen-callingconv)
tablegen(LLVM MMIXGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM MMIXGenGlobalISel.inc -gen-global-isel)
tablegen(LLVM MMIXGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM MMIXGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM MMIXGenMCPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM MMIXGenRegisterBank.inc -gen-register-bank)
tablegen(LLVM MMIXGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM MMIXGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM MMIXGenSystemOperands.inc -gen-searchable-tables)

add_public_tablegen_target(MMIXCommonTableGen)

add_llvm_target(MMIX
  MMIXAsmPrinter.cpp
  MMIXCallingConv.cpp
  MMIXCodeGenPassBuilder.cpp
  MMIXFrameLowering.cpp
  MMIXInstrInfo.cpp
  MMIXPassConfig.cpp
  MMIXRegisterInfo.cpp
  MMIXSubtarget.cpp
  MMIXTargetLowering.cpp
  MMIXTargetMachine.cpp
  MMIXTargetObjectFile.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  Core
  CodeGen
  GlobalISel
  MC
  MCDisassembler
  MCParser
  ScalarOpts
  SelectionDAG
  Support
  Target
  TransformUtils

  ADD_TO_COMPONENT
  MMIX
  )
add_library(LLVMMMIXCodeGen ALIAS LLVMMMIX)

add_subdirectory(GlobalISel)
add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
