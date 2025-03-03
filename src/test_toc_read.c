#include "crc.h"
#include "hwreg.h"
#include "framework.h"

void test_fetch_toc()
{
    int i, j;

    unsigned short crc_accum;
    unsigned char *crc;
    unsigned short *subcode;

    printf("# test_fetch_toc()\n");

    resetcdic();

    CDIC_DBUF = 0;
    CDIC_TIME = 0x40400000; /* Timecode is just ignored */
    CDIC_CMD = CMD_TOC;     /* Command = Fetch TOC */
    CDIC_DBUF = 0xc000;     /* Execute command */

    bufpos = 0;
    timecnt = 0;
    while (bufpos < 90)
    {
        if (cdic_irq_occured)
        {
            cdic_irq_occured = 0;
            crc_accum = 0;

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

#if 0
            /* Is the playback not started yet? Do we have an audio sector in the ADPCM buffer 0? Then play! */
            if (bufpos==0)
            {
                /* Start playback. Must be performed to hear something */
                CDIC_AUDCTL = 0x0800;
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
    # test_fetch_toc()
      0  ff01 ff00 ff02 ff01 ff17 ff16 ff00 ff03 ff32 ff00 ff86 ffc0 ffff d801 d7fe 0029  CRC OK
      1  ff01 ff00 ff02 ff01 ff17 ff17 ff00 ff03 ff32 ff00 ff2c ff91 ffff d800 d7fe 0297  CRC OK
      2  ff01 ff00 ff03 ff01 ff17 ff18 ff00 ff07 ff00 ff50 ffeb ff79 ffff 5801 d7fe 02f2  CRC OK
      3  ff01 ff00 ff03 ff01 ff17 ff19 ff00 ff07 ff00 ff50 ff41 ff28 ffff 5800 d7fe 02e4  CRC OK
      4  ff01 ff00 ff03 ff01 ff17 ff20 ff00 ff07 ff00 ff50 ffe5 ffba ffff 5801 d7fe 02e2  CRC OK
      5  ff01 ff00 ffa0 ff01 ff17 ff21 ff00 ff01 ff00 ff00 ffe9 ff22 ffff 5800 d7fe 02f1  CRC OK
      6  ff01 ff00 ffa0 ff01 ff17 ff22 ff00 ff01 ff00 ff00 ff07 fff0 ffff 5801 d7fe 02e6  CRC OK
      7  ff01 ff00 ffa0 ff01 ff17 ff23 ff00 ff01 ff00 ff00 ffad ffa1 ffff 5800 d7fe 02e6  CRC OK
      8  ff01 ff00 ffa1 ff01 ff17 ff24 ff00 ff03 ff00 ff00 ffe3 ffc6 ffff 5801 d7fe 02f3  CRC OK
      9  ff01 ff00 ffa1 ff01 ff17 ff25 ff00 ff03 ff00 ff00 ff49 ff97 ffff 5800 d7fe 02e7  CRC OK
     10  ff01 ff00 ffa1 ff01 ff17 ff26 ff00 ff03 ff00 ff00 ffa7 ff45 ffff 5801 d7fe 02e2  CRC OK
     11  ff01 ff00 ffa2 ff01 ff17 ff27 ff00 ff10 ff37 ff70 ff3d ffc7 ffff 5800 d7fe 02f1  CRC OK
     12  ff01 ff00 ffa2 ff01 ff17 ff28 ff00 ff10 ff37 ff70 ff58 ff3e ffff 5801 d7fe 02e5  CRC OK
     13  ff01 ff00 ffa2 ff01 ff17 ffa9 ff00 ff10 ff37 ff70 fff2 ff6f dd2f 5880 d7fe 02e2  CRC FAIL
     14  ff01 ff00 ff01 ff01 ff17 ff30 ff00 ff00 ff02 ff00 ffd7 ff41 ffff 5801 d7fe 02f3  CRC OK
     15  ff01 ff00 ff01 ff01 ff17 ff31 ff00 ff00 ff02 ff00 ff7d ff10 ffff 5800 d7fe 02e7  CRC OK
     16  ff01 ff00 ff01 ff01 ff17 ff32 ff00 ff00 ff02 ff00 ff93 ffc2 ffff 5801 d7fe 02e4  CRC OK
     17  ff01 ff00 ff02 ff01 ff17 ff33 ff00 ff03 ff32 ff00 ffad ff23 ffff 5800 d7fe 02f4  CRC OK
     And so on
     */
}