#include <cdfm.h>
#include <csd.h>
#include <modes.h>
#include <sysio.h>
#include <ucm.h>
#include <stdio.h>
#include <memory.h>
#include "cdic.h"
#include "irq.h"

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned int
#define NULL 0

/* #define PRINTF(...) printf(__VA_ARGS__) */

char cdic_irq_occured;
unsigned short int_abuf;
unsigned short int_xbuf;

/* Used to store register information during a test */
unsigned long reg_buffer[100][15];
int bufpos;

unsigned short abuf;
unsigned short xbuf;
unsigned short dmactl;
unsigned short audctl;
unsigned short dbuf;

/* Do whatever is known to bring the CDIC into a known state */
void resetcdic()
{
	int temp;

	CDIC_ABUF = 0;
	CDIC_XBUF = 0;
	CDIC_DBUF = 0;
	CDIC_AUDCTL = 0;

	temp = CDIC_ABUF; /* Reset IRQ flag via reading */
	temp = CDIC_XBUF; /* Reset IRQ flag via reading */

	bufpos = 0;
}

void collect_registers(int marker, int timeout)
{
	/* As read by IRQ handler */
	reg_buffer[bufpos][0] = int_abuf;
	reg_buffer[bufpos][1] = int_xbuf;

	reg_buffer[bufpos][2] = CDIC_ABUF;
	reg_buffer[bufpos][3] = CDIC_XBUF;
	reg_buffer[bufpos][4] = CDIC_AUDCTL;

	/* Meta data */
	reg_buffer[bufpos][5] = marker;
	reg_buffer[bufpos][6] = timeout;

	int_abuf = 0;
	int_xbuf = 0;

	bufpos++;
}

void observe_registers(int maxpos, int marker)
{
	int timeout = 0;
	while (bufpos < maxpos)
	{
		if (cdic_irq_occured)
		{
			cdic_irq_occured = 0;

			collect_registers(marker, timeout);
			timeout = 0;
		}

		timeout++;
		if (timeout > 80000)
		{
			collect_registers(marker, timeout);
			break;
		}
	}
}

void print_reg_buffer()
{
	int i, j;
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

/* Tests playback of audiomap */
void test_audiomap1()
{
	resetcdic();

	/* [:cdic] Coding 05, 2 channels, 4 bits, 000049d4 frequency -> 106 ms between IRQs */
	*((unsigned short *)0x30280a) = 0x0005;
	*((unsigned short *)0x30320a) = 0x0005;

	/* [:cdic] Coding 04s, 1 channels, 4 bits, 000049d4 frequency ->  213 ms between IRQs */
	*((unsigned short *)0x30280a) = 0x0004;
	*((unsigned short *)0x30320a) = 0x0004;

	printf("Start audiomap and stop it with 0xff coding\n");
	CDIC_AUDCTL = 0x2800;

	observe_registers(6, 1);

	/* Stop audiomap */
	*((unsigned short *)0x30280a) = 0x00ff;
	*((unsigned short *)0x30320a) = 0x00ff;

	observe_registers(12, 2);

	print_reg_buffer();

	/*
	Start audiomap and stop it with 0xff coding
	0  ffff 7fff 7fff 7fff fffe 0001 4f31
	1  ffff 7fff 7fff 7fff fffe 0001 4fc8
	2  ffff 7fff 7fff 7fff fffe 0001 4fc8
	3  ffff 7fff 7fff 7fff fffe 0001 4fdd
	4  ffff 7fff 7fff 7fff fffe 0001 4fa5
	5  ffff 7fff 7fff 7fff fffe 0001 4fce
	6  ffff 7fff 7fff 7fff f7ff 0002 4fcb
	7  0000 0000 7fff 7fff f7fe 0002 13881

	Bit 15 of ABUF is set when a single audiomap buffer has been played. (causing an IRQ?)
	Bit 0 of AUDCTL is set when the audiomap was stopped by 0xff coding
	Bit 0 of AUDCTL is reset when reading the register.
	Bit 11 is reset is reset when the audiomap was stoppped by 0xff coding
	*/

	printf("Start audiomap again with 0xff coding\n");
	CDIC_AUDCTL = 0x2800;
	bufpos = 0;
	observe_registers(12, 3);
	print_reg_buffer();

	/*
	Start audiomap again with 0xff coding
	0  0000 0000 7fff 7fff f7ff 0003 13881

	Bit 0 of AUDCTL is set, Bit 11 is reset.
	No IRQ. No Bit 15 of ABUF.
	*/
}

/* Tests abort of audiomap */
void test_audiomap2()
{
	resetcdic();

	/* [:cdic] Coding 05, 2 channels, 4 bits, 000049d4 frequency -> 106 ms between IRQs */
	*((unsigned short *)0x30280a) = 0x0005;
	*((unsigned short *)0x30320a) = 0x0005;

	/* [:cdic] Coding 04s, 1 channels, 4 bits, 000049d4 frequency ->  213 ms between IRQs */
	*((unsigned short *)0x30280a) = 0x0004;
	*((unsigned short *)0x30320a) = 0x0004;

	printf("Start audiomap and abort it during playback\n");
	CDIC_AUDCTL = 0x2800;

	observe_registers(6, 1);

	CDIC_AUDCTL = 0; /* Abort */

	observe_registers(12, 2);

	print_reg_buffer();

	/*
	Start audiomap and abort it during playback
	0  ffff 7fff 7fff 7fff fffe 0001 4cb8
	1  ffff 7fff 7fff 7fff fffe 0001 4fd5
	2  ffff 7fff 7fff 7fff fffe 0001 4fbd
	3  ffff 7fff 7fff 7fff fffe 0001 4fdd
	4  ffff 7fff 7fff 7fff fffe 0001 4fb4
	5  ffff 7fff 7fff 7fff fffe 0001 4fba
	6  0000 0000 ffff 7fff d7fe 0002 13881

	Bit 0 of AUDCTL is not set.
	Bit 15 of ABUF is set but no IRQ is generated.
	*/
}

/* Overwrite CDIC driver IRQ handling */
void take_system()
{
	/* TODO I don't understand why this works */
	store_a6();
	*((unsigned long *)0x200) = CDIC_IRQ;
	CDIC_IVEC = 0x2480;
}

int main(argc, argv)
int argc;
char *argv[];
{
	int bytes;
	int wait;
	int framecnt = 0;
	u_long atten;

	printf("Hello CDIC!\n");

	take_system();

	test_audiomap1();
	test_audiomap2();

	exit(0);
}
