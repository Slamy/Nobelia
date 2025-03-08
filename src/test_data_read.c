
void test_data_read()
{
	int i, j;
	int timecnt = 0;

	resetcdic();

	cdic_irq_occured = 0;
	print_state();

	CDIC_AUDCTL = 0;
	CDIC_ACHAN = 0;
	CDIC_CMD = 0x002e;
	CDIC_DBUF = 0xC000;

	while (!cdic_irq_occured)
		;

	print_state();
	CDIC_DBUF = 0;
	/* Zelda - Wand of Gamelon - Map Theme*/
	CDIC_FILE = 0x0100;		/* MODE2 File filter */
	CDIC_CHAN = 0x0001;		/* MODE2 Channel filter Select which sectors to handle at all */
	CDIC_ACHAN = 0x0000;	/* Reset to 0, to fetch even audio sectors into normal data buffers */
	CDIC_TIME = 0x24362100; /* MSF 24:36:21 */
	CDIC_CMD = 0x002a;		/* Command = Read MODE2 */
	CDIC_DBUF = 0xc000;		/* Execute command */

	bufpos = 0;
	timecnt = 0;
	while (bufpos < 90)
	{
		if (cdic_irq_occured)
		{
			for (i = 0; i < 16; i++)
			{
				reg_buffer[bufpos][i] = CDIC_RAM_DBUF0[i];
				reg_buffer[bufpos][i + 16] = CDIC_RAM_DBUF1[i];
			}
			reg_buffer[bufpos][32] = CDIC_DBUF;
			reg_buffer[bufpos][33] = timecnt;
			timecnt = 0;

			cdic_irq_occured = 0;
			bufpos++;

		}
		timecnt++;
	}

	resetcdic();

	for (i = 0; i < bufpos; i++)
	{
		printf("%3d ", i);
		for (j = 0; j < 34; j++)
		{
			printf(" %04x", reg_buffer[i][j]);
		}

		printf("\n");
	}
}
