include "llvm/TableGen/SearchableTable.td"

def MMIXALPredefsTable : GenericTable {
  let FilterClass = "MMIXALPredefsEntry";
  let Fields = ["Name", "Value"];
}

class MMIXALPredefsEntry<bits<64> v> {
  string Name = NAME;
  bits<64> Value = v;
}

// ROUND
def ROUND_CURRENT : MMIXALPredefsEntry<0>;
def ROUND_OFF : MMIXALPredefsEntry<1>;
def ROUND_UP : MMIXALPredefsEntry<2>;
def ROUND_DOWN : MMIXALPredefsEntry<3>;
def ROUND_NEAR : MMIXALPredefsEntry<4>;

// constant
def Inf : MMIXALPredefsEntry<!shl(0x7FF, 52)>;

// segment
def Data_Segment : MMIXALPredefsEntry<!shl(2, 60)>;
def Pool_Segment : MMIXALPredefsEntry<!shl(4, 60)>;
def Stack_Segment : MMIXALPredefsEntry<!shl(6, 60)>;

// BIT
def D_BIT : MMIXALPredefsEntry<!shl(1, 7)>;
def V_BIT : MMIXALPredefsEntry<!shl(1, 6)>;
def W_BIT : MMIXALPredefsEntry<!shl(1, 5)>;
def I_BIT : MMIXALPredefsEntry<!shl(1, 4)>;
def O_BIT : MMIXALPredefsEntry<!shl(1, 3)>;
def U_BIT : MMIXALPredefsEntry<!shl(1, 2)>;
def Z_BIT : MMIXALPredefsEntry<!shl(1, 1)>;
def X_BIT : MMIXALPredefsEntry<!shl(1, 0)>;

// Handler
def D_Handler : MMIXALPredefsEntry<0x10>;
def V_Handler : MMIXALPredefsEntry<0x20>;
def W_Handler : MMIXALPredefsEntry<0x30>;
def I_Handler : MMIXALPredefsEntry<0x40>;
def O_Handler : MMIXALPredefsEntry<0x50>;
def U_Handler : MMIXALPredefsEntry<0x60>;
def Z_Handler : MMIXALPredefsEntry<0x70>;
def X_Handler : MMIXALPredefsEntry<0x80>;

// IO
def StdIn :  MMIXALPredefsEntry<0>;
def StdOut :  MMIXALPredefsEntry<1>;
def StdErr :  MMIXALPredefsEntry<2>;

// IO mode
def TextRead : MMIXALPredefsEntry<0>;
def TextWrite : MMIXALPredefsEntry<1>;
def BinaryRead : MMIXALPredefsEntry<2>;
def BinaryWrite : MMIXALPredefsEntry<3>;
def BinaryReadWrite : MMIXALPredefsEntry<4>;

// special function
def Halt : MMIXALPredefsEntry<0>;
def Fopen : MMIXALPredefsEntry<1>;
def Fclose : MMIXALPredefsEntry<2>;
def Fread : MMIXALPredefsEntry<3>;
def Fgets : MMIXALPredefsEntry<4>;
def Fgetws : MMIXALPredefsEntry<5>;
def Fwrite : MMIXALPredefsEntry<6>;
def Fputs : MMIXALPredefsEntry<7>;
def Fputws : MMIXALPredefsEntry<8>;
def Fseek : MMIXALPredefsEntry<9>;
def Ftell : MMIXALPredefsEntry<10>;

// special register
def rB : MMIXALPredefsEntry<0>;    // bootstrap register (trip)
def rD : MMIXALPredefsEntry<1>;    // dividend register
def rE : MMIXALPredefsEntry<2>;    // epsilon register
def rH : MMIXALPredefsEntry<3>;    // himult register
def rJ : MMIXALPredefsEntry<4>;    // return-jump register
def rM : MMIXALPredefsEntry<5>;    // multiplex mask register
def rR : MMIXALPredefsEntry<6>;    // remainder register
def rBB : MMIXALPredefsEntry<7>;  // bootstrap register (trap)
def rC : MMIXALPredefsEntry<8>;    // continuation register
def rN : MMIXALPredefsEntry<9>;    // serial number
def rO : MMIXALPredefsEntry<10>;   // register stack offset
def rS : MMIXALPredefsEntry<11>;   // register stack pointer
def rI : MMIXALPredefsEntry<12>;   // interval counter
def rT : MMIXALPredefsEntry<13>;   // trap address register
def rTT : MMIXALPredefsEntry<14>; // dynamic trap address register
def rK : MMIXALPredefsEntry<15>;   // interrupt mask register
def rQ : MMIXALPredefsEntry<16>;   // interrupt request register
def rU : MMIXALPredefsEntry<17>;   // usage counter
def rV : MMIXALPredefsEntry<18>;   // virtual translation register
def rG : MMIXALPredefsEntry<19>;   // global threshold register
def rL : MMIXALPredefsEntry<20>;   // local threshold register
def rA : MMIXALPredefsEntry<21>;   // arithmetic status register
def rF : MMIXALPredefsEntry<22>;   // failure location register
def rP : MMIXALPredefsEntry<23>;   // prediction register
def rW : MMIXALPredefsEntry<24>;   // where-interrupted register (trip)
def rX : MMIXALPredefsEntry<25>;   // execution register (trip)
def rY : MMIXALPredefsEntry<26>;   // Y operand (trip)
def rZ : MMIXALPredefsEntry<27>;   // Z operand (trip)
def rWW : MMIXALPredefsEntry<28>; // where-interrupted register (trap)
def rXX : MMIXALPredefsEntry<29>; // execution register (trap)
def rYY : MMIXALPredefsEntry<30>; // Y operand (trap)
def rZZ : MMIXALPredefsEntry<31>; // Z operand (trap)
