#include <sys/stat.h>

#include "common.h"

// MOV
// ----------------------+------------+---------------+---------------+---------------+
// reg/mem to/from reg   |  100010dw  |  mod reg r/m  |   (DISP-LO)   |   (DISP-HI)   |
// ----------------------+------------+---------------+---------------+---------------+---------------+----------------+
// immediate to reg/mem  |  1100011w  |  mod 000 r/m  |   (DISP-LO)   |   (DISP-HI)   |      data     |  data if w     |
// ----------------------+------------+---------------+---------------+---------------+---------------+----------------+
// immediate to reg      |  1011wreg  |     data      |  data if w    |
// ----------------------+------------+---------------+---------------+
// mem to accumulator    |  1010000w  |    addr-lo    |    addr-hi    |
// ----------------------+------------+---------------+---------------+
// accumulator to mem    |  1010001w  |    addr-lo    |    addr-hi    |
// ----------------------+------------+---------------+---------------+----------------+
// reg/mem to seg reg    |  10001110  |  mod 0 sr r/m |    (DISP-LO)  |   (DISP-HI)    |
// ----------------------+------------+---------------+---------------+----------------+
// seg reg to reg/mem    |  10001100  |  mod 0 sr r/m |    (DISP-LO)  |   (DISP-HI)    |
// ----------------------+------------+---------------+---------------+----------------+
//
// ADD
// ----------------------+------------+---------------+---------------+----------------+
// reg/mem to/from reg   |  000000dw  |  mod reg r/m  |   (DISP-LO)   |   (DISP-HI)    |
// ----------------------+------------+---------------+---------------+----------------+---------------+------------------+
// immediate to reg/mem  |  100000sw  |  mod 000 r/m  |   (DISP-LO)   |   (DISP-HI)    |      data     |  data if w & ~s  |
// ----------------------+------------+---------------+---------------+----------------+---------------+------------------+
// immediate to acc      |  0000010w  |     data      |   data if w   |
// ----------------------+------------+---------------+---------------+
//
// ADC
// ----------------------+------------+---------------+---------------+----------------+
// reg/mem to/from reg   |  000100dw  |  mod reg r/m  |   (DISP-LO)   |   (DISP-HI)    |
// ----------------------+------------+---------------+---------------+----------------+---------------+------------------+
// immediate to reg/mem  |  100000sw  |  mod 010 r/m  |   (DISP-LO)   |   (DISP-HI)    |      data     |  data if w & ~s  |
// ----------------------+------------+---------------+---------------+----------------+---------------+------------------+
// immediate to acc      |  0001010w  |     data      |   data if w   |
// ----------------------+------------+---------------+---------------+
//
// SUB
// ----------------------+------------+---------------+---------------+----------------+
// reg/mem to/from reg   |  001010dw  |  mod reg r/m  |   (DISP-LO)   |   (DISP-HI)    |
// ----------------------+------------+---------------+---------------+----------------+---------------+------------------+
// immediate to reg/mem  |  100000sw  |  mod 101 r/m  |   (DISP-LO)   |   (DISP-HI)    |      data     |  data if w & ~s  |
// ----------------------+------------+---------------+---------------+----------------+---------------+------------------+
// immediate to acc      |  0010110w  |     data      |   data if w   |
// ----------------------+------------+---------------+---------------+
//
// SBB
// ----------------------+------------+---------------+---------------+----------------+
// reg/mem to/from reg   |  000110dw  |  mod reg r/m  |   (DISP-LO)   |   (DISP-HI)    |
// ----------------------+------------+---------------+---------------+----------------+---------------+------------------+
// immediate to reg/mem  |  100000sw  |  mod 011 r/m  |   (DISP-LO)   |   (DISP-HI)    |      data     |  data if w & ~s  |
// ----------------------+------------+---------------+---------------+----------------+---------------+------------------+
// immediate to acc      |  0001110w  |     data      |   data if w   |
// ----------------------+------------+---------------+---------------+
//
// CMP
// ----------------------+------------+---------------+---------------+----------------+
// reg/mem to/from reg   |  001110dw  |  mod reg r/m  |   (DISP-LO)   |   (DISP-HI)    |
// ----------------------+------------+---------------+---------------+----------------+---------------+------------------+
// immediate to reg/mem  |  100000sw  |  mod 111 r/m  |   (DISP-LO)   |   (DISP-HI)    |      data     |  data if w & ~s  |
// ----------------------+------------+---------------+---------------+----------------+---------------+------------------+
// immediate to acc      |  0011110w  |     data      |   data if w   |
// ----------------------+------------+---------------+---------------+
//
// OR
// ----------------------+------------+---------------+---------------+----------------+
// reg/mem to/from reg   |  000010dw  |  mod reg r/m  |   (DISP-LO)   |   (DISP-HI)    |
// ----------------------+------------+---------------+---------------+----------------+---------------+------------------+
// immediate to reg/mem  |  100000sw  |  mod 001 r/m  |   (DISP-LO)   |   (DISP-HI)    |      data     |  data if w & ~s  |
// ----------------------+------------+---------------+---------------+----------------+---------------+------------------+
// immediate to acc      |  0000110w  |     data      |   data if w   |
// ----------------------+------------+---------------+---------------+
//
// XOR
// ----------------------+------------+---------------+---------------+----------------+
// reg/mem to/from reg   |  001100dw  |  mod reg r/m  |   (DISP-LO)   |   (DISP-HI)    |
// ----------------------+------------+---------------+---------------+----------------+---------------+------------------+
// immediate to reg/mem  |  100000sw  |  mod 110 r/m  |   (DISP-LO)   |   (DISP-HI)    |      data     |  data if w & ~s  |
// ----------------------+------------+---------------+---------------+----------------+---------------+------------------+
// immediate to acc      |  0011010w  |     data      |   data if w   |
// ----------------------+------------+---------------+---------------+
//
// AND
// ----------------------+------------+---------------+---------------+----------------+
// reg/mem to/from reg   |  001000dw  |  mod reg r/m  |   (DISP-LO)   |   (DISP-HI)    |
// ----------------------+------------+---------------+---------------+----------------+---------------+------------------+
// immediate to reg/mem  |  100000sw  |  mod 100 r/m  |   (DISP-LO)   |   (DISP-HI)    |      data     |  data if w & ~s  |
// ----------------------+------------+---------------+---------------+----------------+---------------+------------------+
// immediate to acc      |  0010010w  |     data      |   data if w   |
// ----------------------+------------+---------------+---------------+
//
// JCC
// ----------------------+------------+---------------+
// JE/JZ                 |  01110100  |    IP-INC8    |
// ----------------------+------------+---------------+
// JL/JNGE               |  01111100  |    IP-INC8    |
// ----------------------+------------+---------------+
// JLE/JNG               |  01111110  |    IP-INC8    |
// ----------------------+------------+---------------+
// JB/JNAE               |  01110010  |    IP-INC8    |
// ----------------------+------------+---------------+
// JBE/JNA               |  01110110  |    IP-INC8    |
// ----------------------+------------+---------------+
// JP/JPE                |  01111010  |    IP-INC8    |
// ----------------------+------------+---------------+
// JO                    |  01110000  |    IP-INC8    |
// ----------------------+------------+---------------+
// JS                    |  01111000  |    IP-INC8    |
// ----------------------+------------+---------------+
// JNE/JNZ               |  01110101  |    IP-INC8    |
// ----------------------+------------+---------------+
// JNL/JGE               |  01111101  |    IP-INC8    |
// ----------------------+------------+---------------+
// JNLE/JG               |  01111111  |    IP-INC8    |
// ----------------------+------------+---------------+
// JNB/JAE               |  01110011  |    IP-INC8    |
// ----------------------+------------+---------------+
// JNBE/JA               |  01110111  |    IP-INC8    |
// ----------------------+------------+---------------+
// JNP/JPO               |  01111011  |    IP-INC8    |
// ----------------------+------------+---------------+
// JNO                   |  01110001  |    IP-INC8    |
// ----------------------+------------+---------------+
// JNS                   |  01111001  |    IP-INC8    |
// ----------------------+------------+---------------+
//
// LOOPCC
// ----------------------+------------+---------------+
// LOOP                  |  11100010  |    IP-INC8    |
// ----------------------+------------+---------------+
// LOOPZ/LOOPE           |  11100001  |    IP-INC8    |
// ----------------------+------------+---------------+
// LOOPNZ/LOOPNE         |  11100000  |    IP-INC8    |
// ----------------------+------------+---------------+
// JCXZ                  |  11100011  |    IP-INC8    |
// ----------------------+------------+---------------+


