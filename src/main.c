#include <cdfm.h>
#include <csd.h>
#include <modes.h>
#include <sysio.h>
#include <ucm.h>
#include <stdio.h>
#include <memory.h>
#include "video.h"
#include "graphics.h"
#include "sample.h"
#include "cdic.h"
#include "irq.h"

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned int

#define NULL 0

/* Sounds */
#define MAXSOUNDS 1

char cdic_irq_occured;
unsigned short int_abuf;
unsigned short int_xbuf;

#define DEBUG

int Sound;					   /* Audio device handle */
int SoundID[MAXSOUNDS * 2];	   /* Soundmap IDs. Create a set for each frog */
WORD SoundLengths[MAXSOUNDS] = /* Length of each sound */
	{
		2304 * 2};

void CreateSoundMap(Index, Filename, Length);

void InitSound()
{
	WORD Loop;
	char *DeviceName;
	char *Data = SoundData;

#ifdef DEBUG
	printf("Initialize Sound\n");
#endif

	/* Set the audio device handle to invalid */
	Sound = -1;

	/* Set sound id handle to invalid */
	for (Loop = 0; Loop < MAXSOUNDS * 2; Loop++)
	{
		SoundID[Loop] = -1;
	}

	/* Find the audio device */
	DeviceName = csd_devname(DT_AUDIO, 1);

	if (NULL == DeviceName)
	{
#ifdef DEBUG
		printf("Unable to find audio device\n");
#endif
		return;
	}

	/* Open the audio device */
	Sound = open(DeviceName, S_IREAD | S_IWRITE);

	/* Unable to open the sound device */
	if (-1 == Sound)
	{
#ifdef DEBUG
		printf("Unable to open audio device\n");
#endif
		/* Release the memory for the device name */
		free(DeviceName);

		return;
	}

	/* Release the memory for the device name, as it's no longer needed. */
	free(DeviceName);

	/* Set maximum volume in left-left and right-right channels */
	sc_atten(Sound, 0x00800080);

	/* Create soundmap for both frogs, using the same sound data */
	for (Loop = 0; Loop < MAXSOUNDS; Loop++)
	{
		CreateSoundMap(Loop, (DWORD *)Data, SoundLengths[Loop]);

		/* Advance the sound pointer to the next sound */
		Data += SoundLengths[Loop];
	}
}

void CloseSound()
{
	WORD Loop;

	/* If the Sound device isn't invalid, free the sound device, and sounds */
	if (Sound != -1)
	{
		for (Loop = 0; Loop < MAXSOUNDS; Loop++)
		{
			if (SoundID[Loop] != -1)
			{
				sm_close(Sound, SoundID[Loop]);
			}
		}

		/* Stop sound device */
		sm_off(Sound);

		/* Close the sound device */
		close(Sound);

		/* Mark the sound device as invalid */
		Sound = -1;
	}
}

void CreateSoundMap(Index, Data, Length)
	WORD Index;
char *Data;
WORD Length;
{
	WORD Loop;
	char *Buffer;

	/* Create the sound map for the first frog */
#ifdef DEBUG
	printf("Creating sound map %d\n", Index + 1);
#endif

	/* Create a sound map, based on the length of the sound */
	SoundID[Index] = sm_creat(Sound, D_CMONO, Length * 18 / 2304, &Buffer);

	/* If the sound map invalid, fail out. */
	if (-1 == SoundID[Index])
	{
#ifdef DEBUG
		printf("Unable to create sound map %d\n", Index + 1);
#endif
		return;
	}

	/* Copy data to sound buffer */
	memcpy(Buffer, Data, Length);

	/* Create the sound map for the second frog */
	Index += MAXSOUNDS;

#ifdef DEBUG
	printf("Creating sound map %d\n", Index + 1);
#endif

	/* Create a sound map, based on the length of the sound */
	SoundID[Index] = sm_creat(Sound, D_CMONO, Length / 128 * 18, (char *)&Buffer);

	/* If the sound map invalid, fail out. */
	if (-1 == SoundID[Index])
	{
#ifdef DEBUG
		printf("Unable to create sound map %d\n", Index + 1);
#endif
		return;
	}

	/* Copy data to sound buffer */
	memcpy(Buffer, Data, Length);
}

#define SIG_AUDIO 0x0010
static STAT_BLK Status;
short finished = 0;
void PlaySound(SoundNumber)
	WORD SoundNumber;
{
	int result;
	static AudioStatus audiostat;

	Status.asy_sig = SIG_AUDIO;
	finished = 0;

	/* Play sound */
	/* sc_atten(Sound, 0x00600060);*/

	result = sm_out(Sound, SoundID[SoundNumber], &Status);
#ifdef DEBUG
	printf("Playing sound %d -> %d\n", SoundNumber + 1, result);
#endif

	result = sm_stat(Sound, &audiostat);
#ifdef DEBUG
	printf("First Stat sound %d   %d %d %d %d\n", result, audiostat.sms_sctnum, audiostat.sms_totsec, audiostat.sms_lpcnt, audiostat.sms_res);
#endif
}

