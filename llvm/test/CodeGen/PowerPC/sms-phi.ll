; REQUIRES: asserts
; RUN: llc < %s -mtriple=powerpc64le-unknown-linux-gnu -verify-machineinstrs\
; RUN:       -mcpu=pwr9 --ppc-enable-pipeliner -debug-only=pipeliner 2>&1 \
; RUN:       >/dev/null | FileCheck %s
define dso_local void @sha512(ptr %p) #0 {
;CHECK: prolog:
;CHECK:        %{{[0-9]+}}:g8rc = ADD8 %{{[0-9]+}}:g8rc, %{{[0-9]+}}:g8rc
;CHECK: epilog:
;CHECK:        %{{[0-9]+}}:g8rc_and_g8rc_nox0 = PHI %{{[0-9]+}}:g8rc_and_g8rc_nox0, %bb.3, %{{[0-9]+}}:g8rc_and_g8rc_nox0, %bb.4
;CHECK-NEXT:   %{{[0-9]+}}:g8rc = PHI %{{[0-9]+}}:g8rc, %bb.3, %{{[0-9]+}}:g8rc, %bb.4
;CHECK-NEXT:   %{{[0-9]+}}:g8rc = PHI %{{[0-9]+}}:g8rc, %bb.3, %{{[0-9]+}}:g8rc, %bb.4
  br label %1

1:                                                ; preds = %1, %0
  %2 = phi i64 [ 0, %0 ], [ %12, %1 ]
  %3 = phi i64 [ undef, %0 ], [ %11, %1 ]
  %4 = phi i64 [ undef, %0 ], [ %3, %1 ]
  %5 = getelementptr inbounds [80 x i64], ptr %p, i64 0, i64 %2
  %6 = load i64, ptr %5, align 8
  %7 = add i64 0, %6
  %8 = and i64 %3, %4
  %9 = or i64 0, %8
  %10 = add i64 0, %9
  %11 = add i64 %10, %7
  %12 = add nuw nsw i64 %2, 1
  %13 = icmp eq i64 %12, 80
  br i1 %13, label %14, label %1

14:                                               ; preds = %1
  %15 = add i64 %4, 0
  store i64 %15, ptr undef, align 8
  ret void
}

