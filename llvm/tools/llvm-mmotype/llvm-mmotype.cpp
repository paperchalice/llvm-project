#include "llvm/Object/Binary.h"
#include "llvm/Object/MMIXObjectFile.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/raw_ostream.h"
#include <iomanip>
#include <sstream>
#include <string>
using namespace llvm;
using namespace std;
using namespace llvm::object;
using namespace support::endian;

namespace {
cl::opt<bool> Lst("s", cl::desc("List only the symbol table"));
cl::opt<bool> Verbose("v", cl::desc("List tetrabytes of input"));
cl::opt<std::string> InputFilename(cl::Positional, cl::desc("<input file>"),
                                   cl::init("-"));

uint8_t getX(const uint8_t *A) { return A[1]; }
uint8_t getY(const uint8_t *A) { return A[2]; }
uint8_t getZ(const uint8_t *A) { return A[3]; }
uint16_t getYZ(const uint8_t *A) { return read16be(A + 2); }

class MMOType {
  const MMIXObjectFile &Obj;
  uint64_t CurLoc = 0;
  size_t TetraCnt = 0;
  int ListedFile = -1;
  int CurFile = -1;
  int CurLine = 0;
  bool IsQuote = false;
  vector<StringRef> FileNames;
  uint64_t Tmp;
  const uint8_t *Iter;

  uint32_t outTetra() {
    auto Ret = read32be(Iter);
    if (Verbose) {
      outs() << "  " << format_hex_no_prefix(Ret, 8) << "\n";
    }
    Iter += 4;
    ++TetraCnt;
    return Ret;
  }

  template <typename T>
  void outAddressContent(uint64_t Loc, T Data, bool WithNewLine = true) {
    if (!Lst) {
      outs() << formatv("{0}: {1}", format_hex_no_prefix(Loc, 16),
                        format_hex_no_prefix(Data, 2 * sizeof(T)));
      if (WithNewLine)
        outs() << "\n";
    }
  }

  void outExtra() {
    if (!Lst) {
      if (CurLine == 0) {
        outs() << "\n";
      } else if (CurLoc & (0xEUL << 60)) {
        outs() << "\n";
      } else {
        if (CurFile == ListedFile) {
          outs() << formatv(" (line {0})\n", CurLine);
        } else {
          outs() << formatv(" (\"{0}\", line {1})\n", FileNames[CurFile],
                            CurLine);
          ListedFile = CurFile;
        }
      }
    }
    CurLoc += 4;
    CurLoc &= -4; // align current loc with 4
    ++CurLine;
    if (IsQuote)
      IsQuote = false;
  }

  void listPreamble() {
    uint8_t Z = getZ(Iter);
    outTetra();
    if (Z) {
      if (!Lst) {
        time_t Time = outTetra();
        stringstream Ss;
        Ss << put_time(localtime(&Time), "%c");
        outs() << "File was created " << Ss.str() << '\n';
      }
    }
    for (int I = 1; I < Z; ++I) {
      outTetra();
    }
  }

  void listStab() {
    outs() << formatv("Symbol table (beginning at tetra {0}):\n", TetraCnt);
    auto STabVec = Obj.getSTab();
    std::sort(STabVec.begin(), STabVec.end(),
              [](const MMO::Symbol &S1, const MMO::Symbol &S2) {
                return S1.PrintPos < S2.PrintPos;
              });
    for (const auto &S : STabVec) {
      while (Iter < S.PrintPos) {
        outTetra();
      }
      if (S.Type == MMO::REGISTER) {
        outs() << formatv("    {0} = ${1} ({2})\n",
                          StringRef(S.Name).drop_front(), S.Equiv, S.Serial);
      } else {
        outs() << formatv("    {0} = #{1} ({2})\n",
                          StringRef(S.Name).drop_front(),
                          format_hex_no_prefix(S.Equiv, 1), S.Serial);
      }
    }
    assert(Iter[0] == MMO::MM && Iter[1] == MMO::LOP_END);
    if (Verbose) {
      outs() << "  " << format_hex_no_prefix(read32be(Iter), 8) << "\n";
      outs() << "Symbol table ends at tetra " << TetraCnt + 1 << ".\n";
    }
  }

