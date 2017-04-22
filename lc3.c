/*
	lc3.c
	
	Programmer: George Mobus
	Date: 4/18/17
	Version: 1.0

	Edited: Carter Odem, Mamadou Barry
	Date: 4/20/2017
	
	Simulates the simulation of the LC-3 computer in Patt & Patel
*/
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include "lc3.h"

Register sext(Register immed, int extend)
{
    if (immed & extend)
    {
	switch (extend)
	{
	case EXT5:
	    immed |= NEG5;
	    break;
	case EXT6:
	    immed |= NEG6;
	    break;
	case EXT9:
	    immed |= NEG9;
	    break;
	case EXT11:
	    immed |= NEG11;
	    break;
	}
    }
    return immed;
}

int printStatus(CPU_p cpu, Register mem[])
{
    //printf("Contents of PC = %04X\n", cpu->pc);
    //printf("Contents of MAR = %04X\n", cpu->mar);
    //printf("Contents of MDR = %04X\n", cpu->mdr);
    //printf("Contents of IR = %04X\n", cpu->ir);
    //printf("Contents of PSR = %04X\n", cpu->psr);
    //printf("Contents of Main Bus = %04X\n", cpu->main_bus);
    int i;
    // for (i=0; i<5; i++) //printf("Contents of register[%02X]: %04X\n", i, cpu->reg_file[i]);
    // for (i=0; i<10; i++) //printf("Contents of memory[%04X]: %04X\n", i, mem[i]);
}

int setCC(CPU_p cpu)
{
    int sign_bit;
    cpu->psr &= CC_CLEAR_MASK; // clear CC flags in PSR
    if (cpu->main_bus == 0)
	cpu->psr |= ZERO_FLAG_MASK;
    sign_bit = (int)(cpu->main_bus & SIGN_BIT_MASK);
    if (sign_bit)
	cpu->psr |= NEG_FLAG_MASK;
    else
	cpu->psr |= POS_FLAG_MASK;
}

