if(NOT "MMIX" IN_LIST LLVM_TARGETS_TO_BUILD)
  return()
endif()

option(LLVM_INSTALL_MMIXWARE_SYMLINKS
  "Install symlinks from the mmixware tool names to the corresponding LLVM tools." OFF)

add_subdirectory(llvm-mmixal)
