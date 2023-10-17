#ifndef LLVM_LIB_TARGET_MMIX_MMIXINSTRPATTERN_TD
#define LLVM_LIB_TARGET_MMIX_MMIXINSTRPATTERN_TD

//===----------------------------------------------------------------------===//
// Instruction Patterns
//===----------------------------------------------------------------------===//

// load and store
def :Pat<(i64 (sextloadi8 (add i64:$Y, i64:$Z))), (LDB i64:$Y, i64:$Z)>;
def :Pat<(i64 (sextloadi8 (add i64:$Y, ui8imm:$Z))), (i64 (LDBI i64:$Y, i64:$Z))>;
def :Pat<(i64 (zextloadi8 (add i64:$Y, i64:$Z))), (i64 (LDBU i64:$Y, i64:$Z))>;
def :Pat<(i64 (zextloadi8 (add i64:$Y, ui8imm:$Z))), (i64 (LDBUI i64:$Y, i64:$Z))>;

def : Pat<(sextloadi16(add i64 : $Y, i64 : $Z)), (i64(LDW i64 : $Y, i64 : $Z))>;
def : Pat<(sextloadi16(add i64
                       : $Y, ui8imm
                       : $Z)),
          (i64(LDWI i64
               : $Y, i64
               : $Z))>;
def : Pat<(zextloadi16(add i64
                       : $Y, i64
                       : $Z)),
          (i64(LDWU i64
               : $Y, i64
               : $Z))>;
def : Pat<(zextloadi16(add i64
                       : $Y, ui8imm
                       : $Z)),
          (i64(LDWUI i64
               : $Y, i64
               : $Z))>;

def :Pat<(sextloadi32 (add i64:$Y, i64:$Z)), (i64 (LDT i64:$Y, i64:$Z))>;
def :Pat<(sextloadi32 (add i64:$Y, ui8imm:$Z)), (i64 (LDTI i64:$Y, i64:$Z))>;
def :Pat<(zextloadi32 (add i64:$Y, i64:$Z)), (i64 (LDTU i64:$Y, i64:$Z))>;
def :Pat<(zextloadi32 (add i64:$Y, ui8imm:$Z)), (i64 (LDTUI i64:$Y,
i64:$Z))>;

def :Pat<(sextloadi64 (add i64:$Y, i64:$Z)), (i64 (LDO i64:$Y, i64:$Z))>;
def :Pat<(sextloadi64 (add i64:$Y, ui8imm:$Z)), (i64 (LDOI i64:$Y, i64:$Z))>;
def :Pat<(zextloadi64 (add i64:$Y, i64:$Z)), (i64 (LDOU i64:$Y, i64:$Z))>;
def :Pat<(zextloadi64 (add i64:$Y, ui8imm:$Z)), (i64 (LDOI i64:$Y, i64:$Z))>;

def :Pat<(shl (sextloadi32 (add i64:$Y, i64:$Z)), (i8 32)), (i64 (LDHT
i64:$Y, i64:$Z))>; def :Pat<(shl (sextloadi32 (add i64:$Y, ui8imm:$Z)), (i8
32)), (i64 (LDHTI i64:$Y, i64:$Z))>;

def :Pat<(truncstorei8 i64:$X, (add i64:$Y, i64:$Z)), (STB i64:$X, i64:$Y,
i64:$Z)>; def :Pat<(truncstorei8 i64:$X, (add i64:$Y, ui8imm:$Z)), (STBI
i64:$X, i64:$Y, i64:$Z)>; def :Pat<(truncstorei8 i64:$X, (add i64:$Y,
i64:$Z)), (STBU i64:$X, i64:$Y, i64:$Z)>; def :Pat<(truncstorei8 i64:$X, (add
i64:$Y, ui8imm:$Z)), (STBUI i64:$X, i64:$Y, i64:$Z)>;

def :Pat<(truncstorei16 i64:$X, (add i64:$Y, i64:$Z)), (STW i64:$X, i64:$Y,
i64:$Z)>; def :Pat<(truncstorei16 i64:$X, (add i64:$Y, ui8imm:$Z)), (STWI
i64:$X, i64:$Y, i64:$Z)>; def :Pat<(truncstorei16 i64:$X, (add i64:$Y,
i64:$Z)), (STWU i64:$X, i64:$Y, i64:$Z)>; def :Pat<(truncstorei16 i64:$X,
(add i64:$Y, ui8imm:$Z)), (STWUI i64:$X, i64:$Y, i64:$Z)>;

def :Pat<(truncstorei32 i64:$X, (add i64:$Y, i64:$Z)), (STT i64:$X, i64:$Y,
i64:$Z)>; def :Pat<(truncstorei32 i64:$X, (add i64:$Y, ui8imm:$Z)), (STTI
i64:$X, i64:$Y, i64:$Z)>; def :Pat<(truncstorei32 i64:$X, (add i64:$Y,
i64:$Z)), (STTU i64:$X, i64:$Y, i64:$Z)>; def :Pat<(truncstorei32 i64:$X,
(add i64:$Y, ui8imm:$Z)), (STTUI i64:$X, i64:$Y, i64:$Z)>;

