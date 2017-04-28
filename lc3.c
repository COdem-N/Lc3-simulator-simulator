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
#include <unistd.h>
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
	    case LEA:
		immed_offset = sext(cpu->ir & OFFSET9_MASK, EXT9);
		effective_addr = immed_offset + cpu->pc;
		break;
	    case LD:
	    case ST:
	    case BR:
		immed_offset = sext(cpu->ir & OFFSET9_MASK, EXT9);
		effective_addr = immed_offset + cpu->pc;
		cpu->mar = effective_addr;
		break;
		case STR:
		immed_offset = cpu->ir & OFFSET6_MASK;
		break;
	    case JMP: // nothing needed
		break;
	    case JSRR:
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
		cpu->reg_file[7] = cpu->pc;
		break;
	    case LD:
		cpu->mdr = mem[cpu->mar - 0x3000];
		break;
	    case ST:
		cpu->mdr = cpu->reg_file[dr]; // in this case dr is actually the source reg
		break;
		case STR:
		cpu->mar = immed_offset + cpu->reg_file[sr1];
		cpu->mdr = dr;
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
		traproutine(cpu, mem, immed_offset);
		break;
	    case LD:
		break;
	    case ST:
		break;
	    case JMP:
		cpu->pc = cpu->reg_file[sr1]; // sr1 == base reg.
		break;
	    case JSRR:
		cpu->reg_file[7] = cpu->pc;
		cpu->pc = cpu->reg_file[sr1];
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
	    case LEA:
		cpu->reg_file[dr] = effective_addr;
		break;
	    case ST:
		mem[cpu->mar - 0x3000] = cpu->mdr;
		break;
		case STR:
		mem[cpu->mar] = cpu->mdr;
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
    unsigned short aMemLocation = 0;

    initscr(); /* start the curses mode */

    mvprintw(0, 10, "Welcome To the Lc3 Simulator^2");
    mvprintw(1, 5, "Registers");
    mvprintw(1, 30, "Memory");
    move(20, 4);
    clrtoeol();
    mvprintw(20, 4, "Please enter a command");

    //instructions
    mvprintw(18, 3, "[1.Load]");
    mvprintw(18, 12, "[3.Step]");
    mvprintw(18, 21, "[5.Display Memory] [9.exit]");

    // loop the Text based gui
    while (flag == 0)
    {

	aMemLocation = 0;

	//memory
	count = 0;
	i = aMemLocation;
	if (cpu->pc - mem[0] > 15)
	{
	    i += 16;
	}
	aMemLocation += i;
	while (count < 16)
	{
	    move(2 + count, 27);
	    clrtoeol();
	    mvprintw(2 + count, 28, "x%04X:", (mem[0] + aMemLocation));
	    mvprintw(2 + count, 35, "x%04X", mem[i + 1]);
	    aMemLocation++;
	    count++;
	    i++;
	}

	mvprintw(((cpu->pc - mem[0]) % 16) + 2, 27, ">");

	//Registers
	i = 0;
	while (i < 8)
	{
	    mvprintw(2 + i, 5, "R%d:", i);
	    mvprintw(2 + i, 12, "x%04X", cpu->reg_file[i]);
	    i++;
	}

	// specialty regesters
	mvprintw(13, 3, "PC:x%04X", cpu->pc);
	mvprintw(13, 15, "IR:x%04X", cpu->ir);
	mvprintw(14, 3, "MDR:x%04X", cpu->mdr);
	mvprintw(14, 15, "MAR:x%04X", cpu->mar);
	mvprintw(15, 3, "A:x%04X", cpu->alu->A);
	mvprintw(15, 15, "B:x%04X", cpu->alu->B);
	mvprintw(16, 3, "CC:");
	int p = cpu->psr & POS_FLAG_MASK;
	int z = (cpu->psr & ZERO_FLAG_MASK) >> 1;
	int n = (cpu->psr & NEG_FLAG_MASK) >> 2;
	mvprintw(16, 7, "N:%d", n);
	mvprintw(16, 11, "P:%d", p);
	mvprintw(16, 15, "Z:%d", z);

	// get user input for instructions
	move(19, 4);
	clrtoeol();
	mvprintw(19, 4, "%s", mesg);
	getstr(str);

	if (str[0] == '1')
	{
	    move(20, 4);
	    clrtoeol();
	    mvprintw(20, 4, "Please enter a .hex file");
	    move(19, 4);
	    clrtoeol();
	    mvprintw(19, 4, "%s", mesg);
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
		move(20, 4);
		clrtoeol();
		mvprintw(20, 4, "File %s loaded successful", filename);
		cpu->pc = mem[0];
	    }
	    else
	    {
		move(20, 4);
		clrtoeol();
		mvprintw(20, 4, "##Error404: File Not Found");
	    }
	}
	else if (str[0] == '3')
	{

	    move(20, 4);
	    clrtoeol();
	    mvprintw(20, 4, "COPMUTING:  x%04X", mem[cpu->pc - mem[0] + 1]);

	    move(21, 4);
	    refresh();
	    sleep(1);
	    return 0;
	}
	else if (str[0] == '5')
	{
	    int set = 0;
	    char *temp;
	    move(20, 4);
	    clrtoeol();
	    mvprintw(20, 4, "Please enter a Memory address");
	    while (set == 0)
	    {
		set = 1;

		move(19, 4);
		clrtoeol();
		mvprintw(19, 4, "%s", mesg);
		getstr(str);

		currentMemLocation = strtol(str, &temp, 16) - mem[0];

		if (currentMemLocation > 100)
		{
		    move(20, 4);
		    clrtoeol();
		    mvprintw(20, 4, "Error pls enter valid address");
		    set = 0;
		}
		else
		{
		    move(20, 4);
		    clrtoeol();
		    mvprintw(20, 4, "moving to location %X press return to continue", currentMemLocation + mem[0]);
		    i = currentMemLocation;
		    count = 0;
		    while (count < 16)
		    {
			move(2 + count, 27);
			clrtoeol();
			mvprintw(2 + count, 28, "x%04X:", (mem[0] + currentMemLocation));
			mvprintw(2 + count, 35, "x%04X", mem[i + 1]);
			currentMemLocation++;
			count++;
			i++;
		    }

		    move(19, 4);
		    clrtoeol();
		    mvprintw(19, 4, "%s", mesg);
		    getstr(str);
		}
	    }
	}
	else if (str[0] == '9')
	{
	    move(20, 4);
	    clrtoeol();
	    mvprintw(20, 4, "Program exiting: press return to exit");
	    move(19, 4);
	    clrtoeol();
	    mvprintw(19, 4, "%s", mesg);
	    getstr(str);
	    endwin();
	    return 1;
	}
	else
	{
	    move(20, 4);
	    clrtoeol();
	    mvprintw(20, 4, "Input error try again");
	}
    }
}

