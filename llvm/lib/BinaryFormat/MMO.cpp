#include "llvm/BinaryFormat/MMO.h"
#include "llvm/Support/Endian.h"

using namespace llvm;
using namespace llvm::support::endian;
using namespace llvm::MMO;

namespace {
inline const char *ToCharP(const unsigned char *p) {
  return reinterpret_cast<const char *>(p);
}
} // namespace

Quote::Quote(const std::uint8_t *&Iter)
    : Value(ArrayRef<uint8_t>(Iter + 4, 4)) {
  Iter += 8;
}

Loc::Loc(const std::uint8_t *&Iter, Error &E) : HighByte(Iter[2]) {
  uint8_t TetraCount = Iter[3];
  Iter += 4;
  switch (TetraCount) {
  case 1:
    Offset = read32be(Iter);
    Iter += 4;
    break;
  case 2:
    Offset = read64be(Iter);
    Iter += 8;
    break;
  default:
    E = createStringError(std::errc::invalid_argument, "Z field of lop_loc must be 1 or 2!");
    break;
  }
}

Skip::Skip(const std::uint8_t *&Iter) : Delta(read16be(Iter + 2)) {
  Iter += 4;
}

Fixo::Fixo(const std::uint8_t *&Iter, Error &E)
    : HighByte(Iter[2]) {
  uint8_t TetraCount = Iter[3];
  Iter += 4;
  switch (TetraCount) {
  case 1:
    Offset = read32be(Iter);
    Iter += 4;
    break;
  case 2:
    Offset = read64be(Iter);
    Iter += 8;
    break;
  default:
    E = createStringError(std::errc::invalid_argument, "`Z` field of `lop_fixo` must be 1 or 2!");
    break;
  }
}

Fixr::Fixr(const std::uint8_t *&Iter) : Delta(read16be(Iter + 2)) {
  Iter += 4;
}

Fixrx::Fixrx(const std::uint8_t *&Iter, Error &E) : Z(Iter[3]) {
  if (!(Z == 16 || Z == 24))
    E = createStringError(std::errc::invalid_argument, "Z field in lop_fixrx must be 16 or 24!");
  Iter += 4;
  Delta = read64be(Iter);
  Iter += 8;
}

File::File(const std::uint8_t *&Iter) : Number(Iter[3]) {
  uint8_t TetraCount = Iter[3];
  Iter += 4;
  if (TetraCount != 0) {
    Name = StringRef(ToCharP(Iter), 4 * TetraCount);
  }
  Iter += 4 * TetraCount;
}

Line::Line(const std::uint8_t *&Iter) : Number(read16be(Iter + 2)) {
  Iter += 4;
}

Spec::Spec(const std::uint8_t *&Iter) : Type(read16be(Iter + 2)) {
  Iter += 4;
}

void SymNode::computeMasterByte(int &M) {
  if (IsRegister) {
    M += 0xF;
  } else {
    std::uint32_t X = 0;
    if ((Equiv & (0xFFFFUL << (32 + 16))) == (0x20000000UL << 32)) {
      M += 8;
      X = ((Equiv - (0x20000000UL << 32)) >> 32);
    } else {
      X = (Equiv >> 32);
    }
    if (X)
      M += 4;
    else
      X = Equiv & 0xFFFF'FFFF;
    int j = 1;
    for (; j < 4; ++j) {
      if (X < static_cast<uint32_t>(1 << (8 * j)))
        break;
    }
    M += j;
  }
}

void SymNode::writeBinEquiv(raw_ostream &OS, int &M) {
  if (M == 0xF)
    M = 1;
  else if (M > 8)
    M -= 8;
  for (; M != 0; --M) {
    if (M > 4) {
      OS << static_cast<std::uint8_t>(((Equiv >> 32) >> (8 * (M - 5))) & 0xFF);
    } else {
      OS << static_cast<std::uint8_t>(
          static_cast<std::uint32_t>((Equiv & 0xFFFF'FFFF)) >> (8 * (M - 1)) &
          0xFF);
    }
  }
}

void SymNode::writeBinSerial(raw_ostream &OS, int &M) {
  for (M = 0; M < 4; ++M) {
    if (Serial < static_cast<std::uint32_t>(1 << (7 * (M + 1))))
      break;
  }
  for (; M >= 0; --M) {
    OS << static_cast<std::uint8_t>((Serial >> (7 * (M)) & 0x7F) +
                                    (M ? 0 : 0x80));
  }
}
