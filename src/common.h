#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

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
