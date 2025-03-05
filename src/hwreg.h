
/* --- IMS66490 CD-Interface Controller (CDIC) ---  */

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
