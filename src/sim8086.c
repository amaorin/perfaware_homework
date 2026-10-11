#include <sys/stat.h>

#include "common.h"

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
PrintOperation(char* op_name, bool swap_order, Operand dst, Operand src)
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
		bool wide = (src.immed > 0xFF);

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

	// MOV
	// ----------------------+------------+---------------+---------------+---------------+
	// reg/mem to/from reg   |  100010dw  |  mod reg r/m  |   (DISP-LO)   |   (DISP-HI)   |
	// ----------------------+------------+---------------+---------------+---------------+---------------+----------------+
	// immediate to reg/mem  |  1100011w  |  mod 000 r/m  |   (DISP-LO)   |   (DISP-HI)   |      data     |  data if w=1   |
	// ----------------------+------------+---------------+---------------+---------------+---------------+----------------+
	// immediate to reg      |  1011wreg  |     data      |  data if w=1  |
	// ----------------------+------------+---------------+---------------+
	// mem to accumulator    |  1010000w  |    addr-lo    |    addr-hi    |
	// ----------------------+------------+---------------+---------------+
	// accumulator to mem    |  1010001w  |    addr-lo    |    addr-hi    |
	// ----------------------+------------+---------------+---------------+----------------+
	// reg/mem to seg reg    |  10001110  |  mod 0 sr r/m |    (DISP-LO)  |   (DISP-HI)    |
	// ----------------------+------------+---------------+---------------+----------------+
	// seg reg to reg/mem    |  10001100  |  mod 0 sr r/m |    (DISP-LO)  |   (DISP-HI)    |
	// ----------------------+------------+---------------+---------------+----------------+
	
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

			PrintOperation("mov", d, dst, src);
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
			PrintOperation("mov", false, dst, src);
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

			PrintOperation("mov", false, dst, src);
		}
		// 101000dw | accumulator to/from mem
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

			PrintOperation("mov", d, dst, src);
		}
		else
		{
			//// ERROR
			NOT_IMPLEMENTED;
		}
	}

	return 0;
}
