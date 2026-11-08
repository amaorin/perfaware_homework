#include <stdint.h>
#include <stdbool.h>

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#define S8_MIN  ((s8) (0x80))
#define S16_MIN ((s16)(0x8000))
#define S32_MIN ((s32)(0x80000000L))
#define S64_MIN ((s64)(0x8000000000000000LL))

#define S8_MAX  ((s8) (0x7F))
#define S16_MAX ((s16)(0x7FFF))
#define S32_MAX ((s32)(0x7FFFFFFFL))
#define S64_MAX ((s64)(0x7FFFFFFFFFFFFFFFLL))

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define U8_MAX  ((u8) (0xFF))
#define U16_MAX ((u16)(0xFFFF))
#define U32_MAX ((u32)(0xFFFFFFFFLU))
#define U64_MAX ((u64)(0xFFFFFFFFFFFFFFFFLLU))

typedef s64 smm;
typedef u64 umm;

typedef float f32;
typedef double f64;

#define ASSERT(EX) ((EX) ? (1) : ((*(volatile int*)0 = 0), 0))

bool
Char_IsDigit(char c)
{
	return ((u8)(c - '0') < (u8)10);
}

bool
Char_IsAlpha(char c)
{
	return ((u8)((c & 0xDF) - 'A') <= (u8)('Z' - 'A'));
}

umm
CString_Len(char* cstring)
{
	if (cstring == 0) return 0;

	umm result = 0;

	while (cstring[result] != 0) ++result;

	return result;
}

typedef struct String
{
	char* data;
	u32 len;
} String;

#define STRING(S) (String){ .data = (char*)(S), .len = (u32)(sizeof(S)-1) }

String
String_FromCString(char* cstring)
{
	umm len = CString_Len(cstring);
	ASSERT(len <= U32_MAX);

	return (String){
		.data = cstring,
		.len  = len,
	};
}

bool
String_Equal(String a, String b)
{
	if (a.len != b.len) return false;

	for (umm i = 0; i < a.len; ++i)
	{
		if (a.data[i] != b.data[i])
		{
			return false;
		}
	}

	return true;
}

bool
String_ParseU64(String string, u64* result)
{
	if (string.len == 0)
	{
		//// ERROR: no digits
		return false;
	}

	umm i = 0;

	umm leading_zeros = 0;
	for (; i < string.len; ++i)
	{
		if (string.data[i] != '0')
		{
			break;
		}

		++leading_zeros;
	}

	u64 value = 0;
	for (; i < string.len; ++i)
	{
		if (!Char_IsDigit(string.data[i]))
		{
			//// ERROR: non digit character
			return false;
		}

		value *= 10;
		value += (string.data[i] & 0xF);
	}

	umm digit_count = string.len - leading_zeros;

	// TODO: this as a bunch of false positives since log2(10^19) ~= 63.1166
	if (digit_count >= 20)
	{
		//// ERROR: overflow
		return false;
	}

	*result = value;
	return true;
}
