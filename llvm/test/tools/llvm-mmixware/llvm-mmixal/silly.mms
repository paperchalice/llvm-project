% RUN: llvm-mmixal -strict-mode -o - %s | obj2yaml - | FileCheck %s
* A program that exercises all MMIX operations (more or less)
small       GREG   #abc
neg_zero    GREG   #8000000000000000
half        GREG   #3fe0000000000000
inf         GREG   #7ff0000000000000
sig_nan     GREG   #7ff1000000000000
round_off   GREG   ROUND_OFF<<16
round_up    GREG   ROUND_UP<<16
round_down  GREG   ROUND_DOWN<<16
addy        GREG   #7f6001b4c67bc809
addz        GREG   #ff5ffb6a4534a3f7
flip        GREG   #0102040810204080
ry          GREG
rz          GREG
            LOC    Data_Segment
            GREG   @
Start_Inst  SUB    $4,half,$1
Final_Inst  SRU    $4,half,1
Load_Test   OCTA   #8081828384858687
            OCTA   #88898a8b8c8d8e8f
Jmp_Pop     JMP    @+8
            POP
Load_Begin  TETRA  #5f030405
Load_End    LDUNC  $3,$4,5
Big_Begin   GO     $40,ry,5
Big_End     ANDNL  $40,(ry-$0)<<8+5

            LOC    #100
Main        FCMP   $0,neg_zero,$5
            FCMP   $1,neg_zero,inf
            FCMP   $2,inf,sig_nan
            FUN    $3,sig_nan,sig_nan
            FEQL   $4,$4,neg_zero
            FADD   $5,half,inf
            FADD   $6,half,neg_zero
            FADD   $7,half,half
            FADD   $8,half,sig_nan
            FSUB   $9,half,small
            PUT    rA,round_off
            FSUB   $9,half,small
            FSUB   $9,small,half
            FSQRT  $10,$9
            FSUB   $11,sig_nan,$10
            PUT    rA,round_down
            FSUB   $12,half,half
            FSUB   $12,$20,$21
            FSUB   $12,$20,neg_zero
            PUT    rA,round_up
            SUB    $0,inf,1           % $0 = largest normal number
            FADD   $12,$0,small
            FIX    $12,half
            FIXU   $14,ROUND_DOWN,$9
            FLOT   $15,ROUND_DOWN,addy
            FLOT   $16,ROUND_UP,addy
            NEG    $1,1               % $1 = -1
            FLOT   $17,1
            FLOT   $17,$1
            FLOTU  $18,255
            FLOTU  $18,neg_zero
            FIX    $13,ROUND_NEAR,$18
            SFLOT  $18,ROUND_DOWN,addy
            SFLOT  $19,ROUND_UP,addy
            FSUB   $20,$18,$19
            FSUB   $20,$16,$15
            SFLOT  $20,1
            SFLOT  $20,$1
            SFLOTU $21,$1
            SFLOTU $21,255
            FMUL   $22,neg_zero,inf
            FMUL   $22,half,half
            FMUL   $23,small,$0
            PUT    rE,half
            FCMPE  $24,half,$21
            FCMPE  $24,neg_zero,small
            FCMPE  $24,neg_zero,half
            FCMPE  $24,half,inf
            FEQLE  $24,$15,$16
            PUT    rE,neg_zero
            FEQLE  $24,half,half
            FUNE   $24,half,half
            FSQRT  $25,ROUND_UP,$0
            FDIV   $26,$0,$25
            PUT    rA,$50
            FDIV   $26,$0,$25
            FMUL   $27,$25,$25
            FREM   $28,$9,half
            FREM   $29,$9,small
            FINT   $30,$9
            FINT   $30,ROUND_UP,small
            MUL    $31,flip,flip
            MUL    $32,flip,$1
            MUL    $33,flip,2
            DIV    $32,$32,$1
            DIV    $32,neg_zero,$1
            MULU   $32,flip,$1
            MULU   $31,flip,flip
            GET    $33,rH
            PUT    rD,$33
            DIV    $33,$1,3
            DIVU   $34,$31,flip
            ADD    $35,addy,addz
            FADD   $36,addy,addz
            CMP    $37,$36,$35
            GETA   $3,1F
            PUT    rW,$3
            LDT    $6,Start_Inst
            LDTU   $7,Final_Inst
1H          CMP    $5,$6,$7
            BNN    $5,1F
            INCML  $6,#100           % increase the opcode
            PUT    rX,$6             % ropcode 0
            RESUME                   % return to 1B
