#ifndef __CPU_CRC16_H__
#define __CPU_CRC16_H__


#include "typedef.h"



u16 CRC16(const void *ptr, u32 len);

void spi_crc16_set(u16 crc);
u16 spi_crc16_get(void);

void CrcDecode(void *buf, u16 len);

u16 get_page_efuse(u32 page, u32 delay_cnt);









#endif


