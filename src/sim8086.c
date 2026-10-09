#include <stdlib.h>
#include <stdio.h>
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
	// reg/mem to/from reg   |  100010dw  |  mod ref r/m  |   (DISP-LO)   |   (DISP-HI)   |
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
			bool d = !!(*cursor & 0x10);
			bool w = !!(*cursor & 0x01);

			--len;
			++cursor;

			if (len <= 0)
			{
				//// ERROR
				NOT_IMPLEMENTED;
			}

			u8 mod = (*cursor & 0xC0) >> 6;
			u8 reg = (*cursor & 0x38) >> 3;
			u8 rm  = (*cursor & 0x07) >> 0;

			--len;
			++cursor;

			if (mod != 0x3) NOT_IMPLEMENTED;
			else
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

				char** regs_table = (w ? regs_w1 : regs_w0);

				char* dst_reg = regs_table[rm];
				char* src_reg = regs_table[reg];

				if (d)
				{
					char* tmp = dst_reg;
					dst_reg = src_reg;
					src_reg = dst_reg;
				}

				printf("mov %s, %s\n", dst_reg, src_reg);
			}
		}
		else
		{
			//// ERROR
			NOT_IMPLEMENTED;
		}
	}

	return 0;
}
