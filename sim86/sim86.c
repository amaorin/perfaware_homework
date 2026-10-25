// this code is uttter hargabe and I am too stupud to fix it
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
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

#ifndef ASSERT_HANDLER
#  define ASSERT_HANDLER(FILE, LINE, EX) DefaultAssertHandler((FILE), (LINE), (EX))
#endif

#define ASSERT(EX) ((EX) ? 1 : (ASSERT_HANDLER(__FILE__, __LINE__, #EX), (*(volatile int*)0 = 0), 0))
#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")

void
DefaultAssertHandler(char* file, int line, char* ex)
{
	fprintf(stderr, "*** ASSERTION FAILED ***\n");
	fprintf(stderr, "%s(%d): %s\n", file, line, ex);
}

#define OUT
#define INOUT

#define IS_POW2(N) ((N) != 0 && ((N) & ((N) - 1)) == 0)
#define ALIGN(N, A) (((N) + (A) - 1) & (-(A)))

typedef struct Memory
{
	u8 data[1 << 20];
} Memory;

bool
ReadFileIntoMemory(char* path, Memory* memory)
{
	struct stat stat_info = {0};
	if (stat(path, &stat_info) != 0)
	{
		//// ERROR
		fprintf(stderr, "ERROR: failed to stat \"%s\"\n", path);
		return false;
	}

	if (stat_info.st_size > sizeof(Memory))
	{
		//// ERROR
		fprintf(stderr, "ERROR: the input binary is too large\n");
		return false;
	}

	FILE* file       = fopen(path, "rb");
	umm fread_result = 0;
	if (file != 0)
	{
		fread_result = fread(memory->data, 1, len, file);
		fclose(file);
	}

	if (file == 0 || fread_result != len)
	{
		//// ERROR
		fprintf(stderr, "ERROR: failed to read the input binary at path \"%s\"\n", path);
		return false;
	}

	return true;
}

int
main(int argc, char** argv)
{
	if (argc != 2)
	{
		//// ERROR
		fprintf(stderr, "Invalid Arguments\n");
		return 1;
	}

	char* path = argv[1];

	Memory* memory = malloc(sizeof(Memory));
	if (!ReadFileIntoMemory(path, memory))
	{
		//// ERROR
		return 1;
	}

	return 0;
}
