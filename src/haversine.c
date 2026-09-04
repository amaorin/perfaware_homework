#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"
#include "reference_haversine.h"

typedef struct Pair
{
	f64 x0;
	f64 y0;
	f64 x1;
	f64 y1;
} Pair;

bool
ReadEntireFile(char* path, char** out_contents, umm* out_file_size)
{
	bool encountered_errors = false;

	*out_contents  = 0;
	*out_file_size = 0;

	FILE* file          = 0;
	char* file_contents = 0;
	umm file_size       = 0;
	do
	{
		file = fopen(path, "rb");

		if (file == 0)
		{
			//// ERROR
			encountered_errors = true;
			break;
		}

		struct stat file_stat;
		if (stat(path, &file_stat) != 0)
		{
			//// ERROR
			encountered_errors = true;
			break;
		}

		file_size = file_stat.st_size;

		file_contents = malloc(file_size + 1);

		if (file_contents == 0)
		{
			//// ERROR
			encountered_errors = true;
			break;
		}

		if (fread(file_contents, 1, file_size, file) != file_size)
		{
			//// ERROR
			encountered_errors = true;
			break;
		}

		file_contents[file_size] = 0;
	} while (0);

	if (file != 0) fclose(file);

	if (encountered_errors) free(file_contents);
	else
	{
		*out_contents  = file_contents;
		*out_file_size = file_size;
	}

	return !encountered_errors;
}

int
main(int argc, char** argv)
{
	char* usage = "Usage: haversine <path to json file> <path to answer file>";

	if (argc < 2 || argc > 3)
	{
		//// ERROR
		fprintf(stderr, "Invalid number of arguments.\n%s\n", usage);
		return 1;
	}

	char* json_path   = argv[1];
	char* answer_path = (argc == 3 ? argv[2] : 0);

	char* json_contents = 0;
	umm json_file_size  = 0;
	if (!ReadEntireFile(json_path, &json_contents, &json_file_size))
	{
		//// ERROR
		fprintf(stderr, "ERROR: failed to read json file\n");
		return 1;
	}

	f64* answers                 = 0;
	umm expected_number_of_pairs = 0;
	f64 expected_sum             = 0;
	if (answer_path != 0)
	{
		char* answer_contents = 0;
		umm answer_file_size  = 0;
		if (!ReadEntireFile(answer_path, &answer_contents, &answer_file_size))
		{
			//// ERROR
			fprintf(stderr, "ERROR: failed to read answer file\n");
			return 1;
		}

		if (answer_file_size % sizeof(f64) != 0)
		{
			//// ERROR
			fprintf(stderr, "ERROR: answer file is corrupt (size is not divisible by the size of a 64 bit float)\n");
			return 1;
		}

		if (answer_file_size / sizeof(f64) < 2)
		{
			//// ERROR
			fprintf(stderr, "ERROR: answer file is corrupt (it contains fewer entries than the minimum of 1 answer and 1 expected sum)\n");
			return 1;
		}

		answers                  = (f64*)answer_contents;
		expected_number_of_pairs = (answer_file_size / sizeof(f64)) - 1;
		expected_sum             = answers[expected_number_of_pairs];
	}

	f64 haversine_sum   = 0;
	umm number_of_pairs = 0;

	umm pairs_cap = 1000;
	Pair* pairs   = malloc(pairs_cap*sizeof(Pair));

	/*
	{
		

		if (number_of_pairs >= pairs_cap)
		{
			pairs_cap += 1 + pairs_cap/2;
			pairs = realloc(pairs, pairs_cap*sizeof(Pair));
		}

		pairs[number_of_pairs] = (Pair){
			.x0 = x0,
			.y0 = y0,
			.x1 = x1,
			.y1 = y1,
		};
	}
	*/

	f64 res_number_of_pairs = 1 / (f64)number_of_pairs;
	for (umm i = 0; i < number_of_pairs; ++i)
	{
		Pair* p = &pairs[i];

		f64 haversine_dist = ReferenceHaversine(p->x0, p->y0, p->x1, p->y1, EARTH_RADIUS);

		haversine_sum += haversine_dist*res_number_of_pairs;

		if (answers != 0)
		{
			f64 diff      = fabs(haversine_dist - answers[i]);
			f64 tolerance = 0.00001;

			if (diff > tolerance) // bad way of comparing since float precision is not uniform
			{
				//// ERROR
				fprintf(stderr, "ERROR: pair #%lu failed.", i);
				fprintf(stderr, "Coords: x0: %21.16f, y0: %20.16f, x1: %21.16f, y1: %20.16f\n", p->x0, p->y0, p->x1, p->y1);
				fprintf(stderr, "Produced: %.16f\n", haversine_dist);
				fprintf(stderr, "Expected: %.16f\n", answers[i]);
				fprintf(stderr, "Difference: %.16f\n", diff);
				return 1;
			}
		}
	}

	printf("Input size: %lu\n", json_file_size);
	printf("Pair count: %lu\n", number_of_pairs);
	printf("Haversine sum: %.16f\n", haversine_sum);

	if (answers != 0)
	{
		printf("\n");
		printf("Validation:\n");
		printf("Reference sum: %.16f\n", expected_sum);
		printf("Difference: %.16f\n", fabs(haversine_sum - expected_sum));
	}

	return 0;
}
