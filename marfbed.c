#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "marfbed.h"
#include "init.h"
#include "marf.h"
#include "gfx.h"

void verify_mac_uniq(marfbed_t * b);

int main(int argc, char ** argv)
{
	marfbed_t bed;
	init(&bed);
	verify_mac_uniq(&bed);
	while (0xbed)
	{
		mainloop_gfx(&bed);
		mainloop_marf(&bed);
		//usleep(0x1000);
	}
	return 0;
}

void verify_mac_uniq(marfbed_t * b)
{
	int uniq = 1;
	int i, j;
	for (i=0; i<MARF_MAX; i++)
		for (j=i+1; j<MARF_MAX; j++)
			if (!memcmp(b->marf[i].mac, b->marf[j].mac, MAC_LEN))
			{
				printf("duplicate MAC [%d, %d]: ", i, j);
				print_mac(&b->marf[i]);
				uniq = 0;
				b->marf[i].color = COLOR_DUPLICATE;
				b->marf[j].color = COLOR_DUPLICATE;
			}
	printf("%sduplicate MAC addresses found\n", uniq ? "no " : "");
}