bool
EatData(u8** cursor, smm* len, bool wide, u16* data)
{
	if (wide)
	{
		if (*len < 2)
		{
			//// ERROR
			return false;
		}

		*data = ((u16)(*cursor)[1] << 8) | (*cursor)[0];

		*len    -= 2;
		*cursor += 2;

		return true;
	}
	else
	{
		if (*len < 1)
		{
			//// ERROR
			return false;
		}

		*data = **cursor;

		*len    -= 1;
		*cursor += 1;

		return true;
	}
}

bool
EatSignedData(u8** cursor, smm* len, bool wide, s16* data)
{
	u16 unsigned_data = 0;
	if (!EatData(cursor, len, wide, &unsigned_data))
	{
		//// ERROR
		return false;
	}

	if (wide) *data = (s16)unsigned_data;
	else      *data = (s16)(s8)unsigned_data;

	return true;
}

bool
EatModRegRm(u8** cursor, smm* len, u8* mod, u8* reg, u8* rm)
{
	if (*len < 1)
	{
		//// ERROR
		return false;
	}

	*mod = (**cursor & 0xC0) >> 6;
	*reg = (**cursor & 0x38) >> 3;
	*rm  = (**cursor & 0x07) >> 0;

	*len    -= 1;
	*cursor += 1;

	return true;
}