int controller(CPU_p cpu, Register mem[])
{
    // check to make sure both pointers are not NULL
    // do any initializations here
    unsigned int opcode;
    unsigned int dr;
    unsigned int sr1;
    unsigned int sr2;
    unsigned int bit5;
    unsigned int immed_offset; // fields for the IR
    unsigned int effective_addr;
    int ben;
    int sign_bit;
    int flag = 0;

    // fill the registers with random numbers to start off in final simulation code.

    int state = FETCH;

    while (flag == 0)
    { // efficient endless loop
	switch (state)
	{

	case FETCH: // microstates 18, 33, 35 in the book
		    //printf("Here in FETCH\n");
		    // microstates
	    flag = textgui(cpu, mem);

	    cpu->mar = cpu->pc;
	    cpu->pc++;
	    cpu->mdr = mem[(cpu->mar - 0x3000) + 1]; // ignore time delays
	    cpu->main_bus = cpu->mdr;
	    cpu->ir = cpu->main_bus;

	    // get memory[PC] into IR - memory is a global array
	    // increment PC
	    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	    // put //printf statements in each state and microstate to see that it is working
	    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	    //	printStatus(cpu, mem);
	    state = DECODE;
	    break;
	case DECODE: //
	    //printf("Here in DECODE\n");
	    opcode = (cpu->ir & OPCODE_MASK) >> OPCODE_SHIFT;
	    // OK to extract registers
	    dr = (cpu->ir & DR_MASK) >> DR_SHIFT;
	    sr1 = (cpu->ir & SR1_MASK) >> SR1_SHIFT;
	    sr2 = cpu->ir & SR2_MASK;
	    bit5 = (cpu->ir & BIT5_MASK ? 1 : 0);

	    ben = (((cpu->ir & NEG_BIT_MASK) >> 11) & ((cpu->psr & NEG_FLAG_MASK) >> 2)) |
		  (((cpu->ir & ZERO_BIT_MASK) >> 11) & ((cpu->psr & ZERO_FLAG_MASK) >> 1)) |
		  (((cpu->ir & POS_BIT_MASK) >> 11) & (cpu->psr & POS_FLAG_MASK));

	    state = EVAL_ADDR;
	    //printf("opcode = %02X, dr = %02X, sr1 = %02X, sr2 = %02X\n", opcode, dr, sr1, sr2);
	    printStatus(cpu, mem);
	    break;
	case EVAL_ADDR: //
			//printf("Here in EVAL_ADDR\n");
	    switch (opcode)
	    { // microstates
	      // compute effective address, e.g. add sext(immed7) to register
	    case ADD:
	    case AND:
		if (bit5)
		{
		    immed_offset = sext(cpu->ir & IMM5_MASK, EXT5);
		}
		break;
	    case NOT: // nothing needed
		break;
	    case TRAP:
		immed_offset = cpu->ir & TRAP_VECT8_MASK; // same as ZEXT
		break;
	    case LD: // these 3 need offset 9 address
	    case ST:
	    case BR:
		immed_offset = sext(cpu->ir & OFFSET9_MASK, EXT9);
		effective_addr = immed_offset + cpu->pc;
		cpu->mar = effective_addr;
		break;
	    case JMP: // nothing needed
		break;
	    }
	    printStatus(cpu, mem);
	    state = FETCH_OP;
	    break;
	case FETCH_OP: // Look at ST. Microstate 23 example of getting a value out of a register
	    //printf("Here in FETCH_OP\n");
	    switch (opcode)
	    {
	    // get operands out of registers into A, B of ALU
	    // or get memory for load instr.
	    case ADD:
	    case AND:
		cpu->alu->A = cpu->reg_file[sr1];
		if (!bit5)
		{
		    cpu->alu->B = cpu->reg_file[sr2];
		}
		else
		    cpu->alu->B = immed_offset; // second operand
		break;
	    case NOT:
		cpu->alu->A = cpu->reg_file[sr1];
		break;
	    case TRAP:
		break;
	    case LD:
		cpu->mdr = mem[cpu->mar - 0x3000];
		break;
	    case ST:
		cpu->mdr = cpu->reg_file[dr]; // in this case dr is actually the source reg
		break;
	    case JMP:
		// nothing
		break;
	    case BR:
		// nothing
		break;
	    }
	    printStatus(cpu, mem);
	    state = EXECUTE;
	    break;
	case EXECUTE: // Note that ST does not have an execute microstate
		      //printf("Here in EXECUTE\n");
	    switch (opcode)
	    {
	    // do what the opcode is for, e.g. ADD
	    // in case of TRAP: call trap(int trap_vector) routine, see below for TRAP x25 (HALT)
	    case ADD:
		cpu->alu->R = cpu->alu->A + cpu->alu->B;
		break;
	    case AND:
		cpu->alu->R = cpu->alu->A & cpu->alu->B;
		break;
	    case NOT:
		cpu->alu->R = ~cpu->alu->A;
		break;
	    case TRAP:
		break;
	    case LD:
		break;
	    case ST:
		break;
	    case JMP:
		cpu->pc = cpu->reg_file[sr1]; // sr1 == base reg.
		break;
	    case BR:
		if (ben)
		    cpu->pc = effective_addr;
		break;
	    }
	    printStatus(cpu, mem);
	    state = STORE;
	    break;
	case STORE: // Look at ST. Microstate 16 is the store to memory
		    //printf("Here in STORE\n");
	    switch (opcode)
	    {
	    // write back to register or store MDR into memory
	    case ADD:
	    case AND:
	    case NOT:
		cpu->main_bus = cpu->alu->R;
		cpu->reg_file[dr] = cpu->main_bus;
		setCC(cpu);
		break;
	    case TRAP:
		break;
	    case LD:
		cpu->main_bus = cpu->mdr;
		cpu->reg_file[dr] = cpu->main_bus;
		setCC(cpu);
		break;
	    case ST:
		mem[cpu->mar - 0x3000] = cpu->mdr;
		break;
	    case JMP:
		// nothing
		break;
	    case BR:
		// nothing
		break;
	    }

	    state = FETCH;

	    break;
	}
    }
}

