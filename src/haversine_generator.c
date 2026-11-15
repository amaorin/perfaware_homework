#include "common.h"
#include "haversine_reference.h"

#include <stdlib.h>
#include <stdarg.h>
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
	bool should_indent;
	TJSON_State state;
	u32 indent;
} TJSON;

TJSON
TJSON_Init(FILE* file, bool should_indent)
{
	return (TJSON){
		.file          = file,
		.should_indent = should_indent,
		.state         = TJSONState_Root,
		.indent        = 0,
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

	if(tjson->should_indent)
	{
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

typedef struct Xorshift64_State
{
	u64 x;
} Xorshift64_State;

Xorshift64_State
Xorshift64_Seed(u64 seed)
{
	if (seed == 0) seed = 1;

	return (Xorshift64_State){ .x = seed };
}

u64
Xorshift64_Next(Xorshift64_State* state)
{
	state->x ^= state->x << 13;
	state->x ^= state->x >> 17;
	state->x ^= state->x << 5;

	return state->x;
}

u64
RandomInRangeU64(u64 uniform_random_number, u64 min_val, u64 max_val)
{
	// TODO: this is biased
	return min_val + uniform_random_number % (1 + (max_val - min_val));
}

f64
Random01F64(u64 uniform_random_number)
{
	// 1      11                                 52
	// seee eeee eeee mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm
	//
	// bias = 2^10 - 1
	// => 2^0 -> 0 + 1023 = 1023
	// 1.xxx * 2^0 -> 3F FX XX XX XX XX XX XX

	u64 zero_exponent   = 0x3FF0000000000000;
	u64 random_mantissa = uniform_random_number & 0x000FFFFFFFFFFFFF;

	f64 random_number_from_1_to_2 = (F64_Bits){ .bits = zero_exponent | random_mantissa }.f;

	return random_number_from_1_to_2 - 1;
}

f64
RandomInRangeF64(u64 uniform_random_number, f64 min_val, f64 max_val)
{
	f64 result = min_val + Random01F64(uniform_random_number)*(max_val - min_val);
	if (result < min_val) result = min_val;
	if (result > max_val) result = max_val;

	return result;
}

char*
Format(char* format, ...)
{
	char* result = "FORMAT_ERROR";

	va_list args;
	va_start(args, format);
	int num_chars = vsnprintf(0, 0, format, args);
	va_end(args);

	if (num_chars >= 0)
	{
		char* memory = malloc(num_chars + 1);

		if (memory != 0)
		{
			va_start(args, format);
			vsnprintf(memory, num_chars+1, format, args);
			va_end(args);

			memory[num_chars] = 0;

			result = memory;
		}
	}

	return result;
}

typedef struct Cluster
{
	f64 min_x;
	f64 min_y;
	f64 width;
	f64 height;
} Cluster;

typedef struct Pair
{
	f64 x0;
	f64 y0;
	f64 x1;
	f64 y1;
} Pair;

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

	Xorshift64_State rng_state = Xorshift64_Seed(random_seed);

	Cluster clusters[8] = {};
	f64 min_cluster_size = 5;
	f64 max_cluster_size = 60;

	if (should_cluster)
	{
		for (umm i = 0; i < ARRAY_LEN(clusters); ++i)
		{
			clusters[i] = (Cluster){
				.min_x  = RandomInRangeF64(Xorshift64_Next(&rng_state), -180, +180),
				.min_y  = RandomInRangeF64(Xorshift64_Next(&rng_state),  -90,  +90),
				.width  = RandomInRangeF64(Xorshift64_Next(&rng_state), min_cluster_size, max_cluster_size),
				.height = RandomInRangeF64(Xorshift64_Next(&rng_state), min_cluster_size, max_cluster_size),
			};
		}
	}

	Pair* pairs = malloc(sizeof(Pair)*num_pairs);

	if (pairs == 0)
	{
		//// ERROR
		fprintf(stderr, "ERROR: failed to allocate memory for %lu pairs.\n", num_pairs);
		return 1;
	}

	for (umm i = 0; i < num_pairs; ++i)
	{
		Cluster cluster_0 = (Cluster){ .min_x = -180, .min_y = -90, .width = 360, .height = 180 };
		Cluster cluster_1 = (Cluster){ .min_x = -180, .min_y = -90, .width = 360, .height = 180 };

		if (should_cluster)
		{
			cluster_0 = clusters[RandomInRangeU64(Xorshift64_Next(&rng_state), 0, ARRAY_LEN(clusters)-1)];
			cluster_1 = clusters[RandomInRangeU64(Xorshift64_Next(&rng_state), 0, ARRAY_LEN(clusters)-1)];
		}

		f64 x0 = cluster_0.min_x + Random01F64(Xorshift64_Next(&rng_state))*cluster_0.width;
		f64 y0 = cluster_0.min_y + Random01F64(Xorshift64_Next(&rng_state))*cluster_0.height;
		f64 x1 = cluster_1.min_x + Random01F64(Xorshift64_Next(&rng_state))*cluster_1.width;
		f64 y1 = cluster_1.min_y + Random01F64(Xorshift64_Next(&rng_state))*cluster_1.height;

		// we only add on top of a value that is already in range, so it is impossible for
		// the result to be less than the minimum value. Therefore it is only necessary to
		// renormalize the overshoot over the maximum value.
		if (x0 > 180) x0 -= 360;
		if (x1 > 180) x1 -= 360;
		if (y0 >  90) y0 -= 180;
		if (y1 >  90) y1 -= 180;

		pairs[i] = (Pair){
			.x0 = x0,
			.y0 = y0,
			.x1 = x1,
			.y1 = y1,
		};
	}

	char* json_filename = Format("input_%lu.json", num_pairs);
	FILE* json_file = fopen(json_filename, "wb");
	if (json_file == 0)
	{
		//// ERROR
		fprintf(stderr, "ERROR: failed to open \"%s\" for writing output json.\n", json_filename);
		return 1;
	}
	else
	{
		TJSON tjson = TJSON_Init(json_file, true);

		TJSON_WriteObject(&tjson)
		{
			TJSON_WritePropertyName(&tjson, "pairs");
			TJSON_WriteArray(&tjson)
			{
				for (umm i = 0; i < num_pairs; ++i)
				{
					TJSON_WriteObject(&tjson)
					{
						TJSON_WritePropertyName(&tjson, "x0");
						TJSON_WriteValueF64(&tjson, pairs[i].x0);

						TJSON_WritePropertyName(&tjson, "y0");
						TJSON_WriteValueF64(&tjson, pairs[i].y0);

						TJSON_WritePropertyName(&tjson, "x1");
						TJSON_WriteValueF64(&tjson, pairs[i].x1);

						TJSON_WritePropertyName(&tjson, "y1");
						TJSON_WriteValueF64(&tjson, pairs[i].y1);
					}
				}
			}
		}

		fclose(json_file);
	}

	f64 expected_sum = 0;

	char* binary_filename = Format("answer_%lu.bin", num_pairs);
	FILE* binary_file = fopen(binary_filename, "wb");
	if (binary_file == 0)
	{
		//// ERROR
		fprintf(stderr, "ERROR: failed to open \"%s\" for writing output json.\n", binary_filename);
		return 1;
	}
	else
	{
		fwrite(&num_pairs, sizeof(u64), 1, binary_file);

		for (umm i = 0; i < num_pairs; ++i)
		{
			Pair* p = &pairs[i];
			f64 answer = ReferenceHaversineOfDegrees(p->x0, p->y0, p->x1, p->y1, EARTH_RADIUS_METERS);

			fwrite(&answer, sizeof(f64), 1, binary_file);

			expected_sum += answer * (1 / (f64)(num_pairs));
		}

		fwrite(&expected_sum, sizeof(f64), 1, binary_file);

		fclose(binary_file);
	}

	printf("Method:       %s\n", (should_cluster ? "cluster" : "uniform"));
	printf("Random seed:  %lu\n", random_seed);
	printf("Pair count:   %lu\n", num_pairs);
	printf("Expected sum: %f\n", expected_sum);

	return 0;
}
