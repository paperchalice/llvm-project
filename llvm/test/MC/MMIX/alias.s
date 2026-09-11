% RUN: llvm-mc -triple=mmix %s --show-encoding 2>&1 | FileCheck %s

% CHECK: encoding: [0x20,0x00,0x00,0x00]
  ADD $0,$0,$0

% CHECK: encoding: [0x21,0x00,0x00,0x01]
  ADD $0,$0,1

% CHECK: encoding: [0x21,0x00,0x00,0x01]
  ADDI $0,$0,1

% CHECK: encoding: [0x00,0x00,0x07,0x01]
  TRAP 0,Fputs,StdOut

% CHECK: encoding: [0x00,0x00,0x07,0x01]
  TRAP 0,7,1

% CHECK: SET $0,$1
  OR $0,$1,0
% CHECK: SET $0,1
  SET $0,1

% CHECK: LDB $0,$0
  LDB $0,$0,0
% CHECK: LDB $0,$0,1
  LDB $0,$0,1

% CHECK: NEG $0,1
  NEG $0,0,1
% CHECK: NEG $0,$1
  NEG $0,0,$1
