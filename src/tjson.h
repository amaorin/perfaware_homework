#include <stdio.h>

typedef enum TJSONWriter_State
{
	TJSONWriterState_Root     = 0,
	TJSONWriterState_Object   = 1,
	TJSONWriterState_Array    = 2,

	TJSONWriterState_Property = 4,
	TJSONWriterState_Value    = 8,
} TJSONWriter_State;

typedef void TJSONWriter_Token;

typedef struct TJSONWriter
{
	FILE* file;
	bool should_indent;
	TJSONWriter_State state;
	u32 indent;
} TJSONWriter;

TJSONWriter
TJSONWriter_Init(FILE* file, bool should_indent)
{
	return (TJSONWriter){
		.file          = file,
		.should_indent = should_indent,
		.state         = TJSONWriterState_Root,
		.indent        = 0,
	};
}

void
TJSONWriter__Stitch(TJSONWriter* tjson, int indent_delta)
{
	if (((tjson->state & TJSONWriterState_Array) || (tjson->state & TJSONWriterState_Object))
	 && (tjson->state & TJSONWriterState_Value)
	 && (indent_delta >= 0))
	{
		fprintf(tjson->file, ",");
	}

	if (indent_delta == -1)
	{
		tjson->indent -= 1;
	}

	if(tjson->should_indent)
	{
		if (tjson->state & TJSONWriterState_Property)
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
	}

	if (indent_delta == 1)
	{
		tjson->indent += 1;
	}

	tjson->state &= ~(TJSONWriterState_Property | TJSONWriterState_Value);
}

TJSONWriter_Token*
TJSONWriter_WriteStartObject(TJSONWriter* tjson)
{
	TJSONWriter__Stitch(tjson, 1);
	fprintf(tjson->file, "{");

	TJSONWriter_Token* token = (TJSONWriter_Token*)tjson->state;
	tjson->state = TJSONWriterState_Object;

	return token;
}

void
TJSONWriter_WriteEndObject(TJSONWriter* tjson, TJSONWriter_Token* token)
{
	ASSERT(((tjson->state & ~TJSONWriterState_Value) & TJSONWriterState_Object) == TJSONWriterState_Object);
	TJSONWriter__Stitch(tjson, -1);
	fprintf(tjson->file, "}");

	tjson->state = (TJSONWriter_State)(umm)token | TJSONWriterState_Value;
}

TJSONWriter_Token*
TJSONWriter_WriteStartArray(TJSONWriter* tjson)
{
	TJSONWriter__Stitch(tjson, 1);
	fprintf(tjson->file, "[");

	TJSONWriter_Token* token = (TJSONWriter_Token*)tjson->state;
	tjson->state = TJSONWriterState_Array;

	return token;
}

void
TJSONWriter_WriteEndArray(TJSONWriter* tjson, TJSONWriter_Token* token)
{
	ASSERT(((tjson->state & ~TJSONWriterState_Value) & TJSONWriterState_Array) == TJSONWriterState_Array);
	TJSONWriter__Stitch(tjson, -1);
	fprintf(tjson->file, "]");

	tjson->state = (TJSONWriter_State)(umm)token | TJSONWriterState_Value;
}

void
TJSONWriter_WritePropertyName(TJSONWriter* tjson, char* name)
{
	ASSERT(((tjson->state & ~TJSONWriterState_Value) & TJSONWriterState_Object) == TJSONWriterState_Object);
	TJSONWriter__Stitch(tjson, 0);
	fprintf(tjson->file, "\"%s\":", name);

	tjson->state |= TJSONWriterState_Property;
}

void
TJSONWriter__Value(TJSONWriter* tjson)
{
	ASSERT(tjson->state != TJSONWriterState_Root);
	TJSONWriter__Stitch(tjson, 0);
	tjson->state |= TJSONWriterState_Value;
}

void
TJSONWriter_WriteNull(TJSONWriter* tjson)
{
	TJSONWriter__Value(tjson);
	fprintf(tjson->file, "null");
}

void
TJSONWriter_WriteValueBool(TJSONWriter* tjson, bool value)
{
	TJSONWriter__Value(tjson);
	fprintf(tjson->file, "%s", (value ? "true" : "false"));
}

void
TJSONWriter_WriteValueF64(TJSONWriter* tjson, f64 value)
{
	TJSONWriter__Value(tjson);
	fprintf(tjson->file, "%.17f", value);
}

#define CONCAT__(A, B) A##B
#define CONCAT_(A, B) CONCAT__(A, B)
#define CONCAT(A, B) CONCAT_(A, B)

#define TJSONWriter__SCOPED(TJSONWriter, BEGIN, END) for (TJSONWriter_Token CONCAT(*tjson_loop, __LINE__) = (TJSONWriter_Token*)1, CONCAT(*tjson_token, __LINE__) = BEGIN(TJSONWriter); CONCAT(tjson_loop, __LINE__) != 0; END((TJSONWriter), CONCAT(tjson_token, __LINE__)), CONCAT(tjson_loop, __LINE__) = 0)
#define TJSONWriter_WriteObject(TJSONWriter) TJSONWriter__SCOPED(TJSONWriter, TJSONWriter_WriteStartObject, TJSONWriter_WriteEndObject)
#define TJSONWriter_WriteArray(TJSONWriter) TJSONWriter__SCOPED(TJSONWriter, TJSONWriter_WriteStartArray, TJSONWriter_WriteEndArray)
