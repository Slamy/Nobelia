#include "hwreg.h"
#include "framework.h"

/* Reads the map theme of Zelda - Wand of Gamelon as data */
void test_mode1_read()
{
    int i, j;

    printf("# test_mode1_read()\n");

    resetcdic();
    print_state();
    cdic_irq_occured = 0;

    /* Fill with random data to recognize changes */
    for (i = 0; i < 16; i++)
    {
        CDIC_RAM_DBUF0[i] = 0x5555;
        CDIC_RAM_DBUF1[i] = 0x5555;
    }

    /* Zelda - Wand of Gamelon - Map Theme*/
    CDIC_FILE = 0x0000;     /* MODE2 File filter is ignored here*/
    CDIC_CHAN = 0x0000;     /* Channel filter is ignored here */
    CDIC_ACHAN = 0x0000;    /* Audio channel filter is ignored here */
    CDIC_TIME = 0x24362100; /* MSF 24:36:21 */
    CDIC_CMD = CMD_MODE1;   /* Command = Read MODE1 */
    CDIC_DBUF = 0xc000;     /* Execute command */

    bufpos = 0;
    timecnt = 0;
    while (bufpos < 8)
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
            reg_buffer[bufpos][34] = int_audctl;

            timecnt = 0;

            bufpos++;
        }
        timecnt++;
        if (timecnt > 300000)
        {
            for (i = 0; i < 16; i++)
            {
                reg_buffer[bufpos][i] = CDIC_RAM_DBUF0[i];
                reg_buffer[bufpos][i + 16] = CDIC_RAM_DBUF1[i];
            }
            reg_buffer[bufpos][32] = int_dbuf;
            reg_buffer[bufpos][33] = timecnt;
            reg_buffer[bufpos][34] = int_audctl;

            bufpos++;

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
    # test_mode1_read()
    State INT: 0000 0000 0000 d7fe  Now: 7fff 7fff 0801 d7fe
    Got 8 entries
    0  2436 2102 0100 6401 0100 6401 2c3b 1c1c 2c3b 1c1c 1c1c 1c1c 1c1c 1c1c 00d4 dcf4 2436 2202 0100 6000 0100 6000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 4820 1755f d7fe
    1  2436 2302 0100 6000 0100 6000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 2436 2202 0100 6000 0100 6000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 4821 02cf d7fe
    2  2436 2302 0100 6000 0100 6000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 2436 2402 0100 6000 0100 6000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 4820 02d0 d7fe
    3  2436 2502 0100 6401 0100 6401 1c1c 1c1c 1c1c 1c1c 1c1c 1c1c 1c1c 1c1c 2103 c0d5 2436 2402 0100 6000 0100 6000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 4821 02dd d7fe
    4  2436 2502 0100 6401 0100 6401 1c1c 1c1c 1c1c 1c1c 1c1c 1c1c 1c1c 1c1c 2103 c0d5 2436 2602 0100 6000 0100 6000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 4820 02d0 d7fe
    5  2436 2702 0100 6000 0100 6000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 2436 2602 0100 6000 0100 6000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 4821 02d0 d7fe
    6  2436 2702 0100 6000 0100 6000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 2436 2802 0100 6000 0100 6000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 4820 02dc d7fe
    7  2436 2902 0100 6401 0100 6401 1c1c 1c1c 1c1c 1c1c 1c1c 1c2c 1c1c 1c2c fe04 e52e 2436 2802 0100 6000 0100 6000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 4821 02d0 d7fe
    */
}
