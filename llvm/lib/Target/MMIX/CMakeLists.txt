add_llvm_component_group(MMIX)

block()
set(LLVM_TARGET_DEFINITIONS MMIX.td)
tablegen(LLVM MMIXGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM MMIXGenALMatcher.inc -gen-asm-matcher -asmparsernum=1)
tablegen(LLVM MMIXGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM MMIXGenCallingConv.inc -gen-callingconv)
tablegen(LLVM MMIXGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM MMIXGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM MMIXGenMCPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM MMIXGenRegisterBankInfo.inc -gen-register-bank)
tablegen(LLVM MMIXGenGlobalISel.inc -gen-global-isel)
tablegen(LLVM MMIXGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM MMIXGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM MMIXGenSubtargetInfo.inc -gen-subtarget)

set(LLVM_TARGET_DEFINITIONS MMIXOptions.td)
tablegen(LLVM MMIXOptions.inc -gen-opt-parser-defs)

set(LLVM_TARGET_DEFINITIONS MCTargetDesc/MMIXMCOptions.td)
tablegen(LLVM MMIXMCOptions.inc -gen-opt-parser-defs)

add_public_tablegen_target(MMIXCommonTableGen)
endblock()

add_llvm_target(MMIXCodeGen
  GISel/MMIXCallLowering.cpp
  GISel/MMIXInstructionSelector.cpp
  GISel/MMIXLegalizerInfo.cpp
  GISel/MMIXRegisterBankInfo.cpp
  MMIXAsmPrinter.cpp
  MMIXCallingConvention.cpp
  MMIXCodeGenPassBuilder.cpp
  MMIXFrameLowering.cpp
  MMIXInstrInfo.cpp
  MMIXMCInstLower.cpp
  MMIXRegisterInfo.cpp
  MMIXSubtarget.cpp
  MMIXTargetLowering.cpp
  MMIXTargetMachine.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  CodeGenTypes
  Core
  GlobalISel
  MC
  Option
  Passes
  SelectionDAG
  Support
  Target
  TargetParser

  MMIXDesc
  MMIXInfo

  ADD_TO_COMPONENT
  MMIX
)

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
add_subdirectory(Utils)
