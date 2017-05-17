/*
	lc3.c

	Programmer: Carter Odem, =
	Date:5-20-2017
	
	Simulates the simulation of the LC-3 computer in Patt & Patel
*/
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

int controller(CPU_p cpu, Register mem[], RES_p res)
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

	    flag = textgui(cpu, mem, res);
	    cpu->mar = cpu->pc;
	    cpu->pc++;
	    cpu->mdr = mem[(cpu->mar - 0x3000) + 1]; // ignore time delays
	    cpu->main_bus = cpu->mdr;
	    cpu->ir = cpu->main_bus;

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

	    ben = (((dr & NEG_FLAG_MASK) >> 2) & ((cpu->psr & NEG_FLAG_MASK) >> 2)) |
		  (((dr & ZERO_FLAG_MASK) >> 1) & ((cpu->psr & ZERO_FLAG_MASK) >> 1)) |
		  ((dr & POS_FLAG_MASK) & (cpu->psr & POS_FLAG_MASK));

	    state = EVAL_ADDR;

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
	    case LDR:
		immed_offset = sext(cpu->ir & OFFSET6_MASK, EXT6);
		break;
	    case LD:
		immed_offset = sext(cpu->ir & OFFSET9_MASK, EXT9);
		effective_addr = immed_offset + cpu->pc + 1;
		cpu->mar = effective_addr;
		break;
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
		immed_offset = cpu->ir & OFFSET11_MASK;
		effective_addr = immed_offset + cpu->pc;
		break;
	    }

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
	    case LDR:
		cpu->mar = (immed_offset + cpu->reg_file[sr1]) - mem[0] + 1;
		cpu->mdr = mem[cpu->mar];
		break;
	    case ST:
		cpu->mdr = cpu->reg_file[dr]; // in this case dr is actually the source reg
		break;
	    case STR:
		cpu->mar = immed_offset + cpu->reg_file[sr1] - mem[0] + 1;
		cpu->mdr = cpu->reg_file[dr];
		break;
	    case JMP:
		// nothing
		break;
	    case BR:
		// nothing
		break;
	    }
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
		traproutine(cpu, mem, immed_offset, res);
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
		cpu->pc = effective_addr;
		break;
	    case BR:
		if (ben)
		    cpu->pc = effective_addr;
		break;
	    }
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
		setCC(cpu);
		break;
	    case LDR:
		cpu->main_bus = cpu->mdr;
		cpu->reg_file[dr] = cpu->main_bus;
		setCC(cpu);
		break;
	    case ST:
		mem[cpu->mar - 0x3000] = cpu->mdr;
		setCC(cpu);
		break;
	    case STR:
		mem[cpu->mar] = cpu->mdr;
		setCC(cpu);
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

