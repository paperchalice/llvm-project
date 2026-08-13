add_llvm_component_library(LLVMMMIXAsmParser
  MMIXAsmParser.cpp
  MMIXOperand.cpp

  LINK_COMPONENTS
  MMIXDesc
  MMIXInfo
  MMIXUtils
  CodeGenTypes
  MC
  MCParser
  Support
  TargetParser

  ADD_TO_COMPONENT
  MMIX
)

target_include_directories(LLVMMMIXAsmParser
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/..
)