typedef enum Reg
{
	Reg_AL,
	Reg_CL,
	Reg_DL,
	Reg_BL,
	Reg_AH,
	Reg_CH,
	Reg_DH,
	Reg_BH,

	Reg_AX,
	Reg_CX,
	Reg_DX,
	Reg_BX,
	Reg_SP,
	Reg_BP,
	Reg_SI,
	Reg_DI,

	REG_COUNT
} Reg;

char* RegStrings[REG_COUNT] = {
	[Reg_AL] = "al",
	[Reg_CL] = "cl",
	[Reg_DL] = "dl",
	[Reg_BL] = "bl",
	[Reg_AH] = "ah",
	[Reg_CH] = "ch",
	[Reg_DH] = "dh",
	[Reg_BH] = "bh",

	[Reg_AX] = "ax",
	[Reg_CX] = "cx",
	[Reg_DX] = "dx",
	[Reg_BX] = "bx",
	[Reg_SP] = "sp",
	[Reg_BP] = "bp",
	[Reg_SI] = "si",
	[Reg_DI] = "di",
};

Reg
Reg_FromBits(bool wide, u8 reg_bits)
{
	return (wide ? 0x8 : 0x0) | reg_bits;
}

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

	EFFECTIVE_ADDR_COUNT
} Effective_Addr_Kind;

char* EffectiveAddrStrings[EFFECTIVE_ADDR_COUNT] = {
	[EffectiveAddr_BXpSI] = "bx + si",
	[EffectiveAddr_BXpDI] = "bx + di",
	[EffectiveAddr_BPpSI] = "bp + si",
	[EffectiveAddr_BPpDI] = "bp + di",
	[EffectiveAddr_SI]    = "si",
	[EffectiveAddr_DI]    = "di",
	[EffectiveAddr_BP]    = "bp",
	[EffectiveAddr_BX]    = "bx"
};