1H          BN     $0,@+4*6
            PBN    $0,@-4*1
            BNN    $0,@+4*6
            PBN    $0,@+4*5
            PBNN   $0,@+4*5
            BN     $0,@-4*3
            BNN    $0,@-4*3
            PBN    $0,@-4*3
            PBNN   $0,@-4*3
            BZ     $0,@+4*6
            PBZ    $0,@-4*1
            BNZ    $0,@+4*6
            PBZ    $0,@+4*5
            PBNZ   $0,@+4*5
            BZ     $0,@-4*3
            BNZ    $0,@-4*3
            PBZ    $0,@-4*3
            PBNZ   $0,@-4*3
            BP     $0,@+4*6
            PBP    $0,@-4*1
            BNP    $0,@+4*6
            PBP    $0,@+4*5
            PBNP   $0,@+4*5
            BP     $0,@-4*3
            BNP    $0,@-4*3
            PBP    $0,@-4*3
            PBNP   $0,@-4*3
            BOD    $0,@+4*6
            PBOD   $0,@-4*1
            BEV    $0,@+4*6
            PBOD   $0,@+4*5
            PBEV   $0,@+4*5
            BOD    $0,@-4*3
            BEV    $0,@-4*3
            PBOD   $0,@-4*3
            PBEV   $0,@-4*3
            LDA    $4,Load_Test+4
            GETA   $3,1F
            PUT    rW,$3
            LDTU   $7,Load_End
            LDTU   $6,Load_Begin
1H          CMPU   $8,$6,$7
            BNN    $8,1F
            INCML  $6,#100           % increase the opcode
            PUT    rX,$6
            RESUME                   % return to 1B
2H          OCTA   #fedcba9876543210 % becomes Jmp_Pop
            OCTA   #ffeeddccbbaa9988 % becomes Jmp_Pop
            NEG    ry,addy
            SET    rz,flip
            PUT    rM,addz
            POP
1H          GETA   $4,2B
            SETL   $7,4*11
            GO     $7,$7,$4
            GO     $7,$4,4*12
            PRELD  70,$4,$4
            PRELD  70,$4,0
            PREGO  70,$4,$4
            PREGO  70,$4,0
            CSWAP  $3,Load_Test+13
            GETA   $3,1F
            PUT    rW,$3
            SETL   rz,1
            ADD    ry,$4,4
            LDOU   $40,Jmp_Pop
            LDTU   $7,Big_End
            LDTU   $6,Big_Begin
1H          CMPU   $8,$6,$7
            BNN    $8,1F
            INCML  $6,#100           % increase the opcode
            PUT    rX,$6
            SET    $5,rz
            RESUME                   % return to 1B
1H          SL     $40,small,51
            SL     $40,small,52
            SAVE   $255,0
            PUT    rG,small-$0
            INCL   small-1,U_BIT<<8
            FADD   $100,small,$200
            PUT    rA,small-1        % enable underflow trip
            TRIP   1,$100,small
            FSUB   $100,small,$200   % cause underflow trip
            PUT    rL,10
            PUT    rL,small
            PUSHJ  11,@+4
            UNSAVE $255
            TRAP   0,Halt,0          % normal exit

            LOC    U_Handler
            PUSHJ  $255,Handler
3H          TRAP   0,$1
            SUB    $0,$1,1
            POP    2,0
4H          GET    $50,rX
            INCH   $50,#8100         % ropcode 1
            FLOT   $60,1
            PUT    rZ,$60
            JMP    2F

            LOC    0
            GET    $50,rX
            INCH   $50,#8200         % ropcode 2
            INCMH  $50,#ff00-(U_BIT<<8)
            TRAP   1
2H          PUT    rX,$50
            GET    $255,rB
            RESUME
Handler     SETL   $5,#abcd
            GET    $1,rJ
            PUSHJ  3,3B
            SUB    $10,$3,$4
            PUT    rJ,$1
            POP    11,(4B-3B)>>2

