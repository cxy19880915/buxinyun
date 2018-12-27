#ifndef __DSI_H__
#define __DSI_H__

#include "generic/typedef.h"
#include "asm/imd.h"

// #define DSI_DEBUG

//lane config
#define lane_dis 	0x00
#define lane_en  	0x10
#define lane_ex     0x20	// DP/DN exchange enable

#define lane_d0  	0x00
#define lane_d1  	0x01
#define lane_d2  	0x02
#define lane_d3  	0x03
#define lane_clk 	0x04

//dcs command
#define null_packet									0x09
#define set_maximum_return_packet_size 				0x37
#define dcs_short_write_p0 							0x05
#define dcs_short_write_p1 							0x15
#define dcs_long_write    							0x39
#define dcs_short_read_p0   						0x06
#define generic_short_write_p0 						0x03
#define generic_short_write_p1 						0x13
#define generic_short_write_p2 						0x23
#define generic_short_read_p0  						0x04
#define generic_short_read_p1  						0x14
#define generic_short_read_p2  						0x24
#define generic_long_write    						0x29
#define packed_pixel_format_rgb565_16bit			0x0e
#define packed_pixel_format_rgb666_18bit			0x1e
#define packed_pixel_format_rgb666_18bit_loosely	0x2e
#define packed_pixel_format_rgb888_24bit			0x3e

//control
#define vdo_done_isr_en   dsi_sys_con |= BIT(2)
#define vdo_done_isr_dis  dsi_sys_con &=~BIT(2)
#define cmd_done_isr_en   dsi_sys_con |= BIT(3)
#define cmd_done_isr_dis  dsi_sys_con &=~BIT(3)
#define cmd_isr_status    (dsi_sys_con&BIT(3))
#define bta_done_isr_en   dsi_sys_con |= BIT(4)
#define bta_fail_isr_en   dsi_sys_con |= BIT(5)
#define cdet_isr_en       dsi_sys_con |= BIT(6)
#define vdo_done_pnd     (dsi_sys_con &  BIT(9))
#define vdo_done_pnd_clr  dsi_sys_con |= BIT(8)
#define cmd_done_pnd     (dsi_sys_con &  BIT(11))
#define cmd_done_pnd_clr  dsi_sys_con |= BIT(10)
#define bta_done_pnd     (dsi_sys_con &  BIT(13))
#define bta_done_pnd_clr  dsi_sys_con |= BIT(12)
#define bta_fail_pnd     (dsi_sys_con &  BIT(15))
#define bta_fail_pnd_clr  dsi_sys_con |= BIT(14)
#define cdet_pnd         (dsi_sys_con &  BIT(17))
#define cdet_pnd_clr      dsi_sys_con |= BIT(16)

#define dsi_tri_kick  	  dsi_task_con |= BIT(4)
#define dsi_cmd_kick  	  dsi_task_con |= BIT(5)
#define dsi_vdo_kick  	  dsi_task_con |= BIT(6)
#define clock_ready   	 (dsi_bus_con & BIT(5))

#define rx_te   		 (dsi_cmd_con1&BIT(16))
#define rx_ack  		 (dsi_cmd_con1&BIT(17))
#define rx_err  		 (dsi_cmd_con1&BIT(18))
#define rx_eot  		 (dsi_cmd_con1&BIT(19))
#define rx_gsr1 		 (dsi_cmd_con1&BIT(20))
#define rx_gsr2 		 (dsi_cmd_con1&BIT(21))
#define rx_glr  		 (dsi_cmd_con1&BIT(22))
#define rx_dsr1 		 (dsi_cmd_con1&BIT(23))
#define rx_dsr2 		 (dsi_cmd_con1&BIT(24))
#define rx_dlr  		 (dsi_cmd_con1&BIT(25))
#define cmd_err 		 (dsi_cmd_con1&BIT(26))

#define BTA_TIMEOUT		0x00
#define BTA_FAIL		0x01
#define BTA_RX_TE		0x02
#define BTA_RX_ACK		0x03
#define BTA_RX_EOT		0x04
#define BTA_RX_ERR		0x05
#define BTA_CMD_ERR		0x06
#define BTA_RX_GSR1		0x07
#define BTA_RX_GSR2		0x08
#define BTA_RX_GLR		0x09
#define BTA_RX_DSR1		0x0a
#define BTA_RX_DSR2		0x0b
#define BTA_RX_DLR		0x0c
#define BTA_DONE        0x0d

#define _W 0x00
#define _R 0x01
#define DELAY(t) t
#define SIZE(s)  s

struct init_cmd {
    u8 rw;
    u8 delay;
    u8 packet_type;
    u8 size;
    u8 data[0];
};

void dsi_rd_short(u8 *buf, u8 len);
void dsi_rd_long(u8 *buf, u8 len);

void dcs_set_max_packet_size(u8 size);
void dcs_send_short_p0(u8 cmd);
u8   dcs_send_short_p0_bta(u8 cmd);
void dcs_send_short_p1(u8 cmd, u8 val);
u8   dcs_send_short_p1_bta(u8 cmd, u8 val);
void dcs_send_long_packet(u8 *buf, u8 len);
u8   dcs_send_long_packet_bta(u8 *buf, u8 len);
u8   dcs_recv_short_p0_bta(u8 cmd);
u8   dcs_read_parm(u8 cmd, u8 *buf, u8 len);

void generic_set_max_packet_size(u8 size);
void generic_send_short_p1(u8 cmd);
u8   generic_send_short_p1_bta(u8 cmd);
void generic_send_short_p2(u8 cmd, u8 val);
u8   generic_send_short_p2_bta(u8 cmd, u8 val);
void generic_send_long_packet(u8 *buf, u8 len);
u8   generic_send_long_packet_bta(u8 *buf, u8 len);
u8   generic_recv_short_p1_bta(u8 cmd);
u8   generic_read_parm(u8 cmd, u8 *buf, u8 len);

void dcs_set_debug(u8 onoff);
void dsi_video_kickstart();
void dsi_dev_init(struct mipi_dev *dev_t);

extern void delay_2ms(u32 cnt);
#define delay_10us(t) //未定义
#define delay_10ms(t) delay_2ms(5*t)

#endif
