#ifndef __TAGS_H
#define __TAGS_H


#define __TAG(a, b, c, d) 		(((a)<<24) | ((b)<<16) | ((c)<<8) | (d))




#define TAG_SDMMC_INFO 				__TAG('S', 'D', 'M', 0)

#define TAG_UART_INFO 				__TAG('U', 'A', 'R', 0)

#define TAG_SPIFLASH_INFO 			__TAG('F', 'L', 'H', 0)












#endif

