#include "asm/includes.h"
#include "asm/sysinfo.h"
#include "asm/ldo.h"

//如需使用avdd18 avdd28 drcvdd 作为ldo请和硬件同事沟通

void avdd18_crtl(AVDD18_LEV lev, u8 avdd18en)
{
    avdd18en = (avdd18en ? 1 : 0);
    if (lev > 7) {
        lev = 7;
    }
    SFR(LDO_CON, 9, 1, avdd18en);
    SFR(LDO_CON, 3, 3, lev);
}

void avdd28_ctrl(AVDD28_LEV lev, u8 avdd28en)
{
    avdd28en = (avdd28en ? 1 : 0);
    if (lev > 7) {
        lev = 7;
    }
    SFR(LDO_CON, 10, 1, avdd28en);
    SFR(LDO_CON, 6, 3, lev);
}

void ddrvdd_ctrl(DDRVDD_LEV lev, u8 sddr1, u8 ddrvdden)
{
    ddrvdden = (ddrvdden ? 1 : 0);
    sddr1 = (sddr1 ? 1 : 0);
    if (lev > 7) {
        lev = 7;
    }
    SFR(LDO_CON, 15, 1, ddrvdden);
    SFR(LDO_CON, 19, 1, sddr1);
    SFR(LDO_CON, 12, 3, lev);
}

void dvdd_ctrl(DVDD_LEV lev)
{
    if (lev > 7) {
        lev = 7;
    }
    SFR(LDO_CON, 0, 3, lev);
}

