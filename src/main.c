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

char cdic_irq_occured = 0;
unsigned short int_abuf = 0;
unsigned short int_xbuf = 0;
unsigned short int_dbuf = 0;
unsigned short int_audctl = 0;

/* Used to store register information during a test */
/* We don't want to make any prints during the test as the baud rate is too slow */
unsigned long reg_buffer[100][40];
int bufpos = 0;
int timecnt = 0;

/* Do whatever is known to bring the CDIC into a known state */
void resetcdic()
{
	int temp;

	CDIC_ABUF = 0;
	CDIC_XBUF = 0;
	CDIC_DBUF = 0;
	CDIC_AUDCTL = 0;

	temp = CDIC_ABUF;	/* Reset IRQ flag via reading */
	temp = CDIC_XBUF;	/* Reset IRQ flag via reading */
	temp = CDIC_AUDCTL; /* Reset IRQ flag via reading */

	bufpos = 0;
	cdic_irq_occured = 0;
	int_abuf = 0;
	int_xbuf = 0;
	int_dbuf = 0;
}

void print_state()
{
	int_abuf = 0;
	int_xbuf = 0;
	int_dbuf = 0;
	int_audctl = 0;
}

#define _VA_LIST unsigned char *
#define va_start(va, paranm) (void)((va) = (_VA_LIST)__inline_va_start__())
void *__inline_va_start__(void);
#define va_end(va) (void)((va) = (_VA_LIST)0)

void print(char *format, ...)
{
}

/* Overwrite CDIC driver IRQ handling */
void take_system()
{
	/* TODO I don't understand why this works for assembler code. thx to cdifan */
	store_a6();

	CDIC_IVEC = 0x2480;
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
}

void test_cmd23()
{
	printf("# test_cmd23()\n");
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	CDIC_CMD = CMD_STOP_DISC;
	CDIC_DBUF = 0xc000; /* Execute command */
	print_state();
	print_state();
	print_state();
	CDIC_DBUF = 0;
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();

	/* Output of test
	# test_cmd23()
	State INT: 7fff ffff 5801 d7fe  Now: 7fff 7fff 5801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff d801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4881 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4881 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0881 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0881 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0881 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0881 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0881 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0881 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0881 d7fe

	The disc has stopped
	*/
}

void test_cmd24()
{
	printf("# test_cmd24()\n");
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	CDIC_CMD = CMD_UNKNOWN_24;
	CDIC_DBUF = 0xc000; /* Execute command */
	print_state();
	print_state();
	print_state();
	CDIC_DBUF = 0;
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();

	/* Output of test when Audio CD is inserted
	# test_cmd24()
	State INT: 7fff ffff 5801 d7fe  Now: 7fff 7fff 5801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff d801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 1800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 1800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 1801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 1801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 1801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 1800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 1800 d7fe

	Output of test with Zelda - Wand of Gamelon inserted
	# test_cmd24()
	State INT: 7fff ffff 4801 d7fe  Now: 7fff 7fff 4801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff c801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0801 d7fe

	Note the difference: The Audio CD has bit 12 set.
	It is interesting that bit 0 of DBUF still toggles
	*/
}

int main(argc, argv)
int argc;
char *argv[];
{
	int bytes;
	int wait;
	int framecnt = 0;

	/* print("Hello CDIC!\n"); */

	take_system();

	example_crc_calculation();

	slave_stereo_audio_cd_attenuation();
	slave_unmute();

	/* Select ONE test to execute! We don't want the tests to change each other... 
	 * The reset mechanism is still not fully understood
	*/

	/*
	These tests are for Audio CDs. Insert a CD before execution:
	test_fetch_toc();
	test_cdda_play();
	test_where_is_cdda();

	These tests are for "Zelda - Wand of Gamelon". Insert it before execution:
	test_xa_play();
	test_mode2_read();
	test_mode1_read();
	test_mode2_read_stop_read();

	These tests are for "Tetris". Insert it before execution:
	test_xa_read_during_read();

	These tests don't require any CD to be used.
	Still have one inside to have the tests working:
	test_audiomap_play_abort();
	test_audiomap_play_stop();
	test_cmd23();
	test_cmd24();
	*/
	test_audiomap_play_stop();

	resetcdic();
	for (;;)
		;

	exit(0);
}
