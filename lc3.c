/*
	lc3.c

	Programmer: 
	Carter Odem 
	Dmitriy Bliznyuk
	Parker Hayden Olive
	Mamadou S Barry

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
	{
		cpu->psr |= ZERO_FLAG_MASK;
	}
	else
	{
		sign_bit = (int)(cpu->main_bus & SIGN_BIT_MASK);
		if (sign_bit)
		cpu->psr |= NEG_FLAG_MASK;
		else
		cpu->psr |= POS_FLAG_MASK;
	}
}

int controller(Cache instructL1[], Cache L1[], CPU_p cpu, Register mem[], RES_p res)
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
    // Fill the registers with random numbers to start off in final simulation code.

    int state = FETCH;

    while (flag == 0)
    { // efficient endless loop

	switch (state)
	{

	case FETCH: // microstates 18, 33, 35 in the book

		    // microstates

	    flag = textgui(instructL1, L1, cpu, mem, res);

	    cpu->mar = cpu->pc;
	    cpu->pc++;
	    cpu->mdr = readaccess(instructL1, cpu, mem, res, (cpu->mar - mem[0] + 1 ));
	    cpu->main_bus = cpu->mdr;
	    cpu->ir = cpu->main_bus;

	    state = DECODE;
	    break;
	case DECODE: //
	    
	
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
	    case STI:
	    case LDI:
			immed_offset = sext(cpu->ir & OFFSET9_MASK, EXT9);
			effective_addr = immed_offset + cpu->pc;
			cpu->mar = effective_addr;
			
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
			cpu->mdr = readaccess(L1, cpu, mem, res, (cpu->mar - 0x3000)); // shouuld this be plus 1?
			res->cachepos  = (cpu->mar - 0x3000 ) % CACHE_LINES;
			break;
	    case LDR:
			cpu->mar = (immed_offset + cpu->reg_file[sr1]) - mem[0] + 1;
			cpu->mdr = readaccess(L1, cpu, mem, res, (cpu->mar)); // should this be - 0x3000 i think its done above?
			res->cachepos  = (cpu->mar) % CACHE_LINES;
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
	    case STI:
	    case LDI:
			cpu->mdr = readaccess(L1, cpu, mem, res, (cpu->mar - 0x3000 + 1));
			 res->cachepos  = (cpu->mar - 0x3000 + 1) % CACHE_LINES;
			cpu->mar = cpu->mdr;

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
			traproutine(L1,cpu, mem, immed_offset, res);
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
			writeaccess(L1, cpu, mem, res, cpu->mar - 0x3000 + 1, cpu->mdr); //mem[cpu->mar - 0x3000] = cpu->mdr;
			setCC(cpu);
			break;
	    case STR:
			writeaccess(L1, cpu, mem, res, cpu->mar, cpu->mdr); //mem[cpu->mar] = cpu->mdr;
			setCC(cpu);
			break;
	    case JMP:
			// nothing
			break;
	    case BR:
			// nothing
			break;
	    case STI:
			cpu->mdr = cpu->reg_file[dr];
			
			writeaccess(L1, cpu, mem, res, (cpu->mar - 0x3000 + 1), cpu->mdr);
			break;
	    case LDI:
			cpu->mdr = readaccess(L1, cpu, mem, res, (cpu->mar - 0x3000 + 1));
			res->cachepos  = (cpu->mar - 0x3000 + 1) % CACHE_LINES;
			cpu->reg_file[dr] = cpu->mdr;
			break;
	    }

	    state = FETCH;

	    break;
	}
    }
}

int textgui(Cache instructL1[], Cache L1[], CPU_p cpu, Register mem[], RES_p res)
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
	unsigned int tag;
	unsigned int index;
	unsigned long buffer;

    if (res->runflag == 1)
    {
	if (res->bpoint[cpu->pc - mem[0]] == -1)
	{
	    mvwprintw(res->mes_win, MSGLINE_Y, 1, "Break Point Reached");
	    res->runflag = 0;
	}
	else
	{
	    return 0;
	}
    }

    // loop for the interface
    while (flag == 0)
    {
	interface_setup(instructL1, L1, cpu, mem, res);

	mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
	wgetstr(res->mes_win, str);
	//clear their input
	mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
	wclrtoeol(res->mes_win);
	box(res->mes_win, 0, 0);
	wrefresh(res->mes_win);

	if (str[0] == '1') // Load .hex file
	{
	    mvwprintw(res->mes_win, MSGLINE_Y, MSGLINE_X, "Please enter a .hex file");
	    mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
	    wrefresh(res->mes_win);
	    wgetstr(res->mes_win, filename);
		//clear input
	    mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");	
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
			

		mvwprintw(res->mes_win, MSGLINE_Y, MSGLINE_X, "File %s loaded", filename);
		wclrtoeol(res->mes_win);
		box(res->mes_win, 0, 0);
		wrefresh(res->mes_win);
		cpu->pc = mem[0];
	    }
	    else
	    {
			mvwprintw(res->mes_win, MSGLINE_Y, MSGLINE_X, "##Error404: File Not Found");
	    }
	}
	else if (str[0] == '2') //Run
	{

	    mvwprintw(res->mes_win, MSGLINE_Y, MSGLINE_X, "Running");
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    res->runflag = 1;
	    return 0;
	}
	else if (str[0] == '3') // Step
	{
	    if (mem[0] == 0)
	    {

		mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
		wclrtoeol(res->mes_win);
		box(res->mes_win, 0, 0);
		mvwprintw(res->mes_win, MSGLINE_Y, MSGLINE_X, "Please enter .hex file before Stepping");
	    }
	    else
	    {
		mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
		wclrtoeol(res->mes_win);
		box(res->mes_win, 0, 0);
		mvwprintw(res->mes_win, MSGLINE_Y, MSGLINE_X, "Please enter a command, just ran x%04X", mem[cpu->pc - mem[0] + 1]);

		wrefresh(res->mes_win);

		return 0;
	    }
	}
	else if (str[0] == '5') // Go to memory
	{

	    currentMemLocation = getaddress(res, mem);

	    mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    mvwprintw(res->mes_win, MSGLINE_Y, MSGLINE_X, "Moved To x%04X, Enter To Continue", currentMemLocation + mem[0]);
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

	    mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");

	    wrefresh(res->mes_win);
	    wgetstr(res->mes_win, str);
	    mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);

	    mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    mvwprintw(res->mes_win, MSGLINE_Y, MSGLINE_X, "Please Enter A Command");
		 wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    wrefresh(res->mes_win);
	}
	else if (str[0] == '7') //Toggle Break point
	{

	    currentMemLocation = getaddress(res, mem);

	    if (res->bpoint[currentMemLocation] == -1)
	    {
		res->bpoint[currentMemLocation] = 0;
	    }
	    else if (res->bpoint[currentMemLocation] == 0)
	    {
		res->bpoint[currentMemLocation] = -1;
	    }
	    mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    mvwprintw(res->mes_win, MSGLINE_Y, MSGLINE_X, "Break set, Please enter a command");
	}
	else if (str[0] == '8') //Edit memory
	{

	    currentMemLocation = getaddress(res, mem);

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

	    mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    mvwprintw(res->mes_win, MSGLINE_Y, MSGLINE_X, "Moving To x%04X Enter New Instruction", currentMemLocation - 16 + mem[0]);
	    wrefresh(res->mes_win);

	    wmove(res->mem_win, 2, 10);
	    wclrtoeol(res->mem_win);
	    box(res->mem_win, 0, 0);
	    mvwprintw(res->mem_win, 2, 10, ">x");

	    wgetnstr(res->mem_win, str, 4);

	    mem[save] = strtol(str, &temp, 16);

	    mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    mvwprintw(res->mes_win, MSGLINE_Y, 1, "Instruction Saved, Please enter a Command");
	    mvwprintw(res->mes_win, USRINPUT, 1, ">");
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    wrefresh(res->mes_win);
	}
	else if (str[0] == '9')
	{
	    mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    mvwprintw(res->mes_win, MSGLINE_Y, MSGLINE_X, "Program exiting: press return to exit");
	    mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
	    wgetstr(res->mes_win, str);
	    //clear their input
	    mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    wrefresh(res->mes_win);
	    endwin();
	    return 1;
	}
	else if (str[0] == '0')// save file
	{
		int start = getaddress(res, mem);
		int end  = getaddress(res, mem);

	    mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    mvwprintw(res->mes_win, MSGLINE_Y, MSGLINE_X, "Enter Name Of .hex File");
		wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
	    wgetstr(res->mes_win, filename);
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    wrefresh(res->mes_win);

	    if (access(filename, F_OK) != -1)
	    {
		//file exists
		mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
		wclrtoeol(res->mes_win);
		box(res->mes_win, 0, 0);
		mvwprintw(res->mes_win, MSGLINE_Y, MSGLINE_X, "File Already Exist, Overwrite file?(y,n)");
		mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
		wgetstr(res->mes_win, str);
		//clear their input
		mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
		wclrtoeol(res->mes_win);
		box(res->mes_win, 0, 0);
		wrefresh(res->mes_win);
		if (str[0] == 'y' || str[0] == 'Y')
		{
		    file = fopen(filename, "w");
		    int i;
		    for (i = start; i < end; i++)
		    {
			fprintf(file, "%04X\n", mem[i]);
		    }

		    fclose(file);

		    mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
		    wclrtoeol(res->mes_win);
		    box(res->mes_win, 0, 0);
		    mvwprintw(res->mes_win, MSGLINE_Y, MSGLINE_X, "File Saved");
			 wclrtoeol(res->mes_win);
		    box(res->mes_win, 0, 0);
		}
		else
		{
		    mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
		    wclrtoeol(res->mes_win);
		    box(res->mes_win, 0, 0);
		    mvwprintw(res->mes_win, MSGLINE_Y, MSGLINE_X, "Please Enter A Command");
		}
	    }
	    else
	    {
		// file doesn't exist
		file = fopen(filename, "w");
		int i;
		for (i = start; i < end; i++)
		{
		    fprintf(file, "%04X\n", mem[i]);
		}

		fclose(file);

		mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
		wclrtoeol(res->mes_win);
		box(res->mes_win, 0, 0);
		mvwprintw(res->mes_win, MSGLINE_Y, MSGLINE_X, "File Saved, Please Enter A Command");
	    }
	}
	else
	{
	    mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
	    wclrtoeol(res->mes_win);
	    box(res->mes_win, 0, 0);
	    mvwprintw(res->mes_win, MSGLINE_Y, MSGLINE_X, "Input error try again");
	}
    }
}

int traproutine( Cache L1[], CPU_p cpu, Register mem[], unsigned int immed_offset, RES_p res)
{
    char str[50];
    int ch;
    int i = 0;
    char c;
    int k = 0;
    int count = 0;
    if (immed_offset == 0x20) // getc
    {
	//mvwprintw(res->ter_win, USRINPUT, res->currentoutpos - 1, ">");
	wmove(res->ter_win, 3, res->currentoutpos);
	ch = wgetch(res->ter_win);
	cpu->reg_file[0] = ch;
	wmove(res->ter_win, 3, res->currentoutpos);
	wclrtoeol(res->ter_win);
	box(res->ter_win, 0, 0);
	wrefresh(res->ter_win);
	
    } else if (immed_offset == 0x19) //push
    {	
		writeaccess(L1, cpu, mem, res, cpu->reg_file[6] - mem[0],cpu->reg_file[0]);
		cpu->reg_file[6]++;
	}
	else if (immed_offset == 0x18) //pop
    {
			cpu->reg_file[6]--;
		cpu->reg_file[0] =  readaccess(L1, cpu, mem, res, cpu->reg_file[6] - mem[0]);
	
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
			c = winch(res->ter_win);
			while (count < 25)
			{
			mvwaddch(res->ter_win, 1, DEFAULT_X + k, c);
			k++;
			wmove(res->ter_win, 2, DEFAULT_X + k);
			c = winch(res->ter_win);
			count++;
			}
			k = 0;
			count = 0;
			wmove(res->ter_win, 2, DEFAULT_X);
			wclrtoeol(res->ter_win);
			box(res->ter_win, 0, 0);
			wmove(res->ter_win, 3, DEFAULT_X);
			c = winch(res->ter_win);
			while (count < 25)
			{
			mvwaddch(res->ter_win, 2, DEFAULT_X + k, c);
			k++;
			wmove(res->ter_win, 3, DEFAULT_X + k);
			c = winch(res->ter_win);
			count++;
			}
			res->currentoutpos = DEFAULT_X; //reset currser for new line
			wmove(res->ter_win, 23, res->currentoutpos);
			wclrtoeol(res->ter_win);
			box(res->ter_win, 0, 0);
		}
		else
		{
			wmove(res->ter_win, USRINPUT, res->currentoutpos);
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
				count = 0;
					k = 0;
				while (count < 25)
				{
					mvwaddch(res->ter_win, 1, DEFAULT_X + k, c);
					k++;
					wmove(res->ter_win, 2, DEFAULT_X + k);
					c = winch(res->ter_win);
					count++;
				}
			
				wmove(res->ter_win, 2, 4);
				wclrtoeol(res->ter_win);
				box(res->ter_win, 0, 0);

				wmove(res->ter_win, 3, 1);
				c = inch();
				count = 0;
				k = 0;
				while (count < 25)
				{
					mvwaddch(res->ter_win, 2,  k, c);
					k++;
					wmove(res->ter_win, 3, k);
					c = winch(res->ter_win);
					count++;
				}
				
				res->currentoutpos = DEFAULT_X; //reset currser for new line

				wmove(res->ter_win, 3, DEFAULT_X);
				wclrtoeol(res->ter_win);
				box(res->ter_win, 0, 0);
				wrefresh(res->ter_win);
				i++;
				str[0] = mem[i];


			}
			else // if not new lnine
			{
				waddch(res->ter_win, str[0]);
				i++;
				str[0] = mem[i];
				res->currentoutpos++;
				wrefresh(res->ter_win);
			}
		}
    }
    else if (immed_offset == 0x25) //HALT
    {
		mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
		wclrtoeol(res->mes_win);
		box(res->mes_win, 0, 0);
		mvwprintw(res->mes_win, MSGLINE_Y, 1, "HALT HAS BEEN REACHED");
		res->runflag = 0;
		cpu->pc--;

		return -1;
    }
    return 0;
}

void interface_setup(Cache instructL1[], Cache cachemem[], CPU_p cpu, Register mem[], RES_p res)
{
    int i = 1;
    int count;
    unsigned short aMemLocation = 0;
	unsigned int tag;
	unsigned int index;
	unsigned long buffer;


    mvprintw(0, 20, "Welcome To the Lc3 Simulator^2");
    mvprintw(26, 30, "Terminal");
    refresh();

    //terminal
    box(res->ter_win, 0, 0);
    wbkgd(res->ter_win, COLOR_PAIR(1));
    wrefresh(res->ter_win);

    //Cache L1
    box(res->cache_win, 0, 0);
    mvwprintw(res->cache_win, 1, 15, "L1 Cache ");
    aMemLocation = res->cachepos - ((res->cachepos - 1) % 16);
	aMemLocation --;
	 i = aMemLocation;
	 
	 
	count = 0;
    while (count < 10)
    {
		wmove(res->cache_win, 2 + count, 0);
		wclrtoeol(res->cache_win);
		mvwprintw(res->cache_win, 2 + count, 4, "x%04X:", (mem[0] + aMemLocation));
		buffer = cachemem[i + 1] & DATA_MASK;
		mvwprintw(res->cache_win, 2 + count, 10, "x%04X", buffer);
		buffer = cachemem[i+2] & DATA_MASK;
		mvwprintw(res->cache_win, 2 + count, 16, "x%04X", buffer);
		buffer = cachemem[i+3] & DATA_MASK;
		mvwprintw(res->cache_win, 2 + count, 22, "x%04X", buffer);
		buffer = cachemem[i+4] & DATA_MASK;
		mvwprintw(res->cache_win, 2 + count, 28, "x%04X", buffer);
		aMemLocation+=4;
		count++;
		i+=4;
    }
    box(res->cache_win, 0, 0);
    wrefresh(res->cache_win);

	//Instruction cache
	box(res->instcache_win, 0, 0);
    mvwprintw(res->instcache_win, 1, 10, "Instruction Cache");
    aMemLocation = cpu->pc - mem[0];
	aMemLocation = aMemLocation - (aMemLocation % 16);
    i = aMemLocation;


    count = 0;
    while (count < 4)
    {
		wmove(res->instcache_win, 2 + count, 0);
		wclrtoeol(res->cache_win);
		mvwprintw(res->instcache_win, 2 + count, 4, "x%04X:", (mem[0] + aMemLocation));
		buffer = instructL1[i+1] & DATA_MASK;
		mvwprintw(res->instcache_win, 2 + count, 10, "x%04X", buffer);
		buffer = instructL1[i + 2] & DATA_MASK;
		mvwprintw(res->instcache_win, 2 + count, 16, "x%04X", buffer);
		buffer = instructL1[i + 3] & DATA_MASK;
		mvwprintw(res->instcache_win, 2 + count, 22, "x%04X", buffer);
		buffer = instructL1[i + 4] & DATA_MASK;
		mvwprintw(res->instcache_win, 2 + count, 28, "x%04X", buffer);
		aMemLocation+=4;
		count++;
		i+=4;
    }
	box(res->instcache_win, 0, 0);
	wrefresh(res->instcache_win);

	//memory
	box(res->mem_win, 0, 0);
	mvwprintw(res->mem_win, 1, 4, "Memory");
	count = 0;
    aMemLocation = cpu->pc - mem[0];
	aMemLocation = aMemLocation - (aMemLocation % 16);
    i = aMemLocation;
    while (count < 16)
    {
		wmove(res->mem_win, 2 + count, 0);
		wclrtoeol(res->mem_win);
		mvwprintw(res->mem_win, 2 + count, 4, "x%04X:", (mem[0] + aMemLocation));
		mvwprintw(res->mem_win, 2 + count, 11, "x%04X", mem[i + 1]);
		if (res->bpoint[i] == -1)
		{
			wattron(res->mem_win,COLOR_PAIR(3));
			wprintw(res->mem_win, "  ");
			wattroff(res->mem_win,COLOR_PAIR(3));
			
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
    mvwprintw(res->com_win, 1, 9, "Commands");
    mvwprintw(res->com_win, 2, 1, "1.Load");
    mvwprintw(res->com_win, 2, 10, "2.Run");
    mvwprintw(res->com_win, 2, 18, "3.Step");
    mvwprintw(res->com_win, 3, 1, "5.Memory");
    mvwprintw(res->com_win, 3, 10, "7.Break");
    mvwprintw(res->com_win, 3, 18, "8.Edit");
    mvwprintw(res->com_win, 4, 1, "9.Exit");
    mvwprintw(res->com_win, 4, 10, "0.Save");
    box(res->com_win, 0, 0);
    wrefresh(res->com_win);

    //Interface
    box(res->mes_win, 0, 0);
    mvwprintw(res->mes_win, 1, 8, "Simulator Message");
    mvwprintw(res->mes_win, USRINPUT, 1, ">");
    wrefresh(res->mes_win);
}

void writeaccess(Cache cachemem[], CPU_p cpu, Register mem[], RES_p res, unsigned int offset, unsigned short data)
{
    //write to cache
    unsigned int tag = offset / CACHE_LINES;
    unsigned int index = offset % CACHE_LINES;
    unsigned long buffer;
	unsigned int valid_bit;

    // Write to Memory
    mem[offset] = data;

    // Write to Cache
    buffer = tag;
    buffer = buffer << (TAG_SHIFT - INDEX_SHIFT);
    buffer = buffer | index;
    buffer = buffer << (META_SHIFT + INDEX_SHIFT);
    buffer 	= buffer | (unsigned long)data;

    cachemem[index] = buffer;
	res->cachepos = index;
	
}

unsigned short readaccess(Cache cachemem[], CPU_p cpu, Register mem[], RES_p res, unsigned int offset)
{
    unsigned int tag = offset / CACHE_LINES;
    unsigned int index = offset % CACHE_LINES;
    unsigned int soffset = (offset - 1) % CACHE_BLOCK;
    unsigned long buffer;
    unsigned int realaddr = offset;
    int i = 0;
	int valid = cachemem[index] & VALID_MASK;
    // see if its in cache


    // go to where it should be
    buffer = cachemem[index];
    //check tag
    buffer = buffer & META_MASK;
    buffer = buffer >> META_SHIFT;
    buffer = buffer & TAG_MASK;
    buffer = buffer >> TAG_SHIFT;

    if (buffer == tag && valid ) // memory is in cache
    {
	buffer = cachemem[index];

	buffer = buffer & DATA_MASK;

	return buffer;
    }
    else // needs to go to memory
    {

	offset = offset - soffset;

	//move memory to cache around location
	for (i = 0; i < CACHE_BLOCK; i++)
	{
	    tag = offset / CACHE_LINES; // get new meta data	
	    index = offset % CACHE_LINES;

	    buffer = tag; // build buffer
	    buffer = buffer << (TAG_SHIFT - INDEX_SHIFT);

	    buffer = buffer | index;
	    buffer = buffer << (META_SHIFT + INDEX_SHIFT);

		buffer = buffer | VALID_MASK;

	    buffer = buffer | (unsigned long)mem[offset];

	    cachemem[index] = buffer;

	    offset ++;
	}


	return mem[realaddr];
    }
}

long getaddress(RES_p res, Register mem[])
{
    int set = 0;
    long result = 0;
    char *temp;
    char str[80];
    mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
    wclrtoeol(res->mes_win);
    box(res->mes_win, 0, 0);
    mvwprintw(res->mes_win, MSGLINE_Y, MSGLINE_X, "Please enter a Memory address");

    while (set == 0)
    {
	set = 1;
	//get user input
	mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
	wrefresh(res->mes_win);
	wgetstr(res->mes_win, str);
	// clear input
	mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
	wclrtoeol(res->mes_win);
	box(res->mes_win, 0, 0);
	wrefresh(res->mes_win);

	result = strtol(str, &temp, 16) - mem[0];

	if (result - mem[0] > 1000) //is the address out of range throw error message and get input again
	{
	    mvwprintw(res->mes_win, USRINPUT, MSGLINE_X, ">");
	    wclrtoeol(res->mes_win);
	    mvwprintw(res->mes_win, MSGLINE_Y, MSGLINE_X, "Error: Enter Valid Address");
	    box(res->mes_win, 0, 0);

	    set = 0;
	}
    }
    return result;
}

int main(int argc, char *argv[])
{
    int i;

    Register memory[MEMORY_SIZE];

    Cache L1[CACHE_LINES];

    Cache instructL1[CACHE_LINES];

    for (i = 0; i < CACHE_LINES; i++)
    {
		memory[i] = 0;
		instructL1[i] = 0;
		L1[i] = 0;
    }

    RES_p res = (RES_p)malloc(sizeof(RES));
    res->currentoutpos = 1;

    for (i = 0; i < 100; i++)
    {
		res->bpoint[i] = 0;
    }

    CPU_p cpu = (CPU_p)malloc(sizeof(CPU_s));
    cpu->alu = (ALU_p)malloc(sizeof(ALU_s));
    initscr(); /* start the curses mode */

	// Window set up ALL window constants are relative to these locations 
    res->com_win = newwin(6, 29, 21, 46);
    res->reg_win = newwin(20, 15, 1, 1);
    res->mem_win = newwin(20, 19, 1, 16);
    res->mes_win = newwin(5, 45, 21, 1);
    res->ter_win = newwin(5, 74, 27, 1);
    res->cache_win = newwin(13, 40, 1, 35);
	res->instcache_win = newwin(7, 40, 14, 35);
	res->cachepos = 1;

    start_color();

    init_pair(1, COLOR_BLACK, COLOR_BLUE);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);
	init_pair(3, COLOR_RED, COLOR_RED);

    mvwprintw(res->mes_win, 2, 1, "Please Enter A Command");

    controller(instructL1, L1, cpu, memory, res);

    endwin();
}
