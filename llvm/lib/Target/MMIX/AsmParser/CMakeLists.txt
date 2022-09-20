add_library(LLVMMMIXAsmParser ALIAS LLVMMMIX)

target_sources (
  LLVMMMIX PRIVATE
  MMIXAsmParser.cpp
  MMIXOperand.cpp
)
