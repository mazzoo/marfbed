#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "marfbed.h"


static int moving = MARF_MAX;

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

	m->color = COLOR_MOVING;
	m->moving = 1;

	m->enabled = 1;
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

	m->enabled = 1;
	set_rnd_start(m);
	set_rnd_dest(m);
	set_rnd_speed(m);

	m->x = m->start_x;
	m->y = m->start_y;
}


void turnoff(marf_t * m)
{
	if (!m->enabled) return;
	if ( (random()%1000) > RATE_TURNOFF) return;

	m->enabled = 0;
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

	m->color = COLOR_MOVING;
	m->moving = 1;
	printf("moving: %d\n", ++moving);
}


void sitdown(marf_t * m)
{
	if (!m->enabled) return;
	if (m->moving) return;
	if ( (random()%1000) > RATE_SITDOWN) return;

	m->color = COLOR_SITTING;
	m->moving = 0;
	printf("moving: %d\n", --moving);
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
		printf("moving: %d\n", --moving);
	}
}

void mainloop_marf(marfbed_t * b)
{
	int i;
	for (i=0; i<MARF_MAX; i++)
	{
		turnon(&b->marf[i]);
		turnoff(&b->marf[i]);
		standup(&b->marf[i]);
		//sitdown(&b->marf[i]); // there is no sitdown
		move(&b->marf[i]);
	}
}
