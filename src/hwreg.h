
/* --- IMS66490 CD-Interface Controller (CDIC) ---  */

/* Known buffers in RAM, filled by the CDIC */

#define CDIC_RAM_DBUF0 (((unsigned short *)0x300000))
#define CDIC_RAM_DBUF1 (((unsigned short *)0x300A00))
#define CDIC_RAM_UNKNOWN0 (((unsigned short *)0x301400))
#define CDIC_RAM_UNKNOWN1 (((unsigned short *)0x301E00))
#define CDIC_RAM_ADPCM0 (((unsigned short *)0x302800))
#define CDIC_RAM_ADPCM1 (((unsigned short *)0x303200))


/* Assumed to be data in RAM, read by microcode */

#define CDIC_CMD (*((unsigned short *)0x303C00))
#define CDIC_TIME (*((unsigned long *)0x303C02))
#define CDIC_FILE (*((unsigned short *)0x303C06))
#define CDIC_CHAN (*((unsigned long *)0x303C08))
#define CDIC_ACHAN (*((unsigned short *)0x303C0C))

/* What is DSEL at 3C80 ? Is it ever used */

/* Assumed to be real registers */

#define CDIC_ABUF (*((unsigned short *)0x303FF4))
#define CDIC_XBUF (*((unsigned short *)0x303FF6))
#define CDIC_DMACTL (*((unsigned short *)0x303FF8))
#define CDIC_AUDCTL (*((unsigned short *)0x303FFA)) /* Called Z buffer in MAME */
#define CDIC_IVEC (*((unsigned short *)0x303FFC))
#define CDIC_DBUF (*((unsigned short *)0x303FFE))

/* --- Slave Controller --- */

#define SLAVE_CH0 (*((unsigned char *)0x310001))
#define SLAVE_CH1 (*((unsigned char *)0x310003))
#define SLAVE_CH2 (*((unsigned char *)0x310005))

/* --- SCC68070 DMA --- */

#define DMA_CH0_STATUS (*((unsigned byte *)0x80004000))
#define DMA_CH0_ERROR (*((unsigned byte *)0x80004001))
#define DMA_CH0_DEV_CTRL (*((unsigned byte *)0x80004004))
#define DMA_CH0_OP_CTRL (*((unsigned byte *)0x80004005))
#define DMA_CH0_SEQ_CTRL (*((unsigned byte *)0x80004006))
#define DMA_CH0_CHAN_CTRL (*((unsigned byte *)0x80004007))
#define DMA_CH0_MEM_TRANS_CNT (*((unsigned short *)0x8000400a))
#define DMA_CH0_MEM_ADR (*((unsigned long *)0x8000400c))
#define DMA_CH0_DEV_ADR (*((unsigned long *)0x80004014))


