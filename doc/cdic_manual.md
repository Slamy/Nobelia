# CDIC User Manual

To understand the following chapters, basic knowledge of the Green Book and CD-I tracks is assumed.

If you want to play music, ensure that the audio mixing circuit is correctly configured. Otherwise audio might be muted. The CDIC is usually paired with a DAC and some mixing circuitry to control volume envelope.

## A foreword on sectors

A CD sector is 2352 bytes in size.
A CD-I Mode 2 sector has additional header data of 24 bytes

	00 ff ff ff ff ff ff ff ff ff ff 00  12 Byte Sync Pattern
    01 42 71 							 BCD Timecode
    02 									 Mode (always 02)
    01 									 File
    04 									 Channel
    64 									 Submode
    05 									 Coding
    01 04 64 05							 Repetition of the last 4 bytes

Due to this header data, the payload is reduced to 2324 byte

## Resetting the CDIC


## Configuring CPU interrupts

The CDIC is capable of providing an interrupt vector to the CPU.
This feature is used by CD-RTOS in the CD-I Mono I hardware.

    *((unsigned long *)0x200) = irq_handler;
	CDIC_IVEC = 0x2480;

## Reading the Table of Contents

	CDIC_CMD = 0x0027;      /* Command = Fetch TOC */
    CDIC_DBUF = 0xc000;     /* Execute command */

With every IRQ, subcode data from the Lead In area is available.
To calculate the address of the buffer, do this

    unsigned short* subcode = (CDIC_DBUF & 1) ? 0x301324 : 0x300924;

At this position, 12 words are stored by the CDIC. Only the low byte
contains actual data while the high byte is `0xff`.

Example subcode data

	ff01 ff00 ff02 ff01 ff16 ff72 ff00 ff03 ff32 ff00 ff53 ffba

Only the lower bytes

	01 00 02 01 16 72 00 03 32 00 53 ba

## Playing CDDA

	CDIC_TIME = 0x00020000; /* MSF 00:02:00 */
    CDIC_CMD = 0x0028;      /* Command = Play CDDA */
    CDIC_DBUF = 0xc000;     /* Execute command */

Keep in mind that this only starts the reading process.
With every read sector, an IRQ is generated. With the first
IRQ just do this to start the playback:

    CDIC_AUDCTL = 0x0800;

Q Subcode data is available at the same position as during TOC fetch


## Reading CD-I data for CPU processing



## Playing CD-I ADPCM from CD

The CDIC can play ADPCM audio without CPU intervention.
In this example, the map theme of "Zelda - Wand of Gamelon" shall be played.
The song starts at timestamp 24:36:21 and is located in file 0x0100 in channel 0.

	/* Zelda - Wand of Gamelon - Map Theme*/
	CDIC_FILE = 0x0100;		/* MODE2 File filter */
	CDIC_CHAN = 0x0001;		/* MODE2 Channel filter Select which sectors to handle at all */
	CDIC_ACHAN = 0x0001;	/* Without this, the sectors will be written to data buffers */
	CDIC_TIME = 0x24362100; /* MSF 24:36:21 */
	CDIC_CMD = 0x002a;		/* Command = Read MODE2 */
	CDIC_DBUF = 0xc000;		/* Execute command */

After doing so, the CDIC will start the operation.
Keep in mind that the audio playback is not started automatically.
The CDIC will place the first ADPCM sector into the first ADPCM buffer,
resulting into the lowest nibble of DBUF being 4.
When this occurs, the audio playback can be started like this:

	if ((CDIC_AUDCTL & 0x0800) == 0 && (CDIC_DBUF & 0x000f) == 0x0004)
	{
		/* Start playback. Must be performed to hear something */
		CDIC_AUDCTL = 0x0800;
	}

## Playing CD-I ADPCM from CPU

Sometimes this is called a soundmap (Green book) or an audiomap (MAME source code).
The ADPCM data is expected as blocks of 2304 bytes.

At offset 11 in the ADPCM buffer, the coding is expected
and must be written by the CPU. It is not part of the sample data.
[The example sample from frog feast uses coding 0x04](../src/ribbit_sample.h)

At offset 12, 2304 bytes of sample data is expected.
Here an example to prepare data in C for playback

	/* [:cdic] Coding 04s, 1 channels, 4 bits, 000049d4 frequency ->  213 ms between IRQs */
	*((unsigned short *)0x30280a) = 0x0004;
	*((unsigned short *)0x30320a) = 0x0004;
	memcpy((char *)0x30280c, RibbitSoundData, 2304);
	memcpy((char *)0x30320c, RibbitSoundData + 2304, 2304);

## Reading raw data

