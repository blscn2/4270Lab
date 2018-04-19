#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mu-mips.h"
#include "instructions.h"
#include "mu-cache.h"

int stallnum = 0;
int flushes = 0;

mem_region_t MEM_REGIONS[] = {
	{ MEM_TEXT_BEGIN, MEM_TEXT_END, NULL },
	{ MEM_DATA_BEGIN, MEM_DATA_END, NULL },
	{ MEM_KDATA_BEGIN, MEM_KDATA_END, NULL },
	{ MEM_KTEXT_BEGIN, MEM_KTEXT_END, NULL }
};

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
	printf("show\t-- print the current content of the pipeline registers\n");
	printf("forward\t-- toggle forwarding\n");
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
	handle_pipeline();
	//CURRENT_STATE = NEXT_STATE;
	CYCLE_COUNT++;
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
	printf("# Cycles Executed\t: %u\n", CYCLE_COUNT);
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
			if (buffer[1] == 'h' || buffer[1] == 'H'){
				show_pipeline();
			}else {
				runAll(); 
			}
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
		case 'f':
		case 'F':
			if (scanf("%d", (int*)&ENABLE_FORWARDING) != 1) {
				break;
			}
				ENABLE_FORWARDING == 0 ? printf("Forwarding OFF\n") : printf("Forwarding ON\n");
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
	
	memset( &IF_ID, 0, sizeof(CPU_Pipeline_Reg) );
	memset( &ID_EX, 0, sizeof(CPU_Pipeline_Reg) );
	memset( &EX_MEM, 0, sizeof(CPU_Pipeline_Reg) );
	memset( &MEM_WB, 0, sizeof(CPU_Pipeline_Reg) );

	
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
/* maintain the pipeline                                                                                           */ 
/************************************************************/
void handle_pipeline()
{
	/*INSTRUCTION_COUNT should be incremented when instruction is done*/
	/*Since we do not have branch/jump instructions, INSTRUCTION_COUNT should be incremented in WB stage */
	
	WB();
	MEM();
	EX();
	ID();
	IF();
	
	DATA_STALL = 0;
	CONTROL_STALL = 0;
	JUMPED = 0;
}

/************************************************************/
/* writeback (WB) pipeline stage:                                                                          */ 
/************************************************************/
void WB()
{
	/*IMPLEMENT THIS*/
	if( MEM_WB.IR != 0){
		switch(MEM_WB.Control){
			case LOAD_TYPE:
				NEXT_STATE.REGS[GET_RT( MEM_WB.IR )] = MEM_WB.LMD;	
			break;
			case STORE_TYPE:
			break;
			case REGISTER_TYPE: 
				switch(MEM_WB.instr_data.type){
					case R_TYPE: 
						NEXT_STATE.REGS[GET_RD( MEM_WB.IR )] = MEM_WB.ALUOutput; 	
					break;
					case I_TYPE: 
						NEXT_STATE.REGS[GET_RT( MEM_WB.IR )] = MEM_WB.ALUOutput; 
					break;
					default:							 					
					break;
				}
				break;
			case SPECIAL_REGISTER_TYPE:
				NEXT_STATE.LO = MEM_WB.ALUOutput;
				NEXT_STATE.HI = MEM_WB.ALUOutput2;
				break;
			case BRANCH_TYPE:		
				if(MEM_WB.instr_data.opcode == 0x00 && MEM_WB.instr_data.funct_code == 0x09){
					NEXT_STATE.REGS[GET_RD( MEM_WB.IR )] = MEM_WB.ALUOutput;			
				}
				else if(MEM_WB.instr_data.opcode == 0x03){
					NEXT_STATE.REGS[31] = MEM_WB.ALUOutput;
				}	
			default:					
				break;
		}
	}
	
	MEM_WB.IR = 0;
   
    
    CURRENT_STATE = NEXT_STATE;
}

