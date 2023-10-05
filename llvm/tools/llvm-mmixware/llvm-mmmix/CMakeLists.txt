set(LLVM_LINK_COMPONENTS
  Object
  Option
  Support
  )

add_llvm_tool(llvm-mmmix
  llvm-mmmix.cpp
  )