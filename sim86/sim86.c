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

typedef struct Error
{
} Error;

Error
ReadBinaryFile(char* path)
{
	bool succeeded = false;

	FILE* file = 0;

	do
	{
		struct stat info;
		if (stat(path, &info) != 0)
		{
			//// ERROR
			error_msg = "failed to stat the specified file";
			break;
		}

		file = fopen(path, "rb");
		if (file == 0)
		{
			//// ERROR
			error_msg = "failed to open the file for reading in binary mode";
			break;
		}

		if (fread(, 1, info.st_size, file) != info.st_size)
		{
			//// ERROR
			error_msg = "failed to read the contents of the specified file";
			break;
		}

		succeeded = true;

	} while (0);

	if (file != 0) fclose(file);

	return (Error){ .succeeded = succeeded, error_msg = error_msg };
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

	char* input_file_path = argv[1];

	if (!ReadBinaryFile(input_file_path))
	{
		//// ERROR
		fprintf(stderr, "Failed to read input file\n");
		return 1;
	}

	return 0;
}
