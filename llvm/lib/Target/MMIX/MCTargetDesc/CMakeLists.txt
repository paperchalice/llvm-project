add_llvm_component_library(LLVMMMIXDesc
  MMIXInstPrinter.cpp
  MMIXMCAsmInfo.cpp
  MMIXMCTargetDesc.cpp

  LINK_COMPONENTS
  CodeGenTypes
  Core
  MC
  MMIXInfo
  Support
  TargetParser

  ADD_TO_COMPONENT
  MMIX
)