int textgui(CPU_p cpu, Register mem[])
{
    FILE *file;
    char mesg[] = ">";
    char str[80];
    char filename[80];
    int i = 1;
    int count;
    char *temp;
    int flag = 0;
    unsigned short currentMemLocation = 0;
    unsigned short aMemLocation = currentMemLocation;

    initscr(); /* start the curses mode */

     mvprintw(1, 10, "Welcome To the Lc3 Simulator^2");
    mvprintw(3, 5, "Registers");
    mvprintw(3, 30, "Memory");
    //instructions
    mvprintw(20, 3, "[1.Load]");
    mvprintw(20, 12, "[3.Step]");
    mvprintw(20, 21, "[5.Display Memory] [9.exit]");
	

    while (flag == 0)
    {

	aMemLocation = currentMemLocation;
	move(22, 4);
	clrtoeol();
	mvprintw(22, 4, "%s", mesg);

	//memory
	count = 0;
	i = aMemLocation;


	while (count < 16)
	{
	    move(4 + count, 27);
	    clrtoeol();
	    mvprintw(4 + count, 28, "x%04X:", (mem[0] + aMemLocation));
	    mvprintw(4 + count, 35, "x%04X", mem[i + 1]);
	    aMemLocation++;
	    count++;
	    i++;
	}
	if (cpu->pc - mem[0] + 4 < 20)
	{
	    mvprintw((cpu->pc - mem[0]) + 4, 27, ">");
	}
	//Registers
	i = 0;
	while (i < 8)
	{
	    mvprintw(5 + i, 5, "R%d:", i);
	    mvprintw(5 + i, 12, "x%04X", cpu->reg_file[i]);
	    i++;
	}
	// specialty regesters
	mvprintw(14, 3, "PC:x%04X", cpu->pc);
	mvprintw(14, 15, "IR:x%04X", cpu->ir);
	mvprintw(15, 3, "MDR:x%04X", cpu->mdr);
	mvprintw(15, 15, "MAR:x%04X", cpu->mar);
	mvprintw(16, 3, "A:x%04X", cpu->alu->A);
	mvprintw(16, 15, "B:x%04X", cpu->alu->B);
	mvprintw(17, 3, "CC:");

	int p = cpu->psr & POS_FLAG_MASK;
	int z = (cpu->psr & ZERO_FLAG_MASK) >> 1;
	int n = (cpu->psr & NEG_FLAG_MASK) >> 2;

	mvprintw(17, 7, "N:%d", n);
	mvprintw(17, 11, "P:%d", p);
	mvprintw(17, 15, "Z:%d", z);

	mvprintw(22, 4, "%s", mesg);
	getstr(str);

	if (str[0] == '1')
	{
	    move(23, 4);
	    clrtoeol();
	    mvprintw(23, 4, "Please enter a text file");
	    move(22, 4);
	    clrtoeol();
	    mvprintw(22, 4, "%s", mesg);
	    getstr(filename);

	    file = fopen(filename, "r");
	    if (file)
	    {
		i = 0;
		while (fread(str, 1, 6, file) == 6)
		{
		    mem[i] = strtol(str, &temp, 16);
		    i++;
		}
		move(23, 4);
		clrtoeol();
		mvprintw(23, 4, "File %s loaded successful", filename);
		cpu->pc = mem[0];
	    }
	    else
	    {
		move(23, 4);
		clrtoeol();
		mvprintw(23, 4, "##Error404: File Not Found");
	    }
	}
	else if (str[0] == '3')
	{
	    if (cpu->ir == 0xF025)
	    {
		move(23, 4);
		clrtoeol();
		cpu->pc = mem[0];
		mvprintw(23, 4, "HALT HAS BEEN REACHED PC set to x%04X", cpu->pc);
	    }
	    else
	    {
		move(23, 4);
		clrtoeol();
		mvprintw(23, 4, "Steping through");
	    }

	    return 0;
	}
	else if (str[0] == '5')
	{	
		int set = 0;
    	char *temp;
		move(23, 4);
	    clrtoeol();
	    mvprintw(23, 4, "Please enter a Memory address");
		while(set==0)
		{
		set = 1;
	

	    move(22, 4);
	    clrtoeol();
	    mvprintw(22, 4, "%s", mesg);
	    getstr(str);

	    currentMemLocation = strtol(str, &temp, 16) - mem[0];
	
		if(currentMemLocation > 100)
		{
   		move(23, 4);
	    clrtoeol();
	    mvprintw(23, 4, "Error pls enter valid address");
		set = 0;
		}else
		{
	    move(23, 4);
	    clrtoeol();
	    mvprintw(23, 4, "moving to location %X", currentMemLocation + mem[0]);
		}
		
		}

	}
	else if (str[0] == '9')
	{
		move(23, 4);
	    clrtoeol();
	    mvprintw(23, 4, "Program exiting: press return to exit");
	    move(22, 4);
	    clrtoeol();
	    mvprintw(22, 4, "%s", mesg);
	    getstr(str);
		endwin();
	    return 1;
	}
	else
	{
	    move(23, 4);
	    clrtoeol();
	    mvprintw(23, 4, "Input error try again");
	}
    }
}

int main(int argc, char *argv[])
{
 m
    Register memory[100];

    CPU_p cpu = (CPU_p)malloc(sizeof(CPU_s));
    cpu->alu = (ALU_p)malloc(sizeof(ALU_s));


    controller(cpu, memory);
}