int intHandler(sigCode)
int sigCode;
{
	if (sigCode == SIG_BLANK)
	{
		frameDone = 1;
		frameTick++;
	}
	else if (sigCode == SIG_AUDIO)
	{
#ifdef DEBUG
		printf("X %x\n", Status.asy_stat);
#endif
		finished = 1;
	}
	else
	{
#ifdef DEBUG
		printf("%x\n", sigCode);
		cdic_irq_occured = 1;
#endif
	}
}

#define I_BUTTON1 0x01
#define I_BUTTON2 0x02
#define I_BUTTON3 0x04
#define I_BUTTON_ANY (I_BUTTON1 | I_BUTTON2 | I_BUTTON3)
#define I_LEFT 0x10
#define I_RIGHT 0x20
#define I_UP 0x40
#define I_DOWN 0x80
#define I_SIGNAL1 0x0D00
#define I_SIGNAL2 0x0D01

unsigned long reg_buffer[100][16];
int bufpos;

unsigned short abuf;
unsigned short xbuf;
unsigned short dmactl;
unsigned short audctl;
unsigned short dbuf;

/* Do whatever is known to bring the CDIC into a known state */
void resetcdic()
{
	int temp;

	CDIC_ABUF = 0;
	CDIC_XBUF = 0;
	CDIC_DBUF = 0;
	CDIC_AUDCTL = 0;

	temp = CDIC_ABUF; /* Reset IRQ flag via reading */
	temp = CDIC_XBUF; /* Reset IRQ flag via reading */

	bufpos = 0;
}

/* Plays the map theme of Zelda - Wand of Gamelon */
void test_xa_play()
{
	int i, j;

	resetcdic();

#if 1
	cdic_irq_occured = 0;

	CDIC_AUDCTL = 0;
	CDIC_ACHAN = 0;
	CDIC_CMD = 0x002e;
	CDIC_DBUF = 0xC000;

	printf("State %04x %04x %04x %04x %04x %04x\n", int_abuf, int_xbuf, CDIC_ABUF, CDIC_XBUF, CDIC_DBUF, CDIC_AUDCTL);
	
	while (!cdic_irq_occured)
		;

	printf("State %04x %04x %04x %04x %04x %04x\n", int_abuf, int_xbuf, CDIC_ABUF, CDIC_XBUF, CDIC_DBUF, CDIC_AUDCTL);
	CDIC_DBUF = 0;
	printf("State %04x %04x %04x %04x %04x %04x\n", int_abuf, int_xbuf, CDIC_ABUF, CDIC_XBUF, CDIC_DBUF, CDIC_AUDCTL);

	cdic_irq_occured = 0;
#endif

	CDIC_DBUF = 0;

	/* Zelda - Wand of Gamelon - Map Theme*/
	CDIC_FILE = 0x0100;
	CDIC_CHAN = 0x0001;
	CDIC_ACHAN = 0x0001;
	CDIC_TIME = 0x24362100;
	CDIC_CMD = 0x002a;
	CDIC_DBUF = 0xc000;

	bufpos = 0;
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
			reg_buffer[bufpos][14] = CDIC_DBUF;
			/* reg_buffer[bufpos][12] = CDIC_CHAN; */
			/* reg_buffer[bufpos][13] = CDIC_ACHAN; */
			reg_buffer[bufpos][15] = CDIC_AUDCTL;

			if ((CDIC_AUDCTL & 0x0800) == 0 && (CDIC_DBUF & 0x000f) == 0x0004)
			{
				/* Start playback */
				CDIC_AUDCTL = 0x0800;
			}

			bufpos++;
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

	for (;;)
		;
}

/* Overwrite CDIC driver IRQ handling */
void take_system()
{
	/* TODO I don't understand why this works */
	store_a6();
	*((unsigned long *)0x200) = CDIC_IRQ; /* vector delivered by CDIC */
	CDIC_IVEC = 0x2480;

#if 0
	*((unsigned long *)0xF8) = TIMER_IRQ; /* vector 62 */
	*((unsigned long *)0xF4) = VIDEO_IRQ; /* vector 61 */
	*((unsigned long *)0x68) = SLAVE_IRQ; /* vector 26 */
#endif
}

int main(argc, argv)
int argc;
char *argv[];
{
	int i;
	FILE *file;
	int bytes;
	int wait;
	int waitamount = 100;

	u_short input;

#ifdef DEBUG
	printf("Hello World!\n");
#endif

	intercept(intHandler);
	initVideo();
	initGraphics();
	initInput();

	InitSound();

	PlaySound(0);

	for (wait = 0; wait < 4; wait++)
	{
		dc_ssig(videoPath, SIG_BLANK, 0);

		while (!frameDone)
			; /* Wait for SIG_BLANK */
		frameDone = 0;
	}

	take_system();

	resetcdic();

	for (wait = 0; wait < 100; wait++)
	{
		dc_ssig(videoPath, SIG_BLANK, 0);

		while (!frameDone)
			; /* Wait for SIG_BLANK */
		frameDone = 0;
	}

	/*
	printf("Start audiomap again with 0xff coding\n");
	CDIC_AUDCTL = 0x2800;

	for(;;);
	*/

	test_xa_play();

	exit(0);
}