int textgui(CPU_p cpu, Register mem[], RES_p res)
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

    if (res->runflag == 1)
    {
	if (res->bpoint[cpu->pc - mem[0]] == -1)
	{
	    mvwprintw(res->mes_win, 2, 1, "Break Point Reached");
	    res->runflag = 0;
	}
	else
	{
	    return 0;
	}
    }

    // loop the Text based gui
    while (flag == 0)
    {
	interface_setup(cpu, mem, res);

	mvwprintw(res->mes_win, userinputline, 1, ">");
	wgetstr(res->mes_win, str);
	//clear their input
	mvwprintw(res->mes_win, userinputline, 1, ">");
	wclrtoeol(res->mes_win);
	box(res->mes_win, 0, 0);
	wrefresh(res->mes_win);

	if (str[0] == '1') // Load .hex file
	{

	    mvwprintw(res->mes_win, 2, 1, "Please enter a .hex file");
	    mvwprintw(res->mes_win, userinputline, 1, ">");

	    wrefresh(res->mes_win);

	    wgetstr(res->mes_win, filename);

	    mvwprintw(res->mes_win, userinputline, 1, ">");
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    wrefresh(res->mes_win);

	    file = fopen(filename, "r");
	    if (file)
	    {
		i = 0;
		while (fread(str, 1, 6, file) == 6)
		{
		    mem[i] = strtol(str, &temp, 16);
		    i++;
		}
		mvwprintw(res->mes_win, 2, 1, "File %s loaded", filename);
		wclrtoeol(res->mes_win);
		box(res->mes_win, 0, 0);
		wrefresh(res->mes_win);
		cpu->pc = mem[0];
	    }
	    else
	    {
		mvwprintw(res->mes_win, 2, 1, "##Error404: File Not Found");
	    }
	}
	else if (str[0] == '2') //Run
	{

	    mvwprintw(res->mes_win, 2, 1, "Running");
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    res->runflag = 1;
	    return 0;
	}
	else if (str[0] == '3') // Step
	{
	    if (mem[0] == 0)
	    {

		wmove(res->mes_win, messageline, 1);
		wclrtoeol(res->mes_win);
		box(res->mes_win, 0, 0);
		mvwprintw(res->mes_win, 2, 1, "Please enter .hex file before Stepping");
	    }
	    else
	    {
		wmove(res->mes_win, messageline, 1);
		wclrtoeol(res->mes_win);
		box(res->mes_win, 0, 0);
		mvwprintw(res->mes_win, 2, 1, "Please enter a command, just ran x%04X", mem[cpu->pc - mem[0] + 1]);

		wrefresh(res->mes_win);

		return 0;
	    }
	}
	else if (str[0] == '5') // Go to memory
	{
	    int set = 0;
	    wmove(res->mes_win, messageline, 1);
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);

	    mvwprintw(res->mes_win, messageline, 1, "Please enter a Memory address");
	    while (set == 0)
	    {
		set = 1;

		mvwprintw(res->mes_win, 3, 1, ">");

		wrefresh(res->mes_win);

		wgetstr(res->mes_win, str);
		mvwprintw(res->mes_win, 3, 1, ">");

		wclrtoeol(res->mes_win);
		box(res->mes_win, 0, 0);
		wrefresh(res->mes_win);

		currentMemLocation = strtol(str, &temp, 16) - mem[0];

		if (currentMemLocation > 100)
		{
		    wmove(res->mes_win, messageline, 1);
		    wclrtoeol(res->mes_win);
		    mvwprintw(res->mes_win, messageline, 1, "Error: Enter Valid Address");
		    box(res->mes_win, 0, 0);
		    set = 0;
		}
		else
		{
		    wmove(res->mes_win, messageline, 1);
		    wclrtoeol(res->mes_win);
		    box(res->mes_win, 0, 0);
		    mvwprintw(res->mes_win, 2, 1, "Moved To x%04X Enter To Continue", currentMemLocation + mem[0]);
		    i = currentMemLocation;
		    count = 0;
		    while (count < 16)
		    {
			wmove(res->mem_win, 2 + count, 0);
			wclrtoeol(res->mem_win);

			mvwprintw(res->mem_win, 2 + count, 4, "x%04X:", (mem[0] + currentMemLocation));
			mvwprintw(res->mem_win, 2 + count, 11, "x%04X", mem[i + 1]);
			currentMemLocation++;
			count++;
			i++;
		    }

		    box(res->mem_win, 0, 0);
		    wrefresh(res->mem_win);

		    mvwprintw(res->mes_win, userinputline, 1, ">");

		    wrefresh(res->mes_win);
		    wgetstr(res->mes_win, str);
		    mvwprintw(res->mes_win, userinputline, 1, ">");
		    wclrtoeol(res->mes_win);
		    box(res->mes_win, 0, 0);

		    wmove(res->mes_win, messageline, 1);
		    wclrtoeol(res->mes_win);
		    box(res->mes_win, 0, 0);
		    mvwprintw(res->mes_win, 2, 1, "Please Enter A Command");
		    wrefresh(res->mes_win);
		}
	    }
	}
	else if (str[0] == '7') //Toggle Break point
	{
	    int set = 0;

	    wmove(res->mes_win, messageline, 1);
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    mvwprintw(res->mes_win, messageline, 1, "Please Enter A Memory address");
	    while (set == 0)
	    {
		set = 1;

		mvwprintw(res->mes_win, userinputline, 1, ">");
		wgetstr(res->mes_win, str);
		//clear their input
		mvwprintw(res->mes_win, userinputline, 1, ">");
		wclrtoeol(res->mes_win);
		box(res->mes_win, 0, 0);
		wrefresh(res->mes_win);

		currentMemLocation = strtol(str, &temp, 16) - mem[0];

		if (currentMemLocation > 99)
		{
		    wmove(res->mes_win, messageline, 1);
		    wclrtoeol(res->mes_win);
		    mvwprintw(res->mes_win, messageline, 1, "Error: Enter Valid Address");
		    box(res->mes_win, 0, 0);
		    set = 0;
		}
		else
		{
		    if (res->bpoint[currentMemLocation] == -1)
		    {
			res->bpoint[currentMemLocation] = 0;
		    }
		    else if (res->bpoint[currentMemLocation] == 0)
		    {
			res->bpoint[currentMemLocation] = -1;
		    }
		    wmove(res->mes_win, messageline, 1);
		    wclrtoeol(res->mes_win);
		    box(res->mes_win, 0, 0);
		    mvwprintw(res->mes_win, 2, 1, "Break set, Please enter a command");
		}
	    }
	}
	else if (str[0] == '8') //Edit memory
	{
	    int set = 0;

	    wmove(res->mes_win, messageline, 1);
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    mvwprintw(res->mes_win, messageline, 1, "Please Enter A Memory Address To Edit");

	    while (set == 0)
	    {
		set = 1;

		mvwprintw(res->mes_win, userinputline, 1, ">");
		wgetstr(res->mes_win, str);
		//clear their input
		mvwprintw(res->mes_win, userinputline, 1, ">");
		wclrtoeol(res->mes_win);
		box(res->mes_win, 0, 0);
		wrefresh(res->mes_win);

		currentMemLocation = strtol(str, &temp, 16) - mem[0];

		if (currentMemLocation > 100)
		{
		    wmove(res->mes_win, messageline, 1);
		    wclrtoeol(res->mes_win);
		    mvwprintw(res->mes_win, messageline, 1, "Error: Enter Valid Address");
		    box(res->mes_win, 0, 0);
		    set = 0;
		}
		else
		{

		    i = currentMemLocation + 1;
		    int save = currentMemLocation + 1;
		    count = 0;
		    while (count < 16)
		    {
			wmove(res->mem_win, 2 + count, 0);
			wclrtoeol(res->mem_win);

			mvwprintw(res->mem_win, 2 + count, 4, "x%04X:", (mem[0] + currentMemLocation));
			mvwprintw(res->mem_win, 2 + count, 11, "x%04X", mem[i + 1]);
			currentMemLocation++;
			count++;
			i++;
		    }

		    wmove(res->mes_win, messageline, 1);
		    wclrtoeol(res->mes_win);
		    box(res->mes_win, 0, 0);
		    mvwprintw(res->mes_win, messageline, 1, "Moving To x%04X Enter New Instruction", currentMemLocation - 16 + mem[0]);
		    wrefresh(res->mes_win);

		    wmove(res->mem_win, 2, 10);
		    wclrtoeol(res->mem_win);
		    box(res->mem_win, 0, 0);
		    mvwprintw(res->mem_win, 2, 10, ">x");

		    wgetnstr(res->mem_win, str, 4);

		    mem[save] = strtol(str, &temp, 16);

		    wmove(res->mes_win, messageline, 1);
		    wclrtoeol(res->mes_win);
		    box(res->mes_win, 0, 0);
		    mvwprintw(res->mes_win, messageline, 1, "Instruction Saved, Please enter a Command");
		    mvwprintw(res->mes_win, userinputline, 1, ">");
		    wclrtoeol(res->mes_win);
		    box(res->mes_win, 0, 0);
		    wrefresh(res->mes_win);
		}
	    }
	}
	else if (str[0] == '9')
	{
	    wmove(res->mes_win, messageline, 1);
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    mvwprintw(res->mes_win, messageline, 1, "Program exiting: press return to exit");
	    mvwprintw(res->mes_win, userinputline, 1, ">");
	    wgetstr(res->mes_win, str);
	    //clear their input
	    mvwprintw(res->mes_win, userinputline, 1, ">");
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    wrefresh(res->mes_win);
	    endwin();
	    return 1;
	}
	else if (str[0] == '0')
	{
	    wmove(res->mes_win, messageline, 1);
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    mvwprintw(res->mes_win, messageline, 1, "Enter Name Of .hex File");
	    mvwprintw(res->mes_win, userinputline, 1, ">");
	    wgetstr(res->mes_win, str);
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    wrefresh(res->mes_win);

	    if (access(filename, F_OK) != -1)
	    {
		//file exists
		wmove(res->mes_win, messageline, 1);
		wclrtoeol(res->mes_win);
		box(res->mes_win, 0, 0);
		mvwprintw(res->mes_win, messageline, 1, "File Already Exist would you like to overwrite?(y,n)");
		mvwprintw(res->mes_win, userinputline, 1, ">");
		wgetstr(res->mes_win, str);
		//clear their input
		mvwprintw(res->mes_win, userinputline, 1, ">");
		wclrtoeol(res->mes_win);
		box(res->mes_win, 0, 0);
		wrefresh(res->mes_win);
		if (str[0] == 'y' || str[0] == 'Y')
		{
		    file = fopen(filename, "w");
		    int i;
		    for (i = 0; i < 100; i++)
		    {
			fprintf(file, "%04X\n", mem[i]);
		    }

		    fclose(file);

		    wmove(res->mes_win, messageline, 1);
		    wclrtoeol(res->mes_win);
		    box(res->mes_win, 0, 0);
		    mvwprintw(res->mes_win, messageline, 1, "File Saved");
		}
		else
		{
		    wmove(res->mes_win, messageline, 1);
		    wclrtoeol(res->mes_win);
		    box(res->mes_win, 0, 0);
		    mvwprintw(res->mes_win, messageline, 1, "Please Enter A Command");
		}
	    }
	    else
	    {
		// file doesn't exist
		file = fopen(filename, "w");
		int i;
		for (i = 0; i < 100; i++)
		{
		    fprintf(file, "%04X\n", mem[i]);
		}

		fclose(file);

		wmove(res->mes_win, messageline, 1);
		wclrtoeol(res->mes_win);
		box(res->mes_win, 0, 0);
		mvwprintw(res->mes_win, messageline, 1, "File Saved");
	    }
	}
	else
	{
	    wmove(res->mes_win, messageline, 1);
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    mvwprintw(res->mes_win, messageline, 1, "Input error try again");
	}
    }
}

