#include <stdint.h>

#define FALSE 0
#define TRUE  1

/******************************************************************************/
/* MIPS memory layout                                                                                                                                      */
/******************************************************************************/
#define MEM_TEXT_BEGIN  0x00400000
#define MEM_TEXT_END      0x0FFFFFFF
/*Memory address 0x10000000 to 0x1000FFFF access by $gp*/
#define MEM_DATA_BEGIN  0x10010000
#define MEM_DATA_END   0x7FFFFFFF

#define MEM_KTEXT_BEGIN 0x80000000
#define MEM_KTEXT_END  0x8FFFFFFF

#define MEM_KDATA_BEGIN 0x90000000
#define MEM_KDATA_END  0xFFFEFFFF

/*stack and data segments occupy the same memory space. Stack grows backward (from higher address to lower address) */
#define MEM_STACK_BEGIN 0x7FFFFFFF
#define MEM_STACK_END  0x10010000

typedef struct {
	uint32_t begin, end;
	uint8_t *mem;
} mem_region_t;

/* memory will be dynamically allocated at initialization */
mem_region_t MEM_REGIONS[] = {
	{ MEM_TEXT_BEGIN, MEM_TEXT_END, NULL },
	{ MEM_DATA_BEGIN, MEM_DATA_END, NULL },
	{ MEM_KDATA_BEGIN, MEM_KDATA_END, NULL },
	{ MEM_KTEXT_BEGIN, MEM_KTEXT_END, NULL }
};

#define NUM_MEM_REGION 4
#define MIPS_REGS 32

typedef struct CPU_State_Struct {

  uint32_t PC;		                   /* program counter */
  uint32_t REGS[MIPS_REGS]; /* register file. */
  uint32_t HI, LO;                          /* special regs for mult/div. */
} CPU_State;



/***************************************************************/
/* CPU State info.                                                                                                               */
/***************************************************************/

CPU_State CURRENT_STATE, NEXT_STATE;
int RUN_FLAG;	/* run flag*/
uint32_t INSTRUCTION_COUNT;
uint32_t PROGRAM_SIZE; /*in words*/

char prog_file[32];


/***************************************************************/
/* Function Declarations.                                                                                                */
/***************************************************************/
void help();
uint32_t mem_read_32(uint32_t address);
void mem_write_32(uint32_t address, uint32_t value);
void cycle();
void run(int num_cycles);
void runAll();
void mdump(uint32_t start, uint32_t stop) ;
void rdump();
void handle_command();
void reset();
void init_memory();
void load_program();
void initialize();
void print_program();

