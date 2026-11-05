add_llvm_component_group(MMIX)

block()
set(LLVM_TARGET_DEFINITIONS MMIX.td)
llvm_tablegen(MMIXGenAsmMatcher.inc -gen-asm-matcher)
llvm_tablegen(MMIXGenALMatcher.inc -gen-asm-matcher -asmparsernum=1)
llvm_tablegen(MMIXGenAsmWriter.inc -gen-asm-writer)
llvm_tablegen(MMIXGenCallingConv.inc -gen-callingconv)
llvm_tablegen(MMIXGenInstrInfo.inc -gen-instr-info)
llvm_tablegen(MMIXGenMCCodeEmitter.inc -gen-emitter)
llvm_tablegen(MMIXGenMCPseudoLowering.inc -gen-pseudo-lowering)
llvm_tablegen(MMIXGenRegisterBankInfo.inc -gen-register-bank)
llvm_tablegen(MMIXGenGlobalISel.inc -gen-global-isel)
llvm_tablegen(MMIXGenRegisterInfo.inc -gen-register-info)
llvm_tablegen(MMIXGenDisassemblerTables.inc -gen-disassembler)
llvm_tablegen(MMIXGenSubtargetInfo.inc -gen-subtarget)

set(LLVM_TARGET_DEFINITIONS MMIXOptions.td)
llvm_tablegen(MMIXOptions.inc -gen-opt-parser-defs)

set(LLVM_TARGET_DEFINITIONS MCTargetDesc/MMIXMCOptions.td)
llvm_tablegen(MMIXMCOptions.inc -gen-opt-parser-defs)

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