int traproutine(CPU_p cpu, Register mem[], unsigned int immed_offset, RES_p res)
{
    char str[50];
    int ch;
    int i = 0;
    char c;
    int k = 0;
    int count = 0;
    if (immed_offset == 0x20) // getc
    {
	//mvwprintw(res->ter_win, userinputline, res->currentoutpos - 1, ">");
	wmove(res->ter_win, 3, res->currentoutpos);
	ch = wgetch(res->ter_win);
	cpu->reg_file[0] = ch;
    }
    else if (immed_offset == 0x21) //out
    {
	count = 0;
	k = 0;
	if (cpu->reg_file[0] == 10) // if  new line
	{
	    wmove(res->ter_win, 1, DEFAULT_X);
	    wclrtoeol(res->ter_win);
	    box(res->ter_win, 0, 0);
	    wmove(res->ter_win, 2, DEFAULT_X);
	    c = inch();
	    while (count < 25)
	    {
		mvwaddch(res->ter_win, 1, DEFAULT_X + k, c);
		k++;
		wmove(res->ter_win, 2, DEFAULT_X + k);
		c = inch();
		count++;
	    }
	    k = 0;
	    count = 0;
	    wmove(res->ter_win, 2, DEFAULT_X);
	    wclrtoeol(res->ter_win);
	    box(res->ter_win, 0, 0);
	    wmove(res->ter_win, 3, DEFAULT_X);
	    c = inch();
	    while (count < 25)
	    {
		mvwaddch(res->ter_win, 2, DEFAULT_X + k, c);
		k++;
		wmove(res->ter_win, 3, DEFAULT_X + k);
		c = inch();
		count++;
	    }
	    res->currentoutpos = DEFAULT_X; //reset currser for new line
	    wmove(res->ter_win, 23, res->currentoutpos);
	    wclrtoeol(res->ter_win);
	    box(res->ter_win, 0, 0);
	}
	else
	{
	    wmove(res->ter_win, 3, res->currentoutpos);
	    wprintw(res->ter_win, "%c", cpu->reg_file[0]);
	    res->currentoutpos++;
	}
    }
    else if (immed_offset == 0x22) //puts
    {

	count = 0;
	i = cpu->reg_file[0] - mem[0] + 1;
	str[0] = mem[i];
	k = 0;

	while (str[0] >= 10) // printing
	{
	    wmove(res->ter_win, 3, res->currentoutpos);
	    if (str[0] == 10) // if  new line
	    {
		wmove(res->ter_win, 1, DEFAULT_X);
		wclrtoeol(res->ter_win);
		box(res->ter_win, 0, 0);
		wmove(res->ter_win, 2, DEFAULT_X);
		c = inch();
		while (count < 25)
		{
		    mvwaddch(res->ter_win, 1, DEFAULT_X + k, c);
		    k++;
		    wmove(res->ter_win, 2, DEFAULT_X + k);
		    c = inch();
		    count++;
		}
		k = 0;
		wmove(res->ter_win, 2, 4);
		wclrtoeol(res->ter_win);
		box(res->ter_win, 0, 0);
		wmove(res->ter_win, 3, 4);
		c = inch();
		while (count < 25)
		{
		    mvwaddch(res->ter_win, 2, DEFAULT_X + k, c);
		    k++;
		    wmove(res->ter_win, 3, DEFAULT_X + k);
		    c = inch();
		    count++;
		}
		res->currentoutpos = DEFAULT_X; //reset currser for new line

		wmove(res->ter_win, 3, res->currentoutpos);
		wclrtoeol(res->ter_win);
		box(res->ter_win, 0, 0);
	    }
	    else // if not new lnine
	    {
		waddch(res->ter_win, str[0]);
		i++;
		str[0] = mem[i];
		res->currentoutpos++;
	    }
	}
    }
    else if (immed_offset == 0x25) //HALT
    {
	wmove(res->mes_win, messageline, 1);
	wclrtoeol(res->mes_win);
	box(res->mes_win, 0, 0);
	mvwprintw(res->mes_win, messageline, 1, "HALT HAS BEEN REACHED");
	res->runflag = 0;

	return -1;
    }
    return 0;
}

