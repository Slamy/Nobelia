
extern char cdic_irq_occured;
extern unsigned short int_abuf;
extern unsigned short int_xbuf;

/* Used to store register information during a test */
/* We don't want to make any prints during the test as the baud rate is too slow */
extern unsigned long reg_buffer[100][40];
extern int bufpos;

extern unsigned short abuf;
extern unsigned short xbuf;
extern unsigned short dmactl;
extern unsigned short audctl;
extern unsigned short dbuf;


void print(char *format, ...);
void resetcdic();
void print_state();
