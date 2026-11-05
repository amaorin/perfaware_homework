#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef s64 smm;
typedef u64 umm;

#define BREAK_IF(EXPR) if (EXPR) break
#define OUT
#define INOUT

#define ASSERT(EX) ((EX) ? (1) : (AssertHandler(__FILE__, __LINE__, #EX), (*(volatile int*)0 = 0), 0))
#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")

void
AssertHandler(char* file, int line, char* ex)
{
	fprintf(stderr, "*** ASSERTION FAILED ***\n");
	fprintf(stderr, "%s(%d): %s\n", file, line, ex);
}

typedef enum Mnemonic
{
	Mnemonic_Add,
	MNEMONIC_COUNT
} Mnemonic;

char* Mnemonic__CStrings[MNEMONIC_COUNT] = {
	[Mnemonic_Add] = "add",
};

char*
Mnemonic_ToCString(Mnemonic mnemonic)
{
	ASSERT(mnemonic >= 0 && mnemonic < MNEMONIC_COUNT);
	return Mnemonic__CStrings[mnemonic];
}

typedef enum Operand_Kind
{
	Operand_Immed,
	Operand_Reg,
	Operand_Mem,
} Operand_Kind;

typedef struct Operand
{
	Operand_Kind kind;
	union
	{
		u16 immed;
		//Reg reg;
		//Effective_Addr mem;
	};
} Operand;

typedef struct Instruction
{
	Mnemonic mnemonic;
	bool wide;
	bool flip_dst_src;
	Operand dst;
	Operand src;
} Instruction;

void
DecodeInstruction(u8* data, umm cursor, OUT umm* advancement, OUT Instruction* instruction)
{
	(void)data, (void)cursor, (void)advancement, (void)instruction;
	NOT_IMPLEMENTED;
}

void
PrintOperand(Operand operand)
{
	(void)operand;
	NOT_IMPLEMENTED;
}

void
PrintInstruction(Instruction instruction)
{
	printf("%s", Mnemonic_ToCString(instruction.mnemonic));
	NOT_IMPLEMENTED;
}

bool
ReadFile(char* path, u8* data, umm cap, OUT umm* size)
{
	bool succeeded = false;

	FILE* file = 0;

	do
	{
		struct stat file_info;
		BREAK_IF(stat(path, &file_info) != 0);

		*size = file_info.st_size;
		BREAK_IF(*size > cap);

		file = fopen(path, "rb");
		BREAK_IF(file == 0);

		umm read_bytes = fread(data, 1, *size, file);
		BREAK_IF(read_bytes != *size);

		succeeded = true;

	} while (0);

	if (file != 0) fclose(file);

	return succeeded;
}

int
main(int argc, char** argv)
{
	char* usage = "Usage: sim86 [--exec] input_binary";

	bool should_execute = false;
	char* input_path    = 0;
	for (int i = 1; i < argc; ++i)
	{
		bool is_flag = (argv[i][0] == '-');

		if (strcmp(argv[i], "--exec") == 0)
		{
			should_execute = true;
		}
		else if (!is_flag && input_path == 0)
		{
			input_path = argv[i];
		}
		else
		{
			fprintf(stderr, "ERROR: illegal %s \"%s\"\n", (is_flag ? "flag" : "option"), argv[i]);
			fprintf(stderr, "%s\n", usage);
			return 1;
		}
	}

	if (input_path == 0)
	{
		fprintf(stderr, "ERROR: missing path to input binary\n");
		fprintf(stderr, "%s\n", usage);
		return 1;
	}

	umm cap  = 1 << 20;
	u8* data = malloc(cap);
	ASSERT(data != 0);

	umm size = 0;
	if (!ReadFile(input_path, data, cap, &size))
	{
		//// ERROR
		fprintf(stderr, "ERROR: failed to read");
		return 1;
	}

	// TODO: accesses to memory should wrap around
	umm cursor = 0;
	while (cursor < size)
	{
		umm advancement         = 0;
		Instruction instruction = {0};
		DecodeInstruction(data, cursor, &advancement, &instruction);
		PrintInstruction(instruction);
		printf("\n");

		cursor += advancement;
		if (cursor > size)
		{
			fprintf(stderr, "WARNING: instruction decoding had to read bytes beyond the end of the input binary\n");
		}
	}

	if (should_execute) NOT_IMPLEMENTED;

	return 0;
}
