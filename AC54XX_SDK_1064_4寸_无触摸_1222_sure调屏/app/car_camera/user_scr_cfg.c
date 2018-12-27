#include "device/device.h"
#include "asm/imd.h"
#include "user_isp_cfg.h"
#include "app_config.h"
#include "gpio.h"
#include "database.h"

#ifdef CONFIG_UI_ENABLE

const char *screen_cfg_file[] = {
    "scr_night.bin",
    "scr_indoor.bin",
    "scr_outdoor.bin",
};
static struct color_correct effects[3];

int imd_dmm_effects_cfg_night(char scene)
{
    static u8 init = false;
    static int err = 0;
    static void *imd = NULL;

    if (!init) {
        imd = dev_open("imd", NULL);
        if (imd) {
            struct color_effect_cfg cfg;
            cfg.fname = screen_cfg_file[0];
            cfg.adj = &effects[0];
            err = dev_ioctl(imd, IMD_GET_COLOR_CFG, (u32)&cfg);
        } else {
            err = -ENODEV;
        }
        init = true;
    }

    if (!err && imd) {
        log_i("screen night cfg\n");
        dev_ioctl(imd, IMD_SET_COLOR_VALID, (u32)&effects[0]);
    }
	 u32 bu2 = db_select("bu");
   	  if( bu2 == 2){
	  	   
                  gpio_direction_output(IO_PORTG_00, 1);
    	}
    return err;
}
REGISTER_ISP_SCR_WORK(imd_dmm_effects_cfg_night, 0);

int imd_dmm_effects_cfg_indoor(char scene)
{
    static u8 init = false;
    static int err = 0;
    static void *imd = NULL;

    if (!init) {
        imd = dev_open("imd", NULL);
        if (imd) {
            struct color_effect_cfg cfg;
            cfg.fname = screen_cfg_file[1];
            cfg.adj = &effects[1];
            err = dev_ioctl(imd, IMD_GET_COLOR_CFG, (u32)&cfg);
        } else {
            err = -ENODEV;
        }
        init = true;
    }

    if (!err && imd) {
        log_i("screen indoor cfg\n");
        dev_ioctl(imd, IMD_SET_COLOR_VALID, (u32)&effects[1]);
    }
    	 u32 bu2 = db_select("bu");
   	  if( bu2 == 2){
	  	   
                  gpio_direction_output(IO_PORTG_00, 1);
    	}
    return err;
}
REGISTER_ISP_SCR_WORK(imd_dmm_effects_cfg_indoor, 1);

int imd_dmm_effects_cfg_outdoor(char scene)
{
    static u8 init = false;
    static int err = 0;
    static void *imd = NULL;

    if (!init) {
        imd = dev_open("imd", NULL);
        if (imd) {
            struct color_effect_cfg cfg;
            cfg.fname = screen_cfg_file[2];
            cfg.adj = &effects[2];
            err = dev_ioctl(imd, IMD_GET_COLOR_CFG, (u32)&cfg);
        } else {
            err = -ENODEV;
        }
        init = true;
    }

    if (!err && imd) {
        log_i("screen outdoor cfg\n");
        dev_ioctl(imd, IMD_SET_COLOR_VALID, (u32)&effects[2]);
    }
    	 u32 bu2 = db_select("bu");
   	  if( bu2 == 2){
	  	   
                  gpio_direction_output(IO_PORTG_00, 0);
    	}
    return err;
}
REGISTER_ISP_SCR_WORK(imd_dmm_effects_cfg_outdoor, 2);
#endif
