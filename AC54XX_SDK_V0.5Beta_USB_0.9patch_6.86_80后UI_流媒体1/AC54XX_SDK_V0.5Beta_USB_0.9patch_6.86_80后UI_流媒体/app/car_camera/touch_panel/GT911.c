#include "GT911.h"
#include "touch_panel_manager.h"
#include "event.h"
#ifdef CONFIG_TOUCH_PANEL_ENABLE
static struct sw_touch_panel_platform_data *_touch_panel_data = NULL;
/**IIC ADDR**/

#define GT911_IIC_WADDR 0xBA
#define GT911_IIC_RADDR 0xBB

/**REG ADDR**/
#define GT911_REG_CONFIG_DATA         0x8047
#define GT911_REG_VERSION             0x8140
#define GT911_REG_SENSOR_ID           0x814A
#define GT911_REG_CHIP_TYPE           0x8000
#define GT911_READ_COOR_ADDR          0x814E
#define GT911_COMMAND_ADDR            0x8040

/**OTHER DEFINE**/
#define GT911_CONFIG_MIN_LENGTH       186
#define GT911_CONFIG_MAX_LENGTH       240
#define GT911_COMMAND_SLEEP           5

#define GT911_CNT_DOWN        2
#define GT911_CNT_LONG        40
#define GT911_CNT_HOLD        0

#define GT911_MAX_TOUCH_POINT	5

typedef struct {
    u8 pid[5];
    u16 vid;

} TPD_INFO;


typedef struct {
    u8 fresh;
    u16 x;
    u16 y;
    u16 size;
} POINT;


extern void delay(unsigned int i);


TPD_INFO tpd_info;
POINT point[GT911_MAX_TOUCH_POINT];

static int _touch_panel_timer_reg;



void TCH_INT_H()
{
    if (_touch_panel_data) {
        gpio_direction_output(_touch_panel_data->int_pin, 1);
    }
}

void TCH_INT_L()
{
    if (_touch_panel_data) {
        gpio_direction_output(_touch_panel_data->int_pin, 0);
    }
}

void TCH_RST_H()
{
    if (_touch_panel_data) {
        gpio_direction_output(_touch_panel_data->rst_pin, 1);
    }
}

void TCH_RST_L()
{
    if (_touch_panel_data) {
        gpio_direction_output(_touch_panel_data->rst_pin, 0);
    }
}

void TCH_INT_IN()
{
    if (_touch_panel_data) {
        gpio_direction_input(_touch_panel_data->int_pin);
    }
}

extern void delay_2ms(int cnt);


/*
功能： IIC写寄存器操作
输入： addr - 寄存器地址
       buf  - 寄存器值数组指针
       len  - 数组长度
输出： TRUE - 成功
       FALSE- 失败
**/
u8 GT911_iic_write(u16 addr, u8 *buf, u32 len)
{
//   u8 ret;
//   u32 i;
//
//   ret = TRUE;
//   IIC_A_Start();
//
//   if(IIC_A_WriteByte(GT911_IIC_WADDR) != 0) {
//       ret = FALSE;
//       printf("\niic write err 1");
//       IIC_A_Stop();
//       return ret;
//   }
//
//   if(IIC_A_WriteByte((u8)(addr >> 8)) != 0) {
//       ret = FALSE;
//       printf("\niic write err 2");
//       IIC_A_Stop();
//       return ret;
//   }
//
//   if(IIC_A_WriteByte((u8)(addr)) != 0) {
//       ret = FALSE;
//       printf("\niic write err 3");
//       IIC_A_Stop();
//       return ret;
//   }
//
//   for(i = 0; i < len; i++) {
//       if(IIC_A_WriteByte(buf[i]) != 0) {
//           printf("\niic write err at %d", i);
//           ret = FALSE;
//           break;
//       }
//   }
//
//
//   IIC_A_Stop();
//   return ret;

    return _touch_panel_write(GT911_IIC_WADDR, addr, buf, len);
}

