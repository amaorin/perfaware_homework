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
	FILE* file;
	TJSON_State state;
	u32 indent;
} TJSON;

TJSON
TJSON_Init(FILE* file)
{
	return (TJSON){
		.file   = file,
		.state  = TJSONState_Root,
		.indent = 0,
	};
}

void
TJSON__Stitch(TJSON* tjson, int indent_delta)
{
	if (((tjson->state & TJSONState_Array) || (tjson->state & TJSONState_Object))
	 && (tjson->state & TJSONState_Value)
	 && (indent_delta >= 0))
	{
		fprintf(tjson->file, ",");
	}

	if (indent_delta == -1)
	{
		tjson->indent -= 1;
	}

	if (tjson->state & TJSONState_Property)
	{
		fprintf(tjson->file, " ");
	}
	else
	{
		fprintf(tjson->file, "\n");

		for (umm i = 0; i < tjson->indent; ++i)
		{
			fprintf(tjson->file, "    ");
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
	fprintf(tjson->file, "{");

	TJSON_Token* token = (TJSON_Token*)tjson->state;
	tjson->state = TJSONState_Object;

	return token;
}

void
TJSON_WriteEndObject(TJSON* tjson, TJSON_Token* token)
{
	ASSERT(((tjson->state & ~TJSONState_Value) & TJSONState_Object) == TJSONState_Object);
	TJSON__Stitch(tjson, -1);
	fprintf(tjson->file, "}");

	tjson->state = (TJSON_State)(umm)token | TJSONState_Value;
}

TJSON_Token*
TJSON_WriteStartArray(TJSON* tjson)
{
	TJSON__Stitch(tjson, 1);
	fprintf(tjson->file, "[");

	TJSON_Token* token = (TJSON_Token*)tjson->state;
	tjson->state = TJSONState_Array;

	return token;
}

void
TJSON_WriteEndArray(TJSON* tjson, TJSON_Token* token)
{
	ASSERT(((tjson->state & ~TJSONState_Value) & TJSONState_Array) == TJSONState_Array);
	TJSON__Stitch(tjson, -1);
	fprintf(tjson->file, "]");

	tjson->state = (TJSON_State)(umm)token | TJSONState_Value;
}

void
TJSON_WritePropertyName(TJSON* tjson, char* name)
{
	ASSERT(((tjson->state & ~TJSONState_Value) & TJSONState_Object) == TJSONState_Object);
	TJSON__Stitch(tjson, 0);
	fprintf(tjson->file, "\"%s\":", name);

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
	fprintf(tjson->file, "null");
}

void
TJSON_WriteValueBool(TJSON* tjson, bool value)
{
	TJSON__Value(tjson);
	fprintf(tjson->file, "%s", (value ? "true" : "false"));
}

void
TJSON_WriteValueF64(TJSON* tjson, f64 value)
{
	TJSON__Value(tjson);
	fprintf(tjson->file, "%f", value);
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
	char* usage = "haversine_generator {uniform|cluster} <random seed> <number of pairs to generate>";

	if (argc != 4)
	{
		//// ERROR
		fprintf(stderr, "ERROR: invalid number of arguments.\nUsage: %s\n", usage);
		return 1;
	}

	String clustering_arg = String_FromCString(argv[1]);
	String seed_arg       = String_FromCString(argv[2]);
	String num_pairs_arg  = String_FromCString(argv[3]);

	bool should_cluster  = false;
	u64 random_seed      = 0;
	u64 num_pairs        = 0;

	if      (String_Equal(clustering_arg, STRING("cluster"))) should_cluster = true;
	else if (String_Equal(clustering_arg, STRING("uniform"))) should_cluster = false;
	else
	{
		//// ERROR
		fprintf(stderr, "ERROR: invalid clustering option \"%.*s\".\nUsage: %s\n", (int)clustering_arg.len, clustering_arg.data, usage);
		return 1;
	}

	if (!String_ParseU64(seed_arg, &random_seed))
	{
		//// ERROR
		fprintf(stderr, "ERROR: random seed must be a valid 64 bit unsigned integer specified in base 10.\nUsage: %s\n", usage);
		return 1;
	}

	if (!String_ParseU64(num_pairs_arg, &num_pairs))
	{
		//// ERROR
		fprintf(stderr, "ERROR: number of pairs to generate must be a valid 64 bit unsigned integer specified in base 10.\nUsage: %s\n", usage);
		return 1;
	}

	if (num_pairs == 0)
	{
		return 0;
	}

	TJSON tjson = TJSON_Init(stdout);

	TJSON_WriteObject(&tjson)
	{
		TJSON_WritePropertyName(&tjson, "pairs");
		TJSON_WriteArray(&tjson)
		{
			for (umm i = 0; i < num_pairs; ++i)
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
