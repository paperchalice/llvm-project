# RUN: llvm-mc -triple=mmix -show-inst-operands %s 2>&1 | FileCheck %s

Main:
# CHECK: parsed instruction: [jmp, Main]
  JMP Main

# CHECK: parsed instruction: [add, $0, $0, $0]
  ADD $0,$0,$0

# CHECK: parsed instruction: [bn, $0, Main]
  BN $0,Main

# CHECK: parsed instruction: [bn, $1, Main]
  BN $0+1,Main

# CHECK: parsed instruction: [jmp, 8]
  JMP 8
