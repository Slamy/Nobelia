# CDIC User Manual

To understand the following chapters, basic knowledge of the Green Book and CD-I tracks is assumed.

If you want to play music, ensure that the audio mixing circuit is correctly configured. Otherwise audio might be muted. The CDIC is usually paired with a DAC and some mixing circuitry to control volume envelope.

## Resetting the CDIC


## Configuring CPU interrupts

The CDIC is capable of providing an interrupt vector to the CPU.
This feature is used by CD-RTOS in the CD-I Mono I hardware.

    *((unsigned long *)0x200) = irq_handler;
	CDIC_IVEC = 0x2480;

## Reading the Table of Contents


## Playing CDDA from CD


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


## Playing CD-I ADPCM from CPU


## Reading raw data

