if(MMIX IN_LIST LLVM_TARGETS_TO_BUILD)
  add_subdirectory(llvm-mmixal)
  add_subdirectory(llvm-mmotype)
endif()
