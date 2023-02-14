add_llvm_component_library (LLVMMMIXAsmParser
  MMIXAsmParser.cpp
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
