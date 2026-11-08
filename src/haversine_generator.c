#include "common.h"

#include <stdio.h>

typedef enum TJSON_State
{
	TJSONState_Root     = 0,
	TJSONState_Object   = 1,
	TJSONState_Array    = 2,

	TJSONState_Property = 4,
	TJSONState_Value    = 8,
} TJSON_State;

typedef void TJSON_Token;

typedef struct TJSON
{
	TJSON_State state;
	u32 indent;
} TJSON;

TJSON
TJSON_Init()
{
	return (TJSON){
		.state  = TJSONState_Root,
		.indent = 0,
	};
}

void
TJSON__Stitch(TJSON* tjson, int indent_delta)
{
	if (((tjson->state & TJSONState_Array) || (tjson->state & TJSONState_Object)) && (tjson->state & TJSONState_Value))
	{
		printf(",");
	}

	if (indent_delta == -1)
	{
		tjson->indent -= 1;
	}

	if (tjson->state & TJSONState_Property)
	{
		printf(" ");
	}
	else
	{
		printf("\n");

		for (umm i = 0; i < tjson->indent; ++i)
		{
			printf("    ");
		}
	}

	if (indent_delta == 1)
	{
		tjson->indent += 1;
	}

	tjson->state &= ~(TJSONState_Property | TJSONState_Value);
}

TJSON_Token*
TJSON_WriteStartObject(TJSON* tjson)
{
	TJSON__Stitch(tjson, 1);
	printf("{");

	TJSON_Token* token = (TJSON_Token*)tjson->state;
	tjson->state = TJSONState_Object;

	return token;
}

void
TJSON_WriteEndObject(TJSON* tjson, TJSON_Token* token)
{
	ASSERT(((tjson->state & ~TJSONState_Value) & TJSONState_Object) == TJSONState_Object);
	TJSON__Stitch(tjson, -1);
	printf("}");

	tjson->state = (TJSON_State)(umm)token | TJSONState_Value;
}

TJSON_Token*
TJSON_WriteStartArray(TJSON* tjson)
{
	TJSON__Stitch(tjson, 1);
	printf("[");

	TJSON_Token* token = (TJSON_Token*)tjson->state;
	tjson->state = TJSONState_Array;

	return token;
}

void
TJSON_WriteEndArray(TJSON* tjson, TJSON_Token* token)
{
	ASSERT(((tjson->state & ~TJSONState_Value) & TJSONState_Array) == TJSONState_Array);
	TJSON__Stitch(tjson, -1);
	printf("]");

	tjson->state = (TJSON_State)(umm)token | TJSONState_Value;
}

void
TJSON_WritePropertyName(TJSON* tjson, char* name)
{
	ASSERT(((tjson->state & ~TJSONState_Value) & TJSONState_Object) == TJSONState_Object);
	TJSON__Stitch(tjson, 0);
	printf("\"%s\":", name);

	tjson->state |= TJSONState_Property;
}

void
TJSON__Value(TJSON* tjson)
{
	ASSERT(tjson->state != TJSONState_Root);
	TJSON__Stitch(tjson, 0);
	tjson->state |= TJSONState_Value;
}

void
TJSON_WriteNull(TJSON* tjson)
{
	TJSON__Value(tjson);
	printf("null");
}

void
TJSON_WriteValueBool(TJSON* tjson, bool value)
{
	TJSON__Value(tjson);
	printf("%s", (value ? "true" : "false"));
}

void
TJSON_WriteValueF64(TJSON* tjson, f64 value)
{
	TJSON__Value(tjson);
	printf("%f", value);
}

#define CONCAT__(A, B) A##B
#define CONCAT_(A, B) CONCAT__(A, B)
#define CONCAT(A, B) CONCAT_(A, B)

#define TJSON__SCOPED(TJSON, BEGIN, END) for (TJSON_Token CONCAT(*tjson_loop, __LINE__) = (TJSON_Token*)1, CONCAT(*tjson_token, __LINE__) = BEGIN(TJSON); CONCAT(tjson_loop, __LINE__) != 0; END((TJSON), CONCAT(tjson_token, __LINE__)), CONCAT(tjson_loop, __LINE__) = 0)
#define TJSON_WriteObject(TJSON) TJSON__SCOPED(TJSON, TJSON_WriteStartObject, TJSON_WriteEndObject)
#define TJSON_WriteArray(TJSON) TJSON__SCOPED(TJSON, TJSON_WriteStartArray, TJSON_WriteEndArray)

int
main(int argc, char** argv)
{
	TJSON tjson = TJSON_Init();

	TJSON_Token* t0 =TJSON_WriteStartObject(&tjson);
	{
		TJSON_WritePropertyName(&tjson, "pairs");
		TJSON_Token* t1 = TJSON_WriteStartArray(&tjson);
		{
			for (umm i = 0; i < 2; ++i)
			{
				f64 x0 = 0;
				f64 y0 = 0;
				f64 x1 = 0;
				f64 y1 = 0;

				TJSON_Token* t2 = TJSON_WriteStartObject(&tjson);
				{
					TJSON_WritePropertyName(&tjson, "x0");
					TJSON_WriteValueF64(&tjson, x0);

					TJSON_WritePropertyName(&tjson, "y0");
					TJSON_WriteValueF64(&tjson, y0);

					TJSON_WritePropertyName(&tjson, "x1");
					TJSON_WriteValueF64(&tjson, x1);

					TJSON_WritePropertyName(&tjson, "y1");
					TJSON_WriteValueF64(&tjson, y1);
				}
				TJSON_WriteEndObject(&tjson, t2);
			}
		}
		TJSON_WriteEndArray(&tjson, t1);
	}
	TJSON_WriteEndObject(&tjson, t0);


	TJSON_WriteObject(&tjson)
	{
		TJSON_WritePropertyName(&tjson, "pairs");
		TJSON_WriteArray(&tjson)
		{
			for (umm i = 0; i < 2; ++i)
			{
				f64 x0 = 0;
				f64 y0 = 0;
				f64 x1 = 0;
				f64 y1 = 0;

				TJSON_WriteObject(&tjson)
				{
					TJSON_WritePropertyName(&tjson, "x0");
					TJSON_WriteValueF64(&tjson, x0);

					TJSON_WritePropertyName(&tjson, "y0");
					TJSON_WriteValueF64(&tjson, y0);

					TJSON_WritePropertyName(&tjson, "x1");
					TJSON_WriteValueF64(&tjson, x1);

					TJSON_WritePropertyName(&tjson, "y1");
					TJSON_WriteValueF64(&tjson, y1);
				}
			}
		}
	}

	return 0;
}