/**
功能： IIC读寄存器操作
输入： addr - 寄存器地址
        buf  - 寄存器值数组指针
        len  - 数组长度
输出： TRUE - 成功
       FALSE- 失败
**/
u8 GT911_iic_read(u16 addr, u8 *buf, u32 len)
{
//   u8 ret = FALSE;
//   u8 err = 0;
//   u32 i;
//
//   IIC_A_Start();
//
//   if(IIC_A_WriteByte(GT911_IIC_WADDR) == 0) {
//       err = 1;
//
//       if(IIC_A_WriteByte((u8)(addr >> 8)) == 0) {
//           err = 2;
//
//           if(IIC_A_WriteByte((u8)(addr)) == 0) {
//               err = 3;
//               IIC_A_Stop();
//               IIC_A_Start();
//
//               if(IIC_A_WriteByte(GT911_IIC_RADDR) == 0) {
//                   err = 4;
//
//                   for(i = 0 ; i < len; i++) {
//                       if(i == (len - 1)) {
//                           buf[i] = IIC_A_BreadByte();
//                           IIC_A_nACK();
//                       } else {
//                           buf[i] = IIC_A_BreadByte();
//                           IIC_A_ACK();
//                       }
//
//                   }
//
//                   IIC_A_Stop();
//                   ret = TRUE;
//               }
//           }
//       }
//   }
//
//   IIC_A_Stop();
//
//   if(ret != TRUE) {
//       printf("\niic read err at %d", err);
//   }
//
//   return (ret);
//

    return _touch_panel_read(GT911_IIC_WADDR, GT911_IIC_RADDR, addr, buf, len);
}

/**
功能： GT911中断设置
输入： ms - 复位延时
输出： void
**/
void GT911_int_sync(s32 ms)
{
    TCH_INT_H();
    delay_2ms(3);
    TCH_INT_L();
    delay_2ms(ms);
    TCH_INT_IN();
}

/**
功能： GT911设置IIC总线地址
输入： ms - 复位延时
输出： void
**/
void GT911_reset_guitar(s32 ms)
{
    puts("\nGT911 RESET!");
    TCH_RST_L();
    delay_2ms(ms);


    TCH_INT_L();

    delay_2ms(2);
    TCH_RST_H();
    delay_2ms(6);

    GT911_int_sync(50);

}

/**
功能： IIC读寄存器重复校验操作
输入： addr - 寄存器地址
        rxbuf  - 寄存器值数组指针
        len  - 数组长度
输出： TRUE - 成功
       FALSE- 失败
**/
u8 GT911_iic_read_dbl_check(u16 addr, u8 *rxbuf, u8 len)
{
    u8 buf[16] = {0};
    u8 confirm_buf[16] = {0};
    u8 retry = 0;

    while (retry++ < 3) {
        memset(buf, 0xAA, 16);
        GT911_iic_read(addr, buf, len);

        memset(confirm_buf, 0xAB, 16);
        GT911_iic_read(addr, confirm_buf, len);

        if (!memcmp(buf, confirm_buf, len)) {
            memcpy(rxbuf, confirm_buf, len);
            return TRUE;
        }
    }

    printf("\ni2c read 0x%04X, %d bytes, double check failed!", addr, len);
    return FALSE;
}

/**
功能： GT911配置寄存器
输入： buf  - 寄存器值数组指针
        len  - 数组长度
输出： TRUE - 成功
       FALSE- 失败
**/
u8 GT911_send_cfg(u8 *buf, u8 len)
{
    u8 ret = FALSE;

    u8 retry = 0;

    for (retry = 0; retry < 5; retry++) {
        printf("\nDriver Send Config %d ", retry);
        ret = GT911_iic_write(GT911_REG_CONFIG_DATA, buf, len);

        if (ret == TRUE) {
            break;
        }
    }

    return ret;
}

