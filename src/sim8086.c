#include <stdio.h>

#include "common.h"

int
main(int argc, char** argv)
{
	if (argc != 2)
	{
		printf("Usage: sim8086 input_binary\n");
		return 1;
	}

	return 0;
}
