#include "hwreg.h"
#include "framework.h"

/* Plays the map theme of Zelda - Wand of Gamelon */
void test_xa_play()
{
    int i, j;

    printf("# test_xa_play()\n");

    resetcdic();

    cdic_irq_occured = 0;
    print_state();

    printf("Cause an update command without active reading\n");
    CDIC_AUDCTL = 0;
    CDIC_ACHAN = 0;
    CDIC_CMD = CMD_UPDATE;
    CDIC_DBUF = 0xC000;

    while (!cdic_irq_occured)
        ;

    print_state();

    CDIC_DBUF = 0;
    cdic_irq_occured = 0;

    printf("And again\n");

    CDIC_AUDCTL = 0;
    CDIC_ACHAN = 0;
    CDIC_CMD = CMD_UPDATE;
    CDIC_DBUF = 0xC000;

    while (!cdic_irq_occured)
        ;

    print_state();
    cdic_irq_occured = 0;

    CDIC_DBUF = 0;

    /* Some random data into the buffers so we recognize changes */
    *((unsigned short *)0x300000) = 0x5555;
    *((unsigned short *)0x300002) = 0x5555;
    *((unsigned short *)0x300A00) = 0x5555;
    *((unsigned short *)0x300A02) = 0x5555;
    *((unsigned short *)0x301400) = 0x5555;
    *((unsigned short *)0x301402) = 0x5555;
    *((unsigned short *)0x301E00) = 0x5555;
    *((unsigned short *)0x301E02) = 0x5555;
    *((unsigned short *)0x302800) = 0x5555;
    *((unsigned short *)0x302802) = 0x5555;
    *((unsigned short *)0x303200) = 0x5555;
    *((unsigned short *)0x303202) = 0x5555;

    /* Zelda - Wand of Gamelon - Map Theme*/
    CDIC_FILE = 0x0100;     /* MODE2 File filter */
    CDIC_CHAN = 0x0001;     /* MODE2 Channel filter Select which sectors to handle at all */
    CDIC_ACHAN = 0x0001;    /* Without this, the sectors will be written to data buffers */
    CDIC_TIME = 0x24362100; /* MSF 24:36:21 */
    CDIC_CMD = CMD_MODE2;   /* Command = Read MODE2 */
    CDIC_DBUF = 0xc000;     /* Execute command */

    bufpos = 0;
    timecnt = 0;
    while (bufpos < 90)
    {
        if (cdic_irq_occured)
        {
            cdic_irq_occured = 0;

            reg_buffer[bufpos][0] = *((unsigned short *)0x300000);
            reg_buffer[bufpos][1] = *((unsigned short *)0x300002);
            reg_buffer[bufpos][2] = *((unsigned short *)0x300A00);
            reg_buffer[bufpos][3] = *((unsigned short *)0x300A02);
            reg_buffer[bufpos][4] = *((unsigned short *)0x301400);
            reg_buffer[bufpos][5] = *((unsigned short *)0x301402);
            reg_buffer[bufpos][6] = *((unsigned short *)0x301E00);
            reg_buffer[bufpos][7] = *((unsigned short *)0x301E02);
            reg_buffer[bufpos][8] = *((unsigned short *)0x302800);
            reg_buffer[bufpos][9] = *((unsigned short *)0x302802);
            reg_buffer[bufpos][10] = *((unsigned short *)0x303200);
            reg_buffer[bufpos][11] = *((unsigned short *)0x303202);

            reg_buffer[bufpos][12] = int_abuf;
            reg_buffer[bufpos][13] = int_xbuf;
            reg_buffer[bufpos][14] = int_dbuf;
            reg_buffer[bufpos][15] = int_audctl;
            reg_buffer[bufpos][16] = timecnt;
            timecnt = 0;

            /* Is the playback not started yet? Do we have an audio sector in the ADPCM buffer 0? Then play! */
            if ((int_audctl & 0x0800) == 0 && (int_dbuf & 0x000f) == 0x0004)
            {
                /* Start playback. Must be performed to hear something */
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
        for (j = 0; j < 17; j++)
        {
            printf(" %04x", reg_buffer[i][j]);
        }

        printf("\n");
    }
    /* Output of test on 210/05:
    # test_xa_play()
    State INT: 0000 0000 0000 d7fe  Now: 7fff 7fff 0800 d7fe
    Cause an update command without active reading
    State INT: 7fff ffff 4801 d7fe  Now: 7fff 7fff 4801 d7fe
    And again
    State INT: 7fff ffff c801 d7fe  Now: 7fff 7fff c801 d7fe
    0  2436 2202 5555 5555 5555 5555 5555 5555 5555 5555 5555 5555 7fff ffff 4824 d7fe cbd5
    1  2436 2202 2436 2302 5555 5555 5555 5555 5555 5555 5555 5555 7fff ffff 4820 dffe 02e5
    2  2436 2402 2436 2302 5555 5555 5555 5555 5555 5555 5555 5555 7fff ffff 4821 dffe 02d8
    3  2436 2402 2436 2502 5555 5555 5555 5555 5555 5555 5555 5555 7fff ffff 4820 dffe 02d9
    4  2436 2402 2436 2602 5555 5555 5555 5555 5555 5555 5555 5555 7fff ffff 4825 dffe 02e5
    5  2436 2702 2436 2602 5555 5555 5555 5555 5555 5555 5555 5555 7fff ffff 4821 dffe 02d8
    6  2436 2702 2436 2802 5555 5555 5555 5555 5555 5555 5555 5555 7fff ffff 4820 dffe 02d9
    It continues like this
    */
}

/* Plays multiple channels from Tetris. The menu themes are all interleaved on a single track
 * We can use this track to play around with this feature! :3
 * Note: The CDIC can glitch up when changing channels during playback!
 * This can be explained considering the new relevant audio sectors don't have the same interval.
 */
void test_xa_channel_change()
{
    int i, j;
    int channel = 0;
    int sectorcnt;
    printf("# test_xa_channel_change()\n");

    resetcdic();
    cdic_irq_occured = 0;

    /* Tetris - Menu Theme when game is booted */
    CDIC_FILE = 0x0100;     /* MODE2 File filter */
    CDIC_CHAN = 0x0001;     /* Just give us all the channels */
    CDIC_ACHAN = 0x0001;    /* Without this, the sectors will be written to data buffers */
    CDIC_TIME = 0x01426700; /* MSF 01:42:67 */
    CDIC_CMD = CMD_MODE2;   /* Command = Read MODE2 */
    CDIC_DBUF = 0xc000;     /* Execute command */

    bufpos = 0;

    for (i = 0; i < 9; i++)
    {
        /* printf("Channel %d\n", channel); */

        sectorcnt = 0;
        timecnt = 0;
        while (sectorcnt < 20)
        {
            if (cdic_irq_occured)
            {
                sectorcnt++;

                if (sectorcnt < 10)
                {
                    reg_buffer[bufpos][0] = int_abuf;
                    reg_buffer[bufpos][1] = int_xbuf;
                    reg_buffer[bufpos][2] = int_dbuf;
                    reg_buffer[bufpos][3] = int_audctl;
                    reg_buffer[bufpos][4] = channel;
                    reg_buffer[bufpos][5] = timecnt;
                    bufpos++;
                }

                timecnt = 0;
                cdic_irq_occured = 0;

                /* Is the playback not started yet? Do we have an audio sector in the ADPCM buffer 0? Then play! */
                if ((int_audctl & 0x0800) == 0 && (int_dbuf & 0x000f) == 0x0004)
                {
                    /* Start playback. Must be performed to hear something */
                    CDIC_AUDCTL = 0x0800;
                }
            }
            timecnt++;

            if (timecnt > 300000)
            {
                printf("Timeout!\n");
                break;
            }
        }

        /* Now change the channel. The update command is essential for that */
        channel++;
        CDIC_ACHAN = 1 << channel;
        CDIC_CHAN = 1 << channel;
        CDIC_CMD = CMD_UPDATE;
        CDIC_DBUF = 0xc000; /* Execute command */
        /* The update command will not create its own IRQ! */

        /* If this was channel 8, no data is delivered from CD */
    }

    /* Channel 8 doesn't exist in this track. */
    /* Abort audio playback to stop the broken loop */
    CDIC_AUDCTL = 0;

    for (i = 0; i < bufpos; i++)
    {
        printf("%3d ", i);
        for (j = 0; j < 6; j++)
        {
            printf(" %04x", reg_buffer[i][j]);
        }
        printf("\n");
    }

    /* Output of test on 210/05
    # test_xa_channel_change()
    Timeout!
    0  7fff ffff 4824 d7fe 0000 9bb1
    1  7fff ffff 4825 dffe 0000 1b06
    2  7fff ffff 4824 dffe 0000 1b21
    3  7fff ffff 4825 dffe 0000 1af5
    4  7fff ffff 4824 dffe 0000 1b07
    5  7fff ffff 4825 dffe 0000 1b23
    6  7fff ffff 4824 dffe 0000 1b10
    7  7fff ffff 4825 dffe 0000 1b07
    8  7fff ffff 4824 dffe 0000 1b20
    9  7fff ffff 4824 dffe 0001 1e79
    10  7fff ffff 4825 dffe 0001 1b19
    11  7fff ffff 4824 dffe 0001 1b17
    12  7fff ffff 4825 dffe 0001 1b08
    13  7fff ffff 4824 dffe 0001 1b1a
    14  7fff ffff 4825 dffe 0001 1b16
    15  7fff ffff 4824 dffe 0001 1b0b
    16  7fff ffff 4825 dffe 0001 1b17
    17  7fff ffff 4824 dffe 0001 1b16
    18  7fff ffff 4824 dffe 0002 1e79
    19  7fff ffff 4825 dffe 0002 1b16
    20  7fff ffff 4824 dffe 0002 1b15
    21  7fff ffff 4825 dffe 0002 1b0d
    22  7fff ffff 4824 dffe 0002 1b16
    23  7fff ffff 4825 dffe 0002 1b17
    24  7fff ffff 4824 dffe 0002 1b0b
    25  7fff ffff 4825 dffe 0002 1b17
    26  7fff ffff 4824 dffe 0002 1b19
    27  7fff ffff 4824 dffe 0003 1e77
    28  7fff ffff 4825 dffe 0003 1b18
    29  7fff ffff 4824 dffe 0003 1b07
    30  7fff ffff 4825 dffe 0003 1b16
    31  7fff ffff 4824 dffe 0003 1b17
    32  7fff ffff 4825 dffe 0003 1b07
    33  7fff ffff 4824 dffe 0003 1b18
    34  7fff ffff 4825 dffe 0003 1b14
    35  7fff ffff 4824 dffe 0003 1b07
    36  7fff ffff 4824 dffe 0004 1e77
    37  7fff ffff 4825 dffe 0004 1b06
    38  7fff ffff 4824 dffe 0004 1b23
    39  7fff ffff 4825 dffe 0004 1b0b
    40  7fff ffff 4824 dffe 0004 1b09
    41  7fff ffff 4825 dffe 0004 1af9
    42  7fff ffff 4824 dffe 0004 1b0a
    43  7fff ffff 4825 dffe 0004 1b09
    44  7fff ffff 4824 dffe 0004 1b28
    45  7fff ffff 4824 dffe 0005 1e79
    46  7fff ffff 4825 dffe 0005 1b29
    47  7fff ffff 4824 dffe 0005 1b08
    48  7fff ffff 4825 dffe 0005 1b0a
    49  7fff ffff 4824 dffe 0005 1b26
    50  7fff ffff 4825 dffe 0005 1b08
    51  7fff ffff 4824 dffe 0005 1b0c
    52  7fff ffff 4825 dffe 0005 1b24
    53  7fff ffff 4824 dffe 0005 1b0e
    54  7fff ffff 4824 dffe 0006 1e79
    55  7fff ffff 4825 dffe 0006 1b17
    56  7fff ffff 4824 dffe 0006 1b17
    57  7fff ffff 4825 dffe 0006 1b0b
    58  7fff ffff 4824 dffe 0006 1b15
    59  7fff ffff 4825 dffe 0006 1b17
    60  7fff ffff 4824 dffe 0006 1b08
    61  7fff ffff 4825 dffe 0006 1b16
    62  7fff ffff 4824 dffe 0006 1b17
    63  7fff ffff 4824 dffe 0007 1e77
    64  7fff ffff 4825 dffe 0007 1b17
    65  7fff ffff 4824 dffe 0007 1b07
    66  7fff ffff 4825 dffe 0007 1b18
    67  7fff ffff 4824 dffe 0007 1b16
    68  7fff ffff 4825 dffe 0007 1b07
    69  7fff ffff 4824 dffe 0007 1b19
    70  7fff ffff 4825 dffe 0007 1b13
    71  7fff ffff 4824 dffe 0007 1b07

    Note: Channel 8 is never causing an IRQ. This channel doesn't exist on this track.
    */
}

/* Just an experiment. During the read, another MODE2 read command is issued.
 * For Tetris
 */
void test_xa_read_during_read()
{
    int i, j;
    int secondread = 0;
    int sectorcnt;
    printf("# test_xa_read_during_read()\n");

    resetcdic();
    cdic_irq_occured = 0;

    /* Tetris - Menu Theme when game is booted */
    CDIC_FILE = 0x0100;     /* MODE2 File filter */
    CDIC_CHAN = 0x0001;     /* Just give us all the channels */
    CDIC_ACHAN = 0x0001;    /* Without this, the sectors will be written to data buffers */
    CDIC_TIME = 0x01426700; /* MSF 01:42:67 */
    CDIC_CMD = CMD_MODE2;   /* Command = Read MODE2 */
    CDIC_DBUF = 0xc000;     /* Execute command */

    bufpos = 0;
    timecnt = 0;

    while (bufpos < 90)
    {
        if (cdic_irq_occured)
        {
            cdic_irq_occured = 0;

            reg_buffer[bufpos][0] = CDIC_RAM_DBUF0[0];
            reg_buffer[bufpos][1] = CDIC_RAM_DBUF0[1];
            reg_buffer[bufpos][2] = CDIC_RAM_DBUF1[0];
            reg_buffer[bufpos][3] = CDIC_RAM_DBUF1[1];

            /* Note: CDIC_RAM_DBUF1 never contains data.
             * The CDIC seems to write 0xffc0 to the header position for the timecode so
             * it is not mistaken for a valid one.
             */

            reg_buffer[bufpos][4] = int_dbuf;
            reg_buffer[bufpos][5] = int_audctl;
            reg_buffer[bufpos][7] = int_abuf;
            reg_buffer[bufpos][8] = int_xbuf;
            reg_buffer[bufpos][6] = secondread;
            reg_buffer[bufpos][9] = timecnt;
            timecnt = 0;

            /* Is the playback not started yet? Do we have an audio sector in the ADPCM buffer 0? Then play! */
            if ((int_audctl & 0x0800) == 0 && (int_dbuf & 0x000f) == 0x0004)
            {
                /* Start playback. Must be performed to hear something */
                CDIC_AUDCTL = 0x0800;
            }

            if (bufpos == 50)
            {
                CDIC_TIME = 0x01426700; /* MSF 01:42:67 */
                CDIC_CMD = CMD_MODE2;   /* Command = Read MODE2 */
                CDIC_DBUF = 0xc000;     /* Execute command */
                secondread = 1;
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

    /* Stop the probably glitched playback */
    CDIC_AUDCTL = 0;

    for (i = 0; i < bufpos; i++)
    {
        printf("%3d ", i);
        for (j = 0; j < 8; j++)
        {
            printf(" %04x", reg_buffer[i][j]);
        }

        printf("\n");
    }

    /* Output of test on 210/05
     # test_xa_read_during_read()
     0  0142 6802 ffc0 ffc0 4824 d7fe 0000 7fff
     1  0143 0102 ffc0 ffc0 4825 dffe 0000 7fff
     2  0143 0902 ffc0 ffc0 4824 dffe 0000 7fff
     3  0143 1702 ffc0 ffc0 4825 dffe 0000 7fff
     4  0143 2502 ffc0 ffc0 4824 dffe 0000 7fff
     5  0143 3302 ffc0 ffc0 4825 dffe 0000 7fff
     6  0143 4102 ffc0 ffc0 4824 dffe 0000 7fff
     7  0143 4902 ffc0 ffc0 4825 dffe 0000 7fff
     8  0143 5702 ffc0 ffc0 4824 dffe 0000 7fff
     9  0143 6502 ffc0 ffc0 4825 dffe 0000 7fff
    10  0143 7302 ffc0 ffc0 4824 dffe 0000 7fff
    11  0144 0602 ffc0 ffc0 4825 dffe 0000 7fff
    12  0144 1402 ffc0 ffc0 4824 dffe 0000 7fff
    13  0144 2202 ffc0 ffc0 4825 dffe 0000 7fff
    14  0144 3002 ffc0 ffc0 4824 dffe 0000 7fff
    15  0144 3802 ffc0 ffc0 4825 dffe 0000 7fff
    16  0144 4602 ffc0 ffc0 4824 dffe 0000 7fff
    17  0144 5402 ffc0 ffc0 4825 dffe 0000 7fff
    18  0144 6202 ffc0 ffc0 4824 dffe 0000 7fff
    19  0144 7002 ffc0 ffc0 4825 dffe 0000 7fff
    20  0145 0302 ffc0 ffc0 4824 dffe 0000 7fff
    21  0145 1102 ffc0 ffc0 4825 dffe 0000 7fff
    22  0145 1902 ffc0 ffc0 4824 dffe 0000 7fff
    23  0145 2702 ffc0 ffc0 4825 dffe 0000 7fff
    24  0145 3502 ffc0 ffc0 4824 dffe 0000 7fff
    25  0145 4302 ffc0 ffc0 4825 dffe 0000 7fff
    26  0145 5102 ffc0 ffc0 4824 dffe 0000 7fff
    27  0145 5902 ffc0 ffc0 4825 dffe 0000 7fff
    28  0145 6702 ffc0 ffc0 4824 dffe 0000 7fff
    29  0146 0002 ffc0 ffc0 4825 dffe 0000 7fff
    30  0146 0802 ffc0 ffc0 4824 dffe 0000 7fff
    31  0146 1602 ffc0 ffc0 4825 dffe 0000 7fff
    32  0146 2402 ffc0 ffc0 4824 dffe 0000 7fff
    33  0146 3202 ffc0 ffc0 4825 dffe 0000 7fff
    34  0146 4002 ffc0 ffc0 4824 dffe 0000 7fff
    35  0146 4802 ffc0 ffc0 4825 dffe 0000 7fff
    36  0146 5602 ffc0 ffc0 4824 dffe 0000 7fff
    37  0146 6402 ffc0 ffc0 4825 dffe 0000 7fff
    38  0146 7202 ffc0 ffc0 4824 dffe 0000 7fff
    39  0147 0502 ffc0 ffc0 4825 dffe 0000 7fff
    40  0147 1302 ffc0 ffc0 4824 dffe 0000 7fff
    41  0147 2102 ffc0 ffc0 4825 dffe 0000 7fff
    42  0147 2902 ffc0 ffc0 4824 dffe 0000 7fff
    43  0147 3702 ffc0 ffc0 4825 dffe 0000 7fff
    44  0147 4502 ffc0 ffc0 4824 dffe 0000 7fff
    45  0147 5302 ffc0 ffc0 4825 dffe 0000 7fff
    46  0147 6102 ffc0 ffc0 4824 dffe 0000 7fff
    47  0147 6902 ffc0 ffc0 4825 dffe 0000 7fff
    48  0148 0202 ffc0 ffc0 4824 dffe 0000 7fff
    49  0148 1002 ffc0 ffc0 4825 dffe 0000 7fff
    50  0148 1802 ffc0 ffc0 4824 dffe 0000 7fff
    51  0142 6802 ffc0 ffc0 4824 dffe 0001 7fff
    52  0143 0102 ffc0 ffc0 4825 dffe 0001 7fff
    53  0143 0902 ffc0 ffc0 4824 dffe 0001 7fff
    54  0143 1702 ffc0 ffc0 4825 dffe 0001 7fff
    55  0143 2502 ffc0 ffc0 4824 dffe 0001 7fff
    56  0143 3302 ffc0 ffc0 4825 dffe 0001 7fff
    57  0143 4102 ffc0 ffc0 4824 dffe 0001 7fff
    58  0143 4902 ffc0 ffc0 4825 dffe 0001 7fff
    59  0143 5702 ffc0 ffc0 4824 dffe 0001 7fff
    60  0143 6502 ffc0 ffc0 4825 dffe 0001 7fff
    61  0143 7302 ffc0 ffc0 4824 dffe 0001 7fff
    62  0144 0602 ffc0 ffc0 4825 dffe 0001 7fff
    63  0144 1402 ffc0 ffc0 4824 dffe 0001 7fff
    64  0144 2202 ffc0 ffc0 4825 dffe 0001 7fff
    65  0144 3002 ffc0 ffc0 4824 dffe 0001 7fff
    66  0144 3802 ffc0 ffc0 4825 dffe 0001 7fff
    67  0144 4602 ffc0 ffc0 4824 dffe 0001 7fff
    68  0144 5402 ffc0 ffc0 4825 dffe 0001 7fff
    69  0144 6202 ffc0 ffc0 4824 dffe 0001 7fff
    70  0144 7002 ffc0 ffc0 4825 dffe 0001 7fff
    71  0145 0302 ffc0 ffc0 4824 dffe 0001 7fff
    72  0145 1102 ffc0 ffc0 4825 dffe 0001 7fff
    73  0145 1902 ffc0 ffc0 4824 dffe 0001 7fff
    74  0145 2702 ffc0 ffc0 4825 dffe 0001 7fff
    75  0145 3502 ffc0 ffc0 4824 dffe 0001 7fff
    76  0145 4302 ffc0 ffc0 4825 dffe 0001 7fff
    77  0145 5102 ffc0 ffc0 4824 dffe 0001 7fff
    78  0145 5902 ffc0 ffc0 4825 dffe 0001 7fff
    79  0145 6702 ffc0 ffc0 4824 dffe 0001 7fff
    80  0146 0002 ffc0 ffc0 4825 dffe 0001 7fff
    81  0146 0802 ffc0 ffc0 4824 dffe 0001 7fff
    82  0146 1602 ffc0 ffc0 4825 dffe 0001 7fff
    83  0146 2402 ffc0 ffc0 4824 dffe 0001 7fff
    84  0146 3202 ffc0 ffc0 4825 dffe 0001 7fff
    85  0146 4002 ffc0 ffc0 4824 dffe 0001 7fff
    86  0146 4802 ffc0 ffc0 4825 dffe 0001 7fff
    87  0146 5602 ffc0 ffc0 4824 dffe 0001 7fff
    88  0146 6402 ffc0 ffc0 4825 dffe 0001 7fff
    89  0146 7202 ffc0 ffc0 4824 dffe 0001 7fff
    */
}
