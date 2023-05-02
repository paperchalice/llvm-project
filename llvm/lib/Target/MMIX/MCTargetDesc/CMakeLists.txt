add_llvm_component_library (LLVMMMIXDesc
  MMIXAsmBackend.cpp
  MMIXELFStreamer.cpp
  MMIXMMOStreamer.cpp
  MMIXMCExpr.cpp
  MMIXMCInstrInfo.cpp
  MMIXMCTargetDesc.cpp
  MMIXMCAsmInfo.cpp
  MMIXMCInstrAnalysis.cpp
  MMIXInstPrinter.cpp
  MMIXMCCodeEmitter.cpp
  MMIXTargetStreamer.cpp
  MMIXObjectWriter.cpp

  LINK_COMPONENTS
  MC
  MCDisassembler
  Support
  TargetParser
  MMIXInfo

  ADD_TO_COMPONENT
  MMIX
)
