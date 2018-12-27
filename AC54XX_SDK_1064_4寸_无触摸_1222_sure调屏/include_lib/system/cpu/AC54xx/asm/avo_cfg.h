#ifndef __CVE_H__
#define __CVE_H__

#include "generic/typedef.h"

//<AVOUT制式>
#define CVE_NTSC_720      	0x01
#define CVE_PAL_720       	0x02
#define CVE_NTSC_896        0x10
#define CVE_PAL_896         0x20
#define CVE_COLOR_BAR       0x40
#define CVE_OSC_12M			0x80

void cve_pll_init(u8 mode);
void cve_dac_init();
void cve_cfg(u8 mode, u8 v_act_start);

#endif
