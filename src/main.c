#include <cdfm.h>
#include <csd.h>
#include <modes.h>
#include <sysio.h>
#include <ucm.h>
#include <stdio.h>
#include <memory.h>
#include "video.h"
#include "graphics.h"

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned int

#define NULL 0

/* Sounds */
#define MAXSOUNDS 4

#define DEBUG

#define I_BUTTON1 0x01
#define I_BUTTON2 0x02
#define I_BUTTON3 0x04
#define I_BUTTON_ANY (I_BUTTON1 | I_BUTTON2 | I_BUTTON3)
#define I_LEFT 0x10
#define I_RIGHT 0x20
#define I_UP 0x40
#define I_DOWN 0x80
#define I_SIGNAL1 0x0D00
#define I_SIGNAL2 0x0D01

void _CDIC_IRQ();

/*int d0_backup;*/
char got_it;

unsigned short buffer[12];

void store_a6();
int main(argc, argv)
int argc;
char *argv[];
{
	int bytes;
	int wait;
	int framecnt = 0;
	u_long atten;
	unsigned short *subcode;
	int i;
#ifdef DEBUG
	printf("Hello World: %x\n", *((unsigned short *)0x303FFC));
#endif

	store_a6();
	*((unsigned long *)0x200) = _CDIC_IRQ;
	*((unsigned short *)0x303FFC) = 0x2480;

	while (framecnt < 4)
	{
		printf("%d %x %d\n", framecnt, *((unsigned short *)0x303FFe), sizeof(int));
		sleep(1);
		framecnt++;
	}

	*((unsigned short *)0x303FFE) = 0;		   /* Deactivate everything */
	*((unsigned long *)0x303C02) = 0x00000000; /* Timecode */
#if 0
	*((unsigned short *)0x303C00) = 0x28;	   /* Play CDDA command */
#else
	*((unsigned short *)0x303C00) = 0x27;	   /* Request TOC */

#endif
	*((unsigned short *)0x303FFE) = 0xC000; /* Start! */

	for (;;)
	{
		if (got_it)
		{
			got_it = 0;

			subcode = (*((unsigned short *)0x303FFe) & 1) ? 0x301324 : 0x300924;
			memcpy(buffer,subcode,sizeof(buffer));

			for (i = 0; i < 12; i++)
			{
				printf(" %02x", buffer[i] & 0xff);
			}

			if (got_it)
			{
				/* Missed one? discard the next and wait for the next sector */
				got_it = 0;
			}
			printf("\n");
		}
	}

	exit(0);
}