  void listContent() {
    auto End = Obj.getData().bytes_end();
    bool Stop = false;
    while (Iter != End && !Stop) {
      if (Iter[0] == MMO::MM && !IsQuote) {
        switch (getX(Iter)) {
        case MMO::LOP_QUOTE:
          outTetra();
          IsQuote = true;
          break;
        case MMO::LOP_LOC:
          CurLoc = static_cast<uint64_t>(Iter[2]) << 56;
          if (getZ(Iter) == 1) {
            outTetra();
            CurLoc += read32be(Iter);
            outTetra();
          } else if (getZ(Iter) == 2) {
            outTetra();
            CurLoc += read64be(Iter);
            outTetra();
            outTetra();
          }
          break;
        case MMO::LOP_SKIP:
          CurLoc += getYZ(Iter);
          outTetra();
          break;
        case MMO::LOP_FIXO: {
          uint64_t Y = getY(Iter);
          switch (getZ(Iter)) {
          case 1:
            outTetra();
            Tmp = (Y << 56) + outTetra();
            break;
          case 2:
            outTetra();
            Tmp = (Y << 56) + read64be(Iter);
            outTetra();
            outTetra();
            break;
          default:
            break;
          }
        }
          outAddressContent(Tmp, CurLoc);
          break;
        case MMO::LOP_FIXR: {
          uint32_t Delta = getYZ(Iter);
          outTetra();
          Tmp = CurLoc - 4 * Delta;
          outAddressContent(Tmp, Delta);
        } break;
        case MMO::LOP_FIXRX: {
          uint8_t Z = getZ(Iter);
          outTetra();
          bool IsNeg = Iter[0];
          uint32_t Data = outTetra();
          int32_t Offset = Data & 0x00FF'FFFF;
          if (Z == 16 || Z == 24) {
            if (IsNeg)
              Offset -= (1 << Z);
          }
          Tmp = CurLoc - Offset * 4;
          outAddressContent(Tmp, Data);
        } break;
        case MMO::LOP_FILE: {
          uint8_t Cnt = getZ(Iter);
          outTetra();
          CurFile++;
          CurLine = 0;
          FileNames.emplace_back(
              StringRef(reinterpret_cast<const char *>(Iter), 4 * Cnt));
          for (int i = 0; i != Cnt; i++) {
            outTetra();
          }
        } break;
        case MMO::LOP_LINE:
          CurLine = getYZ(Iter);
          outTetra();
          break;
        case MMO::LOP_SPEC: {
          uint16_t Type = getYZ(Iter);
          outTetra();
          if (!Lst) {
            outs() << formatv("Special data {0} at loc {1}", Type,
                              format_hex_no_prefix(CurLoc, 16));
            if (!CurLine)
              outs() << "\n";
            else if (CurFile == ListedFile) {
              outs() << formatv(" (line {0})\n", CurLine);
            } else {
              outs() << formatv("(\"{0}\", line {1})", FileNames[CurFile],
                                CurLine);
              ListedFile = CurFile;
            }
          }
          // list spec data
          while (!(Iter[0] == MMO::MM && getX(Iter) != MMO::LOP_QUOTE)) {
            if (Iter[0] == MMO::MM && getX(Iter) == MMO::LOP_QUOTE) {
              outTetra();
            }
            uint32_t Tet = outTetra();
            if (!Lst) {
              outs() << formatv("{0,+27}\n", format_hex_no_prefix(Tet, 8));
            }
          }
        } break;
        case MMO::LOP_POST: {
          uint8_t Z = getZ(Iter);
          outTetra();
          for (auto i = 0; i != 256 - Z; ++i) {
            uint64_t RegVal = read64be(Iter);
            outTetra();
            outTetra();
            if (!Lst) {
              outs() << formatv("g{0}: {1}\n", Z + i,
                                format_hex_no_prefix(RegVal, 16));
            }
          }
        } break;
        case MMO::LOP_STAB:
          Stop = true;
          outTetra();
          break;
        default:
          outTetra();
          break;
        }
      } else {
        outAddressContent(CurLoc, outTetra(), false);
        outExtra();
      }
    }
  }

public:
  MMOType(const MMIXObjectFile &Obj)
      : Obj(Obj), Iter(Obj.getData().bytes_begin()) {}
  void list() {
    listPreamble();
    listContent();
    listStab();
  }
};

} // namespace

int main(int argc, char *argv[]) {
  InitLLVM X(argc, argv);
  cl::ParseCommandLineOptions(argc, argv);
  auto CreateResult = createBinary(InputFilename);
  ExitOnError ExitOnErr;
  ExitOnErr.setBanner(std::string(argv[0]) + ": ");
  if (auto E = CreateResult.takeError()) {
    ExitOnErr(std::move(E));
  }
  OwningBinary<Binary> OBinary = std::move(*CreateResult);
  Binary &Binary = *OBinary.getBinary();

  if (MMIXObjectFile *MMO = dyn_cast<MMIXObjectFile>(&Binary)) {
    MMOType M(*MMO);
    M.list();
  } else {
    outs().flush();
    WithColor::error(errs(), "llvm-mmotype")
        << "Input is not an MMO file (first two bytes are wrong)!\n";
    return -5;
  }
}