/************************************************************/
/* decode and execute instruction                                                                     */ 
/************************************************************/
void handle_instruction()
{
	uint32_t instr = mem_read_32(CURRENT_STATE.PC);
	uint32_t opcode = instr >> 26;
	
	uint32_t s_opcode = instr & 0x3F; //gets special op;

	//instruction registers
	int rd  = (instr >> 11) & 0x1F;
	int rt = (instr >> 16) & 0x1F;
	int rs = (instr >> 21) & 0x1F;
	int im = instr & 0xFFFF;
	int off = instr & 0xFFFF;
	int r_opcode = rt; //regimm id
	int tar = instr & 0x3FFFFFF; //target
	int base = rs;
	int sa = (instr >> 6) & 0x1F;

	switch(opcode){
		case 0://special
			switch(s_opcode){
				case 0: //Shift left logical
					printf("SLL\n");
					uint32_t sllval;
					
					sllval = CURRENT_STATE.REGS[rt];
					
					NEXT_STATE.REGS[rd] = sllval << sa;
					
					NEXT_STATE.PC += 4;
					break;
					
				case 2: //Shift right logical
					printf("SRL\n");
					uint32_t srlval;
					
					srlval = CURRENT_STATE.REGS[rt];
					
					NEXT_STATE.REGS[rd] = srlval >> sa;
					
					NEXT_STATE.PC += 4;
					break;
					
				case 3: //Shift right arithmetic 
					printf("SRA\n");
					uint32_t sraval;
					
					sraval = CURRENT_STATE.REGS[rt];
					
					NEXT_STATE.REGS[rd] = sraval >> sa;
					
					NEXT_STATE.PC += 4;
					break;
					
				case 8: //Jump register
					printf("JR\n");
					uint32_t jrval;
					
					jrval = CURRENT_STATE.REGS[rs];
					
					NEXT_STATE.PC = jrval;
					break;
					
				case 9: //Jump and link
					printf("JALR\n");
					int32_t jalrval;
					
					jalrval = CURRENT_STATE.REGS[rs];
					
					NEXT_STATE.REGS[rd] = CURRENT_STATE.PC += 4;
					
					NEXT_STATE.PC = jalrval;
					break;
					
				case 12: //System call
					printf("SYSCALL\n");
					int32_t sysval;
					
					sysval = CURRENT_STATE.REGS[2];
					
					switch( sysval ){
						case 0x0A:
							RUN_FLAG = FALSE;
						default:
							return;
					}	
					break;
					
				case 16: //Move from high
					printf("MFHI\n");
					int32_t mfhival;
					
					mfhival = NEXT_STATE.HI;
					
					NEXT_STATE.REGS[rd] = mfhival;
					
					NEXT_STATE.PC += 4;
					break;
					
				case 17: //Move to high
					printf("MTHI\n");
					int32_t mthival;
					
					mthival = CURRENT_STATE.REGS[rs];
					
					NEXT_STATE.HI = mthival;
					
					NEXT_STATE.PC += 4;
					break;
					
				case 18: //Move from low
					printf("MFLO\n");
					int32_t mfloval;
					
					mfloval = NEXT_STATE.LO;
					
					NEXT_STATE.REGS[rd] = mfloval;
					
					NEXT_STATE.PC += 4;
					break;
					
				case 19: //Move to low
					printf("MTLO\n");
					int32_t mtloval;
					
					mtloval = CURRENT_STATE.REGS[rs];
					
					NEXT_STATE.LO = mtloval;
					
					NEXT_STATE.PC += 4;
					break;
					
				case 24: //Multiply
					printf("MULT\n");
					int32_t mult1, mult2;
					int64_t product;
					
					mult1 = CURRENT_STATE.REGS[rs];
					mult2 = CURRENT_STATE.REGS[rd];
					
					product = mult1 * mult2;
					
					NEXT_STATE.HI = ( product >> 32);
					NEXT_STATE.LO = ( product & 0xFFFFFFFF );
					
					NEXT_STATE.PC += 4;
					break;
					
				case 25: //Multiply unsigned
					printf("MULTU\n");
					uint32_t mult3, mult4;
					uint64_t product2;
					
					mult3 = CURRENT_STATE.REGS[rs];
					mult4 = CURRENT_STATE.REGS[rd];
					
					product2 = mult3 * mult4;
					
					NEXT_STATE.HI = ( product2 >> 32);
					NEXT_STATE.LO = ( product2 & 0xFFFFFFFF );
					
					NEXT_STATE.PC += 4;
					break;
					
				case 26: //Divide
					printf("DIV\n");
					int32_t div1, div2, quotient1, remainder1;
					
					div1 = CURRENT_STATE.REGS[rs];
					div2 = CURRENT_STATE.REGS[rt];
					
					quotient1 = div1 / div2;
					remainder1 = div1 % div2;
					
					NEXT_STATE.HI = remainder1;
					NEXT_STATE.LO = quotient1;
					
					NEXT_STATE.PC += 4;
					break;
					
				case 27: //Divide unsigned
					printf("DIVU\n");
					uint32_t div3, div4, quotient2, remainder2;
					
					div3 = CURRENT_STATE.REGS[rs];
					div4 = CURRENT_STATE.REGS[rt];
					
					quotient2 = div3 / div4;
					remainder2 = div3 % div4;
					
					NEXT_STATE.HI = remainder2;
					NEXT_STATE.LO = quotient2;
					
					NEXT_STATE.PC += 4;
					break;
					
				case 32: //Add
					printf("ADD\n");
					int32_t rs_val, rt_val, sum;
					rs_val = CURRENT_STATE.REGS[rs];
					rt_val = CURRENT_STATE.REGS[rt];
					
					sum = rs_val + rt_val;
					
					NEXT_STATE.REGS[rd] = sum;
					
					NEXT_STATE.PC += 4;
					break;
					
				case 33: //Add unsigned
					printf("ADDU\n");
					
					NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
					
					NEXT_STATE.PC += 4;
					break;
					
				case 34: //Subtract
					printf("SUB\n");
					
					NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
					
					NEXT_STATE.PC += 4;
					break;
					
				case 35: //Subtract unsigned
					printf("SUBU\n");
					
					NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
					
					NEXT_STATE.PC += 4;
					break;
					
				case 36: //AND
					printf("AND\n");
					
					NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] & CURRENT_STATE.REGS[rt];
					
					NEXT_STATE.PC += 4;
					break;
					
				case 37: //OR
					printf("OR\n");
					
					NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt];
					
					NEXT_STATE.PC += 4;
					break;
					
				case 38: //XOR
					printf("XOR\n");
					
					NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] ^ CURRENT_STATE.REGS[rt];
					
					NEXT_STATE.PC += 4;
					break;
					
				case 39: //NOR
					printf("NOR\n");
					
					NEXT_STATE.REGS[rd] = ~( CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt] );
					
					NEXT_STATE.PC += 4;
					break;
					
				case 42: //Set on less than
					printf("SLT\n");
					
					NEXT_STATE.REGS[rd] = ( CURRENT_STATE.REGS[rs] < CURRENT_STATE.REGS[rt] ) ? 1 : 0;
					
					NEXT_STATE.PC += 4;
					break;
					
				default:
					break;
			}
			break;

		case 1: //REGIMM
			switch(r_opcode){
				case 0: //Branch on less than zero
					printf("BLTZ\n");
					if( CURRENT_STATE.REGS[rs] < 0x0 ){
							NEXT_STATE.PC = NEXT_STATE.PC + 4 + off;
					}	
					break;
					
				case 1: //Branch on greater than or equal to
					printf("BGEZ\n");
					if( CURRENT_STATE.REGS[rs] >= 0x0 ){
							NEXT_STATE.PC = NEXT_STATE.PC + 4 + off;
					}		
					break;

				default:
					break;
			}
			break;

		case 2: //Jump
			printf("J\n");
			NEXT_STATE.PC = ( CURRENT_STATE.PC & 0xF0000000 ) | ( tar << 2 );
			break;
			
		case 3: //Jump and link
			printf("JAL\n");
			NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
			NEXT_STATE.PC = ( CURRENT_STATE.PC & 0xF0000000 ) | ( tar << 2 );
			break;
			
		case 4: //Branch on equal
			printf("BEQ\n");
			uint32_t beq1 = CURRENT_STATE.REGS[rs];
			uint32_t beq2 = CURRENT_STATE.REGS[rt];
			
			if( beq1 == beq2 ){
				NEXT_STATE.PC = NEXT_STATE.PC + off;
			}	
			break;
			
		case 5: //Branch not equal
			printf("BNE\n");
			uint32_t bne1 = CURRENT_STATE.REGS[rs];
			uint32_t bne2 = CURRENT_STATE.REGS[rt];
			
			if( bne1 != bne2 ){
				NEXT_STATE.PC = NEXT_STATE.PC + off;
			}	
			break;
			
		case 6: //Branch on less than or equal to zero
			printf("BLEZ\n");
			uint32_t blez = CURRENT_STATE.REGS[rs];
			
			if( blez <= 0x0){
				NEXT_STATE.PC = NEXT_STATE.PC + 4 + off;
			}			
			break;
			
		case 7: //Branch greater than zero
			printf("BGTZ\n");
			uint32_t bgtz = CURRENT_STATE.REGS[rs];
			
			if( bgtz >= 0x0){
				NEXT_STATE.PC = NEXT_STATE.PC + 4 + off;
			}	
			break;
			
		case 8: //Add immediate
			printf("ADDI\n");
			int32_t addi = CURRENT_STATE.REGS[rs];
			
			NEXT_STATE.REGS[rt] = addi + im;
			
			NEXT_STATE.PC += 4;
			break;
			
		case 9: //Add immediate unsigned
			printf("ADDIU\n");
			uint32_t addiu = CURRENT_STATE.REGS[rs];
			
			NEXT_STATE.REGS[rt] = addiu + im;
			
			NEXT_STATE.PC += 4;
			break;
			
		case 10: //Set on less than immediate
			printf("SLTI\n");
			int32_t slti = CURRENT_STATE.REGS[rs];
			
			if( slti < im){
				NEXT_STATE.REGS[rt] = 0x1;
			}else{
					NEXT_STATE.REGS[rt] = 0x0;
			}
				
			NEXT_STATE.PC += 4;	
			break;
			
		case 12: //ANDI
			printf("ANDI\n");
			uint32_t andi = CURRENT_STATE.REGS[rs];
			
			NEXT_STATE.REGS[rt] = andi & im;
			
			NEXT_STATE.PC += 4;
			break;
			
		case 13: //ORI
			printf("ORI\n");
			uint32_t ori = CURRENT_STATE.REGS[rs];
			
			NEXT_STATE.REGS[rt] = ori | im;
			
			NEXT_STATE.PC += 4;
			break;
			
		case 14: //XORI
			printf("XOR\n");
			uint32_t xori = CURRENT_STATE.REGS[rs];
			
			NEXT_STATE.REGS[rt] = xori ^ im;
			
			NEXT_STATE.PC += 4;
			break;
			
		case 15: //Load upper immediate
			printf("LUI\n");
			im <<= 16;
			
			NEXT_STATE.REGS[rt] = im;
			
			NEXT_STATE.PC += 4;
			break;
			
		case 32: //Load byte
			printf("LB\n");
			int32_t lb = CURRENT_STATE.REGS[base];
			
			int32_t lbaddress = lb + off;
			
			NEXT_STATE.REGS[rt] = 0xFF & mem_read_32( lbaddress );
			
			NEXT_STATE.PC += 4;
			break;
			
		case 33: //Load half
			printf("LF\n");
			int32_t lh = CURRENT_STATE.REGS[base];
			
			int32_t lhaddress = lh + off;
			
			NEXT_STATE.REGS[rt] = 0xFFF & mem_read_32( lhaddress );
			
			NEXT_STATE.PC += 4;
			break;
			
		case 35: //Load word
			printf("LW\n");
			int32_t lw = CURRENT_STATE.REGS[base];
			
			int32_t lwaddress = lw + off;
			
			NEXT_STATE.REGS[rt] = mem_read_32( lwaddress );
			
			NEXT_STATE.PC += 4;
			break;
			
		case 40: //Store byte
			printf("SB");
			int32_t sbval1 = CURRENT_STATE.REGS[base];
			uint32_t sbval2 = CURRENT_STATE.REGS[rt];
			
			int32_t sbaddress = sbval1 + off;
			
			uint8_t sbval3 = 0xFF & sbval2;
			
			mem_write_32( sbaddress, sbval3 );
			
			NEXT_STATE.PC += 4;
			break;
			
		case 41: //Store half
			printf("SH\n");
			int32_t shval1 = CURRENT_STATE.REGS[base];
			uint32_t shval2 = CURRENT_STATE.REGS[rt];
			
			int32_t shaddress = shval1 + off;
			
			uint8_t shval3 = 0xFFFF & shval2;
			
			mem_write_32( shaddress, shval3);
			
			NEXT_STATE.PC += 4;
			break;
			
		case 43: //Store word
			printf("SW\n");
			int32_t swval1 = CURRENT_STATE.REGS[base];
			uint32_t swval2 = CURRENT_STATE.REGS[rt];
			
			int32_t swaddress = swval1 + off;
			
			mem_write_32( swaddress, swval2 );
			
			NEXT_STATE.PC += 4;
			break;
			
		default:
			break;

	}
	
}


/************************************************************/
/* Print the instruction at given memory address (in MIPS assembly format)    */
/************************************************************/
void print_instruction(uint32_t addr){
	/*IMPLEMENT THIS*/
	
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

}