/**
功能： GT911初始化流程
输入： void
输出： TRUE - 成功
       FALSE- 失败
**/
u8 GT911_init_panel(void)
{
    u8 ret = 0;

    s32 i;
    u8 check_sum = 0;
    u8 opr_buf[16];
    u8 sensor_id = 0;
    u8 drv_cfg_version;
    u8 flash_cfg_version;

    u8 cfg_info_group[] = GT911_CFG_GROUP;
    u8 cfg_info_len = sizeof(cfg_info_group);

    u8 config[GT911_CONFIG_MAX_LENGTH];
    u8 cfg_len = 0;

    printf("\nConfig Lengths: %d", cfg_info_len);

    if (!cfg_info_len) {
        sensor_id = 0;
    } else {
        ret = GT911_iic_read_dbl_check(GT911_REG_SENSOR_ID, &sensor_id, 1);

        if (TRUE == ret) {
            if (sensor_id >= 0x06) {
                printf("\nInvalid sensor_id(0x%02X), No Config Sent!", sensor_id);
                return FALSE;
            }
        } else {
            printf("\nFailed to get sensor_id, No config sent!");
            return FALSE;
        }

        printf("\nSensor_ID: %d", sensor_id);
    }

    cfg_len = cfg_info_len;

    if (cfg_len < GT911_CONFIG_MIN_LENGTH || cfg_len > GT911_CONFIG_MAX_LENGTH) {
        printf("\nCTP_CONFIG_GROUP%d is INVALID CONFIG GROUP! \nNO Config Sent! \nYou need to check you header file CFG_GROUP section!", sensor_id);
        return FALSE;
    }

    {
        ret = GT911_iic_read_dbl_check(GT911_REG_CONFIG_DATA, &opr_buf[0], 1);

        if (ret == TRUE) {
            printf("\nCFG_CONFIG_GROUP%d Config Version: %d, 0x%02X; \nIC Config Version: %d, 0x%02X", sensor_id,
                   cfg_info_group[0], cfg_info_group[0], opr_buf[0], opr_buf[0]);

            flash_cfg_version = opr_buf[0];
            drv_cfg_version = cfg_info_group[0];       // backup  config version

            if (flash_cfg_version < 90 && flash_cfg_version > drv_cfg_version) {
                cfg_info_group[0] = 0x00;
            }
        } else {
            printf("\nFailed to get ic config version!\nNo config sent!");
            return FALSE;
        }
    }
    memset(&config, 0, GT911_CONFIG_MAX_LENGTH);
    memcpy(&config, cfg_info_group, cfg_len);

// GT911_CUSTOM_CFG
    config[1]     = (u8)_touch_panel_data->_MAX_X;
    config[1 + 1] = (u8)(_touch_panel_data->_MAX_X >> 8);
    config[1 + 2] = (u8)_touch_panel_data->_MAX_Y;
    config[1 + 3] = (u8)(_touch_panel_data->_MAX_Y >> 8);

    config[5]     = (u8)(_touch_panel_data->_MAX_POINT);

    config[6] 	 &= 0xfc;
    config[6]    |= _touch_panel_data->_INT_TRIGGER;

    if (_touch_panel_data->_X2Y_EN == 0) { //DISABLE
        config[6] &= 0xf7;
    } else  { //ENABLE
        config[6] |= 0x08;
    }

// GT911_CUSTOM_CFG

    check_sum = 0;

    for (i = 0; i < cfg_len - 2; i++) {
        check_sum += config[i];
    }

    config[cfg_len - 2] = (~check_sum) + 1;

//    if ((abs_x_max == 0) && (abs_y_max == 0))
//    {
//        abs_x_max = (config[1 + 1] << 8) + config[1];
//        abs_y_max = (config[1 + 3] << 8) + config[1 + 2];
//        int_type = (config[6]) & 0x03;
//    }

    {
        ret = GT911_send_cfg(config, cfg_len);

        if (ret != TRUE) {
            printf("\nSend config error.");
        }


        /* for resume to send config */
        if (flash_cfg_version < 90 && flash_cfg_version > drv_cfg_version) {
            config[0] = drv_cfg_version;
            check_sum = 0;

            for (i = 0; i < cfg_len - 2; i++) {
                check_sum += config[i];
            }

            config[cfg_len - 2] = (~check_sum) + 1;
        }

        printf("\nX_MAX = %d, Y_MAX = %d, Points_MAX = %d, TRIGGER = 0x%02x, X2YEn = %x",
               ((config[1 + 1] << 8) + config[1]),
               ((config[1 + 3] << 8) + config[1 + 2]),
               ((config[5])),
               ((config[6]) & 0x03),
               (!!((config[6]) & 0x08))
              );
    }

    delay_2ms(10);
    return TRUE;
}

