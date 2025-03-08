#include "ribbit_sample.h"
#include "hwreg.h"
#include "framework.h"


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
void test_audiomap_play_stop()
{
	printf("# test_audiomap_play_stop()\n");

	resetcdic();

	/* [:cdic] Coding 05, 2 channels, 4 bits, 000049d4 frequency -> 106 ms between IRQs */
	*((unsigned short *)0x30280a) = 0x0005;
	*((unsigned short *)0x30320a) = 0x0005;

	/* [:cdic] Coding 04s, 1 channels, 4 bits, 000049d4 frequency ->  213 ms between IRQs */
	*((unsigned short *)0x30280a) = 0x0004;
	*((unsigned short *)0x30320a) = 0x0004;
	memcpy((char *)0x30280c, RibbitSoundData, 2304);
	memcpy((char *)0x30320c, RibbitSoundData + 2304, 2304);

	print("Start audiomap and stop it with 0xff coding\n");
	CDIC_AUDCTL = 0x2800;

	observe_audiomap_registers(6, 1);

	/* Gracefully stop audiomap with 0xff coding */
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
void test_audiomap_play_abort()
{
	printf("# test_audiomap_play_abort()\n");

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
