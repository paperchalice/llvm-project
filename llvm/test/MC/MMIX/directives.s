# RUN: llvm-mc -triple=mmix %s | FileCheck %s

# CHECK: .short 0
.wyde 0

# CHECK: .long 0
.tetra 0

# CHECK: .org 8, 0
.LOC 8
