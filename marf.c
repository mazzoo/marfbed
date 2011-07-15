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


void init_marf(marf_t * m)
{
	set_rnd_start(m);
	set_rnd_dest(m);
	set_rnd_speed(m);
	set_rnd_MAC(m);

#if 0
	printf("n%d's MAC is ", m->index);
	print_mac(m);
#endif

	m->x = m->start_x;
	m->y = m->start_y;

	m->moving = 1;

	m->enabled = 1;

	m->tick = 1;

	m->proto.hello_count_last_reload = 
		PROTO_HELLO_RATE +
		random() % PROTO_HELLO_JITTER;
	m->proto.hello_count = random() % m->proto.hello_count_last_reload;

	m->proto.state = PROTO_STATE_RX;

	int i;
	for (i=0; i < N_NEIGHBOURS; i++)
	{
		m->neighbour[i].first_seen = 0;
		m->neighbour[i].last_seen = 0;
	}
}



void turnon(marf_t * m)
{
	if (m->enabled) return;
	if ( (random()%1000) > RATE_TURNON) return;

	//printf("marf %2.2d turnon\n", m->index);

	m->enabled = 1;

	m->tick = 1;

	set_rnd_start(m);
	set_rnd_dest(m);
	set_rnd_speed(m);

	m->x = m->start_x;
	m->y = m->start_y;

	m->proto.hello_count_last_reload = 
		PROTO_HELLO_RATE +
		random() % PROTO_HELLO_JITTER;
	m->proto.hello_count = random() % m->proto.hello_count_last_reload;

	m->proto.state = PROTO_STATE_RX;
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

	if (!a->enabled) return;
	if (!b->enabled) return;

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
			printf("n%d and n%d interfere\n", a->index, b->index);
		}
	}
}


/* rx got a HELLO packet from tx */
void neighbour(marf_t * tx, marf_t * rx)
{
	int h;
	for (h=0; h < N_NEIGHBOURS; h++)
	{
		if (rx->neighbour[h].first_seen)
		{
			if (!memcmp(tx->mac, rx->neighbour[h].mac, MAC_LEN))
			{
				rx->neighbour[h].last_seen = rx->tick;
				//printf("n%d saw n%d at %d\n",
				//		rx->index, tx->index, h);
				return;
			}
		}
	}
	/* find a free slot for a new neighbour */
	for (h=0; h < N_NEIGHBOURS; h++)
	{
		if (!rx->neighbour[h].first_seen)
		{
			rx->neighbour[h].first_seen = rx->tick;
			rx->neighbour[h].last_seen = rx->tick;
			memcpy(rx->neighbour[h].mac, tx->mac, MAC_LEN);
			printf("n%d got to know n%d at index %d, tick %d\n",
				rx->index, tx->index, h, rx->tick);
			return;
		}
	}
	printf("WARNING: n%d couldn't store new neighbour n%d\n",
		rx->index, tx->index);
}


void receive(marf_t * tx, marf_t * rx)
{

	if (!tx->enabled) return;
	if (!rx->enabled) return;

	protocol_t * ptx = &tx->proto;
	protocol_t * prx = &rx->proto;

	if (
		(ptx->state == PROTO_STATE_TX) &&
		(prx->state == PROTO_STATE_RX) 
	   )
	{
		if (
			(tx->x - rx->x) * (tx->x - rx->x) +
			(tx->y - rx->y) * (tx->y - rx->y) <
			RADIO_R_SQ
		   )
		{
			switch (ptx->packet[1])
			{
				case PACKET_TYPE_HELLO:
					//printf("n%d got a HELLO from n%d\n",
					//	rx->index, tx->index);
					neighbour(tx, rx);
					break;
				default:
					printf("n%d got an INVALID packet from n%d\n",
						tx->index, rx->index);
					printf("\t%2.2x %2.2x\n",
							prx->packet[0],
							prx->packet[1]);
					printf("\tMAC ");
					int i;
					for (i=0; i<MAC_LEN; i++)
						printf("%2.2x ", rx->mac[i]);
					printf("\n");
			}
		}
	}
}

void mainloop_marf(marfbed_t * b)
{
	int i, j;
	for (i=0; i<MARF_MAX; i++)
	{
		b->marf[i].tick++;

		turnon(&b->marf[i]);
		turnoff(&b->marf[i]);
		standup(&b->marf[i]);
		//sitdown(&b->marf[i]); // there is no sitdown
		move(&b->marf[i]);
		protocol(&b->marf[i]);
	}
	for (i=0; i<MARF_MAX; i++)
		transmit(&b->marf[i]);
	for (i=0; i<MARF_MAX-1; i++)
		for (j=i+1; j<MARF_MAX; j++)
			interfere(&b->marf[i], &b->marf[j]);
	for (i=0; i<MARF_MAX-1; i++)
		for (j=i+1; j<MARF_MAX; j++)
			receive(&b->marf[i], &b->marf[j]);
}

