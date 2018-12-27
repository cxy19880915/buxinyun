#include "ui/includes.h"
#include "system/includes.h"
#include "server/ui_server.h"
#include "style.h"
#include "action.h"
#include "app_config.h"
#include "app_database.h"
#include "server/audio_server.h"
#include "server/video_dec_server.h"
#include "asm/lcd_config.h"
//#include "asm/lcd_config.h"
#include "server/video_server.h"
//#include "video_dec.h"
#include "asm/clock.h"
#include "os/os_api.h"
#include "asm/uart.h"
#include "generic/gpio.h"
#include "spinlock.h"
#include "init.h" 
#include "action.h"


 int GetComma(int num, char *str);
//void uarrx_irq_handler();

struct uartx_debug_reg {
    volatile u32 *ut_con;
    volatile u8  *ut_buf;
    volatile u32 *ut_txdma_addr;
    volatile u32 *ut_txdma_cnt;
};

static struct uartx_debug_reg debug = {NULL, NULL};
static DEFINE_SPINLOCK(lock);
static u8 uart_dma_buf[1024 * 2];
static u8 tx_busy = 1;
static u8 dma_enable = 0;
static u16 data_len = 0;
static u16 r_pos = 0;
static u16 w_pos = 0;
static u16 timer = 0,nn,jj,bb;
static u32  shu;
 u8 receive_gps_flag = 0;
#define SOILDOG_MSG_BUF_SIZE  80

 volatile u8 soildog_msg_buf[SOILDOG_MSG_BUF_SIZE] = {0};
volatile u8 buf1[SOILDOG_MSG_BUF_SIZE] = {0};
  volatile u8 buf2[SOILDOG_MSG_BUF_SIZE] = {0};
volatile u8 buf3[SOILDOG_MSG_BUF_SIZE] = {0};
volatile u8 buf4[SOILDOG_MSG_BUF_SIZE] = {0};
volatile u8 buf5[SOILDOG_MSG_BUF_SIZE] = {0};
volatile u8 buf6[SOILDOG_MSG_BUF_SIZE] = {0};


int soildog_msg_count = 0;

#ifdef CONFIG_UI_STYLE_LY_ENABLE
 char status1 = 0;
 extern u8 ch, tmp; 
extern int sys_cur_mod;  /* 1:rec, 2:tph, 3:dec, 4:audio, 5:music */
extern int menu_sys_lcd_pro_set(u8 sel_item);
extern int menu_sys_auto_off_set(u8 sel_item);
extern int menu_sys_led_fre_set(u8 sel_item);
extern int menu_sys_key_voice_set(u8 sel_item);
extern int menu_sys_language_set(u8 sel_item);
extern int menu_sys_date_set(u8 sel_item);
extern int menu_sys_tv_mod_set(u8 sel_item);
extern int menu_sys_format_set(u8 sel_item);
extern int menu_sys_default_set(u8 sel_item);
 extern int video_rec_control(void *_run_cmd);

static u8 __car_num_set_by_ascii(const char *str);

#define STYLE_NAME  LY



struct video_menu_info {
    volatile char if_in_rec;    /* 是否正在录像 */
    u8 lock_file_flag;          /* 是否当前文件被锁 */

    u8 menu_status;             /*0 menu off, 1 menu on*/
    u8 battery_val;
    u8 battery_char;
    u8 hlight_show_status;  /* 前照灯显示状态 */

    int vanish_line;
    int car_head_line;
};

static struct video_menu_info handler = {0};
#define __this 	(&handler)
#define sizeof_this     (sizeof(struct video_menu_info))

static struct server *audio = NULL;
static FILE *file;
volatile u8 main_lock_flag;
/************************************************************
				    	录像模式设置
************************************************************/
/*
 * rec分辨率设置
 */
static const u8 table_video_resolution[] = {
    VIDEO_RES_1080P,
    VIDEO_RES_720P,
    VIDEO_RES_VGA,
};

static const u8 table_video_bu[] = {
    guan,
    kai,
    zidong,
};
/*
 * rec循环录像设置
 */
static const u8 table_video_cycle[] = {
    0,
    3,
    5,
    10,
};


/*
 * rec曝光补偿设置
 */
static const u8 table_video_exposure[] = {
    3,
    2,
    1,
    0,
    (u8) - 1,
    (u8) - 2,
    (u8) - 3,
};


/*
 * rec重力感应设置
 */
static const u8 table_video_gravity[] = {
    GRA_SEN_OFF,
    GRA_SEN_LO,
    GRA_SEN_MD,
    GRA_SEN_HI,
};



/*
 * rec间隔录影设置, ms
 */
static const u16 table_video_gap[] = {
    0,
    100,
    200,
    500,
};

static const u16 province_gb2312[] = {
    0xA9BE, 0xFEC4, 0xA8B4, 0xA6BB, 0xF2BD, //京，宁，川，沪，津
    0xE3D5, 0xE5D3, 0xE6CF, 0xC1D4, 0xA5D4, //浙，渝，湘，粤，豫
    0xF3B9, 0xD3B8, 0xC9C1, 0xB3C2, 0xDABA, //贵，赣，辽，鲁，黑
    0xC2D0, 0xD5CB, 0xD8B2, 0xF6C3, 0xFABD, //新，苏，藏，闽，晋
    0xEDC7, 0xBDBC, 0xAABC, 0xF0B9, 0xCAB8, //琼，冀，吉，桂，甘，
    0xEECD, 0xC9C3, 0xF5B6, 0xC2C9, 0xE0C7, //皖，蒙，鄂，陕，青，
    0xC6D4                                  //云
};

static const u8 num_table[] = {
    'A', 'B', 'C', 'D', 'E',
    'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y',
    'Z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8',
    '9'
};


struct car_num {
    const char *mark;
    u32 text_id;
    u32 text_index;
};

struct car_num_str {
    u8 province;
    u8 town;
    u8 a;
    u8 b;
    u8 c;
    u8 d;
    u8 e;
};

struct car_num text_car_num_table[] = {
    {"province", TEXT_MN_CN_PROVINCE, 0}, /* 京 */
    {"town",     TEXT_MN_CN_TOWN,     0}, /* A */
    {"a",        TEXT_MN_CN_A,        0}, /* 1 */
    {"b",        TEXT_MN_CN_B,        0}, /* 2 */
    {"c",        TEXT_MN_CN_C,        0}, /* 3 */
    {"d",        TEXT_MN_CN_D,        0}, /* 4 */
    {"e",        TEXT_MN_CN_E,        0}, /* 5 */
};

enum sw_dir {
    /*
     * 切换方向
     */
    DIR_NEXT = 1,
    DIR_PREV,
    DIR_SET,
};
enum set_mod {
    /*
     * 加减方向
     */
    MOD_ADD = 1,
    MOD_DEC,
    MOD_SET,
};
static u16 car_num_set_p = 0xff;
/*
 * text_car_num_table的当前设置指针
 */

/*
 * (begin)提示框显示接口
 */
enum box_msg {
    BOX_MSG_NO_POWER = 1,
    BOX_MSG_MEM_ERR,
    BOX_MSG_NO_MEM,
    BOX_MSG_NEED_FORMAT,
    BOX_MSG_INSERT_SD,
    BOX_MSG_DEFAULT_SET,
    BOX_MSG_FORMATTING,
};
static u8 msg_show_f = 0;
static enum box_msg msg_show_id = 0;

 int GetComma(int num, char *buf4)
{
     int i;
      int j = 0;
      //   printf("1timer buf4 %s\n",buf4);
     int len = strlen(buf4);
     for(i = 0; i < len; i++)
     {
          if(buf4[i] == ',')
            j++;
         if(j == num)
           return i + 1;
     }
     return 0;
}
 u8 ch, tmp,LEN = 0,status2;  
 ___interrupt
void uartx_irq_handler()
{
    irq_handler_enter(UART2_INT);

    *debug.ut_con |= BIT(13);

    spin_lock(&lock);
    tx_busy = 0;
    spin_unlock(&lock);
  //  uart_dma_tx();

    irq_handler_exit(UART2_INT);
}

int communication_putByte(char a)
{

    UT2_BUF = a;
    while((UT2_CON & BIT(15)) == 0);

    UT2_CON |= BIT(13);
    return a;
}


___interrupt
void uarrx_irq_handler()
{

     int a;
      int b;
        // printf("1timer buf4 %s\n",buf4);
     int len ;



     // char ji[100] = {0};
      char RX_BUF = 0,MK = 0;
      u8 i=0,tmp1,r1,r2,r3,r4,r5,r6,r7,r8,r9,ch,nb ;
       GPS_INFO *gps;
	 
    if(UT2_CON & BIT(15)) //tpend
    {
        //发送数据完成
        UT2_CON |= BIT(13);
		
    }
    else if(UT2_CON & BIT(14)) //rpend
    { 
        MK = UT2_BUF;
		//printf("MK %c ===\n",UT2_BUF);
            // printf(" soildog_msg_buf   %s\n",soildog_msg_buf);

                 if(soildog_msg_buf[3] == 0x52)
             {
                  
                 memset(buf4, 0, soildog_msg_count + 1);
		memcpy(buf4,soildog_msg_buf,soildog_msg_count);

              }
		if(soildog_msg_buf[4] == 0x47)
             {
                  
                 memset(buf4, 0, soildog_msg_count + 1);
		memcpy(buf4,soildog_msg_buf,soildog_msg_count);
	//	 printf("===buf4==== %s\n",buf4);

              }
	 
                 RX_BUF = UT2_BUF;//get one byte
                 if(RX_BUF == 0x0A && soildog_msg_buf[4] == 0x47)
                 {
                        r7= GetComma(7, buf4);
			  r8= GetComma(8, buf4);
                         strncpy( buf6, buf4+r7,r7-r6-1);
		//	  printf("buf6 %s\n", buf6);
			   soildog_msg_count = 0;
		   }    
               	if(RX_BUF == 0x0A && soildog_msg_buf[3] == 0x52)
		{
		    //    u8  buf4[80] = "$GPRMC,001731.9,A,2238.647968,N,11400.087306,E,40.0,237.8,270213,,,A*5B";
		      //    printf("===buf4==== %s\n",buf4);
		                        tmp1 = GetComma(9, buf4);
                                      r1 =  GetComma(3, buf4);
					r2 =  GetComma(4, buf4);	
					r3  =   GetComma(5, buf4);	
					 r4  = GetComma(6, buf4);	
					r5  =   GetComma(7, buf4);	
					 r6  = GetComma(8, buf4);	
                                         strncpy( buf2, buf4+r1,r2-r1-1);
                                         strncpy( buf1, buf4+r3,r4-r3-1);
					    strncpy( buf3, buf4+r5,r6-r5-1);
				    shu =((buf3[0] - '0') * 1000+(buf3[1] - '0') * 100+(buf3[2] - '0') * 10+(buf3[ 4] - '0') * 1)*185;
                                  nn=(buf4[tmp1 + 4] - '0') * 10 + (buf4[tmp1 + 5] - '0') + 2000;
                                   jj =(buf4[tmp1 + 2] - '0') * 10 + (buf4[tmp1 + 3] - '0');
                                  bb = (buf4[tmp1 + 0] - '0') * 10 + (buf4[tmp1 + 1] - '0'); //日期
                                //  printf("buf1 %s  buf2 %s shu %d\n", buf1,buf2,shu);
                        //      printf("buf3[0]  %c buf3[1] %c  buf3[2] %c buf3[4] %c  ",buf3[0],buf3[1],buf3[2],buf3[4]);
                               //  printf(" r5 %d bu3 0 %d bu3 1 %d buf3 3%d\n", r5,(buf3[r5 + 0] - '0'),(buf3[r5 + 1] - '0'),(buf3[r5 + 3] - '0'));
		 soildog_msg_count = 0;
		}
		if(RX_BUF == 0x24   || soildog_msg_count >= SOILDOG_MSG_BUF_SIZE )
		{
		  soildog_msg_count = 0;
		}
              soildog_msg_buf[soildog_msg_count++] = UT2_BUF;

              if(soildog_msg_buf[4] == 0x4C)
              {
                 memset(buf5, 0, soildog_msg_count + 1);
	         memcpy(buf5,soildog_msg_buf,soildog_msg_count);      
		//printf("buf5 %s\n",buf5);	 
        	}
	

        //接受数据完成
        UT2_CON |=BIT(12) ;

    }
    else if(UT2_CON & BIT(11))  //otpend
    {
        //接受数据超时
        UT2_CON |= BIT(10);
        UT2_CON |= BIT(7) ; //清UT0_HRXCNT
    }


    irq_handler_exit(UART2_INT);
}