typedef enum Operand_Kind
{
	Operand_Invalid = 0,
	Operand_Immed,
	Operand_Reg,
	Operand_Direct,
	Operand_Index,
	Operand_IndexDisp8,
	Operand_IndexDisp16,
} Operand_Kind;

typedef struct Operand
{
	Operand_Kind kind;
	union
	{
		u16 immed;
		Reg reg;
		u16 direct;
		Effective_Addr_Kind index;
		struct
		{
			Effective_Addr_Kind index;
			s16 disp;
		} index_disp;
	};
} Operand;

void
PrintOperand(Operand op)
{
	switch (op.kind)
	{
		case Operand_Immed:       printf("%u", op.immed);                         break;
		case Operand_Reg:         printf("%s", RegStrings[op.reg]);               break;
		case Operand_Direct:      printf("[%u]", op.direct);                      break;
		case Operand_Index:       printf("[%s]", EffectiveAddrStrings[op.index]); break;
		case Operand_IndexDisp8: // fallthrough
		case Operand_IndexDisp16:
		{
			char* eq = EffectiveAddrStrings[op.index_disp.index];
			s16 disp = op.index_disp.disp;

			if (disp < 0) printf("[%s - %d]", eq, -disp);
			else          printf("[%s + %d]", eq, +disp);
		} break;

		default: NOT_IMPLEMENTED; break;
	}
}

void
PrintOperation(char* op_name, bool wide, bool swap_order, Operand dst, Operand src)
{
	if (swap_order)
	{
		Operand tmp = dst;
		dst = src;
		src = tmp;
	}

	printf("%s ", op_name);

	PrintOperand(dst);

	printf(", ");

	if (src.kind == Operand_Immed && dst.kind != Operand_Reg)
	{
		printf("%s ", (wide ? "word" : "byte"));
	}

	PrintOperand(src);

	printf("\n");
}

bool
EatModRegRmDisp(u8** cursor, smm* len, bool wide, Operand* dst, Operand* src)
{
	u8 mod, reg_bits, rm = 0;
	if (!EatModRegRm(cursor, len, &mod, &reg_bits, &rm))
	{
		//// ERROR
		return false;
	}

	*src = (Operand){ .kind = Operand_Reg, .reg = Reg_FromBits(wide, reg_bits) };

	if (mod == 0x0)
	{
		if (rm != 0x6) // 0x6 is direct address
		{
			*dst = (Operand){ .kind = Operand_Index, .index = (Effective_Addr_Kind)rm };
		}
		else
		{
			ASSERT(rm == 0x6);

			u16 direct = 0;
			if (!EatData(cursor, len, true, &direct))
			{
				//// ERROR
				return false;
			}

			*dst = (Operand){ .kind = Operand_Direct, .direct = direct };
		}
	}
	else if (mod == 0x1 || mod == 0x2)
	{
		bool wide_disp = (mod == 0x2);

		s16 disp = 0;
		if (!EatSignedData(cursor, len, wide_disp, &disp))
		{
			//// ERROR
			return false;
		}

		*dst = (Operand){
			.kind       = (wide_disp ? Operand_IndexDisp16 : Operand_IndexDisp8),
			.index_disp = {
				.index = (Effective_Addr_Kind)rm,
				.disp  = disp
			}
		};
	}
	else
	{
		ASSERT(mod == 0x3);

		*dst = (Operand){ .kind = Operand_Reg, .reg = Reg_FromBits(wide, rm) };
	}

	return true;
}

bool
ReadBinaryFile(char* path, u8** data_out, smm* len_out)
{
	bool succeeded = false;

	*data_out = 0;
	*len_out  = 0;

	FILE* file = 0;
	u8* data   = 0;
	do
	{
		struct stat stats;
		if (stat(path, &stats) != 0)
		{
			//// ERROR
			break;
		}

		smm len = stats.st_size;
		data    = malloc(len);

		file = fopen(path, "rb");
		
		if (file == 0)
		{
			//// ERROR
			break;
		}

		if (fread(data, 1, len, file) != len)
		{
			//// ERROR
			break;
		}

		*data_out = data;
		*len_out  = len;
		succeeded = true;

	} while (0);

	if (file != 0) fclose(file);

	if (!succeeded && data != 0) free(data);

	return succeeded;
}