void traproutine(CPU_p cpu, Register mem[], unsigned int immed_offset)
{
    char str[50];
    unsigned int ch;
    int i = 0;
    if (immed_offset == 0x20) // getc
    {

	mvprintw(23, 4, ">");
	ch = getch();
	cpu->reg_file[0] = ch;
	mvprintw(23, 4, ">");
	clrtoeol();
    }
    else if (immed_offset == 0x21) //out
    {

	str[0] = mem[cpu->reg_file[0] - mem[0]];
	//guioutput(cpu, mem, str);
	printw("%c", str[0]);
    }
    else if (immed_offset == 0x22) //puts
    {

	i = cpu->reg_file[0] - mem[0] + 1;
	str[0] = mem[i];

	printw("output: ");

	while (str[0] != 0)
	{
	    printw("%c", str[0]);

	    i++;
	    str[0] = mem[i];
	}
    }
    else if (immed_offset == 0x25) //HALT
    {
	move(20, 4);
	clrtoeol();
	mvprintw(20, 4, "HALT HAS BEEN REACHED");
    }
}
void guioutput(CPU_p cpu, Register mem[], char *str, int y, int x)
{
    mvprintw(y, x, "%s", str);
}
int main(int argc, char *argv[])
{

    Register memory[100];

    CPU_p cpu = (CPU_p)malloc(sizeof(CPU_s));
    cpu->alu = (ALU_p)malloc(sizeof(ALU_s));

    controller(cpu, memory);
}
