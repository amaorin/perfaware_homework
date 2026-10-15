#include <sys/stat.h>

#include "common.h"

bool
ReadBinaryFile(char* path, OUT u8** data, OUT u64* len)
{
	bool succeeded = false;

	FILE* file = 0;
	u8* memory = 0;

	do
	{
		struct stat info;
		if (stat(path, &info) != 0)
		{
			//// ERROR
			break;
		}

		memory = malloc(info.st_size);

		if (memory == 0)
		{
			//// ERROR
			break;
		}

		file = fopen(path, "rb");

		if (fread(memory, 1, info.st_size, file) != info.st_size)
		{
			//// ERROR
			break;
		}

		*data = memory;
		*len  = info.st_size;
		succeeded = true;

	} while (0);

	if (file != 0) fclose(file);
	if (!succeeded && memory != 0) free(memory);

	return succeeded;
}

dw mod reg r/m
00ALU0dw
100010dw
1000011w
100100dw
10001101
1100010D

// MOV
// reg/mem to/from reg | 100010dw | mod reg r/m | (DISP)
// immed to reg/mem    | 1100011w | mod 000 r/m | (DISP) | DATA
// immed to reg        | 1011wreg | DATA
// memory to/from acc  | 101000dw | ADDR
// seg to/from reg/mem | 100011d0 | mod 0 SR r/m | (DISP)
//
// LEA
// 10001101 | mod reg r/m | (DISP)
//
// LDS, LES
// 1100010D | mod reg r/m | (DISP)
//
// PUSH, POP
// ~P reg/mem | 11111111 | mod 110 r/m | (DISP)
//  P reg/mem | 10001111 | mod 000 r/m | (DISP)
// reg     | 0101Preg
// seg     | 000SR11P
//
// XCHG
// reg/mem with reg | 1000011w | mod reg r/m | (DISP)
// reg with acc     | 10010reg
//
// IN, OUT
// fix | 111001Ow | DATA8
// var | 111011Ow
//
// ADD, ADC, SUB, SBB, CMP, AND, OR, XOR
// reg/mem to/from reg | 00ALU0dw | mod reg r/m | (DISP)
// immed to reg/mem    | 100000sw | mod ALU r/m | (DISP) | DATA
// immed to acc        | 00ALU10w | DATA
//
// TEST
// reg/mem to/from reg | 100100dw | mod reg r/m | (DISP)
// immed to reg/mem    | 1111011w | mod 000 r/m | (DISP)
// immed to acc        | 1010100w | DATA
//
// INC, DEC
// reg/mem | 1111111w mod ALU r/m | (DISP)
// reg     | 01ALUreg
//
// NEG, NOT
// 1111011w | mod ALU r/m | (DISP)
//
// SHL/SAL, SHR, SAR, ROL, ROR, RCL, RCR
// 110100vw | mod ALU r/m | (DISP)
//
// MUL, IMUL, DIV, IDIV
// 1111011w | mod ALU r/m | (DISP)
//
// AAM, AAD
// 1101010D | 00001010 | (DISP)
//
// LAHF, SAHF, PUSHF, POPF
// 100111OP
//
// CALL
// direct intraseg   | 11101000 | IP
// indirect intraseg | 11111111 | mod 010 r/m | (DISP)
// direct interseg   | 10011010 | IP
// indirect interseg | 11111111 | mod 011 r/m | (DISP)
//
// JMP
// direct interseg   | 11101001 | IP
// direct short      | 11101011 | IP8
// indirect intraseg | 11111111 | mod 100 r/m | (DISP)
// direct interseg   | 11101010 | IP
// indirect interseg | 11111111 | mod 101 r/m | (DISP)
//
// RET
// intraseg    | 11000011
// intraseg sp | 11000010 | DATA
// interseg    | 11001011
// interseg sp | 11001010 | DATA
//
// INT
// var | 11001101 | DATA8
// 3   | 11001100
//
// JE, JL, JLE, JB, JBE, JP, JO, JS, JNE, JNL, JNLE, JNB, JNBE, JNP, JNO, JNS
// 0111COND | IP8
//
// LOOP, LOOPZ, LOOPNZ, JCXZ
// 111000CC | IP
//
// ESC
// 11011xxx | mod yyy r/m | (DISP)
//
// SEG
// 001SR110
//
// XLAT 11010111
// AAA  00110111
// DAA  00100111
// AAS  00111111
// DAS  00101111
// CBW  10011000
// CWD  10011001
// REP  1111001z
// MOVS 1010010w
// CMPS 1010011w
// SCAS 1010111w
// LODS 1010110w
// STOS 1010101w
// CLC  11111000
// CMC  11110101
// STC  11111001
// CLD  11111100
// STD  11111101
// CLI  11111010
// STI  11111011
// HLT  11110100
// WAIT 10011011
// LOCK 11110000
// INTO 11001110
// IRET 11001111

