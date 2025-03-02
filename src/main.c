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
unsigned short int_abuf;
unsigned short int_xbuf;

unsigned long reg_buffer[100][15];
int bufpos;

unsigned short abuf;
unsigned short xbuf;
unsigned short dmactl;
unsigned short audctl;
unsigned short dbuf;

#define ABUF (*((unsigned short *)0x303FF4))
#define XBUF (*((unsigned short *)0x303FF6))
#define AUDCTL (*((unsigned short *)0x303FFA))
#define IVEC (*((unsigned short *)0x303FFC))
#define DBUF (*((unsigned short *)0x303FFE))

void testrun(int param)
{
	int i, j;
	int timeout = 0;

	ABUF = 0;
	XBUF = 0;
	DBUF = 0; /* Deactivate everything */

	/* CODING_STEREO, CODING_18KHZ */
	*((unsigned short *)0x30280a) = 0x0045;
	*((unsigned short *)0x30320a) = 0x0045;

	/* [:cdic] Coding 05, 2 channels, 4 bits, 000049d4 frequency -> 106 ms between IRQs */
	*((unsigned short *)0x30280a) = 0x0005;
	*((unsigned short *)0x30320a) = 0x0005;

	/* [:cdic] Coding 04s, 1 channels, 4 bits, 000049d4 frequency ->  213 ms between IRQs */
	*((unsigned short *)0x30280a) = 0x0004;
	*((unsigned short *)0x30320a) = 0x0004;

#if 0
	AUDCTL = 0x2000;
	/*
	0  0000 0000 7fff ffff f7fe 0003 13881
	1  0000 0000 7fff 7fff f7fe 0003 13881
	2  0000 0000 7fff 7fff f7fe 0003 13881
  	*/
#endif

#if 0
	AUDCTL = 0x0800;
	/*
	0  0000 0000 ffff ffff d7ff 0003 13881
	1  0000 0000 7fff 7fff d7fe 0003 13881
	2  0000 0000 7fff 7fff d7fe 0003 13881
	*/
#endif

#if 1
	AUDCTL = 0x0800;
	AUDCTL = 0x0000;
	/*
	0  0000 0000 7fff ffff dffe 0003 13881
	1  0000 0000 7fff 7fff dffe 0003 13881
	2  0000 0000 7fff 7fff dffe 0003 13881
	*/
#endif

#if 0
	AUDCTL = 0;
	/*
	0  0000 0000 7fff ffff d7fe 0003 13881
	1  0000 0000 7fff 7fff d7fe 0003 13881
	2  0000 0000 7fff 7fff d7fe 0003 13881
	*/
#endif


#if 0
	AUDCTL = 0x2800;
	/*
	0  ffff ffff 7fff 7fff fffe 0001 46f4
	1  ffff 7fff 7fff 7fff fffe 0001 46f6
	2  ffff 7fff 7fff 7fff fffe 0001 46e4
	3  ffff 7fff 7fff 7fff fffe 0001 46e2
	4  ffff 7fff 7fff 7fff fffe 0001 46f5
	5  ffff 7fff 7fff 7fff fffe 0001 46e3
	6  ffff 7fff 7fff 7fff f7ff 0002 41e4
	7  ffff 7fff 7fff 7fff f7fe 0003 13881
	8  ffff 7fff 7fff 7fff f7fe 0003 13881
	9  ffff 7fff 7fff 7fff f7fe 0003 13881
	*/
#endif

	timeout = 0;
	bufpos = 0;
	while (bufpos < 6)
	{
		if (got_it)
		{
			got_it = 0;

			reg_buffer[bufpos][0] = int_abuf;
			reg_buffer[bufpos][1] = int_xbuf;
			reg_buffer[bufpos][2] = ABUF;
			reg_buffer[bufpos][3] = XBUF;

			reg_buffer[bufpos][4] = AUDCTL;
			reg_buffer[bufpos][5] = 1;
			reg_buffer[bufpos][6] = timeout;
			bufpos++;
			timeout = 0;
		}

		timeout++;
		if (timeout > 80000)
			break;
	}

	if (param & 1)
		*((unsigned short *)0x30280a) = 0x00ff; /* Stop audiomap */
	if (param & 2)
		*((unsigned short *)0x30320a) = 0x00ff; /* Stop audiomap */

#if 1
	timeout = 0;

	while (bufpos < 10)
	{
		if (got_it)
		{
			got_it = 0;

			reg_buffer[bufpos][0] = int_abuf;
			reg_buffer[bufpos][1] = int_xbuf;
			reg_buffer[bufpos][2] = ABUF;
			reg_buffer[bufpos][3] = XBUF;

			reg_buffer[bufpos][4] = AUDCTL;
			reg_buffer[bufpos][5] = 2;
			reg_buffer[bufpos][6] = timeout;

			bufpos++;
			timeout = 0;
		}

		timeout++;
		if (timeout > 80000)
			break;
	}
#endif

	AUDCTL = 0x0800;

	for (i = 0; i < 3; i++)
	{
		reg_buffer[bufpos][0] = int_abuf;
		reg_buffer[bufpos][1] = int_xbuf;
		reg_buffer[bufpos][2] = ABUF;
		reg_buffer[bufpos][3] = XBUF;

		reg_buffer[bufpos][4] = AUDCTL;
		reg_buffer[bufpos][5] = 3;
		reg_buffer[bufpos][6] = timeout;

		bufpos++;
	}

	for (i = 0; i < bufpos; i++)
	{
		printf("%3d ", i);
		for (j = 0; j < 7; j++)
		{
			printf(" %04x", reg_buffer[i][j]);
		}
		printf("\n");
	}
}

void store_a6();
int main(argc, argv)
int argc;
char *argv[];
{
	int bytes;
	int wait;
	int framecnt = 0;
	u_long atten;

	printf("Hello World: %x\n", *((unsigned short *)0x303FFC));

	store_a6();
	*((unsigned long *)0x200) = _CDIC_IRQ;
	IVEC = 0x2480;

	while (framecnt < 2)
	{
		printf("%d %x %d\n", framecnt, DBUF, sizeof(int));
		sleep(1);
		framecnt++;
	}

#if 0
	testrun(1);
	printf("\n");
	testrun(2);
	printf("\n");
#endif
	testrun(3);

	exit(0);
}
