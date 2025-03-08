#include <cdfm.h>
#include <csd.h>
#include <modes.h>
#include <sysio.h>
#include <ucm.h>
#include <memory.h>
#include <stdio.h>
#include "hwreg.h"
#include "irq.h"

char cdic_irq_occured;
unsigned short int_abuf;
unsigned short int_xbuf;


/* Used to store register information during a test */
/* We don't want to make any prints during the test as the baud rate is too slow */
unsigned long reg_buffer[100][40];
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

void print_state()
{
	printf("State %04x %04x %04x %04x %04x %04x\n", int_abuf, int_xbuf, CDIC_ABUF, CDIC_XBUF, CDIC_DBUF, CDIC_AUDCTL);
	int_abuf = 0;
	int_xbuf = 0;
}


/* Not the best way to do this. But the program is small and I hate the compiler */
#include "test_data_read.c"
#include "test_xa_play.c"
#include "slave.c"

#define _VA_LIST unsigned char *
#define va_start(va, paranm) (void)((va) = (_VA_LIST)__inline_va_start__())
void *__inline_va_start__(void);
#define va_end(va) (void)((va) = (_VA_LIST)0)

void print(char *format, ...)
{
#if 1
	_VA_LIST args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
#endif
}



void collect_audiomap_registers(int marker, int timeout)
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

void observe_audiomap_registers(int maxpos, int marker)
{
	int timeout = 0;
	while (bufpos < maxpos)
	{
		if (cdic_irq_occured)
		{
			cdic_irq_occured = 0;

			collect_audiomap_registers(marker, timeout);
			timeout = 0;
		}

		timeout++;
		if (timeout > 80000)
		{
			collect_audiomap_registers(marker, timeout);
			break;
		}
	}
}

void print_reg_buffer()
{
	int i, j;
	for (i = 0; i < bufpos; i++)
	{
		print("%3d ", i);
		for (j = 0; j < 7; j++)
		{
			print(" %04x", reg_buffer[i][j]);
		}
		print("\n");
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

	print("Start audiomap and stop it with 0xff coding\n");
	CDIC_AUDCTL = 0x2800;

	observe_audiomap_registers(6, 1);

	/* Stop audiomap */
	*((unsigned short *)0x30280a) = 0x00ff;
	*((unsigned short *)0x30320a) = 0x00ff;

	observe_audiomap_registers(12, 2);

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

	print("Start audiomap again with 0xff coding\n");
	CDIC_AUDCTL = 0x2800;
	bufpos = 0;
	observe_audiomap_registers(12, 3);
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

	print("Start audiomap and abort it during playback\n");
	CDIC_AUDCTL = 0x2800;

	observe_audiomap_registers(6, 1);

	CDIC_AUDCTL = 0; /* Abort */

	observe_audiomap_registers(12, 2);

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



void test_raw_read()
{
		int i, j;
	int timecnt = 0;

	/* Zelda - Wand of Gamelon - Map Theme*/
	CDIC_FILE = 0x0000;		/* Should be ignored here */
	CDIC_CHAN = 0x0000;		/* Should be ignored here */
	CDIC_ACHAN = 0x0000;	/* Should be ignored here */
	CDIC_TIME = 0x24362100; /* MSF 24:36:21 */
	CDIC_CMD = 0x0029;		/* Command = Read Mode 1 */
	CDIC_DBUF = 0xc000;		/* Execute command */

	bufpos = 0;
	timecnt = 0;
	while (bufpos < 90)
	{
		if (cdic_irq_occured)
		{

			cdic_irq_occured = 0;
		}
	}

	resetcdic();

	for (i = 0; i < bufpos; i++)
	{
		printf("%3d ", i);
		for (j = 0; j < 17; j++)
		{
			printf(" %04x", reg_buffer[i][j]);
		}

		printf("\n");
	}
}

void test_toc_read()
{
	int i, j;
	int timecnt = 0;

	CDIC_FILE = 0x0100;		/* MODE2 File filter */
	CDIC_CHAN = 0x0001;		/* MODE2 Channel filter Select which sectors to handle at all */
	CDIC_ACHAN = 0x0000;	/* Set to 0, to put even audio into normal data buffers */
	CDIC_TIME = 0x24362100; /* MSF 24:36:21 */
	CDIC_CMD = 0x0027;		/* Command = Read TOC */
	CDIC_DBUF = 0xc000;		/* Execute command */
}

/* Overwrite CDIC driver IRQ handling */
void take_system()
{
	/* TODO I don't understand why this works */
	store_a6();
	*((unsigned long *)0x200) = CDIC_IRQ; /* vector delivered by CDIC */
	CDIC_IVEC = 0x2480;

#if 0
	*((unsigned long *)0xF8) = TIMER_IRQ; /* vector 62 */
	*((unsigned long *)0xF4) = VIDEO_IRQ; /* vector 61 */
	*((unsigned long *)0x68) = SLAVE_IRQ; /* vector 26 */
#endif
}

int main(argc, argv)
int argc;
char *argv[];
{
	int bytes;
	int wait;
	int framecnt = 0;

	print("Hello CDIC!\n");

	take_system();

	slave_stereo_audio_cd_attenuation();
	slave_unmute();

	/* test_xa_play();*/
	test_data_read();

	for(;;);
	exit(0);
}