/**
功能： GT911读数据
输入： void
输出： touch_num 触点个数
**/
u8 GT911_event_handler(void)
{
    u8 touch_info;
    u8 point_data[8 * GT911_MAX_TOUCH_POINT] = {0};
    static u8 fresh_comm = 0;
    u8 tid;
    u8 touch_num = 0;

    int i;
    u8 ret = FALSE;

    ret = GT911_iic_read(GT911_READ_COOR_ADDR, &touch_info, 1);

    if (ret != TRUE) {
        printf("I2C transfer error. errno:%d\n ", ret);
        goto exit_work_func;
    }

    if ((touch_info & 0x80) == 0) {
        goto exit_work_func;
    }

    touch_num = touch_info & 0x0f;

    if (touch_num > _touch_panel_data->_MAX_POINT) {
        goto exit_work_func;
    }

    fresh_comm ++ ;

    if (touch_num > 0) {
        ret = GT911_iic_read(GT911_READ_COOR_ADDR, point_data, 8 * touch_num);
        if (_touch_panel_data->_DEBUGP) {
            putchar('\n');
        }
    }

    if (touch_num) {
        for (i = 0; i < touch_num; i++) {
            tid = point_data[8 * i + 1] & 0x0F;

            if (tid > _touch_panel_data->_MAX_POINT - 1) {
                continue;
            }

            point[tid].fresh ++ ;
            point[tid].x = point_data[8 * i + 2] | point_data[8 * i + 3] << 8;
            point[tid].y = point_data[8 * i + 4] | point_data[8 * i + 5] << 8;
            point[tid].size = point_data[8 * i + 6] | point_data[8 * i + 7] << 8;

            if (_touch_panel_data->_X_MIRRORING) {
                point[tid].x = _touch_panel_data->_MAX_X - point[tid].x;
            }

            if (_touch_panel_data->_Y_MIRRORING) {
                point[tid].y = _touch_panel_data->_MAX_Y - point[tid].y;
            }

            if (_touch_panel_data->_DEBUGP) {
                printf("\n (%d)(%d, %d)[%d]", tid, point[tid].x, point[tid].y, point[tid].size);
            }
        }
    }

    for (i = 0; i < _touch_panel_data->_MAX_POINT; i ++) {
        if (point[i].fresh != fresh_comm) {
            point[i].fresh = fresh_comm ;
            point[i].x = 0x7FFF;
            point[i].y = 0x7FFF;
            point[i].size = 0x7FFF;
        }
    }


exit_work_func:
    ret = GT911_iic_write(GT911_READ_COOR_ADDR, (u8 *)"\0", 1);

    if (ret != TRUE) {
        printf("I2C write end_cmd  error!");
    }

    return touch_num;
}



/**********************************/
/*********以下为接口函数**********/
/**********************************/


