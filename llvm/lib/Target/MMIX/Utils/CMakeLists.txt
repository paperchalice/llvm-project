add_llvm_component_library(LLVMMMIXUtils
  MMIXBaseInfo.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  MMIX
)