void interface_setup(CPU_p cpu, Register mem[], RES_p res)
{
    int i = 1;
    int count;
    unsigned short aMemLocation = 0;

    mvprintw(0, 20, "Welcome To the Lc3 Simulator^2");
    mvprintw(21, 16, "Terminal");

    refresh();

    box(res->mes_win, 0, 0);

    wrefresh(res->mes_win);
    //terminal

    box(res->ter_win, 0, 0);
    wbkgd(res->ter_win, COLOR_PAIR(1));
    wrefresh(res->ter_win);

    //memory

    box(res->mem_win, 0, 0);
    //wbkgd(res->mem_win, COLOR_PAIR(1));

    mvwprintw(res->mem_win, 1, 4, "Memory");

    aMemLocation = 0;
    count = 0;
    i = aMemLocation;

    if (cpu->pc - mem[0] > 15)
    {
	i += 16;
    }
    aMemLocation += i;
    while (count < 16)
    {
	wmove(res->mem_win, 2 + count, 0);
	wclrtoeol(res->mem_win);
	mvwprintw(res->mem_win, 2 + count, 4, "x%04X:", (mem[0] + aMemLocation));
	mvwprintw(res->mem_win, 2 + count, 11, "x%04X", mem[i + 1]);

	if (res->bpoint[i] == -1)
	{
	    wprintw(res->mem_win, "-B");
	}

	aMemLocation++;
	count++;
	i++;
    }

    box(res->mem_win, 0, 0);
    if (mem[0] != 0)
    {
	mvwprintw(res->mem_win, ((cpu->pc - mem[0]) % 16) + 2, 1, "->");
    }

    wrefresh(res->mem_win);

    //Registers
    box(res->reg_win, 0, 0);

    mvwprintw(res->reg_win, 1, 2, "Registers");
    i = 0;
    while (i < 8)
    {
	mvwprintw(res->reg_win, 2 + i, 2, "R%d:", i);
	mvwprintw(res->reg_win, 2 + i, 6, "x%04X", cpu->reg_file[i]);
	i++;
    }

    //specialty regesters
    int p = cpu->psr & POS_FLAG_MASK;
    int z = (cpu->psr & ZERO_FLAG_MASK) >> 1;
    int n = (cpu->psr & NEG_FLAG_MASK) >> 2;

    mvwprintw(res->reg_win, 3 + i, 2, "PC:x%04X", cpu->pc);
    mvwprintw(res->reg_win, 4 + i, 2, "IR:x%04X", cpu->ir);
    mvwprintw(res->reg_win, 5 + i, 2, "MDR:x%04X", cpu->mdr);
    mvwprintw(res->reg_win, 6 + i, 2, "MAR:x%04X", cpu->mar);
    mvwprintw(res->reg_win, 7 + i, 2, "A:x%04X", cpu->alu->A);
    mvwprintw(res->reg_win, 8 + i, 2, "B:x%04X", cpu->alu->B);
    mvwprintw(res->reg_win, 9 + i, 1, "CC:N:%d P:%d", n, p);
    mvwprintw(res->reg_win, 10 + i, 4, "Z:%d", z);
    box(res->reg_win, 0, 0);
    wrefresh(res->reg_win);

    // Command list

    box(res->com_win, 0, 0);

    mvwprintw(res->com_win, 1, 1, "Commands");
    mvwprintw(res->com_win, 3, 1, "1.Load");
    mvwprintw(res->com_win, 4, 1, "2.Run");
    mvwprintw(res->com_win, 5, 1, "3.Step");
    mvwprintw(res->com_win, 6, 1, "5.Memory");
    mvwprintw(res->com_win, 7, 1, "7.Break");
    mvwprintw(res->com_win, 8, 1, "8.Edit");
    mvwprintw(res->com_win, 9, 1, "9.Exit");
    mvwprintw(res->com_win, 10, 1, "0.Save");

    wrefresh(res->com_win);

    //Interface

    //res->mes_win = newwin(5, 30, 1, 35);

    box(res->mes_win, 0, 0);

    mvwprintw(res->mes_win, 1, 8, "Simulator Message");

    mvwprintw(res->mes_win, userinputline, 1, ">");

    wrefresh(res->mes_win);
}

