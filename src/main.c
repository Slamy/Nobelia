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

unsigned short abuf;
unsigned short xbuf;
unsigned short dmactl;
unsigned short audctl;
unsigned short dbuf;

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

	while (framecnt < 2)
	{
		printf("%d %x %d\n", framecnt, *((unsigned short *)0x303FFe), sizeof(int));
		sleep(1);
		framecnt++;
	}
	*((unsigned short *)0x303FF4) = 0;
	*((unsigned short *)0x303FF6) = 0;
	*((unsigned short *)0x303FFE) = 0; /* Deactivate everything */

	*((unsigned short *)0x303C00) = 0x24;
	*((unsigned short *)0x303FFE) = 0xC000; /* Start! */
	sleep(1);

	*((unsigned short *)0x303C06) = 0x0100;
	*((unsigned long *)0x303C08) = 0x8000;
	*((unsigned short *)0x303C0C) = 0x8000;

	*((unsigned long *)0x303C02) = 0x00471800; /* Timecode */
#if 0
	*((unsigned short *)0x303C00) = 0x28; /* Play CDDA command */
	*((unsigned short *)0x303C00) = 0x27; /* Request TOC */
#endif
	*((unsigned short *)0x303C00) = 0x2a;

	*((unsigned short *)0x303FFE) = 0xC000; /* Start! */

	*((unsigned char *)310004) = 0x83;

	*((unsigned char *)310004) = 0xc0;
	*((unsigned char *)310004) = 0x0c;
	*((unsigned char *)310004) = 0x0c;
	*((unsigned char *)310004) = 0x0c;
	*((unsigned char *)310004) = 0x0c;

#if 0
7fff 7fff 3ffe d7fe 4825
7fff 7fff 3ffe d7fe 4824
7fff 7fff 3ffe d7fe 4825
7fff 7fff 3ffe d7fe 4824
7fff 7fff 3ffe d7fe 4825
#endif
	for (;;)
	{
		if (got_it)
		{
			got_it = 0;

			abuf = *((unsigned short *)0x303FF4);
			xbuf = *((unsigned short *)0x303FF6);
			dmactl = *((unsigned short *)0x303FF8);
			audctl = *((unsigned short *)0x303FFA);
			dbuf = *((unsigned short *)0x303FFE);

			printf("%x %x %x %x %x\n", abuf, xbuf, dmactl, audctl, dbuf);

			if (got_it)
			{
				/* Missed one? discard the next and wait for the next sector */
				got_it = 0;
			}
		}
	}

	exit(0);
}
