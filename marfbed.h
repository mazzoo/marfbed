#ifndef MARFBED_H
#define MARFBED_H

#include <stdint.h>

#include "config.h"

typedef struct marf_s
{
	uint32_t index;

	uint16_t x; /* current position */
	uint16_t y;

	uint16_t start_x; /* where I started to move from */
	uint16_t start_y;

	uint16_t dest_x; /* where I wish to move to */
	uint16_t dest_y;

	uint16_t speed;

	uint8_t  mac[MAC_LEN];

	uint32_t color;

	uint8_t  moving;
	uint8_t  enabled;
} marf_t;

typedef struct marfbed_s
{
	marf_t * marf;
} marfbed_t;

#endif /* MARFBED_H */
