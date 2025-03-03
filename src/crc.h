
extern unsigned short s_crc_ccitt_table[256];
#define CRC_CCITT_ROUND(accum, data) (((accum << 8) | data) ^ s_crc_ccitt_table[accum >> 8])