void writeaccess(Cache cachemem[],CPU_p cpu, Register mem[], RES_p res, unsigned int offset, unsigned short data)
{
	//write to cache
	unsigned int tag = offset / CACHE_LINES;
	unsigned int index = offset % CACHE_LINES;
	
	//cachemem[index] =



}

int main(int argc, char *argv[])
{
    int i;
    Register memory[MEMORY_SIZE];

    for (i = 0; i < MEMORY_SIZE; i++)
    {
	memory[i] = 0;
    }

	Cache cmemory[CACHE_LINES];




    RES_p res = (RES_p)malloc(sizeof(RES));

    res->currentoutpos = 2;

    for (i = 0; i < 100; i++)
    {
	res->bpoint[i] = 0;
    }

    CPU_p cpu = (CPU_p)malloc(sizeof(CPU_s));
    cpu->alu = (ALU_p)malloc(sizeof(ALU_s));
    initscr(); /* start the curses mode */

    res->com_win = newwin(12, 10, 6, 35);
    res->reg_win = newwin(20, 15, 1, 1);
    res->mem_win = newwin(20, 19, 1, 16);
    res->mes_win = newwin(5, 45, 1, 35);
    res->ter_win = newwin(5, 50, 22, 1);

    start_color();

    init_pair(1, COLOR_BLACK, COLOR_BLUE);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);

    mvwprintw(res->mes_win, 2, 1, "Please Enter A Command");

    controller(cpu, memory, res);

    endwin();
}
