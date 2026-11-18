#include "common.h"
#include "haversine_reference.h"

#include <stdio.h>

typedef struct JSON_Field JSON_Field;

typedef JSON_Object
{
	JSON_Field* fields;
} JSON_Object;

typedef JSON_Array
{
	u64 elements_len;
	TJSON_Value* elements;
} JSON_Array;

typedef enum JSON_Value_Kind
{
	JSONValue_Object,
	JSONValue_Array,
	JSONValue_String,
	JSONValue_Number,
	JSONValue_Bool,
	JSONValue_Null,
} JSON_Value_Kind;

typedef struct JSON_Value
{
	TJSON_Value* next;
	JSON_Value_Kind kind;
	union
	{
		JSON_Object object;
		JSON_Array array;
		String string;
		f64 number;
		bool boolean;
	};
} JSON_Value;

typedef struct JSON_Field
{
	String name;
	JSON_Value* value;
} JSON_Field;

typedef enum JSON_Token_Kind
{
	JSONToken_Invalid = 0,
	JSONToken_StartOfFile,
	JSONToken_EndOfFile,

	JSONToken_OpenBrace,
	JSONToken_CloseBrace,
	JSONToken_OpenBracket,
	JSONToken_CloseBracket,
	JSONToken_Colon,
	JSONToken_Comma,
	JSONToken_String,
	JSONToken_Number,
	JSONToken_Boolean,
	JSONToken_Null,
} JSON_Token_Kind;

typedef struct JSON_Token
{
	JSON_Token_Kind kind;
	union
	{
		String string;
		f64 number;
		bool boolean;
	};
} JSON_Token;

typedef struct JSON_Tokenizer
{
	String input;
	u64 cursor;
	JSON_Token current_token;
} JSON_Tokenizer;

JSON_Token JSONTokenizer_NextToken(JSON_Tokenizer* state);

JSON_Tokenizer
JSONTokenizer_Init(String input)
{
	JSON_Tokenizer state = {
		.input  = input,
		.cursor = 0,
	};

	JSONTokenizer_NextToken(&state);

	return state;
}

JSON_Token
JSONTokenizer_GetToken(JSON_Tokenizer* state)
{
	return state->current_token;
}

JSON_Token
JSONTokenizer_NextToken(JSON_Tokenizer* state)
{
	if (state->current_token.kind == JSONToken_Invalid) return state->current_token;

	bool encountered_errors = false;

	state->current_token = (JSON_Token){ .kind = JSONToken_Invalid };

	while (state->cursor < state->input.len)
	{
		char c = state->input.data[state->cursor];

		if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
		{
			++state->cursor;
			continue;
		}
		else break;
	}

	if (state->cursor >= state->input.len)
	{
		state->current_token.kind = JSONToken_EOF;
	}
	else
	{
		char c = state->input.data[state->cursor++];

		switch (c)
		{
			case '{': state->current_token.kind = JSONToken_OpenBrace;    break;
			case '}': state->current_token.kind = JSONToken_CloseBrace;   break;
			case '[': state->current_token.kind = JSONToken_OpenBracket;  break;
			case ']': state->current_token.kind = JSONToken_CloseBracket; break;
			case ':': state->current_token.kind = JSONToken_Colon;        break;
			case ',': state->current_token.kind = JSONToken_Comma;        break;

			default:
			{
				if (Char_IsAlpha(c))
				{
					String identifier = {
						.data = state->input.data - 1,
						.len  = 1,
					};

					while (state->cursor < state->input.len && Char_IsAlpha(state->input.data[state->cursor]))
					{
						++state->cursor;
					}

					identifier.len = state->cursor - identifier.data;

					if (String_Equal(identifier, STRING("true")))
					{
						state->current_token.kind    = JSONToken_Boolean;
						state->current_token.boolean = true;
					}
					else if (String_Equal(identifier, STRING("false")))
					{
						state->current_token.kind    = JSONToken_Boolean;
						state->current_token.boolean = false;
					}
					else if (String_Equal(identifier, STRING("null")))
					{
						state->current_token.kind = JSONToken_Null;
					}
					else
					{
						//// ERROR: invalid identifier
						encountered_errors = true;
					}
				}
				else if (c == '"')
				{
					NOT_IMPLEMENTED;
				}
				else if (Char_IsDigit(c) || c == '-')
				{
					bool is_negative = (c == '-');

					// TODO: remember first digit
					NOT_IMPLEMENTED;
				}
				else
				{
					//// ERROR: invalid symbol
					encountered_errors = true;
				}
			} break;
		}
	}

	if (encountered_errors)
	{
		state->curren_token.kind = JSONToken_Invalid;
	}

	return state->current_token;
}

bool
JSONTokenizer_EatToken(JSON_Tokenizer* state, JSON_Token_Kind kind)
{
	if (JSONTokenizer_GetToken(state).kind == kind)
	{
		JSONTokenizer_NextToken(state);
		return true;
	}

	return false;
}

bool
JSON__ParseObject()
{
}

bool
JSON__ParseArray()
{
}

bool
JSON__ParseValue()
{
}

bool
JSON_Parse(String input)
{
}

int
main(int argc, char** argv)
{
	char* usage = "haversine_processor <input json> [answer file]";

	if (argc != 3)
	{
		//// ERROR
		fprintf(stderr, "ERROR: invalid number of arguments\nUsage: %s\n", usage);
		return 1;
	}

	char* input_filename  = argv[1];
	char* answer_filename = (argc > 2 ? argv[2] : 0);

	return 0;
}
