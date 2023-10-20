include "llvm/TableGen/SearchableTable.td"

def MMIXConstant : GenericEnum {
  let FilterClass = "MMIXConstant";
  let NameField = "Name";
  let ValueField = "Value";
}

class MMIXConstant<bits<64> v> {
  string Name = NAME;
  bits<64> Value = v;
}

// ROUND
def ROUND_CURRENT : MMIXConstant<0>;
def ROUND_OFF : MMIXConstant<1>;
def ROUND_UP : MMIXConstant<2>;
def ROUND_DOWN : MMIXConstant<3>;
def ROUND_NEAR : MMIXConstant<4>;

// constant
def Inf : MMIXConstant<!shl(0x7FF, 52)>;

// segment
def Data_Segment : MMIXConstant<!shl(2, 60)>;
def Pool_Segment : MMIXConstant<!shl(4, 60)>;
def Stack_Segment : MMIXConstant<!shl(6, 60)>;

// BIT
def D_BIT : MMIXConstant<!shl(1, 7)>;
def V_BIT : MMIXConstant<!shl(1, 6)>;
def W_BIT : MMIXConstant<!shl(1, 5)>;
def I_BIT : MMIXConstant<!shl(1, 4)>;
def O_BIT : MMIXConstant<!shl(1, 3)>;
def U_BIT : MMIXConstant<!shl(1, 2)>;
def Z_BIT : MMIXConstant<!shl(1, 1)>;
def X_BIT : MMIXConstant<!shl(1, 0)>;

// Handler
def D_Handler : MMIXConstant<0x10>;
def V_Handler : MMIXConstant<0x20>;
def W_Handler : MMIXConstant<0x30>;
def I_Handler : MMIXConstant<0x40>;
def O_Handler : MMIXConstant<0x50>;
def U_Handler : MMIXConstant<0x60>;
def Z_Handler : MMIXConstant<0x70>;
def X_Handler : MMIXConstant<0x80>;

// IO
def StdIn :  MMIXConstant<0>;
def StdOut :  MMIXConstant<1>;
def StdErr :  MMIXConstant<2>;

// IO mode
def TextRead : MMIXConstant<0>;
def TextWrite : MMIXConstant<1>;
def BinaryRead : MMIXConstant<2>;
def BinaryWrite : MMIXConstant<3>;
def BinaryReadWrite : MMIXConstant<4>;

// special function
def Halt : MMIXConstant<0>;
def Fopen : MMIXConstant<1>;
def Fclose : MMIXConstant<2>;
def Fread : MMIXConstant<3>;
def Fgets : MMIXConstant<4>;
def Fgetws : MMIXConstant<5>;
def Fwrite : MMIXConstant<6>;
def Fputs : MMIXConstant<7>;
def Fputws : MMIXConstant<8>;
def Fseek : MMIXConstant<9>;
def Ftell : MMIXConstant<10>;

// special register
def rB : MMIXConstant<0>;    // bootstrap register (trip)
def rD : MMIXConstant<1>;    // dividend register
def rE : MMIXConstant<2>;    // epsilon register
def rH : MMIXConstant<3>;    // himult register
def rJ : MMIXConstant<4>;    // return-jump register
def rM : MMIXConstant<5>;    // multiplex mask register
def rR : MMIXConstant<6>;    // remainder register
def rBB : MMIXConstant<7>;  // bootstrap register (trap)
def rC : MMIXConstant<8>;    // continuation register
def rN : MMIXConstant<9>;    // serial number
def rO : MMIXConstant<10>;   // register stack offset
def rS : MMIXConstant<11>;   // register stack pointer
def rI : MMIXConstant<12>;   // interval counter
def rT : MMIXConstant<13>;   // trap address register
def rTT : MMIXConstant<14>; // dynamic trap address register
def rK : MMIXConstant<15>;   // interrupt mask register
def rQ : MMIXConstant<16>;   // interrupt request register
def rU : MMIXConstant<17>;   // usage counter
def rV : MMIXConstant<18>;   // virtual translation register
def rG : MMIXConstant<19>;   // global threshold register
def rL : MMIXConstant<20>;   // local threshold register
def rA : MMIXConstant<21>;   // arithmetic status register
def rF : MMIXConstant<22>;   // failure location register
def rP : MMIXConstant<23>;   // prediction register
def rW : MMIXConstant<24>;   // where-interrupted register (trip)
def rX : MMIXConstant<25>;   // execution register (trip)
def rY : MMIXConstant<26>;   // Y operand (trip)
def rZ : MMIXConstant<27>;   // Z operand (trip)
def rWW : MMIXConstant<28>; // where-interrupted register (trap)
def rXX : MMIXConstant<29>; // execution register (trap)
def rYY : MMIXConstant<30>; // Y operand (trap)
def rZZ : MMIXConstant<31>; // Z operand (trap)
