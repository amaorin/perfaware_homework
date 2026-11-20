#include "common.h"
#include "haversine_reference.h"

#include <stdio.h>

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
					char* start = &state->input.data[state->cursor-1],

					while (state->cursor < state->input.len && Char_IsAlpha(state->input.data[state->cursor]))
					{
						++state->cursor;
					}

					String identifier = {
						.data = start,
						.len  = &state->input.data[state->cursor] - start,
					};

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
					char* start = &state->input.data[state->cursor];

					bool contains_escape_seq = false;

					while (state->cursor < state->input.len && state->input.data[state->cursor] == '"')
					{
						if (state->input.data[state->cursor] == '\\' && state->cursor+1 < state->input.len)
						{
							state->cursor += 2;
						}
						else state->cursor += 1;
					}

					if (state->cursor >= state->input.len)
					{
						//// ERROR: unterminated string literal
						encountered_errors = true;
					}
					else
					{
						ASSERT(state->input.data[state->cursor] == '"');

						String raw_string = {
							.data = start,
							.len  = &state->input.data[state->cursor] - start,
						};

						state->cursor += sizeof('"');

						char* memory = malloc(raw_string.size);

						String string = {
							.data = memory,
							.len  = 0,
						};

						umm i = 0;
						while (i < raw_string.len)
						{
							if (raw_string.data[i] != '\\')
							{
								string.data[string.len] = raw_string.data[i];
								++string.len;
								++i;
							}
							else
							{
								ASSERT(raw_string.data[i] == '\\');
								++i;

								// no need to check for length first since a terminated string cannot end in a '\'
								char e = raw_string.data[i];
								++i;

								switch (e)
								{
									case 'b': string.data[string.len++] = '\b'; break;
									case 'f': string.data[string.len++] = '\f'; break;
									case 'n': string.data[string.len++] = '\n'; break;
									case 'r': string.data[string.len++] = '\r'; break;
									case 't': string.data[string.len++] = '\t'; break;

									case 'u':
									{
										if (i+4 >= raw_string.len)
										{
											//// ERROR: missing digits
											encountered_errors = true;
											break;
										}

										char* inp = &raw_string.data[i];
										i += 4;

										u32 value = 0;

										for (umm j = 0; j < 4; ++j)
										{
											value <<= 4;

											if      (Char_IsDigit(inp[j]))         value |= inp[j] - '0';
											else if (Char_IsHexAlphaDigit(inp[j])) value |= inp[j] - 'A' + 10;
											else
											{
												//// ERROR: invalid digit
												encountered_errors = true;
												break;
											}
										}

										if (!encountered_errors)
										{
											if (value <= 0x7F)
											{
												string.data[string.len++] = (u8)value;
											}
											else if (value <= 0x07FF)
											{
												string.data[string.len++] = 0xC0 | (u8)((value >> 6);
												string.data[string.len++] = 0x80 | (u8)((value >> 0);
											}
											else
											{
												string.data[string.len++] = 0xE0 | (u8)((value >> 12);
												string.data[string.len++] = 0x80 | (u8)((value >>  6);
												string.data[string.len++] = 0x80 | (u8)((value >>  0);
											}
										}
									} break;

									default:
									{
										//// ERROR: unknown escape sequence
										encountered_errors = true;
									} break;
								}
							}
						}

						if (!encountered_errors)
						{
							state->current_token.kind   = Token_String;
							state->current_token.string = string;
						}
					}
				}
				else if (Char_IsDigit(c) || c == '-')
				{
					bool is_negative  = false;
					umm digit_count   = 0;
					u64 integer_value = 0;
					if (c == '-')
					{
						is_negative   = true;
						digit_count   = 1;
						integer_value = c - '0';
					}

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
