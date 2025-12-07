
#include <csd.h>
#include <sysio.h>
#include <ucm.h>
#include <events.h>
#include <setsys.h>
#include <signal.h>
#include <memory.h>

#include "input.h"
#include "video.h"
#include "graphics.h"
#include "hwreg.h"

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

int performed_writes = 0;
unsigned long int samples[100];
int sample_index = 0;
int exit_app = 0;

int intHandler(sigCode)
int sigCode;
{
	if (sigCode == SIGINT)
	{
		printf("SIGINT!\n");
		exit_app = 1;
	}
	else if (sigCode == SIG_BLANK)
	{
		printf("Not expected!\n");
		dc_ssig(videoPath, SIG_BLANK, 0);
	}
}

void initProgram()
{
#if 1
	dc_wrli(videoPath, lctA, 0, 0, cp_cbnk(0));
	dc_wrli(videoPath, lctA, 0, 1, cp_clut(5, 0, 255, 0));

	dc_wrli(videoPath, lctA, 2, 0, cp_cbnk(0));
	dc_wrli(videoPath, lctA, 2, 1, cp_clut(5, 255, 0, 0));
	dc_wrli(videoPath, lctA, 2, 2, cp_nop());

	dc_wrli(videoPath, lctA, 279, 0, cp_cbnk(0));
	dc_wrli(videoPath, lctA, 279, 1, cp_clut(5, 255, 0, 255));
	dc_wrli(videoPath, lctA, 279, 2, cp_nop());

	dc_wrli(videoPath, lctA, 279 * 2, 0, cp_cbnk(0));
	dc_wrli(videoPath, lctA, 279 * 2, 1, cp_clut(5, 255, 0, 0));
	dc_wrli(videoPath, lctA, 279 * 2, 2, cp_nop());

#endif
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

int number_a = 42;
int number_b = 100;
int number_c;
int opcnt_da1 = 0;
int opcnt_da0 = 0;

#define MCD212_CDSR1_DA 0x80

void runProgram()
{
	int i;

	char *sysram_buf = (char *)srqcmem(30000, SYSRAM);
	char *video1_buf = (char *)srqcmem(30000, VIDEO1);
	char *video2_buf = (char *)srqcmem(30000, VIDEO2);
	char *sysrom_mem = (char *)0x400000;
	char *dvcrom_mem = (char *)0xe40000;

	dc_ssig(videoPath, SIG_BLANK, 0);

	while (!exit_app)
	{
		opcnt_da0 = 0;
		opcnt_da1 = 0;
		while (MCD212_CSR1R & MCD212_CDSR1_DA)
			;
		while ((MCD212_CSR1R & MCD212_CDSR1_DA) == 0)
		{
			number_c = number_a * number_b;
			opcnt_da0++;
		}
		while (MCD212_CSR1R & MCD212_CDSR1_DA)
		{
			number_c = number_a * number_b;
			opcnt_da1++;
		}
		printf("Multi %d %d\n", opcnt_da0, opcnt_da1);

		opcnt_da0 = 0;
		opcnt_da1 = 0;
		while (MCD212_CSR1R & MCD212_CDSR1_DA)
			;
		while ((MCD212_CSR1R & MCD212_CDSR1_DA) == 0)
		{
			number_c = number_a + number_b;
			opcnt_da0++;
		}
		while (MCD212_CSR1R & MCD212_CDSR1_DA)
		{
			number_c = number_a + number_b;
			opcnt_da1++;
		}
		printf("Add %d %d\n", opcnt_da0, opcnt_da1);

		opcnt_da0 = 0;
		opcnt_da1 = 0;
		while (MCD212_CSR1R & MCD212_CDSR1_DA)
			;
		while ((MCD212_CSR1R & MCD212_CDSR1_DA) == 0)
		{
			number_c = number_a / number_b;
			opcnt_da0++;
		}
		while (MCD212_CSR1R & MCD212_CDSR1_DA)
		{
			number_c = number_a / number_b;
			opcnt_da1++;
		}
		printf("Div %d %d\n", opcnt_da0, opcnt_da1);

		opcnt_da0 = 0;
		opcnt_da1 = 0;
		while (MCD212_CSR1R & MCD212_CDSR1_DA)
			;
		while ((MCD212_CSR1R & MCD212_CDSR1_DA) == 0)
		{
			video1_buf[opcnt_da0] = video2_buf[opcnt_da0];
			opcnt_da0++;
		}
		while (MCD212_CSR1R & MCD212_CDSR1_DA)
		{
			video1_buf[opcnt_da1] = video2_buf[opcnt_da1];
			opcnt_da1++;
		}
		printf("Copy VRAM %d %d\n", opcnt_da0, opcnt_da1);

		opcnt_da0 = 0;
		opcnt_da1 = 0;
		while (MCD212_CSR1R & MCD212_CDSR1_DA)
			;
		while ((MCD212_CSR1R & MCD212_CDSR1_DA) == 0)
		{
			sysram_buf[opcnt_da0] = video2_buf[opcnt_da0];
			opcnt_da0++;
		}
		while (MCD212_CSR1R & MCD212_CDSR1_DA)
		{
			sysram_buf[opcnt_da1] = video2_buf[opcnt_da1];
			opcnt_da1++;
		}
		printf("Copy SYSRAM %d %d\n", opcnt_da0, opcnt_da1);

		opcnt_da0 = 0;
		opcnt_da1 = 0;
		while (MCD212_CSR1R & MCD212_CDSR1_DA)
			;
		while ((MCD212_CSR1R & MCD212_CDSR1_DA) == 0)
		{
			sysram_buf[opcnt_da0] = sysrom_mem[opcnt_da0];
			opcnt_da0++;
		}
		while (MCD212_CSR1R & MCD212_CDSR1_DA)
		{
			sysram_buf[opcnt_da1] = sysrom_mem[opcnt_da1];
			opcnt_da1++;
		}
		printf("Copy SYSROM %d %d\n", opcnt_da0, opcnt_da1);

		opcnt_da0 = 0;
		opcnt_da1 = 0;
		while (MCD212_CSR1R & MCD212_CDSR1_DA)
			;
		while ((MCD212_CSR1R & MCD212_CDSR1_DA) == 0)
		{
			sysram_buf[opcnt_da0] = dvcrom_mem[opcnt_da0];
			opcnt_da0++;
		}
		while (MCD212_CSR1R & MCD212_CDSR1_DA)
		{
			sysram_buf[opcnt_da1] = dvcrom_mem[opcnt_da1];
			opcnt_da1++;
		}
		printf("Copy DVCROM %d %d\n", opcnt_da0, opcnt_da1);
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
	sleep(1);
	exit(0);
}