# CHECK: --- !MMO
# CHECK: Preamble:
# CHECK:   Version:         1
# CHECK:   CreatedTime:     {{[0-9]+}}
# CHECK: Segments:
# CHECK:   - OpCode:          LOC
# CHECK:     HighByte:        DATA
# CHECK:     Offset:          0x0
# CHECK:   - StartAddress:    0x2000000000000000
# CHECK:     Data:            2404FC013F04FC01808182838485868788898A8B8C8D8E8FF0000002F80000005F030405970304059F28F305EF28F305
# CHECK:   - OpCode:          LOC
# CHECK:     HighByte:        INSTRUCTION
# CHECK:     Offset:          0x100
# CHECK:   - OpCode:          FILE
# CHECK:     Name:            {{.+}}
# CHECK:     Number:          0
# CHECK:   - OpCode:          LINE
# CHECK:     Number:          30
# CHECK:   - StartAddress:    0x100
# CHECK:     Data:            0100FD050101FDFB0102FBFA0203FAFA030404FD0405FCFB0406FCFD0407FCFC0408FCFA0609FCFEF61500F90609FCFE0609FEFC150A0009060BFA0AF61500F7060CFCFC060C1415060C14FDF61500F82500FB01040C00FE050C00FC070E0309080F03F6081002F63501000109110001081100010B1200FF0A1200FD050D04120C1203F60C1302F6061412130614100F0D1400010C1400010E1500010F1500FF1016FDFB1016FCFC1017FE00F60200FC1118FC151118FDFE1118FDFC1118FCFB13180F10F60200FD1318FCFC1218FCFC15190200141A0019F6150032141A0019101B1919161C09FC161D09FE171E0009171E02FE181FF4F41820F4011921F4021C2020011C20FD011A20F4011A1FF4F4FE210003F60100211D2101031E221FF42023F6F50424F6F530252423F4030000F61800038906F1008B07F104
# CHECK:   - OpCode:          FIXR
# CHECK:     Delta:           0x4
# CHECK:   - StartAddress:    0x23C
# CHECK:     Data:            3005060748050000E6060100F6190006F9000000
# CHECK:   - OpCode:          FIXR
# CHECK:     Delta:           0x4
# CHECK:   - StartAddress:    0x250
# CHECK:     Data:            400000065100FFFF4800000650000005580000054100FFFD4900FFFD5100FFFD5900FFFD420000065300FFFF4A000006520000055A0000054300FFFD4B00FFFD5300FFFD5B00FFFD440000065500FFFF4C000006540000055C0000054500FFFD4D00FFFD5500FFFD5D00FFFD460000065700FFFF4E000006560000055E0000054700FFFD4F00FFFD5700FFFD5F00FFFD2304F10CF4030000F61800038B07F1248B06F120
# CHECK:   - OpCode:          FIXR
# CHECK:     Delta:           0x4
# CHECK:   - StartAddress:    0x2F4
# CHECK:     Data:            3208060748080000E6060100F6190006F9000000FEDCBA98
# CHECK:   - OpCode:          LINE
# CHECK:     Number:          160
# CHECK:   - StartAddress:    0x30C
# CHECK:     Data:            76543210FFEEDDCC
# CHECK:   - OpCode:          LINE
# CHECK:     Number:          161
# CHECK:   - StartAddress:    0x314
# CHECK:     Data:            BBAA998834F300F6C1F2F400F60500F5F8000000
# CHECK:   - OpCode:          FIXR
# CHECK:     Delta:           0xC
# CHECK:   - StartAddress:    0x328
# CHECK:     Data:            F504FFF8E307002C9E0707049F0704309A4604049B4604009C4604049D4604009503F115F4030000F6180003E3F2000121F304048F28F1188B07F12C8B06F128
# CHECK:   - OpCode:          FIXR
# CHECK:     Delta:           0x7
# CHECK:   - StartAddress:    0x368
# CHECK:     Data:            3208060748080000E6060100F6190006C105F200F9000000
# CHECK:   - OpCode:          FIXR
# CHECK:     Delta:           0x5
# CHECK:   - StartAddress:    0x380
# CHECK:     Data:            3928FE333928FE34FAFF0000F71300FEE7FD04000464FEC8F61500FDFF0164FE0664FEC8F714000AF61400FEF20B0001FB0000FF00000000
# CHECK:   - OpCode:          LOC
# CHECK:     HighByte:        INSTRUCTION
# CHECK:     Offset:          0x60
# CHECK:   - OpCode:          LINE
# CHECK:     Number:          204
# CHECK:   - StartAddress:    0x60
# CHECK:     Data:            F2FF00000000000125000101F8020000FE320019E4328100093C0001F61B003CF0000000
# CHECK:   - OpCode:          LOC
# CHECK:     HighByte:        INSTRUCTION
# CHECK:     Offset:          0x0
# CHECK:   - OpCode:          LINE
# CHECK:     Number:          215
# CHECK:   - StartAddress:    0x0
# CHECK:     Data:            FE320019E4328200E532FB0000000001
# CHECK:   - OpCode:          FIXRX
# CHECK:     FixType:         JMP
# CHECK:     Delta:           -28
# CHECK:   - StartAddress:    0x10
# CHECK:     Data:            F6190032FEFF0000F9000000
# CHECK:   - OpCode:          FIXRX
# CHECK:     FixType:         OTHERWISE
# CHECK:     Delta:           -17
# CHECK:   - StartAddress:    0x1C
# CHECK:     Data:            E305ABCDFE010004F2030010240A0304F6040001F80B0003
# CHECK: Postamble:
# CHECK:   G:               241
# CHECK:   Values:          [ 0x2000000000000000, 0x0, 0x0, 0x102040810204080, 0xFF5FFB6A4534A3F7, 
# CHECK:                      0x7F6001B4C67BC809, 0x30000, 0x20000, 0x10000, 0x7FF1000000000000, 
# CHECK:                      0x7FF0000000000000, 0x3FE0000000000000, 0x8000000000000000, 
# CHECK:                      0xABC, 0x100 ]
# CHECK: SymbolTable:
# CHECK:   IsUTF16:         false
# CHECK:   Symbol:
# CHECK:     - Name:            Main
# CHECK:       Serial:          1
# CHECK:       Equiv:           0x100
# CHECK:       Type:            NORMAL
# CHECK:     - Name:            small
# CHECK:       Serial:          2
# CHECK:       Equiv:           0xFE
# CHECK:       Type:            REGISTER
# CHECK:     - Name:            neg_zero
# CHECK:       Serial:          3
# CHECK:       Equiv:           0xFD
# CHECK:       Type:            REGISTER
# CHECK:     - Name:            half
# CHECK:       Serial:          4
# CHECK:       Equiv:           0xFC
# CHECK:       Type:            REGISTER
# CHECK:     - Name:            inf
# CHECK:       Serial:          5
# CHECK:       Equiv:           0xFB
# CHECK:       Type:            REGISTER
# CHECK:     - Name:            sig_nan
# CHECK:       Serial:          6
# CHECK:       Equiv:           0xFA
# CHECK:       Type:            REGISTER
# CHECK:     - Name:            round_off
# CHECK:       Serial:          7
# CHECK:       Equiv:           0xF9
# CHECK:       Type:            REGISTER
# CHECK:     - Name:            round_up
# CHECK:       Serial:          8
# CHECK:       Equiv:           0xF8
# CHECK:       Type:            REGISTER
# CHECK:     - Name:            round_down
# CHECK:       Serial:          9
# CHECK:       Equiv:           0xF7
# CHECK:       Type:            REGISTER
# CHECK:     - Name:            addy
# CHECK:       Serial:          10
# CHECK:       Equiv:           0xF6
# CHECK:       Type:            REGISTER
# CHECK:     - Name:            addz
# CHECK:       Serial:          11
# CHECK:       Equiv:           0xF5
# CHECK:       Type:            REGISTER
# CHECK:     - Name:            flip
# CHECK:       Serial:          12
# CHECK:       Equiv:           0xF4
# CHECK:       Type:            REGISTER
# CHECK:     - Name:            ry
# CHECK:       Serial:          13
# CHECK:       Equiv:           0xF3
# CHECK:       Type:            REGISTER
# CHECK:     - Name:            rz
# CHECK:       Serial:          14
# CHECK:       Equiv:           0xF2
# CHECK:       Type:            REGISTER
# CHECK:     - Name:            Start_Inst
# CHECK:       Serial:          15
# CHECK:       Equiv:           0x2000000000000000
# CHECK:       Type:            NORMAL
# CHECK:     - Name:            Final_Inst
# CHECK:       Serial:          16
# CHECK:       Equiv:           0x2000000000000004
# CHECK:       Type:            NORMAL
# CHECK:     - Name:            Load_Test
# CHECK:       Serial:          17
# CHECK:       Equiv:           0x2000000000000008
# CHECK:       Type:            NORMAL
# CHECK:     - Name:            Jmp_Pop
# CHECK:       Serial:          18
# CHECK:       Equiv:           0x2000000000000018
# CHECK:       Type:            NORMAL
# CHECK:     - Name:            Load_Begin
# CHECK:       Serial:          19
# CHECK:       Equiv:           0x2000000000000020
# CHECK:       Type:            NORMAL
# CHECK:     - Name:            Load_End
# CHECK:       Serial:          20
# CHECK:       Equiv:           0x2000000000000024
# CHECK:       Type:            NORMAL
# CHECK:     - Name:            Big_Begin
# CHECK:       Serial:          21
# CHECK:       Equiv:           0x2000000000000028
# CHECK:       Type:            NORMAL
# CHECK:     - Name:            Big_End
# CHECK:       Serial:          22
# CHECK:       Equiv:           0x200000000000002C
# CHECK:       Type:            NORMAL
# CHECK:     - Name:            Handler
# CHECK:       Serial:          23
# CHECK:       Equiv:           0x1C
# CHECK:       Type:            NORMAL
# CHECK: ...
