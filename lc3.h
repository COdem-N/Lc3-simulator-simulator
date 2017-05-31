/*
	lc3.h

	Programmer: 
	Carter Odem 
	Dmitriy Bliznyuk
	Parker Hayden Olive
	Mamadou S Barry

	Date:5-20-2017
*/
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>


// define states
#define FETCH 0
#define DECODE 1
#define EVAL_ADDR 2
#define FETCH_OP 3
#define EXECUTE 4
#define STORE 5

#define REG_FILE_SIZE 8

#define BR 0
#define ADD 0x1
#define LD 0x2
#define ST 0x3
#define JSRR 0X4
#define AND 0x5
#define LDR 0x6
#define STR 0x7
#define NOT 0x9
#define LDI 0xA
#define STI 0xB
#define JMP 0xC

#define LEA 0xE
#define TRAP 0xF

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

#define META_MASK 0XFFFF0000	 // 1111 1111 1111 1111 0000 0000 0000 0000
#define META_SHIFT 16
#define TAG_MASK 0xC000		     // 1100 0000 0000 0000
#define TAG_SHIFT 14
#define INDEX_MASK 	0x3FC0      // 0011 1111 1100 0000
#define INDEX_SHIFT 6
#define BLOCK_OFFSET 0x003F	    // 0000 0000 0011 1111
#define DATA_MASK 0X0000FFFF	// 0000 0000 0000 0000 1111 1111 1111 1111

#define VALID_MASK 0X00010000

#define MSGLINE_X 1
#define MSGLINE_Y 2 
#define USRINPUT 3

#define DEFAULT_X 2

#define CACHE_BLOCK 4
#define CACHE_LINES 256
#define MEMORY_SIZE 1024

typedef unsigned short Register;
typedef unsigned long Cache;


typedef struct alu_s {
	Register A, B, R;	
} ALU_s;
typedef ALU_s * ALU_p;

typedef struct cpu_s {
	ALU_p alu;
	Register reg_file[REG_FILE_SIZE];
	Register pc, ir, mar, mdr, psr;
	Register main_bus;	// used to store data flows
} CPU_s;
typedef CPU_s * CPU_p;

typedef struct debug_res {
	int currentoutpos;
	int bpoint[100];
	int runflag;
	int cachepos;
	WINDOW *com_win;
	WINDOW *reg_win;
	WINDOW *mem_win;
	WINDOW *mes_win;
	WINDOW *ter_win;
	WINDOW *cache_win;
	WINDOW *instcache_win;

} RES;
typedef RES * RES_p;


// Functions
int traproutine( Cache L1[], CPU_p cpu, Register mem[], unsigned int immed_offset, RES_p res);
int textgui(Cache instructL1[], Cache L1[], CPU_p cpu, Register mem[], RES_p res);
void interface_setup(Cache instructL1[], Cache cachemem[], CPU_p cpu, Register mem[], RES_p res);
int controller(Cache instructL1[],Cache L1[], CPU_p cpu, Register mem[], RES_p res);
int setCC(CPU_p cpu);
Register sext(Register immed, int extend);
void writeaccess(Cache cachemem[], CPU_p cpu, Register mem[], RES_p res, unsigned int offset, unsigned short data);
unsigned short readaccess(Cache cachemem[], CPU_p cpu, Register mem[], RES_p res, unsigned int offset);
long getaddress(RES_p res, Register mem[]);

