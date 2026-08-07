; Smoke test that the MMIX target is registered with and selectable by llc.

; RUN: llc --version | FileCheck %s

; CHECK: Registered Targets:
; CHECK: mmix{{.*}}- A RISC Computer for the Third Millennium
