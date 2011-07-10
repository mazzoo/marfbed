#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#include "marfbed.h"


//static int moving = MARF_MAX;

void set_rnd_start(marf_t * m)
{
	m->start_x = random() % SPACE_X;
	m->start_y = random() % SPACE_Y;
}


void set_rnd_dest(marf_t * m)
{
	m->dest_x = random() % SPACE_X;
	m->dest_y = random() % SPACE_Y;
}


void set_rnd_speed(marf_t * m)
{
	do
		m->speed = random() % SPEED_MAX;
	while (!m->speed);
}


void set_rnd_MAC(marf_t * m)
{
	int i;
	for (i=0; i<MAC_LEN; i++)
		m->mac[i] = random() % 256;
}


void init_marf(marf_t * m)
{
	set_rnd_start(m);
	set_rnd_dest(m);
	set_rnd_speed(m);
	set_rnd_MAC(m);

	m->x = m->start_x;
	m->y = m->start_y;

	m->moving = 1;

	m->enabled = 1;

	m->proto.hello_count_last_reload = 
		PROTO_HELLO_RATE +
		random() % PROTO_HELLO_JITTER;
	m->proto.hello_count = random() % m->proto.hello_count_last_reload;
}


void print_mac(marf_t * m)
{
	int i;
	for (i=0; i<MAC_LEN; i++)
	{
		printf("%2.2x", m->mac[i]);
		if (i<MAC_LEN-1)
			printf(":");
	}
	printf("\n");
}


void turnon(marf_t * m)
{
	if (m->enabled) return;
	if ( (random()%1000) > RATE_TURNON) return;

	//printf("marf %2.2d turnon\n", m->index);

	m->enabled = 1;

	set_rnd_start(m);
	set_rnd_dest(m);
	set_rnd_speed(m);

	m->x = m->start_x;
	m->y = m->start_y;

	m->proto.hello_count_last_reload = 
		PROTO_HELLO_RATE +
		random() % PROTO_HELLO_JITTER;
	m->proto.hello_count = random() % m->proto.hello_count_last_reload;
}


void turnoff(marf_t * m)
{
	if (m->enabled) return;
	if (m->moving) return;
	if ( (random()%1000) > RATE_TURNOFF) return;

	//printf("marf %2.2d turnoff\n", m->index);

	m->enabled = 0;
	m->moving  = 0;
}


void standup(marf_t * m)
{
	if (!m->enabled) return;
	if (m->moving) return;
	if ( (random()%1000) > RATE_STANDUP) return;

	m->start_x = m->x;
	m->start_y = m->y;
	set_rnd_dest(m);
	set_rnd_speed(m);

	m->moving = 1;
}


void sitdown(marf_t * m) /* NOT USED */
{
	if (!m->enabled) return;
	if (m->moving) return;
	if ( (random()%1000) > RATE_SITDOWN) return;

	m->color = COLOR_SITTING;
	m->moving = 0;
}


void move(marf_t * m)
{
	if (!m->enabled) return;
	if (!m->moving) return;

	if (m->x != m->dest_x)
	{
		int32_t step_x = ( m->dest_x -
		                   m->start_x ) /
		                   m->speed;

		uint32_t wrap = m->x + step_x;
		/* - wrap */
		if ( (step_x < 0) &&
		     (wrap > m->x) )
			m->x = m->dest_x;
		else /* + wrap */
		if ( (step_x > 0) &&
		     (wrap < m->x) )
			m->x = m->dest_x;
		else /* normal, no wrap */
			m->x = wrap;

		/* always do atleast one step (unless we're at dest) */
		if (!step_x)
		{
			if (m->x < m->dest_x)
				m->x++;
			else
				m->x--;
		}
	}

	if (m->y != m->dest_y)
	{
		int32_t step_y = ( m->dest_y -
		                   m->start_y ) /
		                   m->speed;

		uint32_t wrap = m->y + step_y;
		/* - wrap */
		if ( (step_y < 0) &&
		     (wrap > m->y) )
			m->y = m->dest_y;
		else /* + wrap */
		if ( (step_y > 0) &&
		     (wrap < m->y) )
			m->y = m->dest_y;
		else /* normal, no wrap */
			m->y = wrap;

		/* always do atleast one step (unless we're at dest) */
		if (!step_y)
		{
			if (m->y < m->dest_y)
				m->y++;
			else
				m->y--;
		}
	}

	/* did we overshoot the destination? */
	/* x */
	if ( (m->start_x < m->dest_x) &&
	     (m->x > m->dest_x) )
		m->x = m->dest_x;
	if ( (m->start_x > m->dest_x) &&
	     (m->x < m->dest_x) )
		m->x = m->dest_x;
	/* y */
	if ( (m->start_y < m->dest_y) &&
	     (m->y > m->dest_y) )
		m->y = m->dest_y;
	if ( (m->start_y > m->dest_y) &&
	     (m->y < m->dest_y) )
		m->y = m->dest_y;

	/* become white at the destination */
	if ( (m->x == m->dest_x) &&
			(m->y == m->dest_y) )
	{
		m->moving = 0;
		m->color  = COLOR_SITTING;
		//printf("moving: %d\n", --moving);
	}
}


void protocol(marf_t * m)
{
	protocol_t * p = &m->proto;

	if (!m->enabled) return;

	p->hello_count--;
	if (!p->hello_count)
	{
		/* reset periodic hello counter */
		p->hello_count = PROTO_HELLO_RATE +
			random() % PROTO_HELLO_JITTER;
		p->hello_count_last_reload = p->hello_count;
		m->color = 0x0000ff00;

		/* prepare packet for TX */
		memcpy(p->sMAC, m->mac, MAC_LEN);
		memset(p->dMAC, 0xff, MAC_LEN); /* broadcast */
		p->packet[0] = 2;
		p->packet[1] = PACKET_TYPE_HELLO;
		p->state = PROTO_STATE_TX;
	}else{
		m->color = COLOR_SITTING;
		if (m->moving)
			m->color = COLOR_MOVING;
		p->state = PROTO_STATE_RX;
	}
}


void transmit(marf_t * m)
{

}


void interfere(marf_t * a, marf_t * b)
{

	protocol_t * pa = &a->proto;
	protocol_t * pb = &b->proto;

	if (
		(pa->state == PROTO_STATE_TX) &&
		(pb->state == PROTO_STATE_TX) 
	   )
	{
		/* distance calculation */
		if (
			(a->x - b->x) * (a->x - b->x) +
			(a->y - b->y) * (a->y - b->y) <
			RADIO_R_JAM_SQ
		   )
		{
			printf("%d and %d interfere\n", a->index, b->index);
		}
	}
}


void receive(marf_t * m)
{
}


void mainloop_marf(marfbed_t * b)
{
	int i, j;
	for (i=0; i<MARF_MAX; i++)
	{
		turnon(&b->marf[i]);
		turnoff(&b->marf[i]);
		standup(&b->marf[i]);
		//sitdown(&b->marf[i]); // there is no sitdown
		move(&b->marf[i]);
		protocol(&b->marf[i]);
	}
	for (i=0; i<MARF_MAX; i++)
		transmit(&b->marf[i]);
	for (i=0; i<MARF_MAX; i++)
		for (j=i+1; j<MARF_MAX; j++)
			interfere(&b->marf[i], &b->marf[j]);
	for (i=0; i<MARF_MAX; i++)
		receive(&b->marf[i]);
}

