#ifndef MARFBED_H
#define MARFBED_H

#include <stdint.h>
#include <SDL/SDL_ttf.h>

#include "config.h"


#define STATE_RX_EN 0x00000001
#define STATE_TX_EN 0x00000002

typedef struct protocol_s
{
	uint32_t state;
	uint32_t hello_count;
	uint32_t hello_count_last_reload;

	uint8_t  sMAC[MAC_LEN];
	uint8_t  dMAC[MAC_LEN];

	uint8_t  packet[MAX_PACKET_LEN];
} protocol_t;

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

	protocol_t proto;
} marf_t;

typedef struct marfbed_s
{
	marf_t * marf;

	TTF_Font *font;

	int    show_ring_1;
	int    show_ring_2;
	int    show_ring_3;
	int    show_lines;
	int    show_info;
	int    show_numbers;
} marfbed_t;

#endif /* MARFBED_H */
