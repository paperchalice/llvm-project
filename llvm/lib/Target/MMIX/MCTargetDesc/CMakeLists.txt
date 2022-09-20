add_library(LLVMMMIXMCTargetDesc ALIAS LLVMMMIX)

target_sources (
  LLVMMMIX PRIVATE
  MMIXAsmBackend.cpp
  MMIXMCInstrInfo.cpp
  MMIXMCTargetDesc.cpp
  MMIXMCAsmInfo.cpp
  MMIXMCInstrAnalysis.cpp
  MMIXInstPrinter.cpp
  MMIXMCCodeEmitter.cpp
  MMIXTargetStreamer.cpp
  MMIXObjectWriter.cpp
)
