#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mu-mips.h"

/***************************************************************/
/* Print out a list of commands available                                                                  */
/***************************************************************/
void help() {        
	printf("------------------------------------------------------------------\n\n");
	printf("\t**********MU-MIPS Help MENU**********\n\n");
	printf("sim\t-- simulate program to completion \n");
	printf("run <n>\t-- simulate program for <n> instructions\n");
	printf("rdump\t-- dump register values\n");
	printf("reset\t-- clears all registers/memory and re-loads the program\n");
	printf("input <reg> <val>\t-- set GPR <reg> to <val>\n");
	printf("mdump <start> <stop>\t-- dump memory from <start> to <stop> address\n");
	printf("high <val>\t-- set the HI register to <val>\n");
	printf("low <val>\t-- set the LO register to <val>\n");
	printf("print\t-- print the program loaded into memory\n");
	printf("?\t-- display help menu\n");
	printf("quit\t-- exit the simulator\n\n");
	printf("------------------------------------------------------------------\n\n");
}

/***************************************************************/
/* Read a 32-bit word from memory                                                                            */
/***************************************************************/
uint32_t mem_read_32(uint32_t address)
{
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) &&  ( address <= MEM_REGIONS[i].end) ) {
			uint32_t offset = address - MEM_REGIONS[i].begin;
			return (MEM_REGIONS[i].mem[offset+3] << 24) |
					(MEM_REGIONS[i].mem[offset+2] << 16) |
					(MEM_REGIONS[i].mem[offset+1] <<  8) |
					(MEM_REGIONS[i].mem[offset+0] <<  0);
		}
	}
	return 0;
}

/***************************************************************/
/* Write a 32-bit word to memory                                                                                */
/***************************************************************/
void mem_write_32(uint32_t address, uint32_t value)
{
	int i;
	uint32_t offset;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) && (address <= MEM_REGIONS[i].end) ) {
			offset = address - MEM_REGIONS[i].begin;

			MEM_REGIONS[i].mem[offset+3] = (value >> 24) & 0xFF;
			MEM_REGIONS[i].mem[offset+2] = (value >> 16) & 0xFF;
			MEM_REGIONS[i].mem[offset+1] = (value >>  8) & 0xFF;
			MEM_REGIONS[i].mem[offset+0] = (value >>  0) & 0xFF;
		}
	}
}

/***************************************************************/
/* Execute one cycle                                                                                                              */
/***************************************************************/
void cycle() {                                                
	handle_instruction();
	CURRENT_STATE = NEXT_STATE;
	INSTRUCTION_COUNT++;
}

/***************************************************************/
/* Simulate MIPS for n cycles                                                                                       */
/***************************************************************/
void run(int num_cycles) {                                      
	
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped\n\n");
		return;
	}

	printf("Running simulator for %d cycles...\n\n", num_cycles);
	int i;
	for (i = 0; i < num_cycles; i++) {
		if (RUN_FLAG == FALSE) {
			printf("Simulation Stopped.\n\n");
			break;
		}
		cycle();
	}
}

/***************************************************************/
/* simulate to completion                                                                                               */
/***************************************************************/
void runAll() {                                                     
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped.\n\n");
		return;
	}

	printf("Simulation Started...\n\n");
	while (RUN_FLAG){
		cycle();
	}
	printf("Simulation Finished.\n\n");
}

/***************************************************************/ 
/* Dump a word-aligned region of memory to the terminal                              */
/***************************************************************/
void mdump(uint32_t start, uint32_t stop) {          
	uint32_t address;

	printf("-------------------------------------------------------------\n");
	printf("Memory content [0x%08x..0x%08x] :\n", start, stop);
	printf("-------------------------------------------------------------\n");
	printf("\t[Address in Hex (Dec) ]\t[Value]\n");
	for (address = start; address <= stop; address += 4){
		printf("\t0x%08x (%d) :\t0x%08x\n", address, address, mem_read_32(address));
	}
	printf("\n");
}

