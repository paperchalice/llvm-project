; RUN: llc -mtriple=x86_64-linux-gnu %s -o - | llc --passes='print<machine-loops>' -o - | FileCheck
