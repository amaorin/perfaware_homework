#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define ASSERT(EX) ((EX) ? (1) : (AssertHandler(__FILE__, __LINE__, #EX), (*(volatile int*)0 = 0), 0))
#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")

void
AssertHandler(char* file, int line, char* ex)
{
	fprintf(stderr, "*** ASSERTION FAILED ***\n");
	fprintf(stderr, "%s(%d): %s\n", file, line, ex);
}

typedef struct Instruction
{
} Instruction;

DecodeInstruction(, Instruction* instruction)
{
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

	if (should_execute)
	{
		NOT_IMPLEMENTED;
	}
	else
	{
		Instruction instruction;
		umm advance;
		DecodeInstruction
	}

	return 0;
}
