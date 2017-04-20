/*
 * for TCSS 372 Spring 2017
 *
 * by Mike Nickels and Carter Odem
 *
 * cpu.h
 * Header file for cpu.c
 */

#ifndef CPU_HEADER
#define CPU_HEADER

// Standard C library includes
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <ncurses.h>

// Microstates of the CPU
#define FETCH 0
#define DECODE 1
#define EVAL_ADDR 2
#define FETCH_OP 3
#define EXECUTE 4
#define STORE 5

// Opcodes (sorted by value)
#define BR_OPCODE 0x0
#define ADD_OPCODE 0x1
#define LD_OPCODE 0x2
#define ST_OPCODE 0x3
#define AND_OPCODE 0x5
#define NOT_OPCODE 0x9
#define JMP_OPCODE 0xC
#define TRAP_OPCODE 0xF

// Single bit bitmasks (labeled as 'BIT_#', where the bit is in the 2^# place in binary representation) (add in order as needed)
#define BIT_0 0x1
#define BIT_1 0x2
#define BIT_2 0x4
#define BIT_4 0x8
#define BIT_5 0x20
#define BIT_8 0x100
#define BIT_15 0x8000

// IR field bitshifts
#define OPCODE_SHIFT 0xC	// >> 12
#define RD_SHIFT 0x9		// >> 9
#define RS1_SHIFT 0x6		// >> 6

// IR field bitmasks
#define OPCODE_MASK 0xF000		// 1111 0000 0000 0000
#define RD_MASK 0x0E00			// 0000 1110 0000 0000
#define RS1_MASK 0x01C0			// 0000 0001 1100 0000
#define RS2_MASK 0x0007			// 0000 0000 0000 0111
#define IMMED5_MASK 0x001F		// 0000 0000 0011 1111
#define OFF9_MASK 0x01FF		// 0000 0001 1111 1111
#define TRAPVECTOR_MASK 0x00FF	// 0000 0000 1111 1111

// sext/zext bitmasks (include octets as binary in a comment)
#define SEXT5_MASK 0xFFE0  // 1111 1111 1110 0000
#define SEXT9_MASK 0xFE00  // 1111 1110 0000 0000

// Other constants
#define MEMORY_SIZE 32
#define REGISTER_FILE_SIZE 8

// define a Register as a 16 bit unsigned short
typedef unsigned short Register;

// ALU struct
typedef struct alu_s {
	Register A;
	Register B;
	Register R;
} ALU_s;

// Instruction Register struct (goes in CPU)
typedef struct inst_reg {
	Register ir;
	unsigned short opcode;
	unsigned short rd;
	unsigned short rs1;
	unsigned short rs2;
	unsigned short immed5;
	unsigned short off9;
	unsigned short trapvector;


} INST_REG_s;

// CPU struct
typedef struct cpu_s {
	Register reg_file[REGISTER_FILE_SIZE];
	INST_REG_s ir;
	Register pc;
	Register sext, mar, mdr;
	ALU_s alu;
} CPU_s;
typedef CPU_s* CPU_p;

// function declarations
int controller(CPU_p cpu);
int main(int argc, char* argv[]);
unsigned short parseIR(INST_REG_s* ir);
unsigned short sext5(unsigned short immed5);
unsigned short sext9(unsigned short immed9);
unsigned short zext(unsigned short trapvector);
void gui(CPU_p cpu);

#endif