#include <stdint.h>
#include <stdbool.h>

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#define S8_MIN  ( (s8)0x80)
#define S16_MIN ((s16)0x8000)
#define S32_MIN ((s32)0x80000000)
#define S64_MIN ((s32)0x8000000000000000LL)

#define S8_MAX  ( (s8)0x7F)
#define S16_MAX ((s16)0x7FFF)
#define S32_MAX ((s32)0x7FFFFFFF)
#define S64_MAX ((s32)0x7FFFFFFFFFFFFFFFLL)

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define U8_MAX  ( (u8)0xFFU)
#define U16_MAX ((u16)0xFFFFU)
#define U32_MAX ((u32)0xFFFFFFFFU)
#define U64_MAX ((u64)0xFFFFFFFFFFFFFFFFULL)

typedef s64 smm;
typedef u64 umm;

#define SMM_MIN S64_MIN
#define SMM_MAX S64_MAX
#define UMM_MAX U64_MAX

typedef float f32;
typedef double f64;

typedef struct String
{
	char* data;
	u64 len;
} String;

#define STRING(S) (String){ .data = (char*)(S), .len = sizeof(S)-1 }

bool
String_Equal(String a, String b)
{
	if (a.len != b.len) return false;

	for (umm i = 0; i < a.len; ++i)
	{
		if (a.data[i] != b.data[i]) return false;
	}

	return true;
}

u64
CString_Len(char* cstring)
{
	if (cstring == 0) return 0;

	u64 len = 0;
	while (cstring[len] != 0) ++len;

	return len;
}

String
String_FromCString(char* cstring)
{
	return (String){ .data = cstring, .len = CString_Len(cstring) };
}

#define ARRAY_LEN(A) (sizeof(A)/sizeof(0[A]))

bool
IsWhitespace(char c)
{
	return (c != '\n' && (u8)(c - 1) < (u8)0x20);
}

bool
IsWhitespaceOrNewline(char c)
{
	return ((u8)(c - 1) < (u8)0x20);
}

bool
ParseS64(String string, s64* result)
{
	umm i = 0;

	bool is_negative = false;
	if (i < string.len && string.data[i] == '-')
	{
		is_negative = true;
		++i;
	}

	umm start_of_number = i;

	while (i < string.len && string.data[i] == '0') ++i;

	umm start_of_non_zero_digits = i;

	u64 value = 0;
	while (i < string.len && (u8)(string.data[i] - 0x30) < (u8)10)
	{
		value *= 10;
		value += (string.data[i] & 0xF);
		++i;
	}

	if (i == start_of_number)
	{
		//// ERROR: no digits
		return false;
	}
	
	if (i != string.len)
	{
		//// ERROR: contains trailing bs
		return false;
	}

	umm digit_count = i - start_of_non_zero_digits;
	if (digit_count > 19 || (value - (is_negative ? 1 : 0)) >= (1ULL << 63))
	{
		//// ERROR: out of range of s64
		return false;
	}

	*result = (is_negative ? -(s64)value : (s64)value);
	return true;
}