/**
功能： GT911读ID
输入： void
输出： TRUE - 成功
       FALSE- 失败
**/
u8 GT911_read_id(void)
{
    u8 ret = FALSE;

    u8 buf[11] = {0};

    GT911_reset_guitar(20);

    ret = GT911_iic_read_dbl_check(GT911_REG_CHIP_TYPE, buf, 10);

    if (ret != TRUE) {
        printf("\nGT911 read id failed");
        return ret;
    }

    buf[10] = 0;
    printf("\nCHIP_TYPE = %s", buf);

    ret = GT911_iic_read(GT911_REG_VERSION, buf, 6);
//    u32 i;
//    putchar('\n');
//    for(i = 0 ; i <  6; i ++)
//    {
//        put_u8hex0(buf[i]);
//    }

    if (ret != TRUE) {
        printf("\nGT911 read id failed");
        return ret;
    }

    memset(tpd_info.pid, 0, 5);
    memcpy(tpd_info.pid, buf, 4);

    if (strncmp((const char *)tpd_info.pid, "911", 3)) {
        printf("\nTch ID %s != \"911\"", tpd_info.pid);
        if(strncmp((const char *)tpd_info.pid, "915", 3) && strncmp((const char *)tpd_info.pid, "968", 3))
        {
            return -1;     
        } 
    }

    tpd_info.vid = (buf[5] << 8) | buf[4];

    printf("\nProduct ID = %s_%d", tpd_info.pid, tpd_info.vid);

    return ret;
}





/**
功能： GT911读坐标及数据处理
输入： void
输出： void
**/
void GT911_read_point(void *arg)
{
    //if (gpio_read(_touch_panel_data->int_pin)) 
    if(1)
    {
        _touch_panel_data->points.point_num = GT911_event_handler();
    } else {
        _touch_panel_data->points.point_num = 0;
    }

    int i;
    for (i = 0; i < _touch_panel_data->_MAX_POINT; i++) {
        _touch_panel_data->points.p[i].e = 0xff;

        if (point[i].x > _touch_panel_data->_MAX_X || point[i].y > _touch_panel_data->_MAX_Y) {
            if (_touch_panel_data->points.p[i].c < GT911_CNT_DOWN) {
                _touch_panel_data->points.p[i].e = 0xff;
            } else {
                _touch_panel_data->points.p[i].e = TOUCH_EVENT_UP;
            }
            _touch_panel_data->points.p[i].c = 0;

        } else {
            _touch_panel_data->points.p[i].c++;
            if (_touch_panel_data->points.p[i].c == GT911_CNT_DOWN) {
                _touch_panel_data->points.p[i].x = point[i].x;
                _touch_panel_data->points.p[i].y = point[i].y;
                _touch_panel_data->points.p[i].e = TOUCH_EVENT_DOWN;
            } else if (_touch_panel_data->points.p[i].c > GT911_CNT_DOWN) {
                if (point[i].x != _touch_panel_data->points.p[i].x ||
                    point[i].y != _touch_panel_data->points.p[i].y) {
                    if (_touch_panel_data->points.p[i].c > GT911_CNT_DOWN + GT911_CNT_DOWN) {
                        _touch_panel_data->points.p[i].x = point[i].x;
                        _touch_panel_data->points.p[i].y = point[i].y;
                        _touch_panel_data->points.p[i].e = TOUCH_EVENT_MOVE;
                        _touch_panel_data->points.p[i].c = GT911_CNT_DOWN;
                    }
                }

                if (_touch_panel_data->points.p[i].c == GT911_CNT_LONG + GT911_CNT_HOLD) {
                    _touch_panel_data->points.p[i].x = point[i].x;
                    _touch_panel_data->points.p[i].y = point[i].y;
                    _touch_panel_data->points.p[i].e = TOUCH_EVENT_HOLD;
                }

                if (_touch_panel_data->points.p[i].c > GT911_CNT_LONG + GT911_CNT_HOLD) {
                    _touch_panel_data->points.p[i].c = GT911_CNT_LONG;
                }
            }
        }

        if (_touch_panel_data->points.p[i].e != 0xff) {
            struct sys_event eve;
            eve.type = SYS_TOUCH_EVENT;
            eve.u.touch.event = _touch_panel_data->points.p[i].e;
            eve.u.touch.pos.x = _touch_panel_data->points.p[i].x;
            eve.u.touch.pos.y = _touch_panel_data->points.p[i].y;
            sys_event_notify(&eve);

            if (_touch_panel_data->_DEBUGE) {
                printf("\n [%d](%d) (%d,%d)", i, _touch_panel_data->points.p[i].e, _touch_panel_data->points.p[i].x, _touch_panel_data->points.p[i].y);
            }
        }
    }
}


