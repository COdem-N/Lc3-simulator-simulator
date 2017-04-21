// lc3.h

// define states
#define FETCH 0
#define DECODE 1
#define EVAL_ADDR 2
#define FETCH_OP 3
#define EXECUTE 4
#define STORE 5

#define REG_FILE_SIZE 8

#define ADD 0x1
#define AND 0x5
#define BR 0
#define NOT 0x9
#define TRAP 0xF
#define LD 0x2
#define ST 0x3
#define JMP 0xD
// others to follow

#define OPCODE_MASK 0xF000
#define OPCODE_SHIFT 12
#define DR_MASK 0x0E00
#define DR_SHIFT 9
#define SR1_MASK 0x01C0
#define SR1_SHIFT 6
#define SR2_MASK 0x0007
// no SR2 shift needed 
#define IMM5_MASK 0x001F
#define OFFSET6_MASK 0x003F
#define OFFSET9_MASK 0x01FF
#define OFFSET11_MASK 0x07FF
#define TRAP_VECT8_MASK 0x00FF

#define EXT5 0x0010
#define NEG5 0xFFF0
#define EXT9 0x0100
#define NEG9 0xFF00
#define EXT6 0x0020
#define NEG6 0xFF70
#define EXT11 0x0400
#define NEG11 0xFC00

#define BIT5_MASK 0x0020
#define SIGN_BIT_MASK 0x8000
#define NEG_FLAG_MASK 0x0004
#define CC_CLEAR_MASK 0xFFF8
#define ZERO_FLAG_MASK 0x0002
#define POS_FLAG_MASK 0x0001
#define NEG_BIT_MASK 0x0800
#define ZERO_BIT_MASK 0x0400
#define POS_BIT_MASK 0x0200

typedef unsigned short Register;

typedef struct alu_s {
	Register A, B, R;	// the real lc-3 does not have an R register, but needed to hold results in simulation
} ALU_s;

typedef ALU_s * ALU_p;

typedef struct cpu_s {
	ALU_p alu;
	Register reg_file[REG_FILE_SIZE];
	Register pc, ir, mar, mdr, psr;
	Register main_bus;	// used to store data flows
} CPU_s;

typedef CPU_s * CPU_p;


