add_llvm_component_group(MMIX)

set(LLVM_TARGET_DEFINITIONS MMIX.td)

tablegen(LLVM MMIXGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM MMIXALGenAsmMatcher.inc -gen-asm-matcher -D GEN_MMIXAL)
tablegen(LLVM MMIXGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM MMIXGenCallingConv.inc -gen-callingconv)
tablegen(LLVM MMIXGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM MMIXGenGlobalISel.inc -gen-global-isel)
tablegen(LLVM MMIXGenGICombiner.inc -gen-global-isel-combiner -combiners="MMIXCombinerHelper")
tablegen(LLVM MMIXGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM MMIXGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM MMIXGenMCPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM MMIXGenRegisterBank.inc -gen-register-bank)
tablegen(LLVM MMIXGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM MMIXGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM MMIXGenSystemOperands.inc -gen-searchable-tables)

add_public_tablegen_target(MMIXCommonTableGen)

add_llvm_target(MMIXCodeGen
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

  GlobalISel/MMIXCallLowering.cpp
  GlobalISel/MMIXCombiner.cpp
  GlobalISel/MMIXInstructionSelector.cpp
  GlobalISel/MMIXLegalizerInfo.cpp
  GlobalISel/MMIXRegisterBankInfo.cpp
  GlobalISel/MMIXValueHandler.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  CodeGenTypes
  Core
  GlobalISel
  IRPrinter
  MC
  SelectionDAG
  Support
  Target
  TransformUtils
  MMIXDesc
  MMIXInfo

  ADD_TO_COMPONENT
  MMIX
)

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
