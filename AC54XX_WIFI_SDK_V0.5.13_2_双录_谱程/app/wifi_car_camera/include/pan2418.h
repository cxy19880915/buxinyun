#include "printf.h"
#include "cpu.h"
#include "asm/spi.h"
#include "timer.h"
#include "app_config.h"
#include "event.h"
#include "system/includes.h"
#include "generic/gpio.h"
#include "generic/typedef.h"
#include "asm/port_waked_up.h"

#include "lbuf.h"

#define RF_TIMEOUT   1000000

#define RF_PAGE_SIZE 256

#define DUMMY_BYTE  0xff

//register map
#define		CONFIG        			0x00
#define		EN_AA					0x01
#define		EN_RXADDR				0x02
#define		SETUP_AW				0x03
#define		SETUP_RETR				0x04
#define		RF_CH					0x05
#define		RF_SETUP				0x06
#define		STATUS					0x07
#define		OBSERVE_TX				0x08
#define		RPD			      		0x09
#define		RX_ADDR_P0				0x0A
#define		RX_ADDR_P1				0x0B
#define		RX_ADDR_P2				0x0C
#define		RX_ADDR_P3				0x0D
#define		RX_ADDR_P4				0x0E
#define		RX_ADDR_P5				0x0F
#define		TX_ADDR				    0x10
#define		RX_PW_P0				0x11
#define		RX_PW_P1				0x12
#define		RX_PW_P2				0x13
#define		RX_PW_P3				0x14
#define		RX_PW_P4				0x15
#define		RX_PW_P5				0x16
#define		FIFO_STATUS			    0x17
#define		PA_RAMP_CTL			    0x18
#define		DEM_CAL				    0x19
#define     RF_CAL2				    0x1A
#define     DEM_CAL2				0x1B
#define		DYNPD					0x1C
#define		FEATURE				    0x1D
#define		RF_CAL					0x1E
#define		BB_CAL					0x1F

//strobe command
#define		R_REGISTER				0x00        //SPI read RF data
#define		W_REGISTER				0x20        //SPI write RF data
#define		R_RX_PAYLOAD			0x61        //Read RX Payload
#define		W_TX_PAYLOAD			0xA0        //Write TX Payload
#define		FLUSH_TX				0xE1        //Flush RX FIFO
#define		FLUSH_RX				0xE2        //Flush TX FIFO
#define		REUSE_TX_PL			    0xE3        //Reuse TX Payload
#define		ACTIVATE				0x50        //ACTIVATE
#define		DEACTIVATE				0x50        //DEACTIVATE
#define		R_RX_PL_WID			    0x60        //Read width of RX data
#define		W_ACK_PAYLOAD		    0xA8        //Data with ACK
#define		W_TX_PAYLOAD_NOACK	    0xB0        //TX Payload no ACK Request
#define		CE_FSPI_ON	        	0xFD        // CE HIGH
#define		CE_FSPI_OFF	        	0xFC        // CE LOW
#define		RST_FSPI	         	0x53        // RESET
#define		NOP_N					0xFF

//mode
#define     DEFAULT_CHANNEL    	   110//73			//初始化频点: 2473 MHz
#define     PAYLOAD_WIDTH          	512
#define     DR_6M                   0xc0		//通信速率6Mbps

#define     DATA_RATE               DR_6M       //传输速率6Mbps

#define		FAST_MODE_EN			1			//置1使能快速发射模式，只对发射有效，置0禁止快速发射模式
#define		FEC_EN					0			//置1使能FEC模式，置0禁止FEC模式,FEC为前向纠错，可以提高接收灵敏度，但数据发送时间会增加一倍
#define		SCRAMBLE_EN				1			//置1使能扰码功能，置0禁止扰码功能，当数据为长0/1时，增加扰码功能可以提高RF性能

