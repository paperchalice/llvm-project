add_llvm_component_library(LLVMMMIXDesc
  MMIXInstPrinter.cpp
  MMIXALStreamer.cpp
  MMIXAsmBackend.cpp
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
  Option
  Support
  TargetParser

  ADD_TO_COMPONENT
  MMIX
)

add_dependencies(LLVMMMIXDesc MMIXCommonTableGen)
