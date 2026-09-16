set(LLVM_LINK_COMPONENTS
  MC
  MCParser
  MMIXAsmParser
  MMIXDesc
  MMIXInfo
  Support
  TargetParser
)

add_llvm_tool(llvm-mmixal
  llvm-mmixal.cpp

  DEPENDS
  GENERATE_DRIVER
)

target_include_directories(llvm-mmixal
  PRIVATE
  ${LLVM_MAIN_SRC_DIR}/lib/Target/MMIX
)

if(LLVM_INSTALL_BINUTILS_SYMLINKS)
  add_llvm_tool_symlink(mmixal llvm-mmixal)
endif()