int muart_init(const struct uart_platform_data *data)
{
    int uart_clk = clk_get("uart");

   // gpio_direction_output(data->tx_pin, 0);
   gpio_direction_input(data->rx_pin);


    switch (data->irq) {

    case UART2_INT:
     //   IOMC3 |= BIT(11) ;//占用相应的IO
      IOMC3 |= BIT(10)|BIT(9)|BIT(8);//uart2输入选择
      IOMC3 &= ~BIT(11);//不占用IO
      IOMC2 |=(BIT(30)|BIT(24));//PG1
#if 0
        if (data->tx_pin == IO_PORTC_04) {

        } else if (data->tx_pin == IO_PORTH_10) {
            IOMC3 |= BIT(12);
	    IOMC3 &= ~(BIT(13)) ;
            gpio_direction_output(IO_PORTH_10,0);
            gpio_direction_input(IO_PORTH_09);
        } else if (data->tx_pin == IO_PORTD_00) {
            IOMC3 |= BIT(13);

        } else if (data->tx_pin == IO_PORTH_00) {
            IOMC3 |= (BIT(12) | BIT(13));

        }
#endif
       UT2_BAUD = (uart_clk / data->baudrate) / 4 - 1;
        UT2_CON =  BIT(0);

        UT2_CON &= ~BIT(1);          //8 bits
        UT2_CON &= ~BIT(6);          //普通模式，不用DMA模式
        UT2_CON |= BIT(13)|BIT(12)|BIT(10); //清TXPND,RXPND,OTPND
        UT2_CON |= BIT(3);//串口接收中断允许
        UT2_CON |= BIT(0);//串口模块使能
        break;
    
    default:
        return -EINVAL;
    }



    request_irq(data->irq, 2, uarrx_irq_handler, 0);

    return 0;
}



void mputbyte(char a)
{
    if (debug.ut_con == NULL) {
        return;
    }

    spin_lock(&lock);

    if (dma_enable) {

        if (a == '\n') {
            uart_dma_buf[w_pos++] = '\r';
            if (w_pos == sizeof(uart_dma_buf)) {
                w_pos = 0;
            }
            data_len++;
            /*cbuf_write(&uart_cbuf, &b, 1);*/
        }
        uart_dma_buf[w_pos++] = a;
        if (w_pos == sizeof(uart_dma_buf)) {
            w_pos = 0;
        }
        data_len++;

        spin_unlock(&lock);


    } else {
        spin_unlock(&lock);

        if (a == '\n') {
            if (tx_busy) {
                while ((*debug.ut_con & BIT(15)) == 0);
            }
            *debug.ut_buf = '\r';
            __asm__ volatile("csync");
            __asm__ volatile("csync");
            __asm__ volatile("csync");
            __asm__ volatile("csync");
            __asm__ volatile("csync");
            tx_busy = 1;
        }
        if (tx_busy) {
            while ((*debug.ut_con & BIT(15)) == 0);
        }
        *debug.ut_buf = a;
        __asm__ volatile("csync");
        __asm__ volatile("csync");
        __asm__ volatile("csync");
        __asm__ volatile("csync");
        __asm__ volatile("csync");
        tx_busy = 1;

    }
}

int mgetbyte(char *c)
{
    if (debug.ut_con == NULL) {
        return 0;
    }
    if ((*debug.ut_con & BIT(14))) {
        *c = *debug.ut_buf;
        *debug.ut_con |= BIT(12);
        return 1;
    }

    return 0;
}


static void uart_dma_mode()
{
    if (debug.ut_txdma_addr) {
        dma_enable = 1;
        tx_busy = 0;
        *debug.ut_con |= BIT(13);
        *debug.ut_con |= BIT(2);
    }
}
late_initcall(uart_dma_mode);

//struct  video_dec1_hdl {

 //   struct server *ui;
//};

//struct video_dec1_hdl rec_handler1;

//#define __this 	(&rec_handler1)
//#define sizeof_this     (sizeof(struct video_dec1_hdl))