/************************************************************/
/* memory access (MEM) pipeline stage:                                                          */ 
/************************************************************/
void MEM()
{
	/*IMPLEMENT THIS*/
	if(EX_MEM.IR != 0 ){


		//To simulate non-cache 
		// 7078 for no cache example
		CYCLE_COUNT += 99;


		//copy values to next phase
		MEM_WB = EX_MEM;
		MEM_WB.IR = EX_MEM.IR;
		MEM_WB.A = EX_MEM.A;
		MEM_WB.B = EX_MEM.B;
		MEM_WB.imm = EX_MEM.imm;
		MEM_WB.ALUOutput = EX_MEM.ALUOutput;
		MEM_WB.ALUOutput2 = EX_MEM.ALUOutput2;
		MEM_WB.Control = EX_MEM.Control;
		MEM_WB.instr_data = EX_MEM.instr_data;

		//access mem by type
		switch(EX_MEM.Control){
			case LOAD_TYPE: 	
				switch(MEM_WB.num_bytes){					
					case BYTE:
						MEM_WB.LMD = 0x00FF & mem_read_32(EX_MEM.ALUOutput);	
					break;
					case HALF_WORD:
						MEM_WB.LMD = 0xFFFF & mem_read_32(EX_MEM.ALUOutput);	
					break;
					case WORD:		
						MEM_WB.LMD = mem_read_32(EX_MEM.ALUOutput);				
					break;
					default: 											
					break;
				}//(*( EX_MEM.instr_data.funct ))( &ID_EX, &EX_MEM );
				break;
			case STORE_TYPE: 	
				switch(EX_MEM.num_bytes){
					case BYTE:		
						mem_write_32(EX_MEM.ALUOutput, 0x00FF & EX_MEM.B); 		
					break;
					case HALF_WORD: 
						mem_write_32(EX_MEM.ALUOutput, 0xFFFF & EX_MEM.B); 		
					break;			
					case WORD: 		
						mem_write_32(EX_MEM.ALUOutput, EX_MEM.B); 				
					break;
					default: 												
					break;				
				}
				break;
			case REGISTER_TYPE:
			case SPECIAL_REGISTER_TYPE:
			case BRANCH_TYPE:
			break;
			default:
			break;
		}
	}
	
	
	EX_MEM.IR = 0;
}

/************************************************************/
/* execution (EX) pipeline stage:                                                                          */ 
/************************************************************/
void EX()
{
	/*IMPLEMENT THIS*/
	if( ID_EX.IR != 0 ) {
		EX_MEM.PC = ID_EX.PC;
		EX_MEM.IR = ID_EX.IR;
		EX_MEM.A = ID_EX.A;
		EX_MEM.B = ID_EX.B;
		EX_MEM.imm = ID_EX.imm;
		EX_MEM.instr_data = ID_EX.instr_data;
	
		// Write new value into ALUOutput
		(*( EX_MEM.instr_data.funct ))( &ID_EX, &EX_MEM );
		
		if( EX_MEM.flush )
        {
            memset( &IF_ID, 0, sizeof(CPU_Pipeline_Reg) );
            memset( &ID_EX, 0, sizeof(CPU_Pipeline_Reg) );
            EX_MEM.flush = 0;
			CURRENT_STATE = NEXT_STATE;
        }
        
	}
	
	ID_EX.IR = 0;
}