int
main(int argc, char** argv)
{
	if (argc != 2)
	{
		//// ERROR
		fprintf(stderr, "Usage: sim8086 input_binary_file\n");
		return 1;
	}

	char* input_binary_path = argv[1];

	u8* input_binary     = 0;
	smm input_binary_len = 0;
	if (!ReadBinaryFile(input_binary_path, &input_binary, &input_binary_len))
	{
		//// ERROR
		fprintf(stderr, "ERROR: Failed to read binary file \"%s\"\n", input_binary_path);
		return 1;
	}
	
	u8* cursor = input_binary;
	smm len    = input_binary_len;

	printf("; %s\n", input_binary_path);
	printf("bits 16\n");

	while (len > 0)
	{
		// 100010dw | MOV reg/mem to/from reg
		if ((*cursor & 0xFC) == 0x88)
		{
			bool d = !!(*cursor & 0x2);
			bool w = !!(*cursor & 0x1);

			--len;
			++cursor;

			Operand dst = {0};
			Operand src = {0};
			if (!EatModRegRmDisp(&cursor, &len, w, &dst, &src))
			{
				//// ERROR
				NOT_IMPLEMENTED;
			}

			PrintOperation("mov", w, d, dst, src);
		}
		// 1011wreg | MOV immediate to reg
		else if ((*cursor & 0xF0) == 0xB0)
		{
			bool w = !!(*cursor & 0x08);
			u8 reg = *cursor & 0x07;

			--len;
			++cursor;

			u16 data = 0;
			if (!EatData(&cursor, &len, w, &data))
			{
				//// ERROR
				NOT_IMPLEMENTED;
			}

			Operand dst = (Operand){ .kind = Operand_Reg,   .reg   = Reg_FromBits(w, reg) };
			Operand src = (Operand){ .kind = Operand_Immed, .immed = data                 };
			PrintOperation("mov", w, false, dst, src);
		}
		// 1100011w | MOV immediate to reg/mem
		else if ((*cursor & 0xFE) == 0xC6)
		{
			bool w = !!(*cursor & 0x1);

			--len;
			++cursor;

			Operand ignored = {0};
			Operand dst     = {0};
			if (!EatModRegRmDisp(&cursor, &len, w, &dst, &ignored))
			{
				//// ERROR
				NOT_IMPLEMENTED;
			}

			// The reg bits should always be 0, assert it just to be sure
			ASSERT(ignored.kind == Operand_Reg && (ignored.reg & 0x7) == 0);

			u16 data = 0;
			if (!EatData(&cursor, &len, w, &data))
			{
				//// ERROR
				NOT_IMPLEMENTED;
			}

			Operand src = (Operand){ .kind = Operand_Immed, .immed = data };

			PrintOperation("mov", w, false, dst, src);
		}
		// 101000dw | MOV accumulator to/from mem
		else if ((*cursor & 0xFC) == 0xA0)
		{
			bool d = !!(*cursor & 0x2);
			bool w = !!(*cursor & 0x1);

			--len;
			++cursor;

			u16 addr = 0;
			if (!EatData(&cursor, &len, true, &addr))
			{
				//// ERROR
				NOT_IMPLEMENTED;
			}

			Operand dst = (Operand){ .kind = Operand_Reg,    .reg = (w ? Reg_AX : Reg_AL) };
			Operand src = (Operand){ .kind = Operand_Direct, .direct = addr };

			PrintOperation("mov", w, d, dst, src);
		}
		// 000000dw | ADD reg/mem to/from reg
		// 000100dw | ADC reg/mem to/from reg
		// 001010dw | SUB reg/mem to/from reg
		// 000110dw | SBB reg/mem to/from reg
		// 001110dw | CMP reg/mem to/from reg
		// 001000dw | AND reg/mem to/from reg
		// 001100dw | XOR reg/mem to/from reg
		// 000010dw | OR  reg/mem to/from reg
		else if ((*cursor & 0xC4) == 0)
		{
			char* mnemonic = 0;
			if      ((*cursor >> 3) == 0x0) mnemonic = "add";
			else if ((*cursor >> 3) == 0x1) mnemonic = "or";
			else if ((*cursor >> 3) == 0x2) mnemonic = "adc";
			else if ((*cursor >> 3) == 0x3) mnemonic = "sbb";
			else if ((*cursor >> 3) == 0x4) mnemonic = "and";
			else if ((*cursor >> 3) == 0x5) mnemonic = "sub";
			else if ((*cursor >> 3) == 0x6) mnemonic = "xor";
			else if ((*cursor >> 3) == 0x7) mnemonic = "cmp";

			bool w = !!(*cursor & 0x1);
			bool d = !!(*cursor & 0x2);

			--len;
			++cursor;

			Operand src = {0};
			Operand dst = {0};
			if (!EatModRegRmDisp(&cursor, &len, w, &dst, &src))
			{
				//// ERROR
				NOT_IMPLEMENTED;
			}

			PrintOperation(mnemonic, w, d, dst, src);
		}
		// 100000sw | ADD immed to reg/mem
		// 100000sw | ADC immed to reg/mem
		// 100000sw | SUB immed to reg/mem
		// 100000sw | SBB immed to reg/mem
		// 100000sw | CMP immed to reg/mem
		// 100000sw | AND immed to reg/mem
		// 100000sw | XOR immed to reg/mem
		// 100000sw | OR  immed to reg/mem
		else if ((*cursor & 0xFC) == 0x80)
		{
			bool w = !!(*cursor & 0x1);
			bool s = !!(*cursor & 0x2);

			--len;
			++cursor;

			Operand reg = {0};
			Operand dst = {0};
			if (!EatModRegRmDisp(&cursor, &len, w, &dst, &reg))
			{
				//// ERROR
				NOT_IMPLEMENTED;
			}

			ASSERT(reg.kind == Operand_Reg);
			u8 alu_op = (u8)reg.reg & 0x7;

			char* mnemonic = 0;
			if      (alu_op == 0x0) mnemonic = "add";
			else if (alu_op == 0x1) mnemonic = "or";
			else if (alu_op == 0x2) mnemonic = "adc";
			else if (alu_op == 0x3) mnemonic = "sbb";
			else if (alu_op == 0x4) mnemonic = "and";
			else if (alu_op == 0x5) mnemonic = "sub";
			else if (alu_op == 0x6) mnemonic = "xor";
			else if (alu_op == 0x7) mnemonic = "cmp";
			else NOT_IMPLEMENTED;

			u16 data = 0;
			if (!EatData(&cursor, &len, w && !s, &data))
			{
				//// ERROR
				NOT_IMPLEMENTED;
			}

			if (w && s) data = (u16)(s16)(s8)data;

			Operand src = (Operand){ .kind = Operand_Immed, .immed = data };

			PrintOperation(mnemonic, w, false, dst, src);
		}
		// 0000010w | ADD immed to acc
		// 0001010w | ADC immed to acc
		// 0010110w | SUB immed to acc
		// 0001110w | SBB immed to acc
		// 0011110w | CMP immed to acc
		// 0010010w | AND immed to acc
		// 0011010w | XOR immed to acc
		// 0000110w | OR  immed to acc
		else if ((*cursor & 0xC6) == 0x4)
		{
			char* mnemonic = 0;
			if      ((*cursor >> 3) == 0x0) mnemonic = "add";
			else if ((*cursor >> 3) == 0x1) mnemonic = "or";
			else if ((*cursor >> 3) == 0x2) mnemonic = "adc";
			else if ((*cursor >> 3) == 0x3) mnemonic = "sbb";
			else if ((*cursor >> 3) == 0x4) mnemonic = "and";
			else if ((*cursor >> 3) == 0x5) mnemonic = "sub";
			else if ((*cursor >> 3) == 0x6) mnemonic = "xor";
			else if ((*cursor >> 3) == 0x7) mnemonic = "cmp";
			else NOT_IMPLEMENTED;

			bool w = !!(*cursor & 0x1);

			--len;
			++cursor;

			u16 data = 0;
			if (!EatData(&cursor, &len, w, &data))
			{
				//// ERROR
				NOT_IMPLEMENTED;
			}

			Operand dst = (Operand){ .kind = Operand_Reg,   .reg = (w ? Reg_AX : Reg_AL) };
			Operand src = (Operand){ .kind = Operand_Immed, .direct = data };

			PrintOperation(mnemonic, w, false, dst, src);
		}
		// 01110100 | JE
		// 01111100 | JL
		// 01111110 | JLE
		// 01110010 | JB
		// 01110110 | JBE
		// 01111010 | JP
		// 01110000 | JO
		// 01111000 | JS
		// 01110101 | JNE
		// 01111101 | JNL
		// 01111111 | JNLE
		// 01110011 | JNB
		// 01110111 | JNBE
		// 01111011 | JNP
		// 01110001 | JNO
		// 01111001 | JNS
		else if ((*cursor & 0xF0) == 0x70)
		{
			char* mnemonic = 0;
			if      (*cursor == 0x70) mnemonic = "jo";
			else if (*cursor == 0x71) mnemonic = "jno";
			else if (*cursor == 0x72) mnemonic = "jb";
			else if (*cursor == 0x73) mnemonic = "jnb";
			else if (*cursor == 0x74) mnemonic = "je";
			else if (*cursor == 0x75) mnemonic = "jne";
			else if (*cursor == 0x76) mnemonic = "jbe";
			else if (*cursor == 0x77) mnemonic = "jnbe";
			else if (*cursor == 0x78) mnemonic = "js";
			else if (*cursor == 0x79) mnemonic = "jns";
			else if (*cursor == 0x7A) mnemonic = "jp";
			else if (*cursor == 0x7B) mnemonic = "jnp";
			else if (*cursor == 0x7C) mnemonic = "jl";
			else if (*cursor == 0x7D) mnemonic = "jnl";
			else if (*cursor == 0x7E) mnemonic = "jle";
			else if (*cursor == 0x7F) mnemonic = "jnle";
			else NOT_IMPLEMENTED;

			--len;
			++cursor;

			s16 rel = 0;
			if (!EatSignedData(&cursor, &len, false, &rel))
			{
				//// ERROR
				NOT_IMPLEMENTED;
			}

			printf("%s $+2%+d\n", mnemonic, rel);
		}
		// 11100000 | LOOPNZ
		// 11100001 | LOOPZ
		// 11100010 | LOOP
		// 11100011 | JCXZ
		else if ((*cursor & 0xFC) == 0xE0)
		{
			char* mnemonic = 0;
			if      ((*cursor & 0x3) == 0x0) mnemonic = "loopnz";
			else if ((*cursor & 0x3) == 0x1) mnemonic = "loopz";
			else if ((*cursor & 0x3) == 0x2) mnemonic = "loop";
			else if ((*cursor & 0x3) == 0x3) mnemonic = "jcxz";

			--len;
			++cursor;

			s16 rel = 0;
			if (!EatSignedData(&cursor, &len, false, &rel))
			{
				//// ERROR
				NOT_IMPLEMENTED;
			}

			printf("%s $+2%+d\n", mnemonic, rel);
		}
		else
		{
			fprintf(stderr, "Unknown opcode 0x%X\n", *cursor);

			//// ERROR
			NOT_IMPLEMENTED;
		}
	}

	return 0;
}
