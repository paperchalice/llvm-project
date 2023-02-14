//===- MMIXObjectFileTest.cpp - Tests for MMIXObjectFile
//-------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Object/MMIXObjectFile.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Testing/Support/Error.h"
#include "gtest/gtest.h"

using namespace llvm;
using namespace llvm::object;

namespace {
char Data[] = {
    '\x98', '\x09', '\x01', '\x01', '\x36', '\xf4', '\xa3', '\x63', '\x98',
    '\x01', '\x20', '\x01', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
    '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x61', '\x62', '\x00',
    '\x00', '\x98', '\x01', '\x00', '\x02', '\x00', '\x00', '\x00', '\x01',
    '\x23', '\x45', '\x67', '\x8c', '\x98', '\x06', '\x00', '\x02', '\x74',
    '\x65', '\x73', '\x74', '\x2e', '\x6d', '\x6d', '\x73', '\x98', '\x07',
    '\x00', '\x07', '\xf0', '\x00', '\x00', '\x00', '\x98', '\x02', '\x40',
    '\x00', '\x98', '\x07', '\x00', '\x09', '\x81', '\x03', '\xfe', '\x01',
    '\x42', '\x03', '\x00', '\x00', '\x98', '\x07', '\x00', '\x0a', '\x00',
    '\x00', '\x00', '\x00', '\x98', '\x01', '\x00', '\x02', '\x00', '\x00',
    '\x00', '\x01', '\x23', '\x45', '\xa7', '\x68', '\x98', '\x05', '\x00',
    '\x10', '\x01', '\x00', '\xff', '\xf5', '\x98', '\x04', '\x0f', '\xf7',
    '\x98', '\x03', '\x20', '\x01', '\x00', '\x00', '\x00', '\x00', '\x98',
    '\x06', '\x01', '\x02', '\x66', '\x6f', '\x6f', '\x2e', '\x6d', '\x6d',
    '\x73', '\x00', '\x98', '\x07', '\x00', '\x04', '\xf0', '\x00', '\x00',
    '\x0a', '\x98', '\x08', '\x00', '\x05', '\x00', '\x00', '\x02', '\x00',
    '\x00', '\xfe', '\x00', '\x00', '\x98', '\x01', '\x20', '\x01', '\x00',
    '\x00', '\x00', '\x0a', '\x00', '\x00', '\x63', '\x64', '\x98', '\x00',
    '\x00', '\x01', '\x98', '\x00', '\x00', '\x00', '\x98', '\x0a', '\x00',
    '\xfe', '\x20', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x08',
    '\x00', '\x00', '\x00', '\x01', '\x23', '\x45', '\x67', '\x8c', '\x98',
    '\x0b', '\x00', '\x00', '\x20', '\x3a', '\x50', '\x40', '\x50', '\x40',
    '\x40', '\x20', '\x41', '\x20', '\x42', '\x20', '\x43', '\x09', '\x44',
    '\x08', '\x83', '\x40', '\x40', '\x20', '\x4d', '\x20', '\x61', '\x20',
    '\x69', '\x05', '\x6e', '\x01', '\x23', '\x45', '\x67', '\x8c', '\x81',
    '\x40', '\x0f', '\x61', '\xfe', '\x82', '\x00', '\x00', '\x98', '\x0c',
    '\x00', '\x0a',
};

constexpr auto DataSize = sizeof(Data);

// MMO is 4 bytes aligned
static_assert(DataSize % 4 == 0);
MemoryBufferRef SampleMMO(StringRef(Data, DataSize), "sampleMMO");
} // namespace

TEST(MMIXObjectFileTest, MMORead) {
  Expected<std::unique_ptr<MMIXObjectFile>> MMIXObjOrErr =
      object::ObjectFile::createMMIXObjectFile(SampleMMO);
  ASSERT_THAT_EXPECTED(MMIXObjOrErr, Succeeded());
  std::unique_ptr<MMIXObjectFile> Obj = std::move(*MMIXObjOrErr);
}