typedef enum Reg
{
} Reg;

typedef enum Effective_Addr_Kind
{
	EffectiveAddr_BXpSI,
	EffectiveAddr_BXpDI,
	EffectiveAddr_BPpSI,
	EffectiveAddr_BPpDI,
	EffectiveAddr_SI,
	EffectiveAddr_DI,
	EffectiveAddr_BP,
	EffectiveAddr_BX,
} Effective_Addr_Kind;

typedef struct Effective_Addr
{
	Effective_Addr_Kind kind;

} Effective_Addr;

typedef enum Operand_Kind
{
	Operand_Immed,
	Operand_Reg,
	Operand_Mem,
} Operand_Kind;

typedef Operand
{
	Operand_Kind kind;

	union
	{
		u16 immed;
		Reg reg;
		Effective_Addr addr;
	};
} Operand;

typedef enum Mnemonic
{
	Instruction_MOV,

	Instruction_LEA,

	Instruction_PUSH,
	Instruction_POP,

	Instruction_XCHG,

	Instruction_IN,
	Instruction_OUT,

	Instruction_ADD,
	Instruction_ADC,
	Instruction_SUB,
	Instruction_SBB,
	Instruction_CMP,
	Instruction_AND,
	Instruction_OR,
	Instruction_XOR,

	Instruction_TEST,

	Instruction_INC,
	Instruction_DEC,

	Instruction_NEG,
	Instruction_NOT,

	Instruction_SHL,
	Instruction_SHR,
	Instruction_SAR,
	Instruction_ROL,
	Instruction_ROR,
	Instruction_RCL,
	Instruction_RCR,

	Instruction_MUL,
	Instruction_IMUL,
	Instruction_DIV,
	Instruction_IDIV,

	Instruction_AAM,
	Instruction_AAD,

	Instruction_LDS,
	Instruction_LES,

	Instruction_LAHF,
	Instruction_SAHF,
	Instruction_PUSHF,
	Instruction_POPF,

	Instruction_CALL,

	Instruction_JMP,

	Instruction_RET,

	Instruction_INT,

	Instruction_JE,
	Instruction_JL,
	Instruction_JLE,
	Instruction_JB,
	Instruction_JBE,
	Instruction_JP,
	Instruction_JO,
	Instruction_JS,
	Instruction_JNE,
	Instruction_JNL,
	Instruction_JNLE,
	Instruction_JNB,
	Instruction_JNBE,
	Instruction_JNP,
	Instruction_JNO,
	Instruction_JNS,

	Instruction_LOOP,
	Instruction_LOOPZ,
	Instruction_LOOPNZ,
	Instruction_JCXZ,

	Instruction_ESC,

	Mnemonic_XLAT,
	Mnemonic_AAA,
	Mnemonic_DAA,
	Mnemonic_AAS,
	Mnemonic_DAS,
	Mnemonic_CBW,
	Mnemonic_CWD,
	Mnemonic_MOVS,
	Mnemonic_CMPS,
	Mnemonic_SCAS,
	Mnemonic_LODS,
	Mnemonic_STOS,
	Mnemonic_CLC,
	Mnemonic_CMC,
	Mnemonic_STC,
	Mnemonic_CLD,
	Mnemonic_STD,
	Mnemonic_CLI,
	Mnemonic_STI,
	Mnemonic_HLT,
	Mnemonic_WAIT,
	Mnemonic_INTO,
	Mnemonic_IRET,
} Mnemonic;

typedef struct Instruction
{
	Mnemonic mnemonic;

	Operand dst;
	Operand src;
} Instruction;

int
main(int argc, char** argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "ERROR: Invalid arguments\n");
		return 1;
	}

	char* binary_path = argv[1];

	u8* binary     = 0;
	u64 binary_len = 0;
	if (!ReadBinaryFile(binary_path, &binary, &binary_len))
	{
		//// ERROR
		fprintf(stderr, "ERROR: Failed to read the input binary\n");
		return 1;
	}

	return 0;
}