static int timer_sys_tph_onchange1(void *ctr, enum element_change_event e, void *arg)
{

    struct ui_time *time = (struct ui_time *)ctr;
	GPS_INFO GPS;

   
    switch (e) {
    case ON_CHANGE_INIT:
		//if()
		         status1= buf4[GetComma(2, buf4)];
//    printf("status1  %c",status1);

        tmp = GetComma(9, buf4);

 
	// printf("nn %d jj %d bb %d\n",nn,jj,bb);
      //  printf("2buf4 %s\n",buf4);
        time->year =nn;//(buf4[tmp + 4] - '0') * 10 + (buf4[tmp + 5] - '0') + 2000;
        time->month =jj;//(buf4[tmp + 2] - '0') * 10 + (buf4[tmp + 3] - '0');
        time->day = bb;//(buf4[tmp + 0] - '0') * 10 + (buf4[tmp + 1] - '0'); //日期
        time->hour = (buf4[7] - '0') * 10 + (buf4[8] - '0') ;        //时间
        time->min =(buf4[9] - '0') * 10 + (buf4[10] - '0');
        time->sec =  (buf4[11] - '0') * 10 + (buf4[12] - '0');
      
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(BASEFORM_93)
.onchange = timer_sys_tph_onchange1,
 .onkey = NULL,
  .ontouch = NULL,
};

static int timer_sys_tph_onchange2(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
   	GPS_INFO GPS;

    switch (e) {
    case ON_CHANGE_INIT:

        time->year =nn;//(buf4[tmp + 4] - '0') * 10 + (buf4[tmp + 5] - '0') + 2000;
        time->month =jj;//(buf4[tmp + 2] - '0') * 10 + (buf4[tmp + 3] - '0');
        time->day = bb;//(buf4[tmp + 0] - '0') * 10 + (buf4[tmp + 1] - '0'); //日期
        time->hour = (buf4[7] - '0') * 10 + (buf4[8] - '0') ;        //时间
        time->min =(buf4[9] - '0') * 10 + (buf4[10] - '0');
        time->sec =  (buf4[11] - '0') * 10 + (buf4[12] - '0');
    
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(BASEFORM_92)
.onchange = timer_sys_tph_onchange2,
 .onkey = NULL,
  .ontouch = NULL,
};
static void __rec_msg_hide(enum box_msg id)
{
    if (msg_show_id == id) {
        if (msg_show_f) {
            msg_show_f = 0;
            ui_hide(LAYER_MSG_REC);
        }
    } else if (id == 0) {
        /*
         * 没有指定ID，强制隐藏
         */
        if (msg_show_f) {
            msg_show_f = 0;
            ui_hide(LAYER_MSG_REC);
        }
    }
}
static void __rec_msg_timeout_func(void *priv)
{
    __rec_msg_hide((enum box_msg)priv);
}
static void __rec_msg_show(enum box_msg msg, u32 timeout_msec)
{
    static int t_id = 0;
    if (msg == 0) {
        ASSERT(0, "__rec_msg_show msg 0!\n");
        return;
    }

    if (msg == msg_show_id) {
        if (msg_show_f == 0) {
            msg_show_f = 1;
            ui_show(LAYER_MSG_REC);
            ui_text_show_index_by_id(TEXT_MSG_REC, msg - 1);
            if (t_id) {
                sys_timeout_del(t_id);
                t_id = 0;
            }
            if (timeout_msec > 0) {
                t_id = sys_timeout_add((void *)msg, __rec_msg_timeout_func, timeout_msec);
            }
        }
    } else {
        msg_show_id = msg;
        if (msg_show_f == 0) {
            msg_show_f = 1;
            ui_show(LAYER_MSG_REC);
        }
        ui_text_show_index_by_id(TEXT_MSG_REC, msg - 1);
        if (t_id) {
            sys_timeout_del(t_id);
            t_id = 0;
        }
        if (timeout_msec > 0) {
            t_id = sys_timeout_add((void *)msg, __rec_msg_timeout_func, timeout_msec);
        }
    }
}
/*
 * (end)
 */

/*****************************布局上部回调 ************************************/
static int video_layout_up_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;

    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        /*
         * 在此获取默认隐藏的图标的状态并显示
         */


        if (__this->lock_file_flag == 1) {
            ui_show(PIC_LOCK_REC);
        }

        if (db_select("mot")) {
            ui_show(PIC_MOTDET_REC);
        } else {
            ui_hide(PIC_MOTDET_REC);
        }

        index = db_select("gra");
        if (index != 0) {
            ui_pic_show_image_by_id(PIC_GRAVITY_REC, index - 1);
        } else {
            ui_hide(PIC_GRAVITY_REC);
        }

        if (db_select("par")) {
            ui_show(PIC_PARK_REC);
        } else {
            ui_hide(PIC_PARK_REC);
        }

        if (db_select("wdr")) {
            ui_show(PIC_HDR_REC);
        } else {
            ui_hide(PIC_HDR_REC);
        }

        index = index_of_table8(db_select("cyc"), TABLE(table_video_cycle));

        if (index != 0) {
            ui_pic_show_image_by_id(PIC_CYC_REC, index - 1);
        } else {
            ui_hide(PIC_CYC_REC);
        }

        index = index_of_table16(db_select("gap"), TABLE(table_video_gap));
        if (index) {
            ui_show(PIC_GAP_REC);
        } else {
            ui_hide(PIC_GAP_REC);
        }

        if (__this->if_in_rec) {
            puts("show run rec timer.\n");
            ui_show(TIMER_RUN_REC);
        } else {
            ui_show(TIMER_REMAI_REC);
        }
        index = index_of_table8(db_select("exp"), TABLE(table_video_exposure));
        ui_pic_show_image_by_id(PIC_EXP_REC, index);

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(LAYOUT_UP_REC)
.onchange = video_layout_up_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
/*****************************布局下部回调 ************************************/
static int video_layout_down_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u32 a, b;

    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        /*
         * 在此获取默认隐藏的图标的状态并显示
         */
        a = db_select("cna");
        b = db_select("cnb");
        text_car_num_table[0].text_index = index_of_table16(a >> 16, TABLE(province_gb2312));
        text_car_num_table[1].text_index  = index_of_table8((a >> 8) & 0xff, TABLE(num_table));
        text_car_num_table[2].text_index  = index_of_table8((a >> 0) & 0xff, TABLE(num_table));
        text_car_num_table[3].text_index  = index_of_table8((b >> 24) & 0xff, TABLE(num_table));
        text_car_num_table[4].text_index  = index_of_table8((b >> 16) & 0xff, TABLE(num_table));
        text_car_num_table[5].text_index  = index_of_table8((b >> 8) & 0xff, TABLE(num_table));
        text_car_num_table[6].text_index  = index_of_table8((b >> 0) & 0xff, TABLE(num_table));

        if (db_select("num")) {
            ui_text_show_index_by_id(TEXT_CAR_PROVINCE, text_car_num_table[0].text_index);
            ui_text_show_index_by_id(TEXT_CAR_TOWN, text_car_num_table[1].text_index);
            ui_text_show_index_by_id(TEXT_CAR_A,    text_car_num_table[2].text_index);
            ui_text_show_index_by_id(TEXT_CAR_B,    text_car_num_table[3].text_index);
            ui_text_show_index_by_id(TEXT_CAR_C,    text_car_num_table[4].text_index);
            ui_text_show_index_by_id(TEXT_CAR_D,    text_car_num_table[5].text_index);
            ui_text_show_index_by_id(TEXT_CAR_E,    text_car_num_table[6].text_index);
        }
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(LAYOUT_DOWN_REC)
.onchange = video_layout_down_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static void get_sys_time(struct sys_time *time)
{
    void *fd = dev_open("rtc", NULL);
    if (!fd) {
        memset(time, 0, sizeof(*time));
        return;
    }
    dev_ioctl(fd, IOCTL_GET_SYS_TIME, (u32)time);
    dev_close(fd);
}

/***************************** 系统时间控件动作 ************************************/
static int timer_sys_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;

    switch (e) {
    case ON_CHANGE_INIT:
        get_sys_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TIMER_HOUR_REC)
.onchange = timer_sys_rec_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
/***************************** 系统日期控件动作 ************************************/
static int timer_sys_date_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;

    switch (e) {
    case ON_CHANGE_INIT:
        get_sys_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TIMER_YEAR_REC)
.onchange = timer_sys_date_rec_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

//static u16 timer = 0;


static u8 timer1 = 0;
static void gps_jwd1_handler(void *priv)
{
      	     if((0 == (buf6[0] -'0')) &&(0 == (buf6[1] -'0')))
	         ui_pic_show_image_by_id(BASEFORM_151, 0);
	      if((0 == (buf6[0] -'0')) &&(1 == (buf6[1] -'0')))
	         ui_pic_show_image_by_id(BASEFORM_151, 1);
	      if((0 == (buf6[0] -'0')) &&(2 == (buf6[1] -'0')))
	         ui_pic_show_image_by_id(BASEFORM_151, 2);
	     if((0 == (buf6[0] -'0')) &&(3 == (buf6[1] -'0')))
	         ui_pic_show_image_by_id(BASEFORM_151, 3);
	     if((0 == (buf6[0] -'0')) &&(4 == (buf6[1] -'0')))
	         ui_pic_show_image_by_id(BASEFORM_151, 4);
	     if((0 == (buf6[0] -'0')) &&(5 == (buf6[1] -'0')))
	         ui_pic_show_image_by_id(BASEFORM_151, 5);
	      if((0 == (buf6[0] -'0')) &&(6 == (buf6[1] -'0')))
	         ui_pic_show_image_by_id(BASEFORM_151, 6);
	      	     if((0 == (buf6[0] -'0')) &&(7 == (buf6[1] -'0')))
	         ui_pic_show_image_by_id(BASEFORM_151, 7);
	      if((0 == (buf6[0] -'0')) &&(8 == (buf6[1] -'0')))
	         ui_pic_show_image_by_id(BASEFORM_151, 8);
	      if((0 == (buf6[0] -'0')) &&(9 == (buf6[1] -'0')))
	         ui_pic_show_image_by_id(BASEFORM_151, 9);
	     if((1 == (buf6[0] -'0')) &&(0 == (buf6[1] -'0')))
	         ui_pic_show_image_by_id(BASEFORM_151, 10);
	     if((1 == (buf6[0] -'0')) &&(1 == (buf6[1] -'0')))
	         ui_pic_show_image_by_id(BASEFORM_151, 11);
	    	     if((1 == (buf6[0] -'0')) &&(2 == (buf6[1] -'0')))
	         ui_pic_show_image_by_id(BASEFORM_151, 12);

}
static void gps_jwd_handler(void *priv)
{   
#if 1
            //printf("buf1 %s  buf2 %s \n", buf1,buf2);
         //   printf("a %d,b %d c %d d %d e %d f %d g %d h %d",(buf2[0]-'0'),(buf2[1]-'0'),(buf2[2]-'0'),(buf2[3]-'0'),(buf2[5]-'0'),(buf2[6]-'0'),(buf2[7]-'0'),(buf2[8]-'0'));
          //   printf("a %d,b %d c %d d %d e %d f %d g %d h %d",(buf1[0]-'0'),(buf1[1]-'0'),(buf1[2]-'0'),(buf1[3]-'0'),(buf1[4]-'0'),(buf1[6]-'0'),(buf1[7]-'0'),(buf1[8]-'0'),(buf1[9]-'0'));
	     if(0 == (buf2[0] -'0'))
	         ui_pic_show_image_by_id(BASEFORM_95, 0);
	     if(0 == (buf2[1] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_96, 0);
	     if(0 == (buf2[2] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_97, 0);
	     if(0 == (buf2[3] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_98, 0);
	     if(0 == (buf2[ 5] -'0'))
		ui_pic_show_image_by_id(BASEFORM_99, 0);
	     if(0 == (buf2[6] -'0'))
		ui_pic_show_image_by_id(BASEFORM_100, 0); 
	     if(0 == (buf2[7] -'0'))
		ui_pic_show_image_by_id(BASEFORM_101, 0);
	     if(0 == (buf2[8] -'0'))
		ui_pic_show_image_by_id(BASEFORM_102, 0);
	      if(0 == (buf1[0] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_103, 0);
	     if(0 == (buf1[1] -'0'))
	        ui_pic_show_image_by_id(BASEFORM_104, 0);
	     if(0 == (buf1[2] -'0'))
		ui_pic_show_image_by_id(BASEFORM_105, 0);
	     if(0 == (buf1[3] -'0'))
		ui_pic_show_image_by_id(BASEFORM_106, 0);
	     if(0 == (buf1[4] -'0'))
               ui_pic_show_image_by_id(BASEFORM_107, 0);
	     if(0 == (buf1[6] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_108, 0);
	     if(0 == (buf1[7] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_109, 0);
	     if(0 == (buf1[8] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_112, 0);	 
	     if(0 == (buf1[9] -'0'))
                ui_pic_show_image_by_id(BASEFORM_149, 0);
	       if(1 == (buf2[0] -'0'))
	         ui_pic_show_image_by_id(BASEFORM_95, 1);
	     if(1 == (buf2[1] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_96, 1);
	     if(1 == (buf2[2] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_97, 1);
	     if(1 == (buf2[3] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_98, 1);
	     if(1 == (buf2[ 5] -'0'))
		ui_pic_show_image_by_id(BASEFORM_99, 1);
	     if(1 == (buf2[6] -'0'))
		ui_pic_show_image_by_id(BASEFORM_100, 1); 
	     if(1 == (buf2[7] -'0'))
		ui_pic_show_image_by_id(BASEFORM_101, 1);
	     if(1 == (buf2[8] -'0'))
		ui_pic_show_image_by_id(BASEFORM_102, 1);
	      if(1 == (buf1[0] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_103, 1);
	     if(1 == (buf1[1] -'0'))
	        ui_pic_show_image_by_id(BASEFORM_104, 1);
	     if(1 == (buf1[2] -'0'))
		ui_pic_show_image_by_id(BASEFORM_105, 1);
	     if(1 == (buf1[3] -'0'))
		ui_pic_show_image_by_id(BASEFORM_106, 1);
	     if(1 == (buf1[4] -'0'))
               ui_pic_show_image_by_id(BASEFORM_107, 1);
	     if(1 == (buf1[6] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_108, 1);
	     if(1 == (buf1[7] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_109, 1);
	     if(1 == (buf1[8] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_112, 1);	 
	     if(1 == (buf1[9] -'0'))
                ui_pic_show_image_by_id(BASEFORM_149, 1);
	      if(2 == (buf2[0] -'0'))
	         ui_pic_show_image_by_id(BASEFORM_95, 2);
	     if(2 == (buf2[1] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_96, 2);
	     if(2 == (buf2[2] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_97, 2);
	     if(2 == (buf2[3] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_98, 2);
	     if(2 == (buf2[ 5] -'0'))
		ui_pic_show_image_by_id(BASEFORM_99, 2);
	     if(2 == (buf2[6] -'0'))
		ui_pic_show_image_by_id(BASEFORM_100, 2); 
	     if(2 == (buf2[7] -'0'))
		ui_pic_show_image_by_id(BASEFORM_101, 2);
	     if(2 == (buf2[8] -'0'))
		ui_pic_show_image_by_id(BASEFORM_102, 2);
	      if(2 == (buf1[0] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_103, 2);
	     if(2 == (buf1[1] -'0'))
	        ui_pic_show_image_by_id(BASEFORM_104, 2);
	     if(2 == (buf1[2] -'0'))
		ui_pic_show_image_by_id(BASEFORM_105, 2);
	     if(2 == (buf1[3] -'0'))
		ui_pic_show_image_by_id(BASEFORM_106, 2);
	     if(2 == (buf1[4] -'0'))
               ui_pic_show_image_by_id(BASEFORM_107, 2);
	     if(2 == (buf1[6] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_108, 2);
	     if(2 == (buf1[7] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_109, 2);
	     if(2 == (buf1[8] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_112, 2);	 
	     if(2 == (buf1[9] -'0'))
                ui_pic_show_image_by_id(BASEFORM_149, 2);
	       if(3 == (buf2[0] -'0'))
	         ui_pic_show_image_by_id(BASEFORM_95, 3);
	     if(3 == (buf2[1] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_96, 3);
	     if(3 == (buf2[2] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_97, 3);
	     if(3 == (buf2[3] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_98, 3);
	     if(3 == (buf2[ 5] -'0'))
		ui_pic_show_image_by_id(BASEFORM_99, 3);
	     if(3 == (buf2[6] -'0'))
		ui_pic_show_image_by_id(BASEFORM_100, 3); 
	     if(3 == (buf2[7] -'0'))
		ui_pic_show_image_by_id(BASEFORM_101, 3);
	     if(3 == (buf2[8] -'0'))
		ui_pic_show_image_by_id(BASEFORM_102, 3);
	      if(3 == (buf1[0] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_103, 3);
	     if(3 == (buf1[1] -'0'))
	        ui_pic_show_image_by_id(BASEFORM_104, 3);
	     if(3 == (buf1[2] -'0'))
		ui_pic_show_image_by_id(BASEFORM_105, 3);
	     if(3 == (buf1[3] -'0'))
		ui_pic_show_image_by_id(BASEFORM_106, 3);
	     if(3 == (buf1[4] -'0'))
               ui_pic_show_image_by_id(BASEFORM_107, 3);
	     if(3 == (buf1[6] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_108, 3);
	     if(3 == (buf1[7] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_109, 3);
	     if(3 == (buf1[8] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_112, 3);	 
	     if(3 == (buf1[9] -'0'))
                ui_pic_show_image_by_id(BASEFORM_149, 3);
	       if(4 == (buf2[0] -'0'))
	         ui_pic_show_image_by_id(BASEFORM_95, 4);
	     if(4 == (buf2[1] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_96, 4);
	     if(4 == (buf2[2] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_97, 4);
	     if(4 == (buf2[3] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_98, 4);
	     if(4 == (buf2[ 5] -'0'))
		ui_pic_show_image_by_id(BASEFORM_99, 4);
	     if(4 == (buf2[6] -'0'))
		ui_pic_show_image_by_id(BASEFORM_100, 4); 
	     if(4 == (buf2[7] -'0'))
		ui_pic_show_image_by_id(BASEFORM_101, 4);
	     if(4 == (buf2[8] -'0'))
		ui_pic_show_image_by_id(BASEFORM_102, 4);
	      if(4 == (buf1[0] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_103, 4);
	     if(4 == (buf1[1] -'0'))
	        ui_pic_show_image_by_id(BASEFORM_104, 4);
	     if(4 == (buf1[2] -'0'))
		ui_pic_show_image_by_id(BASEFORM_105, 4);
	     if(4 == (buf1[3] -'0'))
		ui_pic_show_image_by_id(BASEFORM_106, 4);
	     if(4 == (buf1[4] -'0'))
               ui_pic_show_image_by_id(BASEFORM_107, 4);
	     if(4 == (buf1[6] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_108, 4);
	     if(4 == (buf1[7] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_109,4);
	     if(4 == (buf1[8] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_112, 4);	 
	     if(4 == (buf1[9] -'0'))
                ui_pic_show_image_by_id(BASEFORM_149, 4);
	      if(5 == (buf2[0] -'0'))
	         ui_pic_show_image_by_id(BASEFORM_95, 5);
	     if(5 == (buf2[1] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_96, 5);
	     if(5 == (buf2[2] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_97, 5);
	     if(5 == (buf2[3] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_98, 5);
	     if(5 == (buf2[ 5] -'0'))
		ui_pic_show_image_by_id(BASEFORM_99, 5);
	     if(5 == (buf2[6] -'0'))
		ui_pic_show_image_by_id(BASEFORM_100, 5); 
	     if(5 == (buf2[7] -'0'))
		ui_pic_show_image_by_id(BASEFORM_101, 5);
	     if(5 == (buf2[8] -'0'))
		ui_pic_show_image_by_id(BASEFORM_102, 5);
	      if(5 == (buf1[0] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_103, 5);
	     if(5 == (buf1[1] -'0'))
	        ui_pic_show_image_by_id(BASEFORM_104, 5);
	     if(5 == (buf1[2] -'0'))
		ui_pic_show_image_by_id(BASEFORM_105, 5);
	     if(5 == (buf1[3] -'0'))
		ui_pic_show_image_by_id(BASEFORM_106, 5);
	     if(5 == (buf1[4] -'0'))
               ui_pic_show_image_by_id(BASEFORM_107, 5);
	     if(5 == (buf1[6] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_108, 5);
	     if(5 == (buf1[7] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_109, 5);
	     if(5 == (buf1[8] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_112, 5);	 
	     if(5 == (buf1[9] -'0'))
                ui_pic_show_image_by_id(BASEFORM_149, 5);
             if(6 == (buf2[0] -'0'))
	         ui_pic_show_image_by_id(BASEFORM_95, 6);
	     if(6 == (buf2[1] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_96, 6);
	     if(6 == (buf2[2] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_97, 6);
	     if(6 == (buf2[3] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_98, 6);
	     if(6 == (buf2[ 5] -'0'))
		ui_pic_show_image_by_id(BASEFORM_99, 6);
	     if(6 == (buf2[6] -'0'))
		ui_pic_show_image_by_id(BASEFORM_100, 6); 
	     if(6 == (buf2[7] -'0'))
		ui_pic_show_image_by_id(BASEFORM_101, 6);
	     if(6 == (buf2[8] -'0'))
		ui_pic_show_image_by_id(BASEFORM_102, 6);
	      if(6 == (buf1[0] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_103, 6);
	     if(6 == (buf1[1] -'0'))
	        ui_pic_show_image_by_id(BASEFORM_104, 6);
	     if(6 == (buf1[2] -'0'))
		ui_pic_show_image_by_id(BASEFORM_105, 6);
	     if(6 == (buf1[3] -'0'))
		ui_pic_show_image_by_id(BASEFORM_106, 6);
	     if(6 == (buf1[4] -'0'))
               ui_pic_show_image_by_id(BASEFORM_107, 6);
	     if(6 == (buf1[6] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_108, 6);
	     if(6 == (buf1[7] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_109, 6);
	     if(6 == (buf1[8] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_112, 6);	 
	     if(6 == (buf1[9] -'0'))
                ui_pic_show_image_by_id(BASEFORM_149, 6);
	      if(7 == (buf2[0] -'0'))
	         ui_pic_show_image_by_id(BASEFORM_95, 7);
	     if(7 == (buf2[1] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_96, 7);
	     if(7 == (buf2[2] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_97, 7);
	     if(7 == (buf2[3] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_98, 7);
	     if(7 == (buf2[ 5] -'0'))
		ui_pic_show_image_by_id(BASEFORM_99, 7);
	     if(7 == (buf2[6] -'0'))
		ui_pic_show_image_by_id(BASEFORM_100, 7); 
	     if(7 == (buf2[7] -'0'))
		ui_pic_show_image_by_id(BASEFORM_101, 7);
	     if(7 == (buf2[8] -'0'))
		ui_pic_show_image_by_id(BASEFORM_102, 7);
	      if(7 == (buf1[0] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_103, 7);
	     if(7 == (buf1[1] -'0'))
	        ui_pic_show_image_by_id(BASEFORM_104, 7);
	     if(7 == (buf1[2] -'0'))
		ui_pic_show_image_by_id(BASEFORM_105, 7);
	     if(7 == (buf1[3] -'0'))
		ui_pic_show_image_by_id(BASEFORM_106, 7);
	     if(7 == (buf1[4] -'0'))
               ui_pic_show_image_by_id(BASEFORM_107, 7);
	     if(7 == (buf1[6] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_108, 7);
	     if(7 == (buf1[7] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_109, 7);
	     if(7 == (buf1[8] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_112, 7);	 
	     if(7 == (buf1[9] -'0'))
                ui_pic_show_image_by_id(BASEFORM_149, 7);
	       if(8 == (buf2[0] -'0'))
	         ui_pic_show_image_by_id(BASEFORM_95, 8);
	     if(8 == (buf2[1] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_96, 8);
	     if(8 == (buf2[2] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_97, 8);
	     if(8 == (buf2[3] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_98, 8);
	     if(8 == (buf2[ 5] -'0'))
		ui_pic_show_image_by_id(BASEFORM_99, 8);
	     if(8 == (buf2[6] -'0'))
		ui_pic_show_image_by_id(BASEFORM_100, 8); 
	     if(8 == (buf2[7] -'0'))
		ui_pic_show_image_by_id(BASEFORM_101, 8);
	     if(8 == (buf2[8] -'0'))
		ui_pic_show_image_by_id(BASEFORM_102, 8);
	      if(8 == (buf1[0] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_103, 8);
	     if(8== (buf1[1] -'0'))
	        ui_pic_show_image_by_id(BASEFORM_104, 8);
	     if(8 == (buf1[2] -'0'))
		ui_pic_show_image_by_id(BASEFORM_105, 8);
	     if(8 == (buf1[3] -'0'))
		ui_pic_show_image_by_id(BASEFORM_106, 8);
	     if(8 == (buf1[4] -'0'))
               ui_pic_show_image_by_id(BASEFORM_107, 8);
	     if(8 == (buf1[6] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_108, 8);
	     if(8 == (buf1[7] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_109, 8);
	     if(8 == (buf1[8] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_112, 8);	 
	     if(8 == (buf1[9] -'0'))
                ui_pic_show_image_by_id(BASEFORM_149, 8);
	       if(9 == (buf2[0] -'0'))
	         ui_pic_show_image_by_id(BASEFORM_95, 9);
	     if(9 == (buf2[1] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_96, 9);
	     if(9 == (buf2[2] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_97, 9);
	     if(9 == (buf2[3] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_98, 9);
	     if(9 == (buf2[ 5] -'0'))
		ui_pic_show_image_by_id(BASEFORM_99, 9);
	     if(9 == (buf2[6] -'0'))
		ui_pic_show_image_by_id(BASEFORM_100, 9); 
	     if(9 == (buf2[7] -'0'))
		ui_pic_show_image_by_id(BASEFORM_101, 9);
	     if(9 == (buf2[8] -'0'))
		ui_pic_show_image_by_id(BASEFORM_102, 9);
	      if(9 == (buf1[0] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_103, 9);
	     if(9 == (buf1[1] -'0'))
	        ui_pic_show_image_by_id(BASEFORM_104, 9);
	     if(9 == (buf1[2] -'0'))
		ui_pic_show_image_by_id(BASEFORM_105, 9);
	     if(9 == (buf1[3] -'0'))
		ui_pic_show_image_by_id(BASEFORM_106, 9);
	     if(9 == (buf1[4] -'0'))
               ui_pic_show_image_by_id(BASEFORM_107, 9);
	     if(9 == (buf1[6] -'0'))
		  ui_pic_show_image_by_id(BASEFORM_108, 9);
	     if(9 == (buf1[7] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_109, 9);
	     if(9 == (buf1[8] -'0'))
		 ui_pic_show_image_by_id(BASEFORM_112, 9);	 
	     if(9 == (buf1[9] -'0'))
                ui_pic_show_image_by_id(BASEFORM_149, 9);
#endif
}

static void gps_take_handler(void *priv )
{
     int integer ;
     
// printf(" shushu %d \n", shu);
     int  temp =(int)(shu/1000),i,j,k;	
	 if(temp < 100)
	 	{
     j  = (int)( temp /  10);
	  k =  (int)(temp ) - (int)(j  * 10);
	// int h = 
	// int k =   (int)(temp - i *1000 -j *100);
  //  printf(" YY %d shu%d  j %d k %d\n", temp,shu  ,j ,k);
  //  double  temp = num; 

	 	}
	 if (temp > 100){

           i = (int)(temp /100);
	    j = (int)(temp /10) - (int)(i * 10);
	    k = (int)temp - (int)(i * 100);

  }
 

#if 1
 


   if(i == 0)
           ui_pic_show_image_by_id(BASEFORM_94, 0);
          if(i == 1)
		ui_pic_show_image_by_id(BASEFORM_94,1);

		   if(i == 2)
		ui_pic_show_image_by_id(BASEFORM_94,2);
	     if(0 == j)
            ui_pic_show_image_by_id(BASEFORM_88, 0);
	     if(0 == k)
            ui_pic_show_image_by_id(BASEFORM_84, 0);
	     if(1 == j)
            ui_pic_show_image_by_id(BASEFORM_88, 1);
	     if(1 == k)
            ui_pic_show_image_by_id(BASEFORM_84, 1);
	     if(2 == j)
            ui_pic_show_image_by_id(BASEFORM_88, 2);
	     if(2 ==k)
            ui_pic_show_image_by_id(BASEFORM_84, 2);            
	     if(3 == j)
            ui_pic_show_image_by_id(BASEFORM_88, 3);
	     if(3 == k)
            ui_pic_show_image_by_id(BASEFORM_84, 3);
	     if(4 == j)
            ui_pic_show_image_by_id(BASEFORM_88, 4);
	     if(4 == k)
            ui_pic_show_image_by_id(BASEFORM_84, 4);
	     if(5 == j)
            ui_pic_show_image_by_id(BASEFORM_88, 5);
	     if(5==k)
            ui_pic_show_image_by_id(BASEFORM_84, 5);
	    if(6 == j)
            ui_pic_show_image_by_id(BASEFORM_88, 6);
	     if(6 == k)
            ui_pic_show_image_by_id(BASEFORM_84, 6);
	    if(7 ==j)
            ui_pic_show_image_by_id(BASEFORM_88, 7);
	     if(7 ==k)
            ui_pic_show_image_by_id(BASEFORM_84, 7);
	    if(8 == j)
            ui_pic_show_image_by_id(BASEFORM_88, 8);
	     if(8 == k)
            ui_pic_show_image_by_id(BASEFORM_84, 8);
	     if(9 == j)
            ui_pic_show_image_by_id(BASEFORM_88, 9);
	     if(9 == k)
            ui_pic_show_image_by_id(BASEFORM_84, 9);	 

#endif
 //   return 0;
}
 void gps()
{
  if (!timer1) {
            timer1 = sys_timer_add(NULL, gps_take_handler, 1000);
	     timer1 = sys_timer_add(NULL, gps_jwd_handler, 1000);
	    timer1 = sys_timer_add(NULL, gps_jwd1_handler, 1000);
        }
}
/*
 * sd卡事件处理函数
 */
static void sd_event_handler(struct sys_event *event, void *priv)
{
    if (!strcmp(event->arg, "sd0") || !strcmp(event->arg, "sd1") || !strcmp(event->arg, "sd2")) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
        case DEVICE_EVENT_ONLINE:
            ui_pic_show_image_by_id(PIC_SD_REC, 1);
            ui_show(TIMER_REMAI_REC);
            break;
        case DEVICE_EVENT_OUT:
        case DEVICE_EVENT_OFFLINE:
            ui_pic_show_image_by_id(PIC_SD_REC, 0);
            ui_show(TIMER_REMAI_REC);
            break;
        default:
            break;
        }
    }
}

extern int storage_device_ready();
/***************************** SD 卡图标动作 ************************************/
static int pic_sd_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u16 id = 0;

    switch (e) {
    case ON_CHANGE_INIT:
        if (storage_device_ready() == 0) {
            ui_pic_set_image_index(pic, 0);
        } else {
            ui_pic_set_image_index(pic, 1);
        }
        id = register_sys_event_handler(SYS_DEVICE_EVENT, 0, 0, sd_event_handler);
        break;
    case ON_CHANGE_RELEASE:
        unregister_sys_event_handler(id);
        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PIC_SD_REC)
.onchange = pic_sd_rec_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static void no_power_msg_box_timer(void *priv)
{
    static u8 cnt = 0;
    if (__this->battery_val <= 20 && __this->menu_status == 0 && __this->battery_char == 0) {
        cnt++;
        if ((cnt % 2) == 0) {
            __rec_msg_show(BOX_MSG_NO_POWER, 0);
        } else {
            __rec_msg_hide(BOX_MSG_NO_POWER);
        }
    } else {
        __rec_msg_hide(BOX_MSG_NO_POWER);
        cnt = 0;
    }
}


/*
 * battery事件处理函数
 */
static void battery_event_handler(struct sys_event *event, void *priv)
{

    if (event->type == SYS_DEVICE_EVENT) {
        if (!ASCII_StrCmp(event->arg, "powerdet", 8)) {
            if (event->u.dev.event == DEVICE_EVENT_CHANGE) {
                __this->battery_val = event->u.dev.value * 20;
                if (__this->battery_val > 100) {
                    __this->battery_val = 100;
                }
                if (__this->battery_char == 0) {
                    ui_battery_level_change(__this->battery_val, 0);
                }
            }
        } else if (!ASCII_StrCmp(event->arg, "charger", 7)) {
            if (event->u.dev.event == DEVICE_EVENT_IN) {
                ui_battery_level_change(100, 1);
                __this->battery_char = 1;
            } else if (event->u.dev.event == DEVICE_EVENT_OUT) {
                ui_battery_level_change(__this->battery_val, 0);
                __this->battery_char = 0;
            }
        }
    }
}
/***************************** 电池控件动作 ************************************/
static int battery_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_battery *battery = (struct ui_battery *)ctr;
    static u16 id = 0;
    static u32 timer_handle = 0;
    static void *fd = NULL;
    switch (e) {
    case ON_CHANGE_INIT:
        id = register_sys_event_handler(SYS_DEVICE_EVENT, 0, fd, battery_event_handler);
        break;
    case ON_CHANGE_FIRST_SHOW:
        fd = dev_open("powerdet", NULL);
        if (fd) {
            u32 power_level = 0;
            dev_ioctl(fd, POWER_DET_GET_LEVEL, (u32)&power_level);
            __this->battery_val = power_level * 20;
            if (__this->battery_val > 100) {
                __this->battery_val = 100;
            }
            dev_close(fd);
        }
        __this->battery_char = (usb_is_charging() ? 1 : 0);
        ui_battery_level_change(__this->battery_val, __this->battery_char);
        timer_handle = sys_timer_add(NULL, no_power_msg_box_timer, 1000);
        break;
    case ON_CHANGE_RELEASE:
        unregister_sys_event_handler(id);
        if (timer_handle) {
            sys_timer_del(timer_handle);
            timer_handle = 0;
        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(BATTERY_REC)
.onchange = battery_rec_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/***************************** 录像剩余时间控件动作 ************************************/
static int timer_rec_remain_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct intent it;
    int err, i, s;
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW_PROBE:
        if (storage_device_ready() == 0) {
            /*
             * 第一次显示需要判断一下SD卡是否在线
             */
            time->hour = 0;
            time->min = 0;
            time->sec = 0;
            break;
        }

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_CHANGE_STATUS;
        it.data = "reTIME:";
        err = start_app(&it);
        if (err) {
            puts("\n err == 0, :rec reTIME\n");
            break;
        }
        i = 0;
        s = 10;
        time->hour = 0;
        while (it.data[i] != '.' && it.data[i] != '\0') {
            if (it.data[i] >= '0' && it.data[i] <= '9') {
                time->hour += ((it.data[i] - '0') * s);
            }
            i++;
            s = s / 10;
        }
        i++;
        s = 10;
        time->min = 0;
        while (it.data[i] != '.' && it.data[i] != '\0') {
            if (it.data[i] >= '0' && it.data[i] <= '9') {
                time->min += ((it.data[i] - '0') * s);
            }
            i++;
            s = s / 10;
        }
        i++;
        s = 10;
        time->sec = 0;
        while (it.data[i] != '.' && it.data[i] != '\0') {
            if (it.data[i] >= '0' && it.data[i] <= '9') {
                time->sec += ((it.data[i] - '0') * s);
            }
            i++;
            s = s / 10;
        }
        printf("reTIME hour:%d, min:%d, sec:%d\n", time->hour, time->min, time->sec);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TIMER_REMAI_REC)
.onchange = timer_rec_remain_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/***************************** 录像时间控件动作 ************************************/
static int timer_rec_red_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        time->hour = 0;
        time->min = 0;
        time->sec = 0;
        break;
    case ON_CHANGE_HIDE:
        time->hour = 0;
        time->min = 0;
        time->sec = 0;
        break;
    case ON_CHANGE_SHOW_PROBE:
        if ((time->sec % 2) == 0) {
            ui_show(PIC_RED_DOT_REC);
        } else {
            ui_hide(PIC_RED_DOT_REC);
        }
        break;
    case ON_CHANGE_SHOW_POST:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TIMER_RUN_REC)
.onchange = timer_rec_red_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int timer_GPS_red_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        time->hour = 0;
        time->min = 0;
        time->sec = 0;
        break;
    case ON_CHANGE_HIDE:
        time->hour = 0;
        time->min = 0;
        time->sec = 0;
        break;
    case ON_CHANGE_SHOW_PROBE:

        break;
    case ON_CHANGE_SHOW_POST:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(BASEFORM_150)
.onchange = timer_GPS_red_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/***************************** MIC 图标动作 ************************************/
static int pic_mic_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_pic_set_image_index(pic, db_select("mic"));
        /*
         * 禁止录音
         */
        return TRUE;
    default:
        return FALSE;
    }
    return FALSE;
}

REGISTER_UI_EVENT_HANDLER(PIC_MIC_REC)
.onchange = pic_mic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/***************************** 分辨率文字动作 ************************************/
static int text_res_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("res"), TABLE(table_video_resolution));
        ui_text_set_index(text, index);
        return TRUE;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(TEXT_RES_REC)
.onchange = text_res_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



static int rec_timer_handle = 0;
static int rec_cnt = 0;
static void rec_timer_cnt(void *priv)
{
    /*
     * 录像计时的定时器,1s
     */
    struct utime time_r;
    rec_cnt++;
    time_r.hour = rec_cnt / 60 / 60;
    time_r.min = rec_cnt / 60 % 60;
    time_r.sec = rec_cnt % 60;
    ui_time_update_by_id(TIMER_RUN_REC, &time_r);
}

/*
 * (begin)APP状态变更，UI响应回调
 */
static int rec_on_handler(const char *type, u32 arg)
{
    printf("PPPPPP\n");
    __this->if_in_rec = TRUE;
    rec_cnt = 0;
    ui_hide(TIMER_REMAI_REC);
    ui_show(TIMER_RUN_REC);
    if (rec_timer_handle == 0) {
        rec_timer_handle = sys_timer_add(NULL, rec_timer_cnt, 1000);
    }
    return 0;
}

static int rec_off_handler(const char *type, u32 arg)
{
 //   printf("PPPPPPTAICHU\n");
    __this->if_in_rec = FALSE;
    rec_cnt = 0;
    if (__this->lock_file_flag) {
        __this->lock_file_flag = 0;
        ui_hide(PIC_LOCK_REC);
    }
    ui_hide(TIMER_RUN_REC);
    ui_hide(PIC_RED_DOT_REC);
    ui_show(TIMER_REMAI_REC);
    sys_timer_del(rec_timer_handle);
    rec_timer_handle = 0;
    return 0;
}

static int rec_save_handler(const char *type, u32 arg)
{
    struct utime time_r;

    time_r.hour = 0;
    time_r.min = 0;
    time_r.sec = 0;
    ui_time_update_by_id(TIMER_RUN_REC, &time_r);

    if (__this->lock_file_flag) {
        __this->lock_file_flag = 0;
        ui_hide(PIC_LOCK_REC);
    }
    rec_cnt = 0;
    return 0;
}
static int rec_no_card_handler(const char *type, u32 arg)
{
    __rec_msg_show(BOX_MSG_INSERT_SD, 3000);
    return 0;
}
static int rec_fs_err_handler(const char *type, u32 arg)
{
    __rec_msg_show(BOX_MSG_NEED_FORMAT, 3000);
    return 0;
}
static int rec_on_mic_handler(const char *type, u32 arg)
{
    ui_pic_show_image_by_id(PIC_MIC_REC, 1);

    return 0;
}
static int rec_off_mic_handler(const char *type, u32 arg)
{
    ui_pic_show_image_by_id(PIC_MIC_REC, 0);
    return 0;
}
static int rec_lock_handler(const char *type, u32 arg)
{
    __this->lock_file_flag = 1;
    ui_show(PIC_LOCK_REC);
    return 0;
}
static int rec_unlock_handler(const char *type, u32 arg)
{
    __this->lock_file_flag = 0;
    ui_hide(PIC_LOCK_REC);
    return 0;
}

extern void play_voice_file(const char *file_name);
static int rec_headlight_on_handler(const char *type, u32 arg)
{
    if (__this->menu_status == 0) {
        puts("rec_headlight_on_handler\n");
        if (__this->hlight_show_status == 0) {
            __this->hlight_show_status = 1;
            ui_show(ANI_FLIG_REC);//show head light
            play_voice_file("mnt/spiflash/audlogo/olight.adp");
        }
    } else {
        __this->hlight_show_status = 0;
    }

    return 0;
}
extern u8 home_rec_touch;
 static u8 rec_cnt1 =0;
static void home_rec_timer_cnt(void *priv)
{
    /*
     * ???????????,1s
     */
    struct utime time_r;
    rec_cnt1++;//??????
    time_r.hour = rec_cnt1 / 60 / 60;
    time_r.min = rec_cnt1 / 60 % 60;
    time_r.sec = rec_cnt1 % 60;

}
 static void rec_control_ok(void *p, int err)
{
    if (err == 0) {
        puts("---rec control ok\n");
    } else {
        printf("---rec control faild: %d\n", err);
    }
    sys_touch_event_enable();
    //__this->key_disable = 0;
}
static int rec_off_handler1(const char *type, u32 arg)
{
#if 1
    puts("rec_off_handler.\n");
    struct intent it;
   __this-> if_in_rec = FALSE;
     printf("WWWWWWWWWWWWWWW\n");
    sys_timer_del(rec_timer_handle);
   // rec_timer_cnt(void * priv)= 0;
    rec_cnt1 = 0;
 //   if(home_rec_touch){
    //    home_rec_touch = 0;
        ui_hide(BASEFORM_150);  //peng
        

        ui_show(BASEFORM_91);          

        main_lock_flag = 0;
        init_intent(&it);
       it.name = "video_rec";
        it.action = ACTION_BACK;
        start_app_async(&it,rec_control_ok,NULL);
        
  //  }
    return 0;
#endif

}
static int rec_save_handler1(const char *type, u32 arg)                                                                         
{   
        struct utime time_r;

    time_r.hour = 0;
    time_r.min = 0;
    time_r.sec = 0;
    ui_time_update_by_id(BASEFORM_91, &time_r);
   //?????????????????0
    rec_cnt1 = 0;
    return 0;
} 
static int rec_on_handler1(const char *type, u32 arg)
{
#if 0
    puts("\n***home rec_on_handler.***\n");
   __this-> if_in_rec = TRUE;
       printf("gggggggggggggg\n");
    ui_hide(BASEFORM_91 );   
    ui_show(BASEFORM_150); //peng
  //  ui_show(BASEFORM_90);
    if (rec_timer_handle == 0) {
        rec_cnt = 0;
	//home_rec_touch =1;
        rec_timer_handle = sys_timer_add(NULL, home_rec_timer_cnt, 1000);
	printf("rec_timer_handle  %d\n",rec_timer_handle);
    }
    return 0;
#endif
    __this->if_in_rec = TRUE;
    rec_cnt1 = 0;
    ui_hide(BASEFORM_91 );   
	       printf("gggggggggggggg\n");
    ui_show(BASEFORM_150); //peng
    if (rec_timer_handle == 0) {
        rec_timer_handle = sys_timer_add(NULL, home_rec_timer_cnt, 1000);
    }
    return 0;
}
static int rec_headlight_off_handler(const char *type, u32 arg)
{
    if (__this->menu_status == 0) {
        puts("rec_headlight_off_handler\n");
        ui_hide(ANI_FLIG_REC);//hide head light
        __this->hlight_show_status = 0;
    }
    return 0;
}
/*
 * 录像模式的APP状态响应回调
 */
static const struct uimsg_handl rec_msg_handler[] = {
    { "onREC",          rec_on_handler       }, /* 开始录像 */
    { "offREC",         rec_off_handler      }, /* 停止录像 */
    { "saveREC",        rec_save_handler     }, /* 保存录像 */
    { "noCard",         rec_no_card_handler  }, /* 没有SD卡 */
    { "fsErr",          rec_fs_err_handler   }, /* 需要格式化 */
    { "onMIC",          rec_on_mic_handler   }, /* 打开录音 */
    { "offMIC",         rec_off_mic_handler  }, /* 关闭录音 */
    { "lockREC",        rec_lock_handler     }, /* 锁文件 */
    { "unlockREC",      rec_unlock_handler   }, /* 解锁文件 */
    { "HlightOn",      rec_headlight_on_handler   },
    { "HlightOff",     rec_headlight_off_handler  },
    { NULL, NULL},      /* 必须以此结尾！ */
};
/*
 * (end)
 */
static const struct uimsg_handl gps_msg_handler[] = {
	{ "onREC",          rec_on_handler1       }, /* ?????? */ 
    { "offREC",         rec_off_handler1      }, /* ????? */
 { "saveREC",        rec_save_handler1     }, /* ??????? */
     { NULL, NULL},      /* ????????β?? */
};


/*
 * (begin)UI状态变更主动请求APP函数
 */
static void rec_tell_app_exit_menu(void)
{

    int err;
    struct intent it;
    init_intent(&it);
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_CHANGE_STATUS;
    it.data = "exitMENU";
    err = start_app(&it);
    if (err) {
        printf("res exit menu err! %d\n", err);
        /* ASSERT(err == 0, ":rec exitMENU\n"); */
    }
}

static int rec_ask_app_open_menu(void)
{

    int err;
    struct intent it;

    init_intent(&it);
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_CHANGE_STATUS;
    it.data = "opMENU:";
    err = start_app(&it);
    if (err) {
        printf("res ask menu err! %d\n", err);
        return -1;
        /* ASSERT(err == 0, ":rec opMENU fail! %d\n", err); */
    }
    if (!strcmp(it.data, "opMENU:dis")) {
        return -1;
    } else if (!strcmp(it.data, "opMENU:en")) {
    } else {
        ASSERT(0, "opMENU err\n");
    }
    return 0;
    /*
     * 返回0则打开菜单
     */
}



static void menu_lane_det_set(int value)
{
    struct intent it;
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "lan";
    it.exdata = value;
    start_app(&it);
}
static void open_set_lane_page(void *p, int err)
{
    struct intent it;
    puts("\n =============car lane set =========\n");
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_MAIN;
    it.data   = "lan_setting";
    start_app(&it);
}
static void close_set_lane_page(void *p)
{
    struct intent it;
    ui_hide(ui_get_current_window_id());
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_BACK;
    start_app(&it);
    puts("\n =============car lane set exit=========\n");
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_MAIN;
    start_app(&it);
}
static char lane_set_arg[128];
static void send_lane_det_setting_msg(const char *arg, u32 lane)
{
    struct sys_event eve;
    memset(lane_set_arg, 0, sizeof(lane_set_arg));
    sprintf(lane_set_arg, "%s:%d", arg, lane);
    puts(lane_set_arg);
    eve.arg = lane_set_arg;
    eve.type = SYS_DEVICE_EVENT;
    eve.u.dev.event = DEVICE_EVENT_CHANGE;
    sys_event_notify(&eve);
}
/*
 * (end)
 */


void rec_exit_menu_post(void)
{
    /*
     * 退出菜单，回到APP
     */
    ui_hide(LAYER_MENU_REC);
    ui_show(LAYER_UP_REC);
    ui_show(LAYER_DOWN_REC);
    rec_tell_app_exit_menu();

    __this->menu_status = 0;

}
int gps_mode_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        /*
         * 注册APP消息响应
         */
                     gps();
       ui_register_msg_handler(ID_WINDOW_DZG, gps_msg_handler);
       printf("\nlllllllllllllllllllll\n");
       // __this->cur_camera_id = 0;
      //  sys_cur_mod = 4;
       // __this->hlight_show_status = 0;
        break;
    case ON_CHANGE_FIRST_SHOW:

	break;
    case ON_CHANGE_RELEASE:

        ui_hide(ID_WINDOW_VIDEO_SYS);
        /*
         * 要隐藏一下系统菜单页面，防止在系统菜单插入USB进入USB页面
         */
        break;
    default:
        return false;
    }
    return false;
}

int gps_mode_onkey(void *ctr, struct element_key_event *e)
{
    static u8 i = 0;
    struct window *win = (struct window *)ctr;
    static char flag = 0;
    int err;
    struct intent it;
    struct application *app;
	
    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
            case KEY_OK:
	
#if 0
	init_intent(&it);
        it.name	= "video_rec";  
        it.action = ACTION_VIDEO_DZG_REC;//ACTION_VIDEO_REC_CHANGE_STATUS;
        it.data = "reTIME:";
#endif
   //   sys_touch_event_disable();
          //  it.name = "video_rec";
          //  it.action = ACTION_VIDEO_DZG_REC;
       //    start_app_async(&it, rec_control_ok, NULL);
		//home_rec_touch = 1;
            break;
        default:
            return false;
        }
        break;
    default:

        return true;
    }

    return true;
}



REGISTER_UI_EVENT_HANDLER(ID_WINDOW_DZG)
.onchange = gps_mode_onchange,
 .onkey = gps_mode_onkey,
  .ontouch = NULL,
};


/*****************************录像模式页面回调 ************************************/
static int video_mode_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    int err, item, id;
    const char *str = NULL;
    struct intent it;
    int ret;
    u8 timeout;

    switch (e) {
    case ON_CHANGE_INIT:
        puts("\n***rec mode onchange init***\n");
        ui_register_msg_handler(ID_WINDOW_VIDEO_REC, rec_msg_handler);
        /*
         * 注册APP消息响应
         */
        sys_cur_mod = 1;
        __this->hlight_show_status = 0;
        break;
    case ON_CHANGE_RELEASE:
        ui_hide(ID_WINDOW_VIDEO_SYS);
        /*
         * 要隐藏一下系统菜单页面，防止在系统菜单插入USB进入USB页面
         */
        break;
    default:
        return false;
    }
    return false;
}

static int video_mode_onkey(void *ctr, struct element_key_event *e)
{
    struct window *win = (struct window *)ctr;
    static char flag = 0;
    int err;
    struct intent it;

    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_MENU:
            if (__this->if_in_rec == TRUE) {
                break;
            }
            if (rec_ask_app_open_menu() == (int) - 1) {
                break;
            }
            __rec_msg_hide(0);
            ui_hide(LAYER_UP_REC);
            ui_hide(LAYER_DOWN_REC);
            ui_show(LAYER_MENU_REC);
            ui_show(LAYOUT_MN_REC_REC);
            /*
             * 显示菜单1
             */
            sys_key_event_takeover(true, false);
            __this->menu_status = 1;
            break;
        default:
            return false;
        }
        break;
    default:
        if (__this->menu_status) {
            if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {
                rec_exit_menu_post();
                sys_key_event_takeover(false, true);
            }
        }
        return true;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(ID_WINDOW_VIDEO_REC)
.onchange = video_mode_onchange,
 .onkey = video_mode_onkey,
  .ontouch = NULL,
};

static int parking_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        ui_register_msg_handler(ID_WINDOW_PARKING, rec_msg_handler); /* 注册APP消息响应 */
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_PARKING)
.onchange = parking_page_onchange,
 .ontouch = NULL,
};
/*********************************************************************************
 *  		     				菜单动作
 *********************************************************************************/
static int video_menu_rec_table[] = {
    LAYOUT_MN_RES_REC,
    LAYOUT_MN_TWO_REC,
    LAYOUT_MN_CYC_REC,
    //LAYOUT_MN_GAP_REC,
    LAYOUT_MN_HDR_REC,
    LAYOUT_MN_EXP_REC,
    LAYOUT_MN_MOTD_REC,
    LAYOUT_MN_MIC_REC,
    LAYOUT_MN_LABEL_REC,
    LAYOUT_MN_GRAV_REC,
    LAYOUT_MN_PARK_REC,
    //LAYOUT_MN_NUM_REC,
    //LAYOUT_MN_LANE_REC,
    //LAYOUT_MN_FLIG_REC,
    BASEFORM_36,
    //BASEFORM_83,
};


static int video_menu_rec_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    static int sel_item = 0;
    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        ASSERT(sel_item < (sizeof(video_menu_rec_table) / sizeof(int)));
        ui_show(video_menu_rec_table[sel_item]);

        break;
    case KEY_DOWN:
        return FALSE;
        /*
         * 向后分发消息
         */

        break;
    case KEY_UP:
        return FALSE;
        /*
         * 向后分发消息
         */

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_show(ID_WINDOW_VIDEO_SYS);
        ui_hide(LAYER_MENU_REC);
        break;

    default:
        return false;
        break;
    }

    return true;
    /*
     * 不向后分发消息
     */
}


REGISTER_UI_EVENT_HANDLER(VLIST_REC_REC)
.onkey = video_menu_rec_onkey,
 .ontouch = NULL,
};



/***************************** 分辨率设置 ************************************/

static int menu_res_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        item = index_of_table8(db_select("res"), TABLE(table_video_resolution));
        ui_grid_set_item(grid, item);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_res_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;
    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "res";
        it.exdata = table_video_resolution[sel_item];
        err = start_app(&it);
        if (err) {
            printf("res onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_RES_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_RES_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_RES_REC)
.onchange = menu_res_onchange,
 .onkey = menu_res_onkey,
  .ontouch = NULL,
};


/***************************** 双路录像设置 ************************************/
static int menu_double_route_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("two"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_double_route_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "two";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("two onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_TWO_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_TWO_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_TWO_REC)
.onchange = menu_double_route_onchange,
 .onkey = menu_double_route_onkey,
  .ontouch = NULL,
};


/***************************** 循环录影设置 ************************************/
static int menu_cyc_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("cyc"), TABLE(table_video_cycle));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_cyc_rec_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "cyc";
        it.exdata = table_video_cycle[sel_item];
        err = start_app(&it);
        if (err) {
            printf("cyc onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_CYC_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_CYC_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_CYC_REC)
.onchange = menu_cyc_rec_onchange,
 .onkey = menu_cyc_rec_onkey,
  .ontouch = NULL,
};



/***************************** 间隔录像设置 ************************************/
static int menu_gap_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table16(db_select("gap"), TABLE(table_video_gap));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
#if 0
static int menu_gap_rec_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "gap";
        it.exdata = table_video_gap[sel_item];
        err = start_app(&it);
        if (err) {
            printf("gap onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_GAP_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_GAP_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_GAP_REC)
.onchange = menu_gap_rec_onchange,
 .onkey = menu_gap_rec_onkey,
  .ontouch = NULL,
};
#endif


/***************************** 动态范围设置 ************************************/
static int menu_wdr_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("wdr"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_wdr_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "wdr";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("wdr onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_HDR_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_HDR_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_HDR_REC)
.onchange = menu_wdr_onchange,
 .onkey = menu_wdr_onkey,
  .ontouch = NULL,
};



/***************************** 曝光补偿设置 ************************************/

static int menu_exposure_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("exp"), TABLE(table_video_exposure));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_exposure_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "exp";
        it.exdata = table_video_exposure[sel_item];
        err = start_app(&it);
        if (err) {
            printf("exp onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_EXP_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_EXP_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_EXP_REC)
.onchange = menu_exposure_onchange,
 .onkey = menu_exposure_onkey,
  .ontouch = NULL,
};



/***************************** 运动检测设置 ************************************/

static int menu_motdet_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("mot"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_motdet_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "mot";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("mot onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_MOTD_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_MOTD_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_MOTD_REC)
.onchange = menu_motdet_onchange,
 .onkey = menu_motdet_onkey,
  .ontouch = NULL,
};



/***************************** 录音设置 ************************************/

static int menu_audio_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("mic"));
        break;
    default:
        return FALSE;
    }

    return FALSE;
}

static int menu_audio_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "mic";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("audio onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_MIC_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_MIC_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_MIC_REC)
.onchange = menu_audio_onchange,
 .onkey = menu_audio_onkey,
  .ontouch = NULL,
};


/***************************** 日期标签设置 ************************************/

static int menu_date_label_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("dat"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_date_label_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "dat";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("date label onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_LABEL_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_LABEL_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_LABEL_REC)
.onchange = menu_date_label_onchange,
 .onkey = menu_date_label_onkey,
  .ontouch = NULL,
};


/***************************** 重力感应设置 ************************************/

static int menu_gravity_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("gra"), TABLE(table_video_gravity));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_gravity_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "gra";
        it.exdata = table_video_gravity[sel_item];
        err = start_app(&it);
        if (err) {
            printf("gravity onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_GRAV_REC);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_GRAV_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_GRAV_REC)
.onchange = menu_gravity_onchange,
 .onkey = menu_gravity_onkey,
  .ontouch = NULL,
};



/***************************** 停车守卫设置 ************************************/
static int menu_parking_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("par"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

static int menu_parking_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "par";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("park onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_PARK_REC);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_PARK_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_PARK_REC)
.onchange = menu_parking_onchange,
 .onkey = menu_parking_onkey,
  .ontouch = NULL,
};


/***************************** 车牌号码开关设置 ************************************/
static int menu_car_num_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("num"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
#if 0
static int menu_car_num_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;
    const char *str;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "num";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("res car num err! %d\n", err);
            break;
        }

        ui_hide(LAYOUT_MN_NUM_REC);
        if (sel_item) {
            ui_show(LAYOUT_MN_CARNUM_SET_REC);
        }

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_NUM_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_NUM_REC)
.onchange = menu_car_num_onchange,
 .onkey = menu_car_num_onkey,
  .ontouch = NULL,
};
#endif
#if 0
/***************************** 车道偏移开关设置 ************************************/
static int menu_lane_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, !!db_select("lan"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

static int menu_lane_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;
    const char *str;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        if (sel_item == 0) {
            menu_lane_det_set(0);
            ui_hide(LAYOUT_MN_LANE_REC);
        } else {
            struct intent it;
            ui_hide(ui_get_current_window_id());
            rec_exit_menu_post();
            init_intent(&it);
            it.name = "video_rec";
            it.action = ACTION_BACK;
            start_app_async(&it, open_set_lane_page, NULL);
            break;
        }

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_LANE_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_LANE_REC)
.onchange = menu_lane_onchange,
 .onkey = menu_lane_onkey,
  .ontouch = NULL,
};
#endif
static int menu_bu_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        item = index_of_table8(db_select("bu"), TABLE(table_video_bu));
	printf("\n===item : %d===\n",item);
        ui_grid_set_item(grid, item);
	printf("\n===ui_grid_set_item : %d===\n");
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_bu_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;
    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "bu";
        it.exdata = table_video_bu[sel_item];
	printf("\n===sel_item : %d===\n",sel_item);
        err = start_app(&it);
        if (err) {
            printf("res onkey err! %d\n", err);
            break;
        }
        ui_hide(BASEFORM_36);

        break;
    case KEY_DOWN:
        return false;
		
      break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(BASEFORM_36);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(BASEFORM_37)
.onchange = menu_bu_onchange,
 .onkey = menu_bu_onkey,
  .ontouch = NULL,
};

#if 0
/***************************** 前照灯开关设置 ************************************/
static int menu_flig_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("hlw"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

static int menu_flig_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;
    const char *str;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "hlw";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("head light warning err! %d\n", err);
            break;
        }

        ui_hide(LAYOUT_MN_FLIG_REC);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_FLIG_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_FLIG_REC)
.onchange = menu_flig_onchange,
 .onkey = menu_flig_onkey,
  .ontouch = NULL,
};
#endif
#if 0
/***************************** 车牌号码设置 ************************************/

static u16 __car_num_search_by_mark(const char *mark)
{
    u16 p = 0;
    u16 table_sum = sizeof(text_car_num_table) / sizeof(struct car_num);
    while (p < table_sum) {
        if (!strcmp(mark, text_car_num_table[p].mark)) {
            return p;
        }
        p++;
    }
    return -1;
}
static u8 __car_num_reset(void)
{
    car_num_set_p = 0xff;
    return 0;
}
static u8 __car_num_switch(enum sw_dir dir, const char *mark)
{
    u16 table_sum;
    u16 prev_set_p = car_num_set_p;
    u16 p;

    table_sum = sizeof(text_car_num_table) / sizeof(struct car_num);

    ASSERT(dir == DIR_NEXT || dir == DIR_PREV || dir == DIR_SET);
    switch (dir) {
    case DIR_NEXT:
        if (car_num_set_p >= (table_sum - 1)) {
            car_num_set_p = 0;
        } else {
            car_num_set_p++;
        }
        break;
    case DIR_PREV:
        if (car_num_set_p == 0) {
            car_num_set_p = (table_sum - 1);
        } else {
            car_num_set_p--;
        }
        break;
    case DIR_SET:
        p = __car_num_search_by_mark(mark);
        if (p == (u16) - 1) {
            return -1;
        }
        car_num_set_p = p;

        break;
    }
    if (prev_set_p != 0xff) {
        ui_no_highlight_element_by_id(text_car_num_table[prev_set_p].text_id);
    }
    ui_highlight_element_by_id(text_car_num_table[car_num_set_p].text_id);

    return 0;

}
static u8 __car_num_update_show(u8 pos) /* 更新单个时间控件的时间 */
{
    ui_text_show_index_by_id(text_car_num_table[pos].text_id, text_car_num_table[pos].text_index);
    return 0;
}
static u8 __car_num_set_value(enum set_mod mod, u16 value)
{

    u8 p;
    ASSERT(mod == MOD_ADD || mod == MOD_DEC || mod == MOD_SET);
    switch (mod) {
    case MOD_ADD:
        switch (car_num_set_p) {
        case 0: /* province */
            value = text_car_num_table[car_num_set_p].text_index + 1;
            if (value >= 31) {
                value = 0;
            }
            break;
        case 1: /* town */
        case 2: /* a */
        case 3: /* b */
        case 4: /* c */
        case 5: /* d */
        case 6: /* e */
            value = text_car_num_table[car_num_set_p].text_index + 1;
            if (value >= (('Z' - 'A' + 1) + ('9' - '0' + 1))) {
                value = 0;
            }
            break;
        default:
            ASSERT(0, "mod_add car_num_set_p:%d err!", car_num_set_p);
            break;
        }
        break;
    case MOD_DEC:
        switch (car_num_set_p) {
        case 0: /* province */
            value = text_car_num_table[car_num_set_p].text_index;
            if (value == 0) {
                value = 30;
            } else {
                value--;
            }
            break;
        case 1: /* town */
        case 2: /* a */
        case 3: /* b */
        case 4: /* c */
        case 5: /* d */
        case 6: /* e */
            value = text_car_num_table[car_num_set_p].text_index;
            if (value == 0) {
                value = ('Z' - 'A' + 1) + ('9' - '0' + 1) - 1;
            } else {
                value--;
            }
            break;
        default:
            ASSERT(0, "mod_dec car_num_set_p:%d err!", car_num_set_p);
            break;
        }
        break;
    case MOD_SET:
        switch (car_num_set_p) {
        case 0: /* province */
            ASSERT(value >= 0 && value <= 30, "car num set value err!\n");
            break;
        case 1: /* town */
        case 2: /* a */
        case 3: /* b */
        case 4: /* c */
        case 5: /* d */
        case 6: /* e */
            ASSERT(value >= 0 && value <= (('Z' - 'A' + 1) + ('9' - '0' + 1) - 1));
            break;
        default:
            ASSERT(0, "mod_set car_num_set_p:%d err!", car_num_set_p);
            break;
        }
        break;
    }
    text_car_num_table[car_num_set_p].text_index = value;

    printf("car_num_set p:%d, value:%d\n", car_num_set_p, value);
    __car_num_update_show(car_num_set_p); /* 更新当前位显示 */

    return 0;
}

static u8 __car_num_get(struct car_num_str *num)
{
    num->province = text_car_num_table[0].text_index;
    num->town = text_car_num_table[1].text_index;
    num->a = text_car_num_table[2].text_index;
    num->b = text_car_num_table[3].text_index;
    num->c = text_car_num_table[4].text_index;
    num->d = text_car_num_table[5].text_index;
    num->e = text_car_num_table[6].text_index;
    return 0;
}

static u8 __car_num_set_by_ascii(const char *str)
{
    u16 i;
    u16 province;
    /* ASSERT(strlen(str) == 8, "car num err!"); */

    memcpy((char *)&province, str, 2);
    text_car_num_table[0].text_index = 0;
    for (i = 0; i < (sizeof(province_gb2312) / sizeof(u16)); i++) {
        if (province_gb2312[i] == province) {
            text_car_num_table[0].text_index = i;
            break;
        }
    }
    text_car_num_table[1].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[2]) {
            text_car_num_table[1].text_index = i;
            break;
        }
    }
    text_car_num_table[2].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[3]) {
            text_car_num_table[2].text_index = i;
            break;
        }
    }
    text_car_num_table[3].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[4]) {
            text_car_num_table[3].text_index = i;
            break;
        }
    }
    text_car_num_table[4].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[5]) {
            text_car_num_table[4].text_index = i;
            break;
        }
    }
    text_car_num_table[5].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[6]) {
            text_car_num_table[5].text_index = i;
            break;
        }
    }
    text_car_num_table[6].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[7]) {
            text_car_num_table[6].text_index = i;
            break;
        }
    }
    return 0;

}
int menu_rec_car_num_set(void)
{
    struct intent it;
    int err;
    struct car_num_str num;

    __car_num_get(&num);
    u32 part_a = (province_gb2312[num.province] << 16) |
                 (num_table[num.town] << 8) | num_table[num.a];
    u32 part_b = (num_table[num.b] << 24) | (num_table[num.c] << 16) |
                 (num_table[num.d] << 8)  |  num_table[num.e];

    init_intent(&it);
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "cna";
    it.exdata = part_a;
    err = start_app(&it);
    if (err) {
        printf("num set onkey err! %d\n", err);
        return -1;
    }
    it.data = "cnb";
    it.exdata = part_b;
    err = start_app(&it);

    return 0;
}

static int menu_layout_car_num_set_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        layout_on_focus(layout);
        __car_num_reset();
        break;
    case ON_CHANGE_RELEASE:
        layout_lose_focus(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        __car_num_switch(DIR_SET, "province");
        break;
    default:
        return false;
    }
    return false;
}
static int menu_layout_car_num_set_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;
    const char *str;

    switch (e->value) {
    case KEY_OK:
        __car_num_switch(DIR_NEXT, NULL);

        break;
    case KEY_DOWN:
        __car_num_set_value(MOD_ADD, 0);
        return FALSE;

        break;
    case KEY_UP:
        __car_num_set_value(MOD_DEC, 0);
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_CARNUM_SET_REC);
        menu_rec_car_num_set();

        break;
    default:
        return false;
        break;
    }

    return true;
}

REGISTER_UI_EVENT_HANDLER(LAYOUT_MN_CARNUM_SET_REC)
.onchange = menu_layout_car_num_set_onchange,
 .onkey = menu_layout_car_num_set_onkey,
  .ontouch = NULL,
};




/*
 * (begin)菜单中车牌号码的显示初始化动作 ********************************************
 */
static int text_menu_car_num_province_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[0].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_PROVINCE)
.onchange = text_menu_car_num_province_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int text_menu_car_num_town_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[1].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_TOWN)
.onchange = text_menu_car_num_town_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int text_menu_car_num_a_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[2].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_A)
.onchange = text_menu_car_num_a_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int text_menu_car_num_b_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[3].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_B)
.onchange = text_menu_car_num_b_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int text_menu_car_num_c_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[4].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_C)
.onchange = text_menu_car_num_c_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int text_menu_car_num_d_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[5].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_D)
.onchange = text_menu_car_num_d_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int text_menu_car_num_e_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[6].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_E)
.onchange = text_menu_car_num_e_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
/*
 * (end)---------------------------------------------------------------
 */
#endif


#if 0
static u8 lane_set_flag = 0;
static int menu_lane_set_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        layout_on_focus(layout);
        __car_num_reset();
        break;
    case ON_CHANGE_RELEASE:
        layout_lose_focus(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        __this->vanish_line    = ((db_select("lan") >> 16) & 0x0000ffff) * LCD_DEV_HIGHT / 352;
        __this->car_head_line = (db_select("lan") & 0x0000ffff) * LCD_DEV_HIGHT / 352;
        if (__this->vanish_line < 30) {
            __this->vanish_line = 30;
        }
        if (__this->car_head_line > LCD_DEV_HIGHT - 30 || __this->car_head_line < __this->vanish_line + 30) {
            __this->car_head_line = LCD_DEV_HIGHT - 30 ;
        }
        lane_set_flag = 0;
        break;
    default:
        return false;
    }
    return false;
}

#define LANE_LINSE_STEP_SMALL  3
#define LANE_LINSE_STEP_BIG    30

static int menu_lane_set_onkey(void *ctr, struct element_key_event *e)
{
    switch (e->value) {
    case KEY_DOWN:
        if (!(lane_set_flag & 0x02)) {
            lane_set_flag |= 0x02;
            send_lane_det_setting_msg("lane_set_close", 0);
        }
        if ((lane_set_flag & 0x01) == 0) {
            if (e->event == KEY_EVENT_CLICK) {
                __this->vanish_line += LANE_LINSE_STEP_SMALL;
            } else if (e->event == KEY_EVENT_HOLD) {
                __this->vanish_line += LANE_LINSE_STEP_BIG;
            }
            __this->vanish_line = __this->vanish_line > __this->car_head_line - 30 ? __this->car_head_line - 30 : __this->vanish_line;
            ui_hide(TEXT_LANE_TOP);
            ui_show(TEXT_LANE_TOP);
        } else if ((lane_set_flag & 0x01) == 1) {
            if (e->event == KEY_EVENT_CLICK) {
                __this->car_head_line += LANE_LINSE_STEP_SMALL;
            } else if (e->event == KEY_EVENT_HOLD) {
                __this->car_head_line += LANE_LINSE_STEP_BIG;
            }
            __this->car_head_line = __this->car_head_line > LCD_DEV_HIGHT - 30 ? LCD_DEV_HIGHT - 30 : __this->car_head_line;
            ui_hide(TEXT_LANE_BOTTOM);
            ui_show(TEXT_LANE_BOTTOM);
        }
        break;
    case KEY_UP:
        if (!(lane_set_flag & 0x02)) {
            lane_set_flag |= 0x02;
            send_lane_det_setting_msg("lane_set_close", 0);
        }
        if ((lane_set_flag & 0x01) == 0) {
            if (e->event == KEY_EVENT_CLICK) {
                __this->vanish_line -= LANE_LINSE_STEP_SMALL;
            } else if (e->event == KEY_EVENT_HOLD) {
                __this->vanish_line -= LANE_LINSE_STEP_BIG;
            }
            __this->vanish_line = __this->vanish_line < 30 ? 30 : __this->vanish_line;
            ui_hide(TEXT_LANE_TOP);
            ui_show(TEXT_LANE_TOP);
        } else if ((lane_set_flag & 0x01) == 1) {
            if (e->event == KEY_EVENT_CLICK) {
                __this->car_head_line -= LANE_LINSE_STEP_SMALL;
            } else if (e->event == KEY_EVENT_HOLD) {
                __this->car_head_line -= LANE_LINSE_STEP_BIG;
            }
            __this->car_head_line = __this->car_head_line < __this->vanish_line + 30 ? __this->vanish_line + 30 : __this->car_head_line;
            ui_hide(TEXT_LANE_BOTTOM);
            ui_show(TEXT_LANE_BOTTOM);
        }
        break;
    case KEY_OK:
        if (lane_set_flag & 0x02) {
            lane_set_flag &= 0x01;
            int top = __this->vanish_line * 352 / LCD_DEV_HIGHT;
            int bottom = __this->car_head_line * 352 / LCD_DEV_HIGHT;
            menu_lane_det_set(top << 16 | bottom);
            printf("lane set top=%d , bottom=%d\n", top, bottom);

            send_lane_det_setting_msg("lane_set_open", top << 16 | bottom);
        }
        lane_set_flag = !lane_set_flag;
        break;
    case KEY_MODE:
        if (lane_set_flag & 0x02 || lane_set_flag == 0) {
            int top = __this->vanish_line * 352 / LCD_DEV_HIGHT;
            int bottom = __this->car_head_line * 352 / LCD_DEV_HIGHT;
            menu_lane_det_set(top << 16 | bottom);
            printf("lane set top=%d , bottom=%d\n", top, bottom);
        }
        sys_key_event_takeover(false, false);
        close_set_lane_page(NULL);
        break;
    }
    return true;
}

REGISTER_UI_EVENT_HANDLER(LAYOUT_LANE_REC)
.onchange = menu_lane_set_onchange,
 .onkey = menu_lane_set_onkey,
  .ontouch = NULL,
};

static int lane_set_txt_top_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct draw_context *dc = (struct draw_context *)arg;

    switch (e) {
    case ON_CHANGE_SHOW:
        dc->rect.top = __this->vanish_line;
        dc->draw.top = dc->rect.top;
        break;
    default:
        return false;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_LANE_TOP)
.onchange = lane_set_txt_top_onchange,
};
static int lane_set_txt_bottom_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct draw_context *dc = (struct draw_context *)arg;

    switch (e) {
    case ON_CHANGE_SHOW:
        dc->rect.top = __this->car_head_line - 30;
        dc->draw.top = dc->rect.top;
        break;
    default:
        return false;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_LANE_BOTTOM)
.onchange = lane_set_txt_bottom_onchange,
};

#endif


#endif
