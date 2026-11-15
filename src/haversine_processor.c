#include "common.h"
#include "haversine_reference.h"

#include <stdio.h>

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
