/*******************************************************************************************
 File Name: cache.h

 Version: 1.00

 Discription:


 Author:yulin deng

 Email :flowingfeeze@163.com

 Date:2014-01-14 09:55:04

 Copyright:(c)JIELI  2011  @ , All Rights Reserved.
*******************************************************************************************/
#ifndef ASM_CACHE_H
#define ASM_CACHE_H

#include "typedef.h"

#define CACHE_LINE_COUNT   32

void flush_dcache(void *ptr, int len);

void flushinv_dcache(void *ptr, int len);

#endif
