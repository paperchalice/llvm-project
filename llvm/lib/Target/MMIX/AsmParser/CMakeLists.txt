add_llvm_component_library (LLVMMMIXAsmParser
  MMIXAsmParser.cpp
  MMIXALTargetParser.cpp
  MMIXALOperand.cpp
  MMIXOperand.cpp

  LINK_COMPONENTS
  MC
  MCParser
  MMIXCodeGen
  MMIXDesc
  MMIXInfo
  Support

  ADD_TO_COMPONENT
  MMIX
)
