//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

include "llvm/Option/LibraryOptions.td"

def MMIXOptions : OptionsStruct<prefix = "mmix-", perContext = false>;

defm : ValueField<"local-threshold", "unsigned", "32",
  "Initial value of rL local threshold register">;
defm : ValueField<"return-threshold", "unsigned", "2",
  "Number of registers to store return value">;
defm : ValueField<"param-threshold", "unsigned", "16",
  "Number of registers to store function parameters">;
