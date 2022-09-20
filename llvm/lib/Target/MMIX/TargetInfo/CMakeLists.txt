add_library(LLVMMMIXInfo ALIAS LLVMMMIX)

target_sources (
  LLVMMMIX PRIVATE
  MMIXTargetInfo.cpp
)
