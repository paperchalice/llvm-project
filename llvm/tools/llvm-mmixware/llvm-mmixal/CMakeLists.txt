set(LLVM_LINK_COMPONENTS
  AllTargetsAsmParsers
  AllTargetsDescs
  AllTargetsDisassemblers
  AllTargetsInfos
  MC
  MCParser
  Option
  Support
  TargetParser
)

set(LLVM_TARGET_DEFINITIONS MMIXALSearchableTable.td)
tablegen(LLVM MMIXALSearchableTable.inc --gen-searchable-tables)
add_public_tablegen_target(MMIXALSearchableTable)

add_llvm_tool(llvm-mmixal
  MMIXALLexer.cpp
  MMIXALParser.cpp
  MMIXMCAsmInfoMMIXAL.cpp
  llvm-mmixal.cpp

  DEPENDS
  MMIXALSearchableTable
)