def :Pat<(truncstorei64 i64:$X, (add i64:$Y, i64:$Z)), (STO i64:$X, i64:$Y,
i64:$Z)>; def :Pat<(truncstorei64 i64:$X, (add i64:$Y, ui8imm:$Z)), (STOI
i64:$X, i64:$Y, i64:$Z)>; def :Pat<(truncstorei64 i64:$X, (add i64:$Y,
i64:$Z)), (STOU i64:$X, i64:$Y, i64:$Z)>; def :Pat<(truncstorei64 i64:$X,
(add i64:$Y, ui8imm:$Z)), (STOI i64:$X, i64:$Y, i64:$Z)>;

def :Pat<(truncstorei64 ui8imm:$X, (add i64:$Y, i64:$Z)), (STCO ui8imm:$X,
i64:$Y, i64:$Z)>; def :Pat<(truncstorei64 ui8imm:$X, (add i64:$Y,
ui8imm:$Z)), (STCO ui8imm:$X, i64:$Y, ui8imm:$Z)>;

def :Pat<(truncstorei32 (srl i64:$X, (i8 32)), (add i64:$Y, i64:$Z)), (STHT
i64:$X, i64:$Y, i64:$Z)>; def :Pat<(truncstorei32 (srl i64:$X, (i8 32)), (add
i64:$Y, ui8imm:$Z)), (STHT i64:$X, i64:$Y, ui8imm:$Z)>;

// add and sub
def :Pat<(add (mul i64:$Y, 2), i64:$Z), (I2ADDU i64:$Y, i64:$Z)>;
def :Pat<(add (mul i64:$Y, 4), i64:$Z), (I4ADDU i64:$Y, i64:$Z)>;
def :Pat<(add (mul i64:$Y, 8), i64:$Z), (I8ADDU i64:$Y, i64:$Z)>;
def :Pat<(add (mul i64:$Y, 16), i64:$Z), (I16ADDU i64:$Y, i64:$Z)>;

def :Pat<(sub i64:$Y, i64:$Z), (SUBU i64:$Y, i64:$Z)>;
def :Pat<(sub i64:$Y, ui8imm:$Z), (SUBUI i64:$Y, ui8imm:$Z)>;

def :Pat<(sub ui8imm:$Y, i64:$Z), (NEG ui8imm:$Y, i64:$Z)>;
def :Pat<(sub ui8imm:$Y, ui8imm:$Z), (NEGI ui8imm:$Y, ui8imm:$Z)>;
def :Pat<(sub ui8imm:$Y, i64:$Z), (NEGU ui8imm:$Y, i64:$Z)>;
def :Pat<(sub ui8imm:$Y, ui8imm:$Z), (NEGUI ui8imm:$Y, ui8imm:$Z)>;

// bit fiddling
def :Pat<(and i64:$Y, i64:$Z), (AND i64:$Y, i64:$Z)>;
def :Pat<(and i64:$Y, ui8imm:$Z), (ANDI i64:$Y, i64:$Z)>;

def :Pat<(or i64:$Y, i64:$Z), (OR i64:$Y, i64:$Z)>;
def :Pat<(or i64:$Y, ui8imm:$Z), (ORI i64:$Y, i64:$Z)>;

def :Pat<(xor i64:$Y, i64:$Z), (XOR i64:$Y, i64:$Z)>;
def :Pat<(xor i64:$Y, ui8imm:$Z), (XORI i64:$Y, i64:$Z)>;

def :Pat<(and i64:$Y, (not i64:$Z)), (ANDN i64:$Y, i64:$Z)>;
def :Pat<(and i64:$Y, (not ui8imm:$Z)), (ANDNI i64:$Y, ui8imm:$Z)>;

def :Pat<(or i64:$Y, (not i64:$Z)), (ORN i64:$Y, i64:$Z)>;
def :Pat<(or i64:$Y, (not ui8imm:$Z)), (ORNI i64:$Y, ui8imm:$Z)>;

def :Pat<(not (and i64:$Y, i64:$Z)), (NAND i64:$Y, i64:$Z)>;
def :Pat<(not (and i64:$Y, ui8imm:$Z)), (NANDI i64:$Y, ui8imm:$Z)>;

def :Pat<(not (or i64:$Y, i64:$Z)), (NOR i64:$Y, i64:$Z)>;
def :Pat<(not (or i64:$Y, ui8imm:$Z)), (NORI i64:$Y, ui8imm:$Z)>;

def :Pat<(not (xor i64:$Y, i64:$Z)), (NXOR i64:$Y, i64:$Z)>;
def :Pat<(not (xor i64:$Y, ui8imm:$Z)), (NXORI i64:$Y, ui8imm:$Z)>;

