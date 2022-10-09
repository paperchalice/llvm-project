set(LLVM_LINK_COMPONENTS
  Object
  Option
  Support
  Symbolize
  )

add_llvm_tool(llvm-mmotype
  llvm-mmotype.cpp
  )