/************************************************************/
/* instruction decode (ID) pipeline stage:                                                         */ 
/************************************************************/
void ID()
{
	/*IMPLEMENT THIS*/
	if( IF_ID.IR != 0 ) {
		uint8_t rs, rt;
		int16_t imm;
		
		// Get new values for struct
		rs = GET_RS( IF_ID.IR );
		rt = GET_RT( IF_ID.IR );
		imm = GET_IMM( IF_ID.IR );
        
		// Write new values in struct
        ID_EX.instr_data = mips_instr_decode( IF_ID.IR );
      	ID_EX.IR = 0; 

        //Check for data hazards
		if(!ENABLE_FORWARDING)
        	DATA_STALL = checkDataHazard();
		else
			DATA_STALL = checkForward();

        if( !DATA_STALL )
        {
            // Pass on values
            ID_EX.PC = IF_ID.PC;
            ID_EX.IR = IF_ID.IR;
            
			switch(IF_ID.FORWARDA){
            	case 0: 	
					ID_EX.A = CURRENT_STATE.REGS[rs];	
				break;
				case 1:			
					ID_EX.A = EX_MEM.ALUOutput;			
				break;
				case 2:		
					ID_EX.A = EX_MEM.ALUOutput2;		
				break;
				case 3:		
					ID_EX.A = MEM_WB.ALUOutput;			
				break;
				case 4:		
					ID_EX.A = MEM_WB.ALUOutput2;		
				break;	
				case 5:		
					ID_EX.A = MEM_WB.LMD;				
				break;
				case 6: 	
					ID_EX.A = CURRENT_STATE.REGS[2];	
				break;
				case 7:		
					ID_EX.A = CURRENT_STATE.LO;			
				break;
				case 8:		
					ID_EX.A = CURRENT_STATE.HI;			
				break;		
				default:	
					printf("\nERROR FORWARD A");		
				break;
			}
			switch(IF_ID.FORWARDB){
            	case 0: 	
					ID_EX.B = CURRENT_STATE.REGS[rt];	
				break;
				case 1:		
					ID_EX.B = EX_MEM.ALUOutput;			
				break;
				case 2:		
					ID_EX.B = EX_MEM.ALUOutput2;		
				break;
				case 3:		
					ID_EX.B = MEM_WB.ALUOutput;			
				break;
				case 4:		
					ID_EX.B = MEM_WB.ALUOutput2;		
				break;	
				case 5:		
					ID_EX.B = MEM_WB.LMD;				
				break;
				case 6: 	
					ID_EX.B = CURRENT_STATE.REGS[4];	
				break;
				case 7:		
					ID_EX.B = CURRENT_STATE.LO;			
				break;
				case 8:		
					ID_EX.B = CURRENT_STATE.HI;			
				break;								
				default:	
					printf("\nERROR FORWARD B");		
				break;
			}
            ID_EX.imm = (int32_t)imm;


 			// Check for control hazards
        	CONTROL_STALL = checkControlHazard();
        }
		 
       
	}
	else{
		flushes ++;
		printf("FLUSHING... [%d]\n", flushes);
	}
		
	IF_ID.FORWARDA = 0;
	IF_ID.FORWARDB = 0;
	IF_ID.A = 0;
	IF_ID.B = 0;	
}

