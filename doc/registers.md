# CDIC Registers

Due to the lack of documentation, these might not be the correct register names.
The naming is based on [this reverse engineering effort](https://github.com/cdifan/cdichips/blob/master/ims66490cdic.md)

Registers must always be written to and read from using word access.

## Command (CMD) @ 0x3C00

| Value | Description    |
| ----- | -------------- |
| 0x23  | ?              |
| 0x24  | ?              |
| 0x27  | ?              |
| 0x28  | ?              |
| 0x29  | ?              |
| 0x2a  | Read CD-I Data |
| 0x2b  | ?              |
| 0x2e  | ?              |
| 0x2C  | ?              |

The value of this register is only parsed when Bit 15 of DBUF is set

## Time (TIME) @ 0x3C02 - 0x3C04

This register is a long word

| Field   | Description  |
| ------- | ------------ |
| 31 - 24 | BCD Minute   |
| 23 - 16 | BCD Second   |
| 15 -  8 | BCD Fraction |
| 7 - 0   | ?            |

## File (FILE) @ 0x3C06


## Channel mask (CHAN) @ 0x3C08 & 0x3C0A


## Audio Channel Mask (ACHAN) @ 0x3C0C


## Data Select (DSEL) @ 0x3C80

TODO Unused?

## Audio Buffer (ABUF) @ 0x3FF4

| Field  | Description                                                                                      |
| ------ | ------------------------------------------------------------------------------------------------ |
| 31     | Set by CDIC when playback of single ADPCM buffer has finished.<br/>Reset on read<br/>Causes IRQ. |
| 30 - 0 | Always reads as all bits set ?                                                                   |

## Extra Buffer (XBUF) @ 0x3FF6

| Field  | Description                                       |
| ------ | ------------------------------------------------- |
| 31     | Sector  buffer playback completed. Reset on read. |
| 30 - 0 | Always reads as all bits set ?                    |

## DMA Control (DMACTL) @ 0x3FF8


## Audio Control (AUDCTL) @ 0x3FFA

| Field   | Description                                       |
| ------- | ------------------------------------------------- |
| 15 - 14 | Always reads as all bits set ?                    |
| 13      | Sector  buffer playback completed. Reset on read. |
| 12      | Always reads as all bits set ?                    |
| 11      | Sector  buffer playback completed. Reset on read. |
| 10 -  1 | Always reads as all bits set ?                    |
| 0       | TODO                                              |

## Interrupt Vector (IVEC) @ 0x3FFC

## Data Buffer (DBUF) @ 0x3FFE
