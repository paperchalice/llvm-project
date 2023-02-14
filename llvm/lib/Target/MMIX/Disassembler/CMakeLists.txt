add_llvm_component_library (LLVMMMIXDisassembler
  MMIXDisassembler.cpp

  LINK_COMPONENTS
  MCDisassembler
  MMIXDesc
  MMIXInfo
  Support

  ADD_TO_COMPONENT
  MMIX
)
