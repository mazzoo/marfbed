#ifndef CONFIG_H
#define CONFIG_H

#if 0
#define SPACE_X (64*1024)
#define SPACE_Y (64*1024)
#else
#define SPACE_X 800
#define SPACE_Y 600
#endif

#define MARF_MAX       1024

#define SPEED_MAX        64

#define MAC_LEN           3 /* in bytes */

#define RATE_STANDUP      5 /* per mille */
#define RATE_SITDOWN      RATE_STANDUP

#define RATE_TURNOFF     16 /* per mille */
#define RATE_TURNON       RATE_TURNOFF

#define MARF_R_RADIO     10
#define MARF_R_RADIO_JAM 30

#define GFX_X           800
#define GFX_Y           600

#define GFX_MARF_X 2
#define GFX_MARF_Y GFX_MARF_X

#define COLOR_RADIO       0x0000ff69
#define COLOR_RADIO_JAM   0x00ffad00
#define COLOR_MOVING      0x00888888
#define COLOR_SITTING     0x00232323
#define COLOR_DUPLICATE   0x00ff4444 /* FIXME only used @startup */


#endif /* CONFIG_H */