/**
功能： GT911进入睡眠模式
输入： void
输出： TRUE - 成功
       FALSE- 失败
**/
u8 GT911_sleep(void)
{
    u8 command = GT911_COMMAND_SLEEP;
    u8 ret = FALSE;
    ret = GT911_iic_write(GT911_COMMAND_ADDR, &command, 1);
    delay_2ms(50);
    return ret;
}


/**
功能： GT911唤醒
输入： void
输出： void
**/
void GT911_wakeup(void)
{
    GT911_reset_guitar(50);
}










/**
功能： GT911检查ID
输入： void
输出： TRUE - 成功
       FALSE- 失败
**/
char GT911_check(struct sw_touch_panel_platform_data *data)
{
    _touch_panel_data = data;
    return GT911_read_id();
}

/**
功能： GT911初始化
输入： void
输出： TRUE - 成功
       FALSE- 失败
**/
char GT911_init(void)
{
    u8 ret = FALSE;
    int reset_count = 0;

g_reset_proc:
    TCH_RST_L();
    TCH_INT_L();
    delay_2ms(10);

    GT911_reset_guitar(20);

    ret = GT911_read_id();

    if (ret != TRUE) {
        goto g_end;
    }
    #if 0
    ret = GT911_init_panel();

    if (ret != TRUE) {
        goto g_end;
    }
    #endif
    int i;

    for (i = 0; i < GT911_MAX_TOUCH_POINT; i++) {
        point[i].fresh = 0;
        point[i].x = 0x7FFF;
        point[i].y = 0x7FFF;
        point[i].size = 0x7FFF;
    }


g_end:

    if (ret != TRUE) {
        if (++reset_count < 3) {
            goto g_reset_proc;
        }
    }

    if (ret == TRUE) {
        if (_touch_panel_data->enable) {
            _touch_panel_timer_reg = sys_timer_add((void *)0, GT911_read_point, 20);
        }
    }

    return ret;
}

/**
功能： GT911控制
输入： void
输出： TRUE - 成功
       FALSE- 失败
**/
char GT911_ioctl(u32 cmd, u32 arg)
{
    switch (cmd) {
    case TOUCH_PANEL_CMD_SLEEP:
        GT911_sleep();
        break;
    case TOUCH_PANEL_CMD_WAKEUP:
        GT911_wakeup();
        break;
    case TOUCH_PANEL_CMD_RESET:
        GT911_init();
        break;
    case TOUCH_PANEL_CMD_DISABLE:
        if (_touch_panel_data->enable) {
            _touch_panel_data->enable = 0;
            sys_timer_del(_touch_panel_timer_reg);
        }
        break;
    case TOUCH_PANEL_CMD_ENABLE:
        if (!_touch_panel_data->enable) {
            _touch_panel_data->enable = 1;
            _touch_panel_timer_reg = sys_timer_add((void *)0, GT911_read_point, 20);
        }
        break;
    default:
        return 0;
    }
    return 1;
}
#if 1

_TOUCH_PANEL_INTERFACE gt911_ops = {
    .logo 				 = 	"gt911",
    .touch_panel_check   =   GT911_check,
    .touch_panel_init    =   GT911_init,
    .touch_panel_ioctl   =   GT911_ioctl,
};

REGISTER_TOUCH_PANEL(gt911)

.touch_panel_ops = &gt911_ops,

};
#endif
#endif
