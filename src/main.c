#include <cdfm.h>
#include <csd.h>
#include <modes.h>
#include <sysio.h>
#include <ucm.h>
#include <memory.h>
#include <stdio.h>
#include "hwreg.h"
#include "irq.h"
#include "slave.h"
#include "crc.h"
#include "framework.h"

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

/* Overwrite CDIC driver IRQ handling */
void take_system()
{
	/* TODO I don't understand why this works for assembler code. thx to cdifan */
	store_a6();

	CDIC_IVEC = 0x2480;
	/* Only in SUPERVISOR mode, on-chip peripherals can be configured */
	/* We abuse an CDIC IRQ to set the baud rate to 19200 */
	*((unsigned long *)0x200) = SET_UART_BAUD; /* vector delivered by CDIC */
	cdic_irq_occured = 0;
	CDIC_CMD = 0x23;	/* Command = Reset Mode 1 */
	CDIC_DBUF = 0xc000; /* Execute command */
	while (!cdic_irq_occured)
		;
	cdic_irq_occured = 0;

	/* Switch to actual IRQ handler */
	*((unsigned long *)0x200) = CDIC_IRQ; /* vector delivered by CDIC */

#if 0
	*((unsigned long *)0xF8) = TIMER_IRQ; /* vector 62 */
	*((unsigned long *)0xF4) = VIDEO_IRQ; /* vector 61 */
	*((unsigned long *)0x68) = SLAVE_IRQ; /* vector 26 */
#endif
}

void example_crc_calculation()
{
	int i;
	unsigned short crc_accum;
	unsigned char *data[] = {0x01, 0x00, 0x02, 0x01, 0x16, 0x72, 0x00, 0x03, 0x32, 0x00, 0x53, 0xBA};

	crc_accum = 0; /* Init = 0 is assumed */
	for (i = 0; i < 12; i++)
	{
		crc_accum = CRC_CCITT_ROUND(crc_accum, (unsigned short)data[i]);
	}

	/* 0xffff is expected */
	printf("CRC Result %x\n", crc_accum);
}

int main(argc, argv)
int argc;
char *argv[];
{
	int bytes;
	int wait;
	int framecnt = 0;

	take_system();

	print("Hello CDIC!\n");

	example_crc_calculation();

	slave_stereo_audio_cd_attenuation();
	slave_unmute();

	/*
	test_mode2_read();
	test_mode1_read();
	test_xa_play();
	*/

	test_audiomap_play_stop();
	test_audiomap_play_abort();
	test_fetch_toc();
	test_cdda_play();

	printf("\nTest finished. Press Ctrl-C to reset!\n");
	for (;;)
		;

	exit(0);
}
