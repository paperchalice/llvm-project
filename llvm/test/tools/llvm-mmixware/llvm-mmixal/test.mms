% RUN: llvm-mmixal -strict-mode -o - %s | obj2yaml - | FileCheck %s
% A peculiar example of MMIXAL
     LOC   Data_Segment  % location #2000000000000000
     OCTA  1F            % a future reference
a    GREG  @             % $254 is base register for ABCD
ABCD BYTE  "ab"          % two bytes of data
     LOC   #123456789    % switch to the instruction segment
Main JMP   1F            % another future reference
     LOC   @+#4000       % skip past 16384 bytes
2H   LDB   $3,ABCD+1     % use the base register
     BZ    $3,1F; TRAP   % and refer to the future again
# 3 "foo.mms"            % this comment is a line directive
     LOC   2B-4*10       % move 10 tetras before prev loc
1H   JMP   2B            % resolve previous references to 1F
     BSPEC 5             % begin special data of type 5
     TETRA &a<<8         % four bytes of special data
     WYDE  a-$0          % two more bytes of special data
     ESPEC               % end a special data packet
     LOC   ABCD+2        % resume the data segment
     BYTE  "cd",#98      % assemble three more bytes of data

# CHECK: --- !MMO
# CHECK: Preamble:
# CHECK:   Version:         1
# CHECK:   CreatedTime:     {{[0-9]+}}
# CHECK: Segments:
# CHECK:   - OpCode:          LOC
# CHECK:     HighByte:        DATA
# CHECK:     Offset:          0x0
# CHECK:   - Bin:             '000000000000000061620000'
# CHECK:   - OpCode:          LOC
# CHECK:     HighByte:        INSTRUCTION
# CHECK:     Offset:          0x12345678C
# CHECK:   - OpCode:          FILE
# CHECK:     Name:            {{.+}}
# CHECK:     Number:          0
# CHECK:   - OpCode:          LINE
# CHECK:     Number:          8
# CHECK:   - Bin:             F0000000
# CHECK:   - OpCode:          SKIP
# CHECK:     Delta:           0x4000
# CHECK:   - OpCode:          LINE
# CHECK:     Number:          10
# CHECK:   - Bin:             8103FE0142030000
# CHECK:   - OpCode:          LINE
# CHECK:     Number:          11
# CHECK:   - Bin:             '00000000'
# CHECK:   - OpCode:          LOC
# CHECK:     HighByte:        INSTRUCTION
# CHECK:     Offset:          0x12345A768
# CHECK:   - OpCode:          FIXRX
# CHECK:     FixType:         OTHERWISE
# CHECK:     Delta:           -11
# CHECK:   - OpCode:          FIXR
# CHECK:     Delta:           0xFF7
# CHECK:   - OpCode:          FIXO
# CHECK:     HighByte:        0x20
# CHECK:     Offset:          0x0
# CHECK:   - OpCode:          FILE
# CHECK:     Name:            "foo.mms\0"
# CHECK:     Number:          1
# CHECK:   - OpCode:          LINE
# CHECK:     Number:          4
# CHECK:   - Bin:             F000000A
# CHECK:   - OpCode:          SPEC
# CHECK:     Type:            5
# CHECK:   - Bin:             0000020000FE0000
# CHECK:   - OpCode:          LOC
# CHECK:     HighByte:        DATA
# CHECK:     Offset:          0xA
# CHECK:   - Bin:             '00006364'
# CHECK:   - OpCode:          LOC
# CHECK:     HighByte:        DATA
# CHECK:     Offset:          0xC
# CHECK:   - OpCode:          QUOTE
# CHECK:     Value:           '98000000'
# CHECK: Postamble:
# CHECK:   G:               254
# CHECK:   Values:          [ 0x2000000000000008, 0x12345678C ]
# CHECK: SymbolTable:
# CHECK:   IsUTF16:         false
# CHECK:   Symbol:
# CHECK:     - Name:            ':Main'
# CHECK:       Serial:          1
# CHECK:       Equiv:           0x12345678C
# CHECK:       Type:            NORMAL
# CHECK:     - Name:            ':a'
# CHECK:       Serial:          2
# CHECK:       Equiv:           0xFE
# CHECK:       Type:            REGISTER
# CHECK:     - Name:            ':ABCD'
# CHECK:       Serial:          3
# CHECK:       Equiv:           0x2000000000000008
# CHECK:       Type:            NORMAL
# CHECK: ...
