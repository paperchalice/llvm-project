if ("MMIX" IN_LIST LLVM_TARGETS_TO_BUILD)
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

  add_llvm_tool(llvm-mmixal
    MMIXALLexer.cpp
    MMIXALParser.cpp
    MMIXMCAsmInfoMMIXAL.cpp
    llvm-mmixal.cpp
  )
endif()


