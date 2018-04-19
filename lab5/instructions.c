#include <stdio.h>
#include <stdint.h>

#include "instructions.h"
#include "mu-mips.h"
#include "mu-cache.h"


/************************************************************/
// instructions
/************************************************************/

void instr_handler_JR(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("JR\n");
	int32_t rs_val;

	rs_val = (*ID_EX).A;

	NEXT_STATE.PC = rs_val;

	(*EX_MEM).Control = BRANCH_TYPE;
	(*EX_MEM).flush = 1;
	setJump();
}


void instr_handler_JALR(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("JALR\n");
	int32_t rs_val;

	rs_val =(*ID_EX).A;

	NEXT_STATE.PC = rs_val;
	(*EX_MEM).ALUOutput = (*ID_EX).PC + 4;
	(*EX_MEM).Control = BRANCH_TYPE;
	(*EX_MEM).flush = 1;
	setJump();
}

void instr_handler_BLTZ(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("BLTZ\n");
	if( (int32_t)((*ID_EX).A) < 0x0 )
	{
		NEXT_STATE.PC = ( (*ID_EX).PC + 4*((int16_t)(*ID_EX).imm));
		(*EX_MEM).flush = 1;
		setJump();
	}
	else
	{
		NEXT_STATE.PC += 4;
		(*EX_MEM).flush = 0;
	}

	(*EX_MEM).Control = BRANCH_TYPE;
}

void instr_handler_BGEZ(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("BGEZ\n");
	if( (int32_t)((*ID_EX).A) >= 0x0 )
	{
		NEXT_STATE.PC = ( (*ID_EX).PC + 4*((int16_t)(*ID_EX).imm));
		(*EX_MEM).flush = 1;
		setJump();
	}
	else
	{
		NEXT_STATE.PC += 4;
		(*EX_MEM).flush = 0;
	}

	(*EX_MEM).ALUOutput = (*ID_EX).PC + 4;
	(*EX_MEM).Control = BRANCH_TYPE;
}


void instr_handler_J(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("J\n");
	uint32_t instr = mem_read_32( (*ID_EX).PC );
	uint32_t target = GET_ADDRESS( instr );
	NEXT_STATE.PC = ( (*ID_EX).PC & 0xF0000000 ) | ( target << 2 );
	(*EX_MEM).flush = 1;
}


void instr_handler_JAL(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("JAL\n");
	uint32_t instr = mem_read_32( (*ID_EX).PC );
	uint32_t target = GET_ADDRESS( instr );
	NEXT_STATE.PC = ( (*ID_EX).PC & 0xF0000000 ) | ( target << 2 );

	(*EX_MEM).ALUOutput = (*ID_EX).PC + 4;
	(*EX_MEM).Control = BRANCH_TYPE;
	(*EX_MEM).flush = 1;
	setJump();
}

void instr_handler_BEQ(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("BEQ\n");
	if( (*ID_EX).A == (*ID_EX).B )
	{
		NEXT_STATE.PC = ( (*ID_EX).PC + 4*((int16_t)(*ID_EX).imm));
		(*EX_MEM).flush = 1;
		setJump();
	}
	else
	{
		NEXT_STATE.PC += 4;
		(*EX_MEM).flush = 0;
	}
	(*EX_MEM).Control = BRANCH_TYPE;
}

void instr_handler_BNE(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("BNE\n");
	if( (*ID_EX).A != (*ID_EX).B )
	{
		NEXT_STATE.PC = ( (*ID_EX).PC + 4*((int16_t)(*ID_EX).imm));
		(*EX_MEM).flush = 1;
		setJump();
	}
	else
	{
		NEXT_STATE.PC += 4;
		(*EX_MEM).flush = 0;
	}
	(*EX_MEM).Control = BRANCH_TYPE;
}

