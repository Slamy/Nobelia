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

unsigned short reg_buffer[100][15];
int bufpos;

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
	int i, j;
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
	got_it = 0;

	*((unsigned short *)0x303FF4) = 0;
	*((unsigned short *)0x303FF6) = 0;
	*((unsigned short *)0x303FFE) = 0; /* Deactivate everything */

	*((unsigned short *)0x303C06) = 0x0100;	   /* File */
	*((unsigned long *)0x303C08) = 0x0003;	   /* Channel */
	*((unsigned short *)0x303C0C) = 0x0001;	   /* Audio Channel */
	*((unsigned long *)0x303C02) = 0x48303100; /* Timecode */
	*((unsigned short *)0x303C00) = 0x2a;	   /* Read Mode 2 */
	*((unsigned short *)0x303FFE) = 0xC000;	   /* Start! */
	*((unsigned short *)0x303FFA) = 0x800;

	bufpos = 0;
	while (bufpos < 90)
	{
		if (got_it)
		{
			got_it = 0;
			/* subcode = ((*((unsigned short *)0x303FFE) & 0x7) * 0xA00) | 0x300000; */

			abuf = *((unsigned short *)0x303FF4);
			xbuf = *((unsigned short *)0x303FF6);
			dmactl = *((unsigned short *)0x303FF8);
			audctl = *((unsigned short *)0x303FFA);
			dbuf = *((unsigned short *)0x303FFE);
			reg_buffer[bufpos][0] = *((unsigned short *)0x300000);
			reg_buffer[bufpos][1] = *((unsigned short *)0x300002);
			reg_buffer[bufpos][2] = *((unsigned short *)0x300A00);
			reg_buffer[bufpos][3] = *((unsigned short *)0x300A02);

			reg_buffer[bufpos][4] = *((unsigned short *)0x301400);
			reg_buffer[bufpos][5] = *((unsigned short *)0x301402);
			reg_buffer[bufpos][6] = *((unsigned short *)0x301E00);
			reg_buffer[bufpos][7] = *((unsigned short *)0x301E02);

			reg_buffer[bufpos][8] = *((unsigned short *)0x302800);
			reg_buffer[bufpos][9] = *((unsigned short *)0x302802);
			reg_buffer[bufpos][10] = *((unsigned short *)0x303200);
			reg_buffer[bufpos][11] = *((unsigned short *)0x303202);

			reg_buffer[bufpos][12] = dmactl;
			reg_buffer[bufpos][13] = audctl;
			reg_buffer[bufpos][14] = dbuf;


			bufpos++;
		}
	}

	for (i = 0; i < bufpos; i++)
	{
		printf("%3d ", i);
		for (j = 0; j < 15; j++)
		{
			printf(" %02x", reg_buffer[i][j]);
		}
		printf("\n");
	}

	exit(0);
}
