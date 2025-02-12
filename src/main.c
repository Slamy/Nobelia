#define NODEVELOP

#include <csd.h>
#include <sysio.h>
#include <ucm.h>
#include <events.h>

#include <setsys.h>

#include "input.h"
#include "video.h"
#include "graphics.h"

#define LCT_ROW_START 2

#define cl_row(i) (((i >> 2) << 1) + LCT_ROW_START)
#define cl_col(i) ((i & 3) + 1)

#define cl_white(i) cp_clut(i, 255, 255, 255)
#define cl_blue(i) cp_clut(i, 11, 94, 216)
#define cl_red(i) cp_clut(i, 180, 32, 42)
#define cl_dgray(i) cp_clut(i, 38, 43, 68)
#define cl_lgray(i) cp_clut(i, 192, 203, 220)
#define cl_black(i) cp_clut(i, 0, 0, 0)

#define cl_wrli(i, c) dc_wrli(videoPath, lctB, cl_row(i), cl_col(i), c)

#define wr_white(i) cl_wrli(i, cl_white(i))
#define wr_blue(i) cl_wrli(i, cl_blue(i))
#define wr_red(i) cl_wrli(i, cl_red(i))
#define wr_dgray(i) cl_wrli(i, cl_dgray(i))

/* Origin positions of crosses */
#define ORG1 (163 * SCREEN_WIDTH + 89)
#define ORG2 (163 * SCREEN_WIDTH + 279)

#define dpos(x, y) (y * SCREEN_WIDTH + x)

#define SIG_BLANK 0x0100

int frameTick = 0;
int sig_occured = 0;

int performed_multiplies = 0;
unsigned long int samples[100];
int sample_index = 0;

int intHandler(sigCode)
int sigCode;
{
	if (sigCode == SIG_BLANK)
	{
		if (sample_index < 80)
			samples[sample_index++] = performed_multiplies;
		frameTick++;
		sig_occured = 1;
		dc_ssig(videoPath, SIG_BLANK, 0);
	}
}

void initProgram()
{
	dc_wrli(videoPath, lctA, cl_row(30), cl_col(0), cp_sig());
	dc_wrli(videoPath, lctA, cl_row(190), cl_col(0), cp_sig());

	setIcf(ICF_MAX, ICF_MAX);
}

void initSystem()
{
	intercept(intHandler);
	initVideo();
	initGraphics();
	initInput();
	initProgram();
}

void closeSystem()
{
	closeVideo();
	closeInput();
}

int scratch;

void runProgram()
{
	int i;
	dc_ssig(videoPath, SIG_BLANK, 0);

	while (sample_index < 80)
	{
		performed_multiplies++;
		scratch = scratch * 42 + 1;
	}

	for (i = 0; i < sample_index - 1; i++)
	{
		printf("%d\n", samples[i + 1] - samples[i]);
	}
}

int main(argc, argv)
int argc;
char *argv[];
{
	int res;
	printf("Hallo!\r\n");
	initSystem();
	runProgram();
	closeSystem();
	exit(0);
}
