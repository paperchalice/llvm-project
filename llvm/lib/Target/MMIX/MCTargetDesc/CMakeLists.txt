add_llvm_component_library(LLVMMMIXDesc
  MMIXInstPrinter.cpp
  MMIXAsmBackend.cpp
  MMIXAsmStreamer.cpp
  MMIXELFStreamer.cpp
  MMIXMCAsmInfo.cpp
  MMIXMCCodeEmitter.cpp
  MMIXMCExpr.cpp
  MMIXObjectWriter.cpp
  MMIXMCTargetDesc.cpp
  MMIXTargetStreamer.cpp

  LINK_COMPONENTS
  CodeGenTypes
  Core
  MC
  MMIXInfo
  MMIXUtils
  Support
  TargetParser

  ADD_TO_COMPONENT
  MMIX
)
