
#ifndef __LDO_H
#define __LDO_H

//AVDD18_14922 对应压值为 1.4922
typedef enum __AVDD18 {
    AVDD18_14922 = 0x00,
    AVDD18_15969 = 0x01,
    AVDD18_17016 = 0x02,
    AVDD18_18063 = 0x03,
    AVDD18_19110 = 0x04,
    AVDD18_20157 = 0x05,
    AVDD18_21205 = 0x06,
    AVDD18_22252 = 0x07,
} AVDD18_LEV;


typedef enum __AVDD28 {
    AVDD28_2499 = 0x00,
    AVDD28_2600 = 0x01,
    AVDD28_2702 = 0x02,
    AVDD28_2803 = 0x03,
    AVDD28_2905 = 0x04,
    AVDD28_3006 = 0x05,
    AVDD28_3107 = 0x06,
    AVDD28_3208 = 0x07,
} AVDD28_LEV;


typedef enum __DDRVDD {
    //sddr1  0    1
    DDRVDD_1596_2300 = 0x00,
    DDRVDD_1666_2369 = 0x01,
    DDRVDD_1736_2439 = 0x02,
    DDRVDD_1806_2509 = 0x03,
    DDRVDD_1875_2579 = 0x04,
    DDRVDD_1945_2649 = 0x05,
    DDRVDD_2015_2718 = 0x06,
    DDRVDD_2050_2753 = 0x07,
} DDRVDD_LEV;

typedef enum __DVDD {
    DVDD_0904 = 0x00,
    /* DVDD_0904 = 0x01, */
    DVDD_1004 = 0x02,
    DVDD_1104 = 0x03,
    DVDD_1205 = 0x04,
    DVDD_1305 = 0x05,
    DVDD_1355 = 0x06,
    DVDD_1405 = 0x07,
} DVDD_LEV;

void avdd28_ctrl(AVDD28_LEV lev, u8 avdd28en);
void avdd18_crtl(AVDD18_LEV lev, u8 avdd18en);
void ddrvdd_ctrl(DDRVDD_LEV lev, u8 sddr1, u8 ddrvdden);
void dvdd_ctrl(DVDD_LEV lev);
#endif

