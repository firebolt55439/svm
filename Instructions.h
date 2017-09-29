// Instructions //
#define HLT 0x00 // halt
#define LDA 0x01 // load immediate value into register
#define STO 0x02 // parser transforms STO into LDA
#define LDR 0x03 // load register into other: lda r1, r2 means r1 = r2
#define VAL 0x04 // a "value" byte (could be ascii, number, part of a float, etc.)
// Binary Ops:
#define ADD 0x05 // add r4, r5; --> add value of r4 to r5
#define SUB 0x06 // sub r4, r5 --> r5 -= r4
#define MUL 0x07 // mul r4, r5 --> r5 *= r4
#define DIV 0x08 // div r4, r5 --> r5 /= r4
// Some I/O:
#define PRT 0x09 // print r1 --> only for double registers
#define SPT 0x0A // sprint r1 --> only for string registers
#define DPT 0x0B // dprint 'Hello, World!'
// Jumps (+ Compare Instruction):
#define JMP 0x0C // jump (isp)
#define CMP 0x0D // cmp r1, r2
#define JPE 0x0E // je 'sumer' --> jump if equal
#define JPG 0x0F // jg 'sumer' --> jump if greater
#define JPL 0x10 // jl 'sumer' --> jump if less
#define JNE 0x11 // jne 'sumer' --> jump if not equal
#define JNG 0x12 // if not greater
#define JNL 0x13 // if not less
// Branches (Conditional Jumps):
#define BPE 0x14 // branch if equal
#define BPG 0x15 // if greater
#define BPL 0x16 // less
#define BNE 0x17 // neq
#define BNG 0x18 // ng
#define BNL 0x19 // nl
// Bitwise Ops and Other Binary Ops:
#define XOR 0x1A // exclusive-or
#define BOR 0x1B // binary-or
#define AND 0x1C // and
#define NOT 0x1D // bitwise not (i.e. ~val or val's complement in C)
#define LSC 0x1E // left shift by 1 ( << 1)
#define RSC 0x1F // right shift by 1 ( >> 1)
#define AIN 0x20 // additive inverse (basically the unary minus of a number)
#define MOD 0x21 // modulo (%)
#define INC 0x22 // increment (++)
#define DEC 0x23 // decrement (--)
#define ABS 0x24 // absolute value (|x|)
// Trignometric Functions + Other Math Functions:
#define SIN 0x25 // sine
#define COS 0x26 // cosine
#define TAN 0x27 // tangent
#define EXP 0x28 // exponent
#define SQT 0x29 // square root
#define CEL 0x2A // ceil function
#define FLR 0x2B // floor function
#define ASN 0x2C // arc sine
#define ACS 0x2D // arc cosine
#define ATN 0x2E // arc tangent
#define COT 0x2F // cotangent
#define SEC 0x30 // secant
#define CSC 0x31 // cosecant
// More I/O:
#define GTS 0x32 // "gets" --> take string input from STDIN into string register
#define ATF 0x33 // atof
#define ATI 0x34 // atoi
// Heap:
#define HAL 0x35 // heap allocate (type)
#define HFR 0x36 // heap free (type, heap slot index)
#define HLD 0x37 // load heap value into register (heap slot index)
#define HPS 0x38 // restore heap value from register (given heap slot index)
#define HST 0x39 // heap store value (heap slot index, value)
// Functions:
#define FNC 0x3A // function 'test' for example
#define END 0x3B // end of function
#define RET 0x3C // a "premature" end --> return (a value)
#define CAL 0x3D // call 'test' --> a function call
#define PSH 0x3E // user stack --> push (doubles only)
#define POP 0x3F // user stack --> pop (doubles only)

// "Types" for "type" byte (can be v2 depending on context)
#define DOUBLE 0x01
#define ASCII 0x02