/***************************************************************/
/* Dump current values of registers to the teminal                                              */   
/***************************************************************/
void rdump() {                               
	int i; 
	printf("-------------------------------------\n");
	printf("Dumping Register Content\n");
	printf("-------------------------------------\n");
	printf("# Instructions Executed\t: %u\n", INSTRUCTION_COUNT);
	printf("PC\t: 0x%08x\n", CURRENT_STATE.PC);
	printf("-------------------------------------\n");
	printf("[Register]\t[Value]\n");
	printf("-------------------------------------\n");
	for (i = 0; i < MIPS_REGS; i++){
		printf("[R%d]\t: 0x%08x\n", i, CURRENT_STATE.REGS[i]);
	}
	printf("-------------------------------------\n");
	printf("[HI]\t: 0x%08x\n", CURRENT_STATE.HI);
	printf("[LO]\t: 0x%08x\n", CURRENT_STATE.LO);
	printf("-------------------------------------\n");
}

/***************************************************************/
/* Read a command from standard input.                                                               */  
/***************************************************************/
void handle_command() {                         
	char buffer[20];
	uint32_t start, stop, cycles;
	uint32_t register_no;
	int register_value;
	int hi_reg_value, lo_reg_value;

	printf("MU-MIPS SIM:> ");

	if (scanf("%s", buffer) == EOF){
		exit(0);
	}

	switch(buffer[0]) {
		case 'S':
		case 's':
			runAll(); 
			break;
		case 'M':
		case 'm':
			if (scanf("%x %x", &start, &stop) != 2){
				break;
			}
			mdump(start, stop);
			break;
		case '?':
			help();
			break;
		case 'Q':
		case 'q':
			printf("**************************\n");
			printf("Exiting MU-MIPS! Good Bye...\n");
			printf("**************************\n");
			exit(0);
		case 'R':
		case 'r':
			if (buffer[1] == 'd' || buffer[1] == 'D'){
				rdump();
			}else if(buffer[1] == 'e' || buffer[1] == 'E'){
				reset();
			}
			else {
				if (scanf("%d", &cycles) != 1) {
					break;
				}
				run(cycles);
			}
			break;
		case 'I':
		case 'i':
			if (scanf("%u %i", &register_no, &register_value) != 2){
				break;
			}
			CURRENT_STATE.REGS[register_no] = register_value;
			NEXT_STATE.REGS[register_no] = register_value;
			break;
		case 'H':
		case 'h':
			if (scanf("%i", &hi_reg_value) != 1){
				break;
			}
			CURRENT_STATE.HI = hi_reg_value; 
			NEXT_STATE.HI = hi_reg_value; 
			break;
		case 'L':
		case 'l':
			if (scanf("%i", &lo_reg_value) != 1){
				break;
			}
			CURRENT_STATE.LO = lo_reg_value;
			NEXT_STATE.LO = lo_reg_value;
			break;
		case 'P':
		case 'p':
			print_program(); 
			break;
		default:
			printf("Invalid Command.\n");
			break;
	}
}

