#include "hwreg.h"
#include "framework.h"
#include "ribbit_sample.h"

static void collect_registers()
{

    reg_buffer[bufpos][0] = CDIC_RAM_DBUF0[0];
    reg_buffer[bufpos][1] = CDIC_RAM_DBUF0[1];
    reg_buffer[bufpos][2] = CDIC_RAM_DBUF1[0];
    reg_buffer[bufpos][3] = CDIC_RAM_DBUF1[1];

    reg_buffer[bufpos][4] = int_abuf;
    reg_buffer[bufpos][5] = CDIC_ABUF;
    reg_buffer[bufpos][6] = int_xbuf;
    reg_buffer[bufpos][7] = CDIC_XBUF;
    reg_buffer[bufpos][8] = int_dbuf;
    reg_buffer[bufpos][9] = CDIC_DBUF;
    reg_buffer[bufpos][10] = int_audctl;
    reg_buffer[bufpos][11] = CDIC_AUDCTL;

    reg_buffer[bufpos][12] = timecnt;
}

/* Evaluates behavior during the Help cutscene of "Zelda - Wand of Gamelon".
 * Within this cutscene the software switches frequently between audiomap and playback of audio sectors.
 * The MiSTer CD-i core (and MAME as well) have problems especially after Zelda has thrown the bomb.
 * The next spoken line should start with "Remember...". For some reason, the audio channel mask is set too late,
 * causing one missing sector of audio data.
 *
 * This test simulates the situation by also not setting the audio channel mask.
 */
void test_audiomap_to_xa_play(int mode)
{
    int i, j;
    resetcdic();

    printf("# test_audiomap_to_xa_play(%d)\n", mode);

    CDIC_RAM_DBUF0[0] = 0x5555;
    CDIC_RAM_DBUF0[1] = 0x5555;
    CDIC_RAM_DBUF1[0] = 0x5555;
    CDIC_RAM_DBUF1[1] = 0x5555;

#if 1
    /* Let's start with some frog sounds */
    /* [:cdic] Coding 04s, 1 channels, 4 bits, 000049d4 frequency ->  213 ms between IRQs */
    *((unsigned short *)0x30280a) = 0x0004;
    *((unsigned short *)0x30320a) = 0x0004;
    memcpy((char *)0x30280c, RibbitSoundData, 2304);
    memcpy((char *)0x30320c, RibbitSoundData + 2304, 2304);

    CDIC_AUDCTL = 0x2800;
    bufpos = 0;
    timecnt = 0;
    while (bufpos < 10)
    {
        if (cdic_irq_occured)
        {
            cdic_irq_occured = 0;

            collect_registers();
            timecnt = 0;

            bufpos++;
        }
        timecnt++;

        if (timecnt > 300000)
        {
            printf("Timeout!\n");
            print_state();
            break;
        }
    }
#endif

#if 0
	/* Gracefully stop audiomap with 0xff coding */
	*((unsigned short *)0x30280a) = 0x00ff;
	*((unsigned short *)0x30320a) = 0x00ff;
#endif
    /* CDIC_AUDCTL = 0; */

    /* Zelda - Demo cutscene - "Remember, tools can only be used when I'm standing up." */
    CDIC_FILE = 0x0100;                  /* MODE2 File filter */
    CDIC_CHAN = 0x0010;                  /* Just give us all the channels */
    CDIC_ACHAN = mode ? 0x0010 : 0x0000; /* Without this, the sectors will be written to data buffers */
    CDIC_TIME = 0x04040800;              /* MSF 01:42:67 */
    CDIC_CMD = CMD_MODE2;                /* Command = Read MODE2 */
    CDIC_DBUF = 0xc000;                  /* Execute command */

    while (bufpos < 20)
    {
        if (cdic_irq_occured)
        {
            cdic_irq_occured = 0;

            collect_registers();
            timecnt = 0;

/* if (int_dbuf & 0x80) */
#if 0
            if (bufpos == 13)
            {
                /* CDIC_AUDCTL = 0x0800;*/

                CDIC_ACHAN = 0x0010; /* Without this, the sectors will be written to data buffers */
                CDIC_CMD = CMD_UPDATE;
                CDIC_DBUF = 0xc000; /* Execute command */
            }
#endif

            if (mode == 0 && (int_audctl & 0x0800) == 0 && (int_dbuf & 0x4800) == 0x4800)
            {
            }
#if 0
            /* Is the playback not started yet? Do we have an audio sector in the ADPCM buffer 0? Then play! */
            if ((CDIC_AUDCTL & 0x0800) == 0 && (CDIC_DBUF & 0x4800) == 0x4800)
            {
                /* Start playback. Must be performed to hear something */
                CDIC_AUDCTL = 0x0800;

                CDIC_ACHAN = 0x0010; /* Without this, the sectors will be written to data buffers */
                CDIC_CMD = CMD_UPDATE;
                CDIC_DBUF = 0xc000; /* Execute command */
            }
#endif

            bufpos++;
        }
        timecnt++;

        if (timecnt > 300000)
        {
            printf("Timeout!\n");
            print_state();
            break;
        }
    }

    for (i = 0; i < bufpos; i++)
    {
        printf("%3d ", i);
        for (j = 0; j < 13; j++)
        {
            printf(" %04x", reg_buffer[i][j]);
        }
        printf("\n");
    }
    /*
    # test_audiomap_to_xa_play(0)
     0  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0800 0800 fffe fffe 3265
     1  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0800 0800 fffe fffe 3284
     2  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0800 0800 fffe fffe 3297
     3  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0800 0800 fffe fffe 327c
     4  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0800 0800 fffe fffe 3280
     5  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0800 0800 fffe fffe 3297
     6  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0800 0800 fffe fffe 3296
     7  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0800 0800 fffe fffe 327d
     8  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0800 0800 fffe fffe 329c
     9  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0800 0800 fffe fffe 329c
    10  ffc0 ffc0 ffc0 ffc0 7fff 7fff ffff 7fff c801 c801 fffe fffe 008d
    11  ffc0 ffc0 ffc0 ffc0 ffff 7fff 7fff 7fff c8a0 c820 fffe fffe 31cb
    12  0404 0802 ffc0 ffc0 ffff 7fff 7fff 7fff c820 c820 fffe fffe 329c
    13  0404 1102 ffc0 ffc0 ffff 7fff 7fff 7fff 4820 4820 fffe fffe 3296
    14  0404 0802 0404 0902 7fff 7fff ffff 7fff 4820 4820 fffe fffe 1484
    15  0404 0802 0404 1802 ffff 7fff 7fff 7fff 4821 4821 fffe fffe 1df2
    16  0404 2502 0404 2402 7fff 7fff ffff 7fff 4821 4821 fffe fffe 148e
    17  0404 3402 0404 2402 ffff 7fff 7fff 7fff 4820 4820 fffe fffe 1dff
    18  0404 4002 0404 4102 7fff 7fff ffff 7fff 4820 4820 fffe fffe 147e
    19  0404 4002 0404 5002 ffff 7fff 7fff 7fff 4821 4821 fffe fffe 1e02

    Note: DBUF bit 7 is set during this test. It resets on read.
    */
}
