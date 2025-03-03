#include "hwreg.h"
#include "framework.h"

/* Reads the map theme of Zelda - Wand of Gamelon as data */
void test_mode2_read()
{
    int i, j;

    printf("# test_mode2_read()\n");

    resetcdic();
    print_state();
    cdic_irq_occured = 0;

    for (i = 0; i < 16; i++)
    {
        CDIC_RAM_DBUF0[i] = 0x5555;
        CDIC_RAM_DBUF1[i] = 0x5555;
    }

    /* Does this even have an effect? */
    printf("RAM Readback %x %x \n", CDIC_RAM_DBUF0[0], CDIC_RAM_DBUF1[0]);

    /* Zelda - Wand of Gamelon - Map Theme*/
    CDIC_FILE = 0x0100;     /* MODE2 File filter */
    CDIC_CHAN = 0xffff;     /* We want all the channels! */
    CDIC_ACHAN = 0x0000;    /* Reset to 0, to fetch even audio sectors into normal data buffers */
    CDIC_TIME = 0x24362100; /* MSF 24:36:21 */
    CDIC_CMD = CMD_MODE2;   /* Command = Read MODE2 */
    CDIC_DBUF = 0xc000;     /* Execute command */

    bufpos = 0;
    timecnt = 0;
    while (bufpos < 6)
    {
        if (cdic_irq_occured)
        {
            cdic_irq_occured = 0;

            if (timecnt < 100)
            {
                printf("Spurious IRQ!\n");
            }

            for (i = 0; i < 16; i++)
            {
                reg_buffer[bufpos][i] = CDIC_RAM_DBUF0[i];
                reg_buffer[bufpos][i + 16] = CDIC_RAM_DBUF1[i];
            }
            reg_buffer[bufpos][32] = int_dbuf;
            reg_buffer[bufpos][33] = timecnt;
            /* Reading the AUDCTL register is essential for the CDIC to produce IRQs */
            reg_buffer[bufpos][34] = int_audctl;

            timecnt = 0;

            bufpos++;
        }
        timecnt++;
        if (timecnt > 300000)
        {
            printf("Timeout!\n");
            break;
        }
    }

    printf("Got %d entries\n", bufpos);

    for (i = 0; i < bufpos; i++)
    {
        printf("%3d ", i);
        for (j = 0; j < 35; j++)
        {
            printf(" %04x", reg_buffer[i][j]);
        }

        printf("\n");
    }

    /*
    # test_mode2_read()
    State INT: 0000 0000 0000 d7fe  Now: 7fff 7fff 0800 d7fe
    RAM Readback 5555 5555
    Got 6 entries
    0  ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 5555 5555 5555 5555 5555 5555 5555 5555 5555 5555 5555 5555 5555 5555 5555 5555 c800 0083 d7fe
    1  2436 2102 0100 6401 0100 6401 2c3b 1c1c 2c3b 1c1c 1c1c 1c1c 1c1c 1c1c 00d4 dcf4 2436 2202 0100 6000 0100 6000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 4820 f7f6 d7fe
    2  2436 2302 0100 6000 0100 6000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 2436 2202 0100 6000 0100 6000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 4821 02d7 d7fe
    3  2436 2302 0100 6000 0100 6000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 2436 2402 0100 6000 0100 6000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 4820 02e4 d7fe
    4  2436 2502 0100 6401 0100 6401 1c1c 1c1c 1c1c 1c1c 1c1c 1c1c 1c1c 1c1c 2103 c0d5 2436 2402 0100 6000 0100 6000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 4821 02d7 d7fe
    5  2436 2502 0100 6401 0100 6401 1c1c 1c1c 1c1c 1c1c 1c1c 1c1c 1c1c 1c1c 2103 c0d5 2436 2602 0100 6000 0100 6000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 4820 02d7 d7fe
    */
}
