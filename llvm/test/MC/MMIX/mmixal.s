% RUN: llvm-mc -triple=mmix %s --show-encoding --mmix-print-mmixal --print-imm-hex | FileCheck %s

% CHECK: 2ADDU   $0,$0,#0
  ADD2U $0,$0,0
% CHECK: 4ADDU   $0,$0,#0
  ADD4U $0,$0,0
% CHECK: 8ADDU   $0,$0,#0
  ADD8U $0,$0,0
% CHECK: 16ADDU   $0,$0,#f
  ADD16U $0,$0,15