void instr_handler_BLEZ(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{	
	printf("BLEZ\n");
	if( (int32_t)((*ID_EX).A) <= 0x0 )
	{
		NEXT_STATE.PC = ( (*ID_EX).PC + 4*((int16_t)(*ID_EX).imm));
		(*EX_MEM).flush = 1;
		setJump();
	}
	else
	{
		NEXT_STATE.PC += 4;
		(*EX_MEM).flush = 0;
	}
	(*EX_MEM).Control = BRANCH_TYPE;
}

void instr_handler_BGTZ(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("BGTZ\n");
	if( (int32_t)((*ID_EX).A) >= 0x0 )
	{
		NEXT_STATE.PC = ( (*ID_EX).PC + 4*((int16_t)(*ID_EX).imm));
		(*EX_MEM).flush = 1;
		setJump();
	}
	else
	{
		NEXT_STATE.PC += 4;
		(*EX_MEM).flush = 0;
	}
	(*EX_MEM).Control = BRANCH_TYPE;
}

void instr_handler_SYSCALL(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("SYSCALL\n");
	int32_t v0_val;
	int32_t a0_val;

	v0_val = (*ID_EX).A; 
	a0_val = (*ID_EX).B; 
	switch( v0_val ) {
		case 0x01:	printf("%d\n", a0_val);		break;	
		case 0x0A:	WB(); RUN_FLAG = 0;			break;	
		default:								return;
	}
}
void instr_handler_SLL( CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM )
{
	printf("SLL\n");
    uint8_t sa;
    
    sa = ( (*EX_MEM).imm >> 6 ) & 0x1F;
    
	(*EX_MEM).ALUOutput = (*ID_EX).B << sa;
	(*EX_MEM).Control = REGISTER_TYPE;
}

void instr_handler_SRL(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{  
	printf("SRL\n");
    uint8_t sa;
    
    sa = ( (*EX_MEM).imm >> 6 ) & 0x1F;
    
	(*EX_MEM).ALUOutput = (*ID_EX).B >> sa;
	(*EX_MEM).Control = REGISTER_TYPE;
}


void instr_handler_SRA( CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM )
{
	printf("SRA\n");
    uint8_t sa;
    
    sa = ( (*EX_MEM).imm >> 6 ) & 0x1F;
    
	(*EX_MEM).ALUOutput = (int32_t)(*ID_EX).B >> sa;
	(*EX_MEM).Control = REGISTER_TYPE;
}
void instr_handler_MFHI(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("MFHI\n");
	(*EX_MEM).ALUOutput = (*ID_EX).A;
	(*EX_MEM).Control = REGISTER_TYPE;
}

void instr_handler_MTHI(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("MTHI\n");
	(*EX_MEM).ALUOutput2 = (int32_t)((int32_t)(*ID_EX).A);
	(*EX_MEM).ALUOutput = (*ID_EX).B; 
	(*EX_MEM).Control = SPECIAL_REGISTER_TYPE;
    
}

void instr_handler_MFLO(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("MFLO\n");
	(*EX_MEM).ALUOutput = (*ID_EX).A;
	(*EX_MEM).Control = REGISTER_TYPE;
    
}

void instr_handler_MTLO(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("MTLO\n");
	(*EX_MEM).ALUOutput = (int32_t)((int32_t)(*ID_EX).A);
	(*EX_MEM).ALUOutput2 = (*ID_EX).B;
	(*EX_MEM).Control = SPECIAL_REGISTER_TYPE;
	
}


void instr_handler_MULT(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("MULT\n");
	int64_t Product = (int32_t)(*ID_EX).A * (int32_t)(*ID_EX).B;
	(*EX_MEM).ALUOutput 	= (Product & 0xFFFFFFFF);	
	(*EX_MEM).ALUOutput2 	= (Product >> 32);			
	(*EX_MEM).Control = SPECIAL_REGISTER_TYPE;
}


void instr_handler_MULTU(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("MULTU\n");
	uint64_t Product = (*ID_EX).A * (*ID_EX).B;
	(*EX_MEM).ALUOutput 	= (Product & 0xFFFFFFFF);	
	(*EX_MEM).ALUOutput2 	= (Product >> 32);			
	(*EX_MEM).Control = SPECIAL_REGISTER_TYPE;
}


void instr_handler_DIV(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("DIV\n");
	(*EX_MEM).ALUOutput 	= (int32_t)((int32_t)(*ID_EX).A / (int32_t)(*ID_EX).B);	
	(*EX_MEM).ALUOutput2 	= (int32_t)((int32_t)(*ID_EX).A % (int32_t)(*ID_EX).B);	
	(*EX_MEM).Control = SPECIAL_REGISTER_TYPE;
}


void instr_handler_DIVU(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("DIVU\n");
	(*EX_MEM).ALUOutput 	= (*ID_EX).A / (*ID_EX).B;	
	(*EX_MEM).ALUOutput2 	= (*ID_EX).A % (*ID_EX).B;	
	(*EX_MEM).Control = SPECIAL_REGISTER_TYPE;
}


void instr_handler_ADD(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("ADD\n");
	(*EX_MEM).ALUOutput = (int32_t)((int32_t)(*ID_EX).A + (int32_t)(*ID_EX).B);
	(*EX_MEM).Control = REGISTER_TYPE;
}


void instr_handler_ADDU(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("ADDU\n");
	(*EX_MEM).ALUOutput = (*ID_EX).A + (*ID_EX).B;
	(*EX_MEM).Control = REGISTER_TYPE;
}


void instr_handler_SUB(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("SUB\n");
	(*EX_MEM).ALUOutput = (int32_t)((int32_t)(*ID_EX).A - (int32_t)(*ID_EX).B);
	(*EX_MEM).Control = REGISTER_TYPE;
}


void instr_handler_SUBU(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("SUBU\n");
	(*EX_MEM).ALUOutput = (*ID_EX).A - (*ID_EX).B;
	(*EX_MEM).Control = REGISTER_TYPE;
}


void instr_handler_AND(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("AND\n");
	(*EX_MEM).ALUOutput = (*ID_EX).A & (*ID_EX).B;
	(*EX_MEM).Control = REGISTER_TYPE;
}


void instr_handler_OR(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("OR\n");
	(*EX_MEM).ALUOutput = (*ID_EX).A | (*ID_EX).B;
	(*EX_MEM).Control = REGISTER_TYPE;
}


void instr_handler_XOR(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("XOR\n");
	(*EX_MEM).ALUOutput = (*ID_EX).A ^ (*ID_EX).B;
	(*EX_MEM).Control = REGISTER_TYPE;
}


void instr_handler_NOR(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("NOR\n");
	(*EX_MEM).ALUOutput = ~((*ID_EX).A | (*ID_EX).B);
	(*EX_MEM).Control = REGISTER_TYPE;
}


void instr_handler_SLT(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("SLT\n");
	(*EX_MEM).ALUOutput = ((*ID_EX).A < (*ID_EX).B) ? 1 : 0;
	(*EX_MEM).Control = REGISTER_TYPE;
}

void instr_handler_ADDI(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("ADI\n");
	(*EX_MEM).ALUOutput = (int32_t)((int32_t)(*ID_EX).A + (int32_t)(*ID_EX).imm);
	(*EX_MEM).Control = REGISTER_TYPE;
}

void instr_handler_ADDIU(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("ADDIU\n");
	(*EX_MEM).ALUOutput = (*ID_EX).A + (*ID_EX).imm;
	(*EX_MEM).Control = REGISTER_TYPE;

}

void instr_handler_SLTI(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("SLTI\n");
	(*EX_MEM).ALUOutput = ((*ID_EX).A < (*ID_EX).imm) ? 0x1 : 0x0;
	(*EX_MEM).Control = REGISTER_TYPE;
}

void instr_handler_ANDI(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("ANDI\n");
	(*EX_MEM).ALUOutput = (*ID_EX).A & (*ID_EX).imm;
	(*EX_MEM).Control = REGISTER_TYPE;
}

void instr_handler_ORI(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("ORI\n");
	(*EX_MEM).ALUOutput = (*ID_EX).A | (*ID_EX).imm;
	(*EX_MEM).Control = REGISTER_TYPE;
}

void instr_handler_XORI(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("XORI\n");
	(*EX_MEM).ALUOutput = (*ID_EX).A ^ (*ID_EX).imm;
	(*EX_MEM).Control = REGISTER_TYPE;

}

void instr_handler_LUI(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("LUI\n");
	(*EX_MEM).ALUOutput = (*ID_EX).imm << 16;
	(*EX_MEM).Control = REGISTER_TYPE;
	
}

void instr_handler_LB(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("LB\n");
	(*EX_MEM).ALUOutput = (*ID_EX).A + (*ID_EX).imm;
	(*EX_MEM).Control = LOAD_TYPE;
	(*EX_MEM).num_bytes = BYTE;
}

void instr_handler_LH(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("LH\n");
	(*EX_MEM).ALUOutput = (*ID_EX).A + (*ID_EX).imm;
	(*EX_MEM).Control = LOAD_TYPE;
	(*EX_MEM).num_bytes = HALF_WORD;
}

void instr_handler_LW(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("LW\n");
	(*EX_MEM).ALUOutput = (*ID_EX).A + (*ID_EX).imm;
	(*EX_MEM).Control = LOAD_TYPE;
	(*EX_MEM).num_bytes = WORD;
}

void instr_handler_SB(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("SB\n");
	(*EX_MEM).ALUOutput = (*ID_EX).A + (*ID_EX).imm;
	(*EX_MEM).Control = STORE_TYPE;
	(*EX_MEM).num_bytes = BYTE;
}

void instr_handler_SH(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("SH\n");
	(*EX_MEM).ALUOutput = (*ID_EX).A + (*ID_EX).imm;
	(*EX_MEM).Control = STORE_TYPE;
	(*EX_MEM).num_bytes = HALF_WORD;
}

void instr_handler_SW(CPU_Pipeline_Reg* ID_EX, CPU_Pipeline_Reg* EX_MEM)
{
	printf("SW\n");
	(*EX_MEM).ALUOutput = (*ID_EX).A + (*ID_EX).imm;
	(*EX_MEM).Control = STORE_TYPE;
	(*EX_MEM).num_bytes = WORD;
}

uint8_t checkBranch(uint8_t op, uint8_t func){
		return ( ( op == 0x01 ) || 
								( op <= 0x07 && op >= 0x02) || 
								( ( op == 0x00 ) && ( func == 0x08 || func == 0x09 ) ) ) ? 1 : 0;
}

void setJump() {
	JUMPED = 1;
}

mips_instr_t opcode_0x00_table[0x2A + 1] =
{
			// Name			type	OCode	FCode	Makeup			FPtr	                SubTable
	[0x00] = { "SLL",		R_TYPE, 0x00,	0x00,	{RD, RT, SA},	&instr_handler_SLL,		NULL },
	[0x02] = { "SRL",		R_TYPE, 0x00,	0x02,	{RD, RT, SA},	&instr_handler_SRL,		NULL },
	[0x03] = { "SRA",		R_TYPE, 0x00,	0x03,	{RD, RT, SA},	&instr_handler_SRA,		NULL },
	[0x08] = { "JR",		R_TYPE, 0x00,	0x08,	{RS},			&instr_handler_JR,		NULL },
	[0x09] = { "JALR",		R_TYPE, 0x00,	0x09,	{RD, RS},		&instr_handler_JALR,	NULL },
	[0x0C] = { "SYSCALL",	R_TYPE, 0x00,	0x0C,	{ },			&instr_handler_SYSCALL,	NULL },
	[0x10] = { "MFHI",		R_TYPE, 0x00,	0x10,	{RD},			&instr_handler_MFHI,	NULL },
	[0x11] = { "MTHI",		R_TYPE, 0x00,	0x11,	{RS},			&instr_handler_MTHI,	NULL },
	[0x12] = { "MFLO",		R_TYPE, 0x00,	0x12,	{RD},			&instr_handler_MFLO,	NULL },
	[0x13] = { "MTLO",		R_TYPE, 0x00,	0x13,	{RS},			&instr_handler_MTLO,	NULL },
	[0x18] = { "MULT",		R_TYPE, 0x00,	0x18,	{RS, RT},		&instr_handler_MULT,	NULL },
	[0x19] = { "MULTU",		R_TYPE, 0x00,	0x19,	{RS, RT},		&instr_handler_MULTU,	NULL },
	[0x1A] = { "DIV",		R_TYPE, 0x00,	0x1A,	{RS, RT},		&instr_handler_DIV,		NULL },
	[0x1B] = { "DIVU",		R_TYPE, 0x00,	0x1B,	{RS, RT},		&instr_handler_DIVU,	NULL },
	[0x20] = { "ADD",		R_TYPE,	0x00,	0x20,	{RD, RS, RT},	&instr_handler_ADD,		NULL },
	[0x21] = { "ADDU",		R_TYPE, 0x00,	0x21,	{RD, RS, RT},	&instr_handler_ADDU,	NULL },
	[0x22] = { "SUB",		R_TYPE, 0x00,	0x22,	{RD, RS, RT},	&instr_handler_SUB,		NULL },
	[0x23] = { "SUBU",		R_TYPE, 0x00,	0x23,	{RD, RS, RT},	&instr_handler_SUBU,	NULL },
	[0x24] = { "AND",		R_TYPE, 0x00,	0x24,	{RD, RS, RT},	&instr_handler_AND,		NULL },
	[0x25] = { "OR",		R_TYPE, 0x00,	0x25,	{RD, RS, RT},	&instr_handler_OR,		NULL },
	[0x26] = { "XOR",		R_TYPE, 0x00,	0x26,	{RD, RS, RT},	&instr_handler_XOR,		NULL },
	[0x27] = { "NOR",		R_TYPE, 0x00,	0x27,	{RD, RS, RT},	&instr_handler_NOR,		NULL },
	[0x2A] = { "SLT",		R_TYPE, 0x00,	0x2A,	{RD, RS, RT},	&instr_handler_SLT,		NULL }
};

mips_instr_t opcode_0x01_table[2] =
{
			// Name			type	OCode	FCode	Makeup			FPtr					SubTable
	[0x00] = { "BLTZ",		I_TYPE,	0x01,	0x00,	{RS, IMM},	&instr_handler_BLTZ,	NULL },
	[0x01] = { "BGEZ",		I_TYPE, 0x01,	0x00,	{RS, IMM},	&instr_handler_BGEZ,	NULL }
};


mips_instr_t mips_instr_lookup[0x2B + 1] =
{
			//  Name		type	OCode	FCode	Makeup				FunctPtr					SubTable
	[0x00] = { "XXXX",		M_TYPE,	0x00,	0x00,	{ },				NULL,						opcode_0x00_table },
	[0x01] = { "XXXX",		M_TYPE,	0x01,	0x00,	{ },				NULL,						opcode_0x01_table },
	[0x02] = { "J",			J_TYPE,	0x02,	0x00,	{ADDRESS},			&instr_handler_J,			NULL },
	[0x03] = { "JAL",		J_TYPE,	0x03,	0x00,	{ADDRESS},			&instr_handler_JAL,			NULL },
	[0x04] = { "BEQ",		I_TYPE,	0x04,	0x00,	{RS, RT, IMM},	&instr_handler_BEQ,			NULL },
	[0x05] = { "BNE",		I_TYPE,	0x05,	0x00,	{RS, RT, IMM},	&instr_handler_BNE,			NULL },
	[0x06] = { "BLEZ",		I_TYPE,	0x06,	0x00,	{RS, IMM},		&instr_handler_BLEZ,		NULL },
	[0x07] = { "BGTZ",		I_TYPE,	0x07,	0x00,	{RS, IMM},		&instr_handler_BGTZ,		NULL },
	[0x08] = { "ADDI",		I_TYPE,	0x08,	0x00,	{RT, RS, IMM},	&instr_handler_ADDI,		NULL },
	[0x09] = { "ADDIU",		I_TYPE,	0x09,	0x00,	{RT, RS, IMM},	&instr_handler_ADDIU,		NULL },
	[0x0A] = { "SLTI",		I_TYPE,	0x0A,	0x00,	{RT, RS, IMM},	&instr_handler_SLTI,		NULL },
	[0x0C] = { "ANDI",		I_TYPE,	0x0C,	0x00,	{RT, RS, IMM},	&instr_handler_ANDI,		NULL },
	[0x0D] = { "ORI",		I_TYPE,	0x0D,	0x00,	{RT, RS, IMM},	&instr_handler_ORI,			NULL },
	[0x0E] = { "XORI",		I_TYPE,	0x0E,	0x00,	{RT, RS, IMM},	&instr_handler_XORI,		NULL },
	[0x0F] = { "LUI",		I_TYPE,	0x0F,	0x00,	{RT, IMM},		&instr_handler_LUI,         NULL },
	[0x20] = { "LB",		I_TYPE,	0x20,	0x00,	{RT, IMM, RS},	&instr_handler_LB,          NULL },
	[0x21] = { "LH",		I_TYPE,	0x21,	0x00,	{RT, IMM, RS},	&instr_handler_LH,			NULL },
	[0x23] = { "LW",		I_TYPE,	0x23,	0x00,	{RT, IMM, RS},	&instr_handler_LW,			NULL },
	[0x28] = { "SB",		I_TYPE,	0x28,	0x00,	{RT, IMM, RS},	&instr_handler_SB,			NULL },
	[0x29] = { "SH",		I_TYPE,	0x29,	0x00,	{RT, IMM, RS},	&instr_handler_SH,			NULL },
	[0x2B] = { "SW",		I_TYPE,	0x2B,	0x00,	{RT, IMM, RS},	&instr_handler_SW,			NULL }
};

/************************************************************/
// interpret instruction and call proper function
/************************************************************/
mips_instr_t mips_instr_decode( uint32_t instr )
{
	uint8_t opcode;
	uint8_t funct_code;
	uint8_t rt;
	mips_instr_t instr_info;


	opcode = GET_OPCODE( instr );
	funct_code = -1;
	rt = -1;

	if( opcode == 0 )
	{
		funct_code = GET_FUNCTCODE( instr );
		instr_info = mips_instr_lookup[opcode].subtable[funct_code];
	}
	else if( opcode == 1 )
	{
		rt = GET_RT( instr );
		instr_info = mips_instr_lookup[opcode].subtable[rt];
	}
	else
	{
		instr_info = mips_instr_lookup[opcode];

	}

	return instr_info;
}
