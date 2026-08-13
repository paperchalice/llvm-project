add_llvm_component_library (LLVMMMIXDisassembler
  MMIXDisassembler.cpp

  LINK_COMPONENTS
  MC
  MCDisassembler
  MMIXDesc
  MMIXInfo
  MMIXUtils
  Support

  ADD_TO_COMPONENT
  MMIX
)