/***************************************************************/
/* reset registers/memory and reload program                                                    */
/***************************************************************/
void reset() {   
	int i;
	/*reset registers*/
	for (i = 0; i < MIPS_REGS; i++){
		CURRENT_STATE.REGS[i] = 0;
	}
	CURRENT_STATE.HI = 0;
	CURRENT_STATE.LO = 0;
	
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
	
	/*load program*/
	load_program();
	
	/*reset PC*/
	INSTRUCTION_COUNT = 0;
	CURRENT_STATE.PC =  MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/***************************************************************/
/* Allocate and set memory to zero                                                                            */
/***************************************************************/
void init_memory() {                                           
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		MEM_REGIONS[i].mem = malloc(region_size);
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
}

/**************************************************************/
/* load program into memory                                                                                      */
/**************************************************************/
void load_program() {                   
	FILE * fp;
	int i, word;
	uint32_t address;

	/* Open program file. */
	fp = fopen(prog_file, "r");
	if (fp == NULL) {
		printf("Error: Can't open program file %s\n", prog_file);
		exit(-1);
	}

	/* Read in the program. */

	i = 0;
	while( fscanf(fp, "%x\n", &word) != EOF ) {
		address = MEM_TEXT_BEGIN + i;
		mem_write_32(address, word);
		printf("writing 0x%08x into address 0x%08x (%d)\n", word, address, address);
		i += 4;
	}
	PROGRAM_SIZE = i/4;
	printf("Program loaded into memory.\n%d words written into memory.\n\n", PROGRAM_SIZE);
	fclose(fp);
}

/************************************************************/
/* decode and execute instruction                                                                     */ 
/************************************************************/
void handle_instruction()
{
	int i, word, j = 0, specflag = 1;
	uint32_t address;
	/*IMPLEMENT THIS*/
	/* execute one instruction at a time. Use/update CURRENT_STATE and and NEXT_STATE, as necessary.*/
//	int oparray[6];

	for( i=1; i<=32; i++){ 
		address = MEM_TEXT_BEGIN + j;
		word = mem_read_32(address);
		printf("\nRunning instruction %d: 0x%08x \n", i, word );
		
		
	
	

		//here's the code I had started in here, if that's at all useful
	
//		uint32_t instr = mem_read_32(CURRENT_STATE.PC);
//		printf("In handle instruction, 0x%08x\n", word);
		uint32_t opcode = word >> 24;
		if (opcode == 0x00){
			printf("This is a special opcode\n");
			specflag = 1;
			opcode = word << 24;
			printf("Special opcode: 0x%2x\n", opcode);
		}else
			printf("Opcode, 0x%02x\n", opcode);
	

		switch(opcode){
			//ADD & ADDI			
			case 0x20:
			case 0x20000000:
				if(specflag == 1)
				printf("ADD\n");
				else 
				printf("ADDI\n");
				break;

			//ADDU & ADDIU
			case 0x21:
				if(specflag == 1)
				printf("ADDU\n");
				else
				printf("ADDIU\n");
				break;

			//SUB
			case 0x22:
				if(specflag ==1)
				printf("SUB\n");
				break;

			//SUBU
			case 0x23:
				if(specflag == 1)
				printf("SUBU\n");
				break;

			//MULT & BLEZ
			case 0x18:
				if(specflag ==1)
				printf("MULT\n");
				else
				printf("BLEZ\n");
				break;

			//MULTU
			case 0x19:
				if(specflag == 1)
				printf("MULTU\n");
				break;

			//DIV
			case 0x1A:
				if(specflag == 1)
				printf("DIV\n");
				break;

			//DIVU
			case 0x1B:
				if(specflag ==1)
				printf("DIVU\n");
				break;

			//AND
			case 0x24:
				if(specflag == 1)
				printf("AND\n");
				break;

			//ANDI
			case 0x30:
				printf("ANDI\n");
				break;

			//OR	
			case 0x25:
				if(specflag == 1)
				printf("OR\n");
				break;
				
			//ORI
			case 0x34:
				printf("ORI\n");
				break;

			//XOR
			case 0x26:
				if(specflag == 1)
				printf("XOR\n");
				break;

			//XORI
			case 0x38:
				printf("XORI\n");
				break;

			//NOR
			case 0x27:
				if(specflag == 1)
				printf("NOR\n");
				break;

			//SLT
			case 0x2A:
				if(specflag == 1)
				printf("SLT\n");
				break;

			//SLTI
			case 0x28:
				printf("SLTI\n");
				break;

			//SLL
			case 0x00:
				if(specflag == 1)
				printf("SLL\n");
				break;

			//SRL
			case 0x02:
				if(specflag == 1)
				printf("SRL\n");
				break;

			//SRA
			case 0x03:
				if(specflag == 1)
				printf("SRA\n");
				break;

			//LW
			case 0x8c:
				printf("LW\n");
				break;

			//LB
			case 0x80:
				printf("LB\n");
				break;

			//LH
			case 0x88:
				printf("LH\n");
				break;
			//LUI
			case 0x3c:
				printf("LUI\n");
				break;
			//SW
			case 0xac:
				printf("SW\n");
				break;
			//SB
			case 0xA0:
				printf("SB\n");
				break;
			//SH
			case 0xA4:
				printf("SH\n");
			//MFHI & BEQ
			case 0x10:
				if(specflag == 1)
				printf("MFHI\n");
				else
				printf("BEQ\n");
				break;
			//MFLO
			case 0x12:
				if(specflag == 1)
				printf("MFLO\n");
				break;
			//MTHI
			case 0x11:
				if(specflag ==1)
				printf("MTHI\n");
				break;		
			//MTLO
			case 0x13:
				if(specflag ==1)
				printf("MTLO\n");
				break;
			//BNE
			case 0x14:
				printf("BNE\n");
				break;
			//BLTZ or BGEZ
			case 0x04:
				printf("BLTZ or BGEZ\n");
				break;	
			//BGTZ
			case 0x1C:
				printf("BGTZ\n");
				break;
			//J
			case 0x08:
				if(specflag == 1)
				printf("JR\n");
				else
				printf("J\n");
				break;
			//JAL or SYSCALL
			case 0x0c:
				if(specflag == 1)
				printf("SYSCALL\n");
				else
				printf("JAL\n");
				break;
			//JALR
			case 0x09:
				if(specflag ==1)
				printf("JALR\n");
				break;
			//SYSCALL
			
			default:
				break;
	
		}

//		NEXT_STATE.PC += 4;
	

		j += 4;

		specflag = 0;
		RUN_FLAG = FALSE;
	}
}


/************************************************************/
/* Initialize Memory                                                                                                    */ 
/************************************************************/
void initialize() { 
	init_memory();
	CURRENT_STATE.PC = MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/************************************************************/
/* Print the program loaded into memory (in MIPS assembly format)    */ 
/************************************************************/
void print_program(){
	int i;
	uint32_t addr;
	
	for(i=0; i<PROGRAM_SIZE; i++){
		addr = MEM_TEXT_BEGIN + (i*4);
		printf("[0x%x]\t \n", addr);
		print_instruction(addr);
	}
}

/************************************************************/
/* Print the instruction at given memory address (in MIPS assembly format)    */
/************************************************************/
void print_instruction(uint32_t addr){
	/*IMPLEMENT THIS*/
	/*
	uint32_t instr = mem_read_32(addr);
	uint32_t opcode = instr >> 26;
	//printf("\nopcode, 0x%02x\n", opcode);
	uint32_t s_opcode = instr & 0x3F; //get's special op;

	//instruction registers
	int rd  = (instr >> 11) & 0x1F;
	int rt = (instr >> 16) & 0x1F;
	int rs = (instr >> 21) & 0x1F;
	int im = instr & 0xFFFF;
	int off = instr & 0xFFFF;
	int r_opcode = rt; //regimm id
	int tar = instr & 0x2FFFFFF; //target
	int base = rs;
	int sa = (instr >> 6) & 0x1F;
	int code = (instr >> 6) & 0xFFFFF;

	switch(opcode){
		case 0://special
			switch(s_opcode){
				case 0: //SLL
					printf("SLL %d, %d, %d\n", rd, rt, sa);
					break;
				case 2: //SRL
					printf("SRL %d, %d, %d\n", rd, rt, sa);
					break;
				case 3: //SRA
					printf("SRA %d, %d, %d\n", rd, rt, sa);
					break;
				case 8: //JR
					printf("JR %d\n", rs);
					break;
				case 9: //JALR
					if(rd==31){
						printf("JALR %d\n", rs);
					}else
						printf("JALR %d, %d\n", rd, rs);
					break;
				case 12: //SYSCALL
					printf("SYSCALL\n");
					break;
				case 16: //MFHI
					printf("MFHI %d\n", rd);
					break;
				case 17: //MTHI
					printf("MTHI %d\n", rs);
					break;
				case 18: //MFLO
					printf("MFLO %d\n", rd);
					break;
				case 19: //MTLO
					printf("MTLO %d\n", rs);
					break;
				case 24: //MULT
					printf("MULT %d, %d\n", rs, rt);
					break;
				case 25: //MULTU
					printf("MULTU %d, %d\n", rs, rt);
					break;
				case 26: //DIV
					printf("DIV %d, %d\n", rs, rt);
					break;
				case 27: //DIVU
					printf("DIVU %d, %d\n", rs, rt);
					break;
				case 32: //ADD
					printf("ADD %d, %d, %d\n", rd, rs, rt);
					break;
				case 33: //ADDU
					printf("ADDU %d, %d, %d\n", rd, rs, rt);
					break;
				case 34: //SUB
					printf("SUB %d, %d, %d\n", rd, rs, rt);
					break;
				case 35: //SUBU
					printf("SUBU %d, %d, %d\n", rd, rs, rt);
					break;
				case 36: //AND
					printf("AND %d, %d, %d\n", rd, rs, rt);
					break;
				case 37: //OR
					printf("OR %d, %d, %d\n", rd, rs, rt);
					break;
				case 38: //XOR
					printf("XOR %d, %d, %d\n", rd, rs, rt);
					break;
				case 39: //NOR
					printf("NOR %d, %d, %d\n", rd, rs, rt);
					break;
				case 42: //SLT
					printf("SLT %d, %d,%d\n", rd, rs, rt);
					break;
				default:
					break;
			}
			break;

		case 1: //REGIMM
			switch(r_opcode){
				case 0: //BLTZ
					printf("BLTZ %d, %d\n", rs, off);
					break;
				case 1: //BGEZ
					printf("BGEZ %d, %d\n", rs, off);
					break;

				default:
					break;
			}
			break;

		case 2: //J
			printf("J %d\n", tar);
			break;
		case 3: //JAL
			printf("JAL %d\n", tar);
			break;
		case 4: //BEQ
			printf("BEQ %d, %d, %d\n", rs, rt, off);
			break;
		case 5: //BNE
			printf("BNE %d, %d, %d\n", rs, rt, off);
			break;
		case 6: //BLEZ
			printf("BLEZ %d, %d\n", rs, off);
			break;
		case 7: //BGTZ
			printf("BGTZ %d, %d\n", rs, off);
			break;
		case 8: //ADDI
			printf("ADDI %d, %d, %d\n", rt, rs, im);
			break;
		case 9: //ADDIU
			printf("ADDIU %d, %d, %d\n", rt, rs, im);
			break;
		case 10: //SLTI
			printf("SLTI %d, %d, %d\n", rt, rs, im);
			break;
		case 12: //ANDI
			printf("ANDI %d, %d, %d\n", rt, rs, im);
			break;
		case 13: //ORI
			printf("ORI %d, %d, %d\n", rt, rs, im);
			break;
		case 14: //XOR
			printf("XOR %d, %d, %d\n", rt, rs, im);
			break;
		case 15: //LUI
			printf("LUI %d, %d\n", rt, im);
			break;
		case 32: //LB
			printf("LB %d, %d(%d)\n", rt, off, base);
			break;
		case 33: //LH
			printf("LF %d, %d(%d)\n", rt, off, base);
			break;
		case 35: //LW
			printf("LW %d, %d(%d)\n", rt, off, base);
			break;
		case 40: //SB
			printf("SB %d, %d(%d)\n", rt, off, base);
			break;
		case 41: //SH
			printf("SH %d, %d(%d)\n", rt, off, base);
			break;
		case 43: //SW
			printf("SW %d, %d(%d)\n", rt, off, base);
			break;
		default:
			break;

	}
<<<<<<< 730909de5d22b5b5c45bd4923c51b35ad62c67e7
	*/
=======

>>>>>>> Print_instruction complete
}

/***************************************************************/
/* Main */      
/***************************************************************/
int main(int argc, char *argv[]) {                              
	printf("\n**************************\n");
	printf("Welcome to MU-MIPS SIM...\n");
	printf("**************************\n\n");
	
	if (argc < 2) {
		printf("Error: You should provide input file.\nUsage: %s <input program> \n\n",  argv[0]);
		exit(1);
	}

	strcpy(prog_file, argv[1]);
	initialize();
	load_program();
	help();
	while (1){
		handle_command();
	}
	return 0;
}
