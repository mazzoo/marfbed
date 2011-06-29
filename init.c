#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#include "marfbed.h"
#include "marf.h"
#include "gfx.h"

void init_random(void)
{
	uint32_t seed;
	seed = getpid()^time(NULL);
	srand(seed);
	printf("random seed: 0x%8.8x\n", seed);
}

void init(marfbed_t * b)
{
	init_random();

	init_gfx(b);

	/* allocate marfs */
	b->marf = malloc(MARF_MAX * sizeof(*b->marf));
	if (!b->marf)
	{
		printf("ERROR: malloc(): %s\n", strerror(errno));
		exit(1);
	}
	int i;
	for (i=0; i<MARF_MAX; i++)
	{
		init_marf(&(b->marf[i]));
		//print_mac(&b->marf[i]);
	}
}
