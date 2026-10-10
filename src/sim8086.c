#include <sys/stat.h>

#include "common.h"

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

	struct stat stats;
	if (stat(input_binary_path, &stats) != 0)
	{
		//// ERROR
		fprintf(stderr, "ERROR: failed to stat file \"%s\"\n", input_binary_path);
		return 1;
	}

	smm input_binary_len = stats.st_size;
	u8* input_binary     = malloc(input_binary_len);

	FILE* input_binary_file = fopen(input_binary_path, "rb");
	
	if (input_binary_file == 0)
	{
		//// ERROR
		fprintf(stderr, "ERROR: failed to open \"%s\" for binary reading\n", input_binary_path);
		return 1;
	}

	if (fread(input_binary, 1, input_binary_len, input_binary_file) != input_binary_len)
	{
		//// ERROR
		fprintf(stderr, "ERROR: failed to read \"%s\"\n", input_binary_path);
		fclose(input_binary_file);
		return 1;
	}

	fclose(input_binary_file);

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
		char* regs_w0[8] = {
			"al",
			"cl",
			"dl",
			"bl",
			"ah",
			"ch",
			"dh",
			"bh"
		};

		char* regs_w1[8] = {
			"ax",
			"cx",
			"dx",
			"bx",
			"sp",
			"bp",
			"si",
			"di"
		};

		char* effective_addr_equations[8] = {
			"bx + si",
			"bx + di",
			"bp + si",
			"bp + di",
			"si",
			"di",
			"bp",
			"bx"
		};

		// 100010dw | MOV reg/mem to/from reg
		if ((*cursor & 0xFC) == 0x88)
		{
			bool d = !!(*cursor & 0x2);
			bool w = !!(*cursor & 0x1);

			char** regs_table = (w ? regs_w1 : regs_w0);

			--len;
			++cursor;

			if (len < 1)
			{
				//// ERROR
				NOT_IMPLEMENTED;
			}

			u8 mod = (*cursor & 0xC0) >> 6;
			u8 reg = (*cursor & 0x38) >> 3;
			u8 rm  = (*cursor & 0x07) >> 0;

			--len;
			++cursor;

			if (mod == 0x0)
			{
				if (rm != 0x6) // 0x6 is direct address
				{
					char* eq = effective_addr_equations[rm];

					if (d) printf("mov %s, [%s]\n", regs_table[reg], eq);
					else   printf("mov [%s], %s\n", eq, regs_table[reg]);
				}
				else
				{
					ASSERT(rm == 0x6);

					if (len < 2)
					{
						//// ERROR
						NOT_IMPLEMENTED;
					}

					u16 disp = ((u16)cursor[1] << 8) | cursor[0];

					len    -= 2;
					cursor += 2;

					if (d) printf("mov [%u], %s\n", disp, regs_table[reg]);
					else   printf("mov %s, [%u]\n", regs_table[reg], disp);
				}
			}
			else if (mod == 0x1 || mod == 0x2)
			{
				u16 disp = 0;
				if (mod == 0x2)
				{
					if (len < 2)
					{
						//// ERROR
						NOT_IMPLEMENTED;
					}

					disp = ((u16)cursor[1] << 8) | cursor[0];

					len    -= 2;
					cursor += 2;
				}
				else
				{
					if (len < 1)
					{
						//// ERROR
						NOT_IMPLEMENTED;
					}

					disp = *cursor;

					len    -= 1;
					cursor += 1;
				}

				char* eq = effective_addr_equations[rm];

				if (d) printf("mov %s, [%s + %u]\n", regs_table[reg], eq, disp);
				else   printf("mov [%s + %u], %s\n", eq, disp, regs_table[reg]);
			}
			else
			{
				ASSERT(mod == 0x3);

				char* dst = regs_table[rm];
				char* src = regs_table[reg];

				if (d)
				{
					char* tmp = dst;
					dst = src;
					src = dst;
				}

				printf("mov %s, %s\n", dst, src);
			}
		}
		// 1011wreg | MOV immediate to reg
		else if ((*cursor & 0xF0) == 0xB0)
		{
			bool w = !!(*cursor & 0x08);
			u8 reg = *cursor & 0x07;

			--len;
			++cursor;

			u16 data = 0;
			if (w)
			{
				if (len < 2)
				{
					//// ERROR
					NOT_IMPLEMENTED;
				}

				data = ((u16)cursor[1] << 8) | cursor[0];

				len    -= 2;
				cursor += 2;
			}
			else
			{
				if (len < 1)
				{
					//// ERROR
					NOT_IMPLEMENTED;
				}

				data = *cursor;

				len    -= 1;
				cursor += 1;
			}

			char** regs_table = (w ? regs_w1 : regs_w0);

			printf("mov %s, %u\n", regs_table[reg], data);
		}
		// 1100011w | MOV immediate to reg/mem
		else if ((*cursor & 0xFE) == 0xC6)
		{
			bool w = !!(*cursor & 0x1);

			u8 mod = (*cursor & 0xC0) >> 6;
			u8 reg = (*cursor & 0x38) >> 3; // always 0
			u8 rm  = (*cursor & 0x07) >> 0;

			// TODO
			NOT_IMPLEMENTED;
		}
		else
		{
			//// ERROR
			NOT_IMPLEMENTED;
		}
	}

	return 0;
}
