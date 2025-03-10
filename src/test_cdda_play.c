#include "crc.h"
#include "hwreg.h"
#include "framework.h"

/* Start playback at 00:02:00. This test should
 * be compatible with any standard Audio CD.
 * Reading of Subchannel Q at the same time
 */
void test_cdda_play()
{
    int i, j;

    unsigned short crc_accum;
    unsigned char *crc;
    unsigned short *subcode;

    printf("# test_cdda_play()\n");

    resetcdic();
    print_state();

    cdic_irq_occured = 0;

    CDIC_TIME = 0x00020000; /* MSF 00:02:00 */
    CDIC_TIME = 0x03280000; /* MSF 00:02:00 */
    CDIC_CMD = CMD_CDDA;    /* Command = Play CDDA */
    CDIC_DBUF = 0xc000;     /* Execute command */

    bufpos = 0;
    timecnt = 0;
    while (bufpos < 500)
    {
        if (cdic_irq_occured)
        {
            cdic_irq_occured = 0;
            crc_accum = 0;

            /* Read subcode data */
            subcode = (int_dbuf & 1) ? 0x301324 : 0x300924;
            for (i = 0; i < 12; i++)
            {
                reg_buffer[bufpos][i] = subcode[i];
                crc_accum = CRC_CCITT_ROUND(crc_accum, subcode[i] & 0x00ff);
            }

            reg_buffer[bufpos][12] = crc_accum;
            reg_buffer[bufpos][13] = int_dbuf;
            reg_buffer[bufpos][14] = int_audctl;
            reg_buffer[bufpos][15] = timecnt;

            timecnt = 0;

            /* Is the playback not started yet? Then play! */
            if (bufpos == 0)
            {
                /* Start playback. Must be performed to hear something! */
                CDIC_AUDCTL = 0x0800;
            }

#if 0
            if (bufpos == 45)
            {
                /* What does this do?*/
                CDIC_CMD = CMD_UNKNOWN_2b;
                CDIC_DBUF = 0xc000; /* Execute command */
            }
#endif

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

    /*
    # test_cdda_play()
    State INT: 0000 0000 0000 d7fe  Now: 7fff 7fff 1801 d7fe
    0  ff01 ff01 ff01 ff00 ff00 ff03 ff00 ff00 ff02 ff03 ff84 ff99 ffff 5800 d7fe 11d95  CRC OK
    1  ff01 ff01 ff01 ff00 ff00 ff04 ff00 ff00 ff02 ff04 ff93 ffaa ffff 5801 dffe 02ed  CRC OK
    2  ff01 ff01 ff01 ff00 ff00 ff05 ff00 ff00 ff02 ff05 ff29 ffda ffff 5800 dffe 02dd  CRC OK
    3  ff01 ff01 ff01 ff00 ff00 ff06 ff00 ff00 ff02 ff06 fff7 ff6b ffff 5801 dffe 02f4  CRC OK
    4  ff01 ff01 ff01 ff00 ff00 ff07 ff00 ff00 ff02 ff07 ff4d ff1b ffff 5800 dffe 02ed  CRC OK
    5  ff01 ff01 ff01 ff00 ff00 ff08 ff00 ff00 ff02 ff08 ffd9 ff0d ffff 5801 dffe 02dc  CRC OK
    6  ff01 ff01 ff01 ff00 ff00 ff09 ff00 ff00 ff02 ff09 ff63 ff7d ffff 5800 dffe 02f1  CRC OK
    7  ff01 ff01 ff01 ff00 ff00 ff10 ff00 ff00 ff02 ff10 ff4c ff43 ffff 5801 dffe 02eb  CRC OK
    8  ff01 ff01 ff01 ff00 ff00 ff11 ff00 ff00 ff02 ff11 fff6 ff33 ffff 5800 dffe 02db  CRC OK
    9  ff03 ff51 ff54 ffc6 ff20 ff99 ff00 ff25 ff40 ff12 ff53 ff7a ffff 5801 dffe 02f2  CRC OK
    10  ff01 ff01 ff01 ff00 ff00 ff13 ff00 ff00 ff02 ff13 ff92 fff2 ffff 5800 dffe 02ee  CRC OK
    11  ff01 ff01 ff01 ff00 ff00 ff14 ff00 ff00 ff02 ff14 ff85 ffc1 ffff 5801 dffe 02dd  CRC OK
    12  ff01 ff01 ff01 ff00 ff00 ff15 ff00 ff00 ff02 ff15 ff3f ffb1 ffff 5800 dffe 02f3  CRC OK
    13  ff01 ff01 ff01 ff00 ff00 ff16 ff00 ff00 ff02 ff16 ffe1 ff00 ffff 5801 dffe 02ed  CRC OK
    14  ff01 ff01 ff01 ff00 ff00 ff17 ff00 ff00 ff02 ff17 ff5b ff70 ffff 5800 dffe 02db  CRC OK
    15  ff01 ff01 ff01 ff00 ff00 ff18 ff00 ff00 ff02 ff18 ffcf ff66 ffff 5801 dffe 02f1  CRC OK
    16  ff01 ff01 ff01 ff00 ff00 ff19 ff00 ff00 ff02 ff19 ff75 ff16 ffff 5800 dffe 02ec  CRC OK
    17  ff01 ff01 ff01 ff00 ff00 ff20 ff00 ff00 ff02 ff20 ff76 fffe ffff 5801 dffe 02da  CRC OK
    18  ff01 ff01 ff01 ff00 ff00 ff21 ff00 ff00 ff02 ff21 ffcc ff8e ffff 5800 dffe 02f2  CRC OK
    19  ff01 ff01 ff01 ff00 ff00 ff22 ff00 ff00 ff02 ff22 ff12 ff3f ffff 5801 dffe 02f0  CRC OK
    And so on
    */
}

/* This test is used to check for the location where the CDDA data is
 * actually stored.
 * It turns out it is nowhere. Maybe it is directly fed to the DSP?
 */
void test_where_is_cdda()
{
    int i, j;

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
            /* Note: Ok, it is nowhere... played back directly without storage? */

            reg_buffer[bufpos][12] = crc_accum;
            reg_buffer[bufpos][13] = int_dbuf;
            reg_buffer[bufpos][14] = int_audctl;
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
    /*
    # test_where_is_cdda()
    State INT: 7fff ffff 5800 d7fe  Now: 7fff 7fff 5800 d7fe
    0  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5801 d7fe 101f2
    1  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5800 dffe 02db
    2  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5801 dffe 02dc
    3  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5800 dffe 02e1
    4  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5801 dffe 02e0
    5  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5800 dffe 02da
    6  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5801 dffe 02de
    7  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5800 dffe 02de
    8  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5801 dffe 02da
    9  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5800 dffe 02e0
    10  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5801 dffe 02df
    11  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5800 dffe 02dc
    12  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5801 dffe 02de
    13  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5800 dffe 02e1
    14  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5801 dffe 02da
    15  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5800 dffe 02dd
    16  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5801 dffe 02df
    And so on
    */
}
