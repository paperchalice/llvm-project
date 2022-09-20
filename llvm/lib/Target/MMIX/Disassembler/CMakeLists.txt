add_library(LLVMMMIXDisassembler ALIAS LLVMMMIX)

target_sources (
  LLVMMMIX PRIVATE
  MMIXDisassembler.cpp
)