// mux

// comparison
def :Pat<(setugt i64:$Y, i64:$Z), (i64 (CSPI (SETL (IMPLICIT_DEF), 0), (CMPU i64:$Y, i64:$Z), 1))>;
def :Pat<(setugt i64:$Y, ui8imm:$Z), (i64 (CSPI (SETL (IMPLICIT_DEF), 0), (CMPUI i64:$Y, ui8imm:$Z), 1))>;
def :Pat<(setuge i64:$Y, i64:$Z), (i64 (CSNNI (SETL (IMPLICIT_DEF), 0), (CMPU i64:$Y, i64:$Z), 1))>;
def :Pat<(setuge i64:$Y, ui8imm:$Z), (i64 (CSNNI (SETL (IMPLICIT_DEF), 0), (CMPUI i64:$Y, ui8imm:$Z), 1))>;
def :Pat<(setult i64:$Y, i64:$Z), (i64 (CSNI (SETL (IMPLICIT_DEF), 0), (CMPU i64:$Y, i64:$Z), 1))>;
def :Pat<(setult i64:$Y, ui8imm:$Z), (i64 (CSNI (SETL (IMPLICIT_DEF), 0), (CMPUI i64:$Y, ui8imm:$Z), 1))>;
def :Pat<(setule i64:$Y, i64:$Z), (i64 (CSNPI (SETL (IMPLICIT_DEF), 0), (CMPU i64:$Y, i64:$Z), 1))>;
def :Pat<(setule i64:$Y, ui8imm:$Z), (i64 (CSNPI (SETL (IMPLICIT_DEF), 0), (CMPUI i64:$Y, ui8imm:$Z), 1))>;
def :Pat<(seteq i64:$Y, i64:$Z), (i64 (CSZI (SETL (IMPLICIT_DEF), 0), (CMP i64:$Y, i64:$Z), 1))>;
def :Pat<(seteq i64:$Y, ui8imm:$Z), (i64 (CSZI (SETL (IMPLICIT_DEF), 0), (CMPI i64:$Y, ui8imm:$Z), 1))>;
def :Pat<(setgt i64:$Y, i64:$Z), (i64 (CSPI (SETL (IMPLICIT_DEF), 0), (CMP i64:$Y, i64:$Z), 1))>;
def :Pat<(setgt i64:$Y, ui8imm:$Z), (i64 (CSPI (SETL (IMPLICIT_DEF), 0), (CMPI i64:$Y, ui8imm:$Z), 1))>;
def :Pat<(setge i64:$Y, i64:$Z), (i64 (CSNNI (SETL (IMPLICIT_DEF), 0), (CMP i64:$Y, i64:$Z), 1))>;
def :Pat<(setge i64:$Y, ui8imm:$Z), (i64 (CSNNI (SETL (IMPLICIT_DEF), 0), (CMPI i64:$Y, ui8imm:$Z), 1))>;
def :Pat<(setlt i64:$Y, i64:$Z), (i64 (CSNI (SETL (IMPLICIT_DEF), 0), (CMP i64:$Y, i64:$Z), 1))>;
def :Pat<(setlt i64:$Y, ui8imm:$Z), (i64 (CSNI (SETL (IMPLICIT_DEF), 0), (CMPI i64:$Y, ui8imm:$Z), 1))>;
def :Pat<(setle i64:$Y, i64:$Z), (i64 (CSNPI (SETL (IMPLICIT_DEF), 0), (CMP i64:$Y, i64:$Z), 1))>;
def :Pat<(setle i64:$Y, ui8imm:$Z), (i64 (CSNPI (SETL (IMPLICIT_DEF), 0), (CMPI i64:$Y, ui8imm:$Z), 1))>;
def :Pat<(setne i64:$Y, i64:$Z), (i64 (CSNZI (SETL (IMPLICIT_DEF), 0), (CMP i64:$Y, i64:$Z), 1))>;
def :Pat<(setne i64:$Y, ui8imm:$Z), (i64 (CSNZI (SETL (IMPLICIT_DEF), 0), (CMPI i64:$Y, ui8imm:$Z), 1))>;
def :Pat<(select i64:$cond, i64:$b1, i64:$b0), (CSZ i64:$b1, i64:$cond, i64:$b0)>;


// branches and jumps
def : GINodeEquiv<G_BRCOND, brcond>;

def :Pat<(br bb:$target), (JMP bb:$target)>;
def :Pat<(brcond i64:$tst, bb:$target), (BNZ i64:$tst, bb:$target)>;

// multiplication and division

def :Pat<(mul i64:$Y, i64:$Z), (MUL i64:$Y, i64:$Z)>;

#endif // LLVM_LIB_TARGET_MMIX_MMIXINSTRPATTERN_TD
