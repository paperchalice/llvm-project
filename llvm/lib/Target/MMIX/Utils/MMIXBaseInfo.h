//===- MMIXaseInfo.h - Helper methods for MMIX ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_UTILS_MMIXBASEINFO_H
#define LLVM_LIB_TARGET_MMIX_UTILS_MMIXBASEINFO_H

#include "llvm/MC/MCRegister.h"

namespace llvm::MMIX {

MCRegister getSPRFromEnc(unsigned RegEnc);

}

#endif // LLVM_LIB_TARGET_MMIX_UTILS_MMIXBASEINFO_H
