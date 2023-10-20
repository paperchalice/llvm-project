set(LLVM_LINK_COMPONENTS
  Object
  Option
  Support
  )

add_llvm_tool(llvm-mmix
  RegisterStack.cpp
  Simulator.cpp
  Treap.cpp
  llvm-mmix.cpp
  )

set(LLVM_TARGET_DEFINITIONS MMIXConstant.td)
tablegen(LLVM MMIXConstant.inc --gen-searchable-tables)
add_public_tablegen_target(MMIXConstant)

target_include_directories(llvm-mmix
  PRIVATE
  ${LLVM_SOURCE_DIR}/lib/Target/MMIX
  ${LLVM_BINARY_DIR}/lib/Target/MMIX
)
