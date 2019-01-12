#ifndef _CPU_CLOCK_
#define _CPU_CLOCK_

#include "typedef.h"


int clk_early_init();

int clk_get(const char *name);

int clk_set(const char *name, int clk);


#endif

