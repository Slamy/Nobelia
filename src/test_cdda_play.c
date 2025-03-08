#include "crc.h"
#include "hwreg.h"
#include "framework.h"

void test_cdda_play()
{
    int i, j;
    int timecnt = 0;

    unsigned short crc_accum;
    unsigned char *crc;
    unsigned short *subcode;

    printf("# test_cdda_play()\n");

    resetcdic();
    print_state();

    cdic_irq_occured = 0;

    CDIC_TIME = 0x00020000; /* MSF 00:02:00 */
    CDIC_CMD = CMD_CDDA;    /* Command = Play CDDA */
    CDIC_DBUF = 0xc000;     /* Execute command */

    bufpos = 0;
    timecnt = 0;
    while (bufpos < 90)
    {
        if (cdic_irq_occured)
        {
            cdic_irq_occured = 0;
            crc_accum = 0;

            subcode = (CDIC_DBUF & 1) ? 0x301324 : 0x300924;

            for (i = 0; i < 12; i++)
            {
                reg_buffer[bufpos][i] = subcode[i];
                crc_accum = CRC_CCITT_ROUND(crc_accum, subcode[i] & 0x00ff);
            }

            reg_buffer[bufpos][12] = crc_accum;
            reg_buffer[bufpos][13] = CDIC_DBUF;
            reg_buffer[bufpos][14] = CDIC_AUDCTL;
            reg_buffer[bufpos][15] = timecnt;

            timecnt = 0;

            /* Is the playback not started yet? Then play! */
            if (bufpos == 0)
            {
                /* Start playback. Must be performed to hear something! */
                CDIC_AUDCTL = 0x0800;
            }

            bufpos++;
        }
        timecnt++;

        if (timecnt > 300000)
        {
            printf("Timeout!\n");
            break;
        }
    }

    for (i = 0; i < bufpos; i++)
    {
        printf("%3d ", i);
        for (j = 0; j < 16; j++)
        {
            printf(" %04x", reg_buffer[i][j]);
        }

        if (reg_buffer[i][12] == 0xffff)
        {
            printf("  CRC OK");
        }
        else
        {
            printf("  CRC FAIL");
        }

        printf("\n");
    }
}

void test_where_is_cdda()
{
    int i, j;
    int timecnt = 0;

    unsigned short crc_accum;
    unsigned char *crc;

    printf("# test_where_is_cdda()\n");

    print_state();

    cdic_irq_occured = 0;

    CDIC_TIME = 0x00020000; /* MSF 00:02:00 */
    CDIC_CMD = CMD_CDDA;    /* Command = Play CDDA */
    CDIC_DBUF = 0xc000;     /* Execute command */

    bufpos = 0;
    timecnt = 0;
    while (bufpos < 90)
    {
        if (cdic_irq_occured)
        {
            cdic_irq_occured = 0;
            crc_accum = 0;

            reg_buffer[bufpos][0] = CDIC_RAM_DBUF0[200];
            reg_buffer[bufpos][1] = CDIC_RAM_DBUF0[100];
            reg_buffer[bufpos][2] = CDIC_RAM_DBUF1[200];
            reg_buffer[bufpos][3] = CDIC_RAM_DBUF1[100];
            reg_buffer[bufpos][4] = CDIC_RAM_UNKNOWN0[200];
            reg_buffer[bufpos][5] = CDIC_RAM_UNKNOWN0[100];
            reg_buffer[bufpos][6] = CDIC_RAM_UNKNOWN1[200];
            reg_buffer[bufpos][7] = CDIC_RAM_UNKNOWN1[100];
            reg_buffer[bufpos][8] = CDIC_RAM_ADPCM0[200];
            reg_buffer[bufpos][9] = CDIC_RAM_ADPCM0[100];
            reg_buffer[bufpos][10] = CDIC_RAM_ADPCM1[200];
            reg_buffer[bufpos][11] = CDIC_RAM_ADPCM1[100];
            /* Ok, it is nowhere... played back directly without storage? */

            reg_buffer[bufpos][12] = crc_accum;
            reg_buffer[bufpos][13] = CDIC_DBUF;
            reg_buffer[bufpos][14] = CDIC_AUDCTL;
            reg_buffer[bufpos][15] = timecnt;

            timecnt = 0;

            /* Is the playback not started yet? Then play! */
            if (bufpos == 0)
            {
                /* Start playback. Must be performed to hear something! */
                CDIC_AUDCTL = 0x0800;
            }

            bufpos++;
        }
        timecnt++;

        if (timecnt > 300000)
        {
            printf("Timeout!\n");
            break;
        }
    }

    for (i = 0; i < bufpos; i++)
    {
        printf("%3d ", i);
        for (j = 0; j < 16; j++)
        {
            printf(" %04x", reg_buffer[i][j]);
        }

        printf("\n");
    }
}
