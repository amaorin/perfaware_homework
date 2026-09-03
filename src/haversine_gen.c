#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "reference_haversine.h"

f64
Rand01()
{
	return (f64)rand() / RAND_MAX;
}

typedef struct Cluster
{
	f64 xmin;
	f64 xdiff;
	f64 ymin;
	f64 ydiff;
} Cluster;

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

		srand((u32)seed);

#define CLUSTER_LEN_LG2 2
#define CLUSTER_LEN (1 << CLUSTER_LEN_LG2)
		Cluster clusters[CLUSTER_LEN];

		for (umm i = 0; i < CLUSTER_LEN; ++i)
		{
			clusters[i] = (Cluster){
				.xmin  = Rand01(),
				.xdiff = Rand01()*0.125 + 0.125,
				.ymin  = Rand01(),
				.ydiff = Rand01()*0.125 + 0.125,
			};
		}

		fprintf(data_flex_file, "{\n\t\"pairs\":[\n");

		f64 haversine_avg = 0;

		for (s64 i = 0; i < number_of_pairs; ++i)
		{
			f64 x0, y0, x1, y1;

			if (!should_cluster)
			{
				x0 = Rand01();
				y0 = Rand01();
				x1 = Rand01();
				y1 = Rand01();
			}
			else
			{
				Cluster* cluster_0 = &clusters[rand() & ((1 << CLUSTER_LEN_LG2) - 1)];
				Cluster* cluster_1 = &clusters[rand() & ((1 << CLUSTER_LEN_LG2) - 1)];
				
				x0 = Rand01()*cluster_0->xdiff + cluster_0->xmin;
				y0 = Rand01()*cluster_0->ydiff + cluster_0->ymin;
				x1 = Rand01()*cluster_1->xdiff + cluster_1->xmin;
				y1 = Rand01()*cluster_1->ydiff + cluster_1->ymin;

				//                      --                     --
				//                      )  x + 1 , x < 0       )  x - floor(x) , x < 0
				//                     (                      (
				// f(x) : [-1, 2]  =  --   x     , else   =  --   x - floor(x) , else   =  x - floor(x)
				//                     (                      (
				//                      )  x - 1 , x > 1       )  x - floor(x) , x > 1
				//                      --                     --
				x0 -= floor(x0);
				y0 -= floor(y0);
				x1 -= floor(x1);
				y1 -= floor(y1);
			}

			x0 = 360*x0 - 180;
			y0 = 180*y0 -  90;
			x1 = 360*x1 - 180;
			y1 = 180*y1 -  90;

			ASSERT(x0 >= -180 && x0 <= 180);
			ASSERT(y0 >=  -90 && y0 <=  90);
			ASSERT(x1 >= -180 && x1 <= 180);
			ASSERT(y1 >=  -90 && y1 <=  90);

			char* ending = (i == number_of_pairs-1 ? "" : ",");

			fprintf(data_flex_file, "\t\t{ \"x0\": %21.16f, \"y0\": %20.16f, \"x1\": %21.16f, \"y1\": %20.16f }%s\n", x0, y0, x1, y1, ending);

			f64 haversine_dist = ReferenceHaversine(x0, y0, x1, y1, EARTH_RADIUS);
			haversine_avg += haversine_dist;

			fwrite(&haversine_dist, sizeof(f64), 1, data_haveranswer_file);
		}

		fprintf(data_flex_file, "\t]\n}\n");

		haversine_avg /= number_of_pairs;
		fwrite(&haversine_avg, sizeof(f64), 1, data_haveranswer_file);

		printf("%.16f\n", haversine_avg);

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