/************************************************************/
/* instruction fetch (IF) pipeline stage:                                                              */ 
/************************************************************/
void IF()
{
	/*IMPLEMENT THIS*/
	 if( !DATA_STALL && !CONTROL_STALL)
    {
        // Write new values in struct 
        IF_ID.IR = mem_read_32( CURRENT_STATE.PC );
        IF_ID.PC = CURRENT_STATE.PC;
		if(!JUMPED)
			NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
}

uint8_t isSysCallForward()
{
	return (ID_EX.instr_data.opcode == 0x00) && (ID_EX.instr_data.funct_code == 0x0C);
}

/************************************************************/
//check for data hazards
/************************************************************/
uint8_t checkDataHazard()
{
    uint8_t dest;
	uint8_t 	bMultDivForward = ((EX_MEM.IR != 0) && (EX_MEM.instr_data.opcode == 0x00) 
								&& (((EX_MEM.instr_data.funct_code >= 0x18) && (EX_MEM.instr_data.funct_code <= 0x1B))
								|| (EX_MEM.instr_data.funct_code == 0x11) || (EX_MEM.instr_data.funct_code == 0x13)));
    uint8_t		bSysCallForward = isSysCallForward();
	
	if(bSysCallForward){
		IF_ID.FORWARDA = 6;
		IF_ID.FORWARDB = 6;
	}
	else if(ID_EX.instr_data.funct_code == 0x10){
		IF_ID.FORWARDA = 8;
	}
	else if(ID_EX.instr_data.funct_code == 0x11){
		IF_ID.FORWARDB = 7;
	}
	else if(ID_EX.instr_data.funct_code == 0x12){
		IF_ID.FORWARDA = 7;
	}
	else if(ID_EX.instr_data.funct_code == 0x13){
		IF_ID.FORWARDB = 8;
	}

	if(!bMultDivForward){
		// Check for hazard with execute stage
		if( EX_MEM.instr_data.type == R_TYPE )
		{
		    dest = GET_RD( EX_MEM.IR );
		}
		else if( EX_MEM.instr_data.type == I_TYPE )
		{
		    dest = GET_RT( EX_MEM.IR );
		}
		else if( EX_MEM.instr_data.opcode == 0x03)
			dest = 31;
		else
		    dest = 0;
		
		
		if ( ( dest != 0 ) && ( dest == (bSysCallForward ? 2 : GET_RS( IF_ID.IR ) ) ) )
		    return 1;
		
		if ( ( dest != 0 ) && ( dest == (bSysCallForward ? 4 : GET_RT( IF_ID.IR ) ) ) )
		    return 1;
    }
    else{
		// If ex_mem has a mult or div instruction running and we need to stall for high or low
		if((ID_EX.instr_data.funct_code >= 0x10) && (ID_EX.instr_data.funct_code <= 0x13))
			return 1;
	}

	bMultDivForward = ((MEM_WB.IR!=0) && (MEM_WB.instr_data.opcode == 0x00) 
								&& (((MEM_WB.instr_data.funct_code >= 0x18) && (MEM_WB.instr_data.funct_code <= 0x1B))
								|| (MEM_WB.instr_data.funct_code == 0x11) || (MEM_WB.instr_data.funct_code == 0x13)));

	if(!bMultDivForward){
		// Check for hazazrd with Memory stage
		if( MEM_WB.instr_data.type == R_TYPE )
		{
		    dest = GET_RD( MEM_WB.IR );
		}
		else if( MEM_WB.instr_data.type == I_TYPE )
		{
		    dest = GET_RT( MEM_WB.IR );
		}
		else if( MEM_WB.instr_data.opcode == 0x03)
			dest = 31;
		else
		    dest = 0;
		
		if ( ( dest != 0 ) && ( dest == (bSysCallForward ? 2 : GET_RS( IF_ID.IR ) ) ) )
		    return 1;
		
		if ( ( dest != 0 ) && ( dest == (bSysCallForward ? 4 : GET_RT( IF_ID.IR ) ) ) )
		    return 1;
    }
	else{
		// If MEM_WB has a mult or div instruction running and we need to stall from high or low
		if((ID_EX.instr_data.funct_code >= 0x10) && (ID_EX.instr_data.funct_code <= 0x13))
			return 1;
	}

    return 0;
}

/************************************************************/
// check if forwarding needed
/************************************************************/
uint8_t checkForward()
{
   uint8_t dest;
    
	uint8_t		bSysCallForward = isSysCallForward();

	if(bSysCallForward){
		IF_ID.FORWARDA = 6;
		IF_ID.FORWARDB = 6;
	}
	else if(ID_EX.instr_data.funct_code == 0x10){
		IF_ID.FORWARDA = 8;
	}
	else if(ID_EX.instr_data.funct_code == 0x11){
		IF_ID.FORWARDB = 7;
	}
	else if(ID_EX.instr_data.funct_code == 0x12){
		IF_ID.FORWARDA = 7;
	}
	else if(ID_EX.instr_data.funct_code == 0x13){
		IF_ID.FORWARDB = 8;
	}


	uint8_t bMultDivForward = ((MEM_WB.IR != 0) && (MEM_WB.instr_data.opcode == 0x00) 
								&& (((MEM_WB.instr_data.funct_code >= 0x18) && (MEM_WB.instr_data.funct_code <= 0x1B))
								|| (MEM_WB.instr_data.funct_code == 0x11) || (MEM_WB.instr_data.funct_code == 0x13)));

	if(!bMultDivForward) {
		if( MEM_WB.instr_data.type == R_TYPE )
		{
		    dest = GET_RD( MEM_WB.IR );
		}
		else if( MEM_WB.instr_data.type == I_TYPE )
		{
		    dest = GET_RT( MEM_WB.IR );
		}
		else if( MEM_WB.instr_data.opcode == 0x03)
			dest = 31;
		else
		    dest = 0;
		
		if ( ( dest != 0 ) && ( dest == ( (bSysCallForward ? 2 : GET_RS( IF_ID.IR ) ) ) ) ) {
			if (MEM_WB.Control == LOAD_TYPE){
				IF_ID.FORWARDA = 5;	
			}
			else if(MEM_WB.Control!= STORE_TYPE){	
				IF_ID.FORWARDA = 3;	
			}
		}
		
		if ( ( dest != 0 ) && ( dest == (bSysCallForward ? 4 : GET_RT( IF_ID.IR ) ) ) ){
			if (MEM_WB.Control == LOAD_TYPE){
			}
			else if(MEM_WB.Control!= STORE_TYPE){
			}
		}
	}
	else {
			if(ID_EX.instr_data.funct_code == 0x10){
				IF_ID.FORWARDA = 4;
			}	
			else if(ID_EX.instr_data.funct_code == 0x11){
				IF_ID.FORWARDB = 3;
			}
			else if(ID_EX.instr_data.funct_code == 0x12){
				IF_ID.FORWARDA = 3;
			}
			else if(ID_EX.instr_data.funct_code == 0x13){
				IF_ID.FORWARDB = 4;
			}

	}

	bMultDivForward = ((EX_MEM.IR != 0) && (EX_MEM.instr_data.opcode == 0x00) 
								&& (((EX_MEM.instr_data.funct_code >= 0x18) && (EX_MEM.instr_data.funct_code <= 0x1B))
								|| (EX_MEM.instr_data.funct_code == 0x11) || (EX_MEM.instr_data.funct_code == 0x13)));

	if(!bMultDivForward) {
		if( EX_MEM.instr_data.type == R_TYPE )
		{
		    dest = GET_RD( EX_MEM.IR );
		}
		else if( EX_MEM.instr_data.type == I_TYPE )
		{
		    dest = GET_RT( EX_MEM.IR );
		}
		else if( EX_MEM.instr_data.opcode == 0x03)
			dest = 31;
		else
		    dest = 0;
		
		if ( ( dest != 0 ) && ( dest == ( (bSysCallForward ? 2 : GET_RS( IF_ID.IR ) ) ) ) ){
			if (EX_MEM.Control == LOAD_TYPE){
				return 1;	
			}
			else if(EX_MEM.Control!= STORE_TYPE){	
				IF_ID.FORWARDA = 1;		
			}
		}

		if ( ( dest != 0 ) && ( dest == (bSysCallForward ? 4 : GET_RT( IF_ID.IR ) ) ) ){
			if (EX_MEM.Control == LOAD_TYPE){
				return 1;
			}
			else if(EX_MEM.Control!= STORE_TYPE){	
				IF_ID.FORWARDB = 1;	
			}
		}
	}
	else {
			if(ID_EX.instr_data.funct_code == 0x10){
				IF_ID.FORWARDA = 2;
			}	
			else if(ID_EX.instr_data.funct_code == 0x11){
				IF_ID.FORWARDB = 1;
			}
			else if(ID_EX.instr_data.funct_code == 0x12){
				IF_ID.FORWARDA = 1; 
			}
			else if(ID_EX.instr_data.funct_code == 0x13){
				IF_ID.FORWARDB = 2; 
			}
	}
    return 0;
}

/************************************************************/
// check for control hazard
/************************************************************/
uint8_t checkControlHazard()
{
  	uint8_t fcode = IF_ID.instr_data.funct_code;
    uint8_t opcode = IF_ID.instr_data.opcode;
    
   
    if( checkBranch( opcode, fcode ) > 0 )
        return 1;
    
    return 0;
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
	/*IMPLEMENT THIS*/
}

/************************************************************/
/* Print the current pipeline                                                                                    */ 
/************************************************************/
void show_pipeline(){
	/*IMPLEMENT THIS*/
	printf("Current PC: %u\n", CURRENT_STATE.PC);
	printf("IF_ID.IR: %u\n", IF_ID.IR);
	printf("IF_ID.PC: %u\n", IF_ID.PC);
	
	printf("ID_EX.IR: %u\n", ID_EX.IR);
	printf("ID_EX.A: %u\n", ID_EX.A);
	printf("ID_EX.B: %u\n", ID_EX.B);
	printf("ID_EX.imm: %u\n", ID_EX.imm);
	       
	printf("EX_MEM.IR: %u\n", EX_MEM.IR);
	printf("EX_MEM.A: %u\n", EX_MEM.A);
	printf("EX_MEM.B: %u\n", EX_MEM.B);
	printf("EX_MEM.ALUOutput: %u\n", EX_MEM.ALUOutput);
	       
	printf("MEM_WB.IR: %u\n", MEM_WB.IR);
	printf("MEM_WB.ALUOutput: %u\n", MEM_WB.ALUOutput);
	printf("MEM_WB.LMD: %u\n", MEM_WB.LMD);
}


/***************************************************************/
/* main                                                                                                                                   */
/***************************************************************/
int main(int argc, char *argv[]) {                              
	printf("\n**************************\n");
	printf("Welcome to MU-MIPS SIM...\n");
	printf("**************************\n\n");
	
	if (argc < 2) {
		printf("Error: You should provide input file.\nUsage: %s <input program> \n\n",  argv[0]);
		exit(1);
	}

	cache_misses = 0;
	cache_hits = 0;
	ENABLE_FORWARDING = 0;
	strcpy(prog_file, argv[1]);
	initialize();
	load_program();
	help();
	while (1){
		handle_command();
	}
	return 0;
}
