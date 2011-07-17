#ifndef CONFIG_H
#define CONFIG_H

#if 0
#define SPACE_X (64*1024)
#define SPACE_Y (64*1024)
#else
#define SPACE_X 800
#define SPACE_Y 600
#endif

#define MARF_MAX            1080

/* protocol */

#define MAC_LEN                3 /* in bytes */

#define MAX_PACKET_LEN        32

#define PROTO_HELLO_RATE     100
#define PROTO_HELLO_JITTER    80 /* otherwise direct neighbours may see never */

#define PACKET_TYPE_HELLO   0x01

#define PROTO_STATE_RX    0x0001
#define PROTO_STATE_RU    0x0002 /* transmitter ramp up for TX */
#define PROTO_STATE_TX    0x0003
#define PROTO_STATE_RD    0x0004 /* transmitter turnoff for RX */
#define PROTO_STATE_JAM   0x0005 /* RF interference with other TX, packet lost */

#define N_NEIGHBOURS           8

/* node movement */

#define SPEED_MAX             64

#define RATE_STANDUP           1 /* per mille */
#define RATE_SITDOWN    RATE_STANDUP

#define RATE_TURNOFF          16 /* per mille */
#define RATE_TURNON     RATE_TURNOFF


#define RADIO_R               20
#define RADIO_R_JAM           30

#define RADIO_R_SQ      (RADIO_R     * RADIO_R    )
#define RADIO_R_JAM_SQ  (RADIO_R_JAM * RADIO_R_JAM)


#define GFX_X                800
#define GFX_Y                600

#define GFX_MARF_X             3
#define GFX_MARF_Y      GFX_MARF_X

/* ring 1 */
#define COLOR_MOVING    0x00888888
#define COLOR_SITTING   0x00232323
#define COLOR_DUPLICATE 0x00ff4444 /* FIXME only used @startup */

/* ring 2 */
#define COLOR_RADIO     0x00006f49

/* ring 3 */
#define COLOR_RADIO_JAM     0x00603000
#define COLOR_RADIO_JAM_NOW 0x00ffadff

#endif /* CONFIG_H */

