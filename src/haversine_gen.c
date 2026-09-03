#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

bool
GenerateHaversineFiles(bool should_cluster, s64 seed, s64 number_of_pairs)
{
	bool encountered_errors = false;

	FILE* data_flex_file        = 0;
	FILE* data_haveranswer_file = 0;
	do
	{
		char data_flex_filename[sizeof("data__flex.json") + 20] = {0};
		int data_flex_snprintf_result = snprintf(data_flex_filename, sizeof(data_flex_filename), "data_%ld_flex.json", number_of_pairs);
		if (data_flex_snprintf_result < 0 || (unsigned int)data_flex_snprintf_result >= sizeof(data_flex_filename))
		{
			//// ERROR
			fprintf(stderr, "ERROR: Failed to create filename for json output file\n");
			encountered_errors = true;
			break;
		}

		char data_haveranswer_filename[sizeof("data__haveranswer.f64") + 20] = {0};
		int data_haveranswer_snprintf_result = snprintf(data_haveranswer_filename, sizeof(data_haveranswer_filename), "data_%ld_haveranswer.f64", number_of_pairs);
		if (data_haveranswer_snprintf_result < 0 || (unsigned int)data_haveranswer_snprintf_result >= sizeof(data_haveranswer_filename))
		{
			//// ERROR
			fprintf(stderr, "ERROR: Failed to create filename for answer output file\n");
			encountered_errors = true;
			break;
		}

		data_flex_file = fopen(data_flex_filename, "wb");
		if (data_flex_file == 0)
		{
			//// ERROR
			fprintf(stderr, "ERROR: Failed to open json output file");
			encountered_errors = true;
			break;
		}

		data_haveranswer_file = fopen(data_haveranswer_filename, "wb");
		if (data_haveranswer_file == 0)
		{
			//// ERROR
			fprintf(stderr, "ERROR: Failed to open answer output file");
			encountered_errors = true;
			break;
		}

		// TODO
	} while (0);

	if (data_flex_file != 0 && fclose(data_flex_file) != 0)
	{
		//// ERROR
		fprintf(stderr, "ERROR: Failed to properly close json output file\n");
		encountered_errors = true;
	}

	if (data_haveranswer_file != 0 && fclose(data_haveranswer_file) != 0)
	{
		//// ERROR
		fprintf(stderr, "ERROR: Failed to properly close answer output file\n");
		encountered_errors = true;
	}

	return !encountered_errors;
}

int
main(int argc, char** argv)
{
	bool should_cluster = false;
	s64 seed            = 0;
	s64 number_of_pairs = 0;

	char* usage = "Usage: haversine_gen <uniform | cluster> <seed> <number of pairs to generate>";

	if (argc != 4)
	{
		//// ERROR
		fprintf(stderr, "ERROR: Invalid number of arguments\n%s\n", usage);
		return 1;
	}

	char* clustering_arg = argv[1];
	char* seed_arg       = argv[2];
	char* num_pairs_arg  = argv[3];

	if      (strcmp(clustering_arg, "uniform") == 0) should_cluster = false;
	else if (strcmp(clustering_arg, "cluster") == 0) should_cluster = true;
	else
	{
		//// ERROR
		fprintf(stderr, "ERROR: Invalid clustering option \"%s\", should be either \"uniform\" or \"cluster\"\n%s\n", clustering_arg, usage);
		return 1;
	}

	if (!ParseS64(String_FromCString(seed_arg), &seed) || seed <= 0)
	{
		//// ERROR
		fprintf(stderr, "ERROR: Failed to parse seed \"%s\". Seed must be a valid integer greater than 0\n%s\n", seed_arg, usage);
		return 1;
	}

	if (!ParseS64(String_FromCString(num_pairs_arg), &number_of_pairs) || number_of_pairs <= 0)
	{
		//// ERROR
		fprintf(stderr, "ERROR: Failed to parse number of pairs \"%s\". Number of pairs must be a valid integer greater than 0\n%s\n", num_pairs_arg, usage);
		return 1;
	}

	bool succeeded = GenerateHaversineFiles(should_cluster, seed, number_of_pairs);

	return (succeeded ? 0 : 1);
}
