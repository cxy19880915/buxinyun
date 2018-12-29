#include "a7196reg.h"
#include "device/rf_dev.h"
#include "app_config.h"

#include "asm/crc16.h"

#ifdef CONFIG_BOARD_BBM_SENDER_BOARD
#define A7196_RX_SW_PORT  IO_PORTG_08
#define A7196_TX_SW_PORT  IO_PORTG_09
#else
#define A7196_RX_SW_PORT  IO_PORTB_11
#define A7196_TX_SW_PORT  IO_PORTB_12
#endif



struct _a7196_config {
    u8 online;
    u8 mode;
    u8 state;
    u16 width;
    u16 height;
    OS_SEM rx_sem;
    /* OS_SEM rx_ex_sem; */
    OS_SEM tx_sem;
    /* OS_SEM tx_ex_sem; */
    spinlock_t lock;
    struct list_head wlist_head;
    struct lbuff_head  *plbuf;//写入缓冲
    struct rf_fh *fb;
    u8 *data_buf;
    void *spi_fd;
    void *fd;

};
static struct _a7196_config rf_cfg;
#define __this  (&rf_cfg)

static void *port_wakeup_hdl;
/* #define FIFO_LEN  512 */
/* static u8 rec_buf[FIFO_LEN]; */
/* static u8 send_buf[FIFO_LEN]; */
/* #define BUFFER_LEN  512 */
#define BUFFER_LEN  64
static u8 rec_buf[BUFFER_LEN];
static u8 send_buf[BUFFER_LEN];
static const u8  BitCount_Tab[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
/* const u8  ID_Tab[8] = {0xa4, 0x75, 0xC5, 0x2A, 0xC7, 0x33, 0x45, 0xEA}; //ID code */
static const u8  ID_Tab[8] = {0x34, 0x75, 0xC5, 0x2A, 0xC7, 0x33, 0x45, 0xEA}; //ID code
static const u8  KeyData_Tab[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //keyData code
static const u8  FCB_Tab[20] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //FCB code
static const u8  PageTab[12] = {0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xA0, 0xB0}; //page select
static const u8  PN9_Tab[] = {
    0xFF, 0x83, 0xDF, 0x17, 0x32, 0x09, 0x4E, 0xD1,
    0xE7, 0xCD, 0x8A, 0x91, 0xC6, 0xD5, 0xC4, 0xC4,
    0x40, 0x21, 0x18, 0x4E, 0x55, 0x86, 0xF4, 0xDC,
    0x8A, 0x15, 0xA7, 0xEC, 0x92, 0xDF, 0x93, 0x53,
    0x30, 0x18, 0xCA, 0x34, 0xBF, 0xA2, 0xC7, 0x59,
    0x67, 0x8F, 0xBA, 0x0D, 0x6D, 0xD8, 0x2D, 0x7D,
    0x54, 0x0A, 0x57, 0x97, 0x70, 0x39, 0xD2, 0x7A,
    0xEA, 0x24, 0x33, 0x85, 0xED, 0x9A, 0x1D, 0xE0
};


const u8 code_a7196config[] = {
#if 10
    0x00,		//MODE_REG
    0x62,		//MODECTRL_REG
    0x00,		//CALIBRATION_REG
    0x3F,		//FIFO1_REG
    0x00,		//FIFO2_REG
    0x00,		//FIFO_REG
    0x00,		//IDCODE_REG
    0x00,		//RCOSC1_REG
    0x00,		//RCOSC2_REG
    0x0C,		//RCOSC3_REG
    0x00,		//CKO_REG
    0x01,		//GIO1_REG
    /* 0x00,		//GIO1_REG 				 */
    0x00,		//GIO2_REG
    0xEF,		//DATARATE_REG
    0x5A,		//PLL1_REG
    0x16,		//PLL2_REG
    0x64,		//PLL3_REG
    0x00,		//PLL4_REG
    0x02,		//PLL5_REG
    0x3C,		//CHGROUP1_REG
    0x78,		//CHGROUP2_REG
    0x2F,		//TX1_REG  				//0x2F Gaussian OFF 6F ON
    0x40,		//TX2_REG
    0x18,		//DELAY1_REG
    0x40,		//DELAY2_REG
    0x72,		//RX_REG
    0xFC,		//RXGAIN1_REG
    0xCA,		//RXGAIN2_REG
    0xFD,		//RXGAIN3_REG ENABLE RSSI
    /* 0xFC,		//RXGAIN3_REG			 */
    0xCA,		//RXGAIN4_REG
    0x00,		//RSSI_REG
    0xF1,		//ADC_REG
    /* 0x04,		//CODE1_REG id 4 bytes			   */
    0x0C,		//CODE1_REG
    0x0F,		//CODE2_REG
    0x2A,		//CODE3_REG
    0x45,		//IFCAL1_REG
    0x51,		//IFCAL2_REG
    0xCF,		//VCOCCAL_REG
    0xD0,		//VCOCAL1_REG
    0x80,		//VCOCAL2_REG
    0xB0,		//VCODEVCAL1_REG
    0x36,		//VCODEVCAL2_REG
    0x00,		//DASP_REG
    0xFF,		//VCOMODDELAY_REG
    0x70,		//BATTERY_REG
    0x7F,		//TXTEST_REG
    0x57,		//RXDEM1_REG
    0x74,		//RXDEM2_REG
    0xF3,		//CPC1_REG
    0x33,		//CPC2_REG
    0x4D,		//CRYSTALTEST_REG
    0x19,		//PLLTEST_REG
    0x0A,		//VCOTEST_REG
    0x00,		//RFANALOG_REG
    0x00,		//KEYDATA_REG
    0xB7,		//CHSELECT_REG
    0x00,		//ROMP_REG
    0x00,		//DATARATECLOCK
    0x00,		//FCR_REG
    0x00,		//ARD_REG
    0x00,		//AFEP_REG
    0x00,		//FCB_REG
    0x00,		//KEYC_REG
    0x00 		//USID_REG

#endif
};

const u8 code_a7196_Addr2A_Config[] = {
    0x3E, 	//page0,
    0x11, 	//page1,
    0xF0, 	//Page2,
    0x80, 	//page3,
    0x80, 	//page4,
    0x08, 	//page5,
    0x82, 	//page6,
    0xC0, 	//page7,
    0x00, 	//page8,
    0x4C, 	//page9,
    0x00, 	//pageA,
    0x00, 	//pageB,
};

const u8 code_a7196_Addr38_Config[] = {

#if 10
    0x00, 	//page0,
    0x00, 	//page1,
    0x30, 	//page2,
    0xB4, 	//page3,
    0x20, 	//page4,

#endif
};
void A7196_WriteReg(u8 addr, u8 dataByte)
{
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 0);
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, addr);
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, dataByte);
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 1);
}

u8 A7196_ReadReg(u8 addr)
{
    u8 rdata = 0;
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 0);
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, addr | 0x40);
    dev_read(__this->spi_fd, &rdata, 1);
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 1);
    return rdata;
}

/*********************************************************************
** A7196_ReadReg_Page
*********************************************************************/
u8 A7196_ReadReg_Page(u8 addr, u8 page)
{
    u8 tmp;

    A7196_WriteReg(RFANALOG_REG, (code_a7196config[RFANALOG_REG] & 0x0F) | PageTab[page]); //page select
    tmp = A7196_ReadReg(addr);
    return tmp;
}

/*********************************************************************
** A7196_WriteReg_Page
*********************************************************************/
void A7196_WriteReg_Page(u8 addr, u8 wbyte, u8 page)
{
    A7196_WriteReg(RFANALOG_REG, (code_a7196config[RFANALOG_REG] & 0x0F) | PageTab[page]); //page select
    A7196_WriteReg(addr, wbyte);
}

static void Rf_WriteBuf(u8 addr, u8 *buf, u32 len)
{
    int err = 0;
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 0);
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, addr);
    err = dev_write(__this->spi_fd, buf, len);
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 1);
}
static void Rf_ReadBuf(u8 addr, u8 *rdata, u32 len)
{
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 0);
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, addr | 0x40);
    dev_read(__this->spi_fd, rdata, len);
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 1);
}
/*********************************************************************
** Strobe Command
*********************************************************************/
static void StrobeCmd(u8 cmd)
{
    int err;
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 0);
    err = dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, cmd);
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 1);
}
static void switch_TXRX(u8 type)
{
#if 10
    if (type) { //发送
        gpio_direction_output(A7196_RX_SW_PORT, 1);
        gpio_direction_output(A7196_TX_SW_PORT, 0);

    } else { //收
        gpio_direction_output(A7196_RX_SW_PORT, 0);
        gpio_direction_output(A7196_TX_SW_PORT, 1);
    }
#endif
}
/*********************************************************************
** setCH
*********************************************************************/
void setCH(u8 ch)
{
    A7196_WriteReg(PLL1_REG, ch); //RF freq = RFbase + (CH_Step * ch)
}


/*********************************************************************
** CHGroupCal
*********************************************************************/
static u8 CHGroupCal(u8 ch)
{
    u8 tmp, ret = 0;
    u8 vb, vbcf, vcb, vccf, adag;
    u8 deva, adev;

    A7196_WriteReg(PLL1_REG, ch);
    A7196_WriteReg(CALIBRATION_REG, 0x1C);
    do {
        tmp = A7196_ReadReg(CALIBRATION_REG) & 0x1C;
    } while (tmp);

    //for check
    tmp = A7196_ReadReg(VCOCCAL_REG);
    vcb = tmp & 0x0F;
    vccf = (tmp >> 4) & 0x01;

    tmp = A7196_ReadReg(VCOCAL1_REG);
    vb = tmp & 0x07;
    vbcf = (tmp >> 3) & 0x01;

    tmp = A7196_ReadReg(VCOCAL2_REG);
    adag = tmp;
    tmp = A7196_ReadReg(VCODEVCAL1_REG);
    deva = tmp;

    tmp = A7196_ReadReg(VCODEVCAL2_REG);
    adev = tmp;

    if (vbcf || vccf) {
        ret = 1;
    }
    /* Err_State();//error    */
    return ret;
}


/*********************************************************************
** calibration
*********************************************************************/
u8 A7196_Cal(void)
{
    u8 tmp, tmp1, ret = 0;
    u8 rhc, rlc, fb, fbcf, fcd, rcrs, rcts;
    u16 rct;
    u8 i;
    StrobeCmd(CMD_PLL); //calibration @PLL state

    //IF,RSSI,RC procedure
    A7196_WriteReg(CALIBRATION_REG, 0x23);
    do {
        tmp = A7196_ReadReg(CALIBRATION_REG) & 0x23;
    } while (tmp);


    //calibration VBC,VDC procedure
    /* CHGroupCal(30); //calibrate channel group Bank I */
    /* CHGroupCal(90); //calibrate channel group Bank II */
    /* CHGroupCal(150); //calibrate channel group Bank III */
    if (CHGroupCal(30)) {
        ret = 1;
        printf("\n\n a7196 cal Bank I err \n\n ");
    }
    if (CHGroupCal(90)) {
        ret = 1;
        printf("\n\n a7196 cal Bank II err \n\n ");
    }
    if (CHGroupCal(150)) {
        ret = 1;
        printf("\n\n a7196 cal Bank III err \n\n ");
    }



    StrobeCmd(CMD_STBY); //return to STBY state

    tmp = A7196_ReadReg(IFCAL1_REG);
    printf("\n tmp:0x%x \n", tmp);
    fb = tmp & 0x0F;
    fbcf = (tmp >> 4) & 0x01;


    fcd = A7196_ReadReg(IFCAL2_REG) & 0x1F;

    rhc = A7196_ReadReg(RXGAIN2_REG);
    rlc = A7196_ReadReg(RXGAIN3_REG);

    tmp = A7196_ReadReg_Page(0x2A, 8);
    rcrs = tmp & 0x0F;
    tmp1 = (tmp & 0xF0) >> 4;

    tmp = A7196_ReadReg_Page(0x2A, 9);
    rcts = (tmp & 0x03) << 4 | tmp1;
    tmp1 = (tmp & 0xF0) >> 4;

    tmp = A7196_ReadReg_Page(0x2A, 10);
    rct = tmp | ((tmp1) * 256);

    if (fbcf) {
        ret = 1;
        printf("\n\n a7196 cal fbcf err fb:%d;fcd 0x%x \n\n ", fb, fcd);
    }
    /* Err_State(); */
    if (ret) {
        printf("\n\n a7196 cal err \n\n ");
    }
    return ret;
}



/*********************************************************************
** Write A7196_KeyData
*********************************************************************/
void A7196_KeyData(void)
{
    u8 i;
    u8 addr;

    addr = KEYDATA_REG; //send address 0x06, bit cmd=0, r/w=0
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 0);
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, addr);
    /* for (i=0; i < 16; i++) */
    /* ByteSend(KeyData_Tab[i]); */
    dev_write(__this->spi_fd, KeyData_Tab, sizeof(KeyData_Tab));
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 1);
}

/*********************************************************************
** Write A7196_FCB
*********************************************************************/
void A7196_FCB(void)
{
    u8 i;
    u8 addr;

    addr = FCB_REG; //send address 0x06, bit cmd=0, r/w=0
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 0);
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, addr);
    dev_write(__this->spi_fd, FCB_Tab, sizeof(FCB_Tab));
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 1);
}

/*********************************************************************
** A7196_Config
*********************************************************************/
void a7196_config(void)
{
    u8 i;

    //0x00 mode register, for reset
    //0x05 fifo data register
    //0x06 id code register
    //0x3F USID register, read only
    //0x36 key data, 16 bytes
    //0x3D FCB register,4 bytes

    for (i = 0x01; i <= 0x02; i++) {
        A7196_WriteReg(i, code_a7196config[i]);
    }

    /* SCS = 0; */
    /* ByteSend(0x03); */
    /* ByteSend(0x3F); */
    /* ByteSend(0x00); */
    /* SCS = 1; */
    u8 cmd_fifo[3] = {0x03, 0x3f, 0x00};
    /* u8 cmd_fifo[3] = {0x03, 0xff, 0x01}; */
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 0);
    dev_write(__this->spi_fd, cmd_fifo, 3);
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 1);

    A7196_WriteReg(0x04, code_a7196config[0x04]);

    for (i = 0x07; i <= 0x29; i++) {
        A7196_WriteReg(i, code_a7196config[i]);
    }

    for (i = 0; i <= 11; i++) { //0x2A DAS
        A7196_WriteReg_Page(0x2A, code_a7196_Addr2A_Config[i], i);
    }
#if 10
    A7196_WriteReg_Page(0x2A, code_a7196_Addr2A_Config[9], 9);
    A7196_WriteReg(0x2A, code_a7196_Addr2A_Config[9]);

    A7196_WriteReg_Page(0x2A, code_a7196_Addr2A_Config[10], 10);
    A7196_WriteReg(0x2A, code_a7196_Addr2A_Config[10]);
#endif
    for (i = 0x2B; i <= 0x35; i++) {
        A7196_WriteReg(i, code_a7196config[i]);
    }

    A7196_KeyData();

    A7196_WriteReg(0x37, code_a7196config[0x37]);

    for (i = 0; i < sizeof(code_a7196_Addr38_Config); i++) { //0x38 ROM
        A7196_WriteReg_Page(0x38, code_a7196_Addr38_Config[i], i);
    }
    printf("\n addr38 len %d \n", i);
    for (i = 0x39; i <= 0x3C; i++) {
        A7196_WriteReg(i, code_a7196config[i]);
    }

    A7196_FCB();

    A7196_WriteReg(0x3E, code_a7196config[0x3E]);
    A7196_WriteReg(0x3F, code_a7196config[0x3F]);
}

void a7196_writeID(void)
{
    u8 i;
    u8 id_code[8];
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 0);
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, IDCODE_REG);
    dev_write(__this->spi_fd, ID_Tab, sizeof(ID_Tab));
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 1);

    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 0);
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, IDCODE_REG | 0x40);
    dev_read(__this->spi_fd, id_code, 8);
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 1);
    for (i = 0; i < 8; i++) {
        printf(" %x ;", id_code[i]);
    }
}

/************************************************************************
**  Reset_RF
************************************************************************/
void a7196_reset(void)
{
    A7196_WriteReg(MODE_REG, 0x00); //reset RF chip
}


/*********************************************************************
** initRF
*********************************************************************/
static u8 initRF(void)
{
    u8 ret = 0;
    u16 i;
    //init io pin
    /* SCS = 1; */
    /* SCK = 0; */
    /* SDIO = 1; */
    /* CKO = 1; */
    /* GIO1 = 1; */
    /* GIO2 = 1; */

    /* dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 1); */
    a7196_reset(); //reset A7105 RF chip
    a7196_writeID(); //write ID code
    a7196_config(); //config A7105 chip
    /* A7196_Cal(); //calibration IF,VCO,VCOC */
    if (A7196_Cal()) {
        ret = 1;
    }
    StrobeCmd(CMD_SLEEP);
    for (i = 0; i < 1000; i++);
    StrobeCmd(CMD_STBY);
    for (i = 0; i < 1000; i++);

    return ret;
}

static void set_a7196_fifo(void)
{
    int err;
    u8 cmd_fifo[3] = {FIFO1_REG, 0x3f, 0x00};
    /* u8 cmd_fifo[3] = {FIFO1_REG, 0xff, 0x01}; */
    /* u8 cmd_fifo[3] = {FIFO1_REG, FIFO_LEN_L, FIFO_LEN_H}; */
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 0);
    err = dev_write(__this->spi_fd, cmd_fifo, 3);
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 1);
}

static u32 rf_buf_free_space(struct rf_fh *fh)
{
    return videobuf_stream_free_space(&fh->video_q);
}

static void *rf_buf_malloc(struct rf_fh *fh, u32 size)
{
    struct videobuf_buffer *b;

    b = videobuf_stream_alloc(&fh->video_q, size);
    if (!b) {
        return NULL;
    }
    b->len = size;
    return b->data;
}

static void *rf_buf_realloc(struct rf_fh *fh, void *buf, int size)
{
    struct videobuf_buffer *b = container_of(buf, struct videobuf_buffer, data);

    b->len = size;
    videobuf_stream_realloc(&fh->video_q, b, size);

    return b->data;
}


static void rf_buf_free(struct rf_fh *fh, void *buf)
{
    struct videobuf_buffer *b = container_of(buf, struct videobuf_buffer, data);

    ASSERT(buf != NULL, "rf_buf_free\n");

    videobuf_stream_free(&fh->video_q, b);
}


static void *rf_buf_ptr(void *buf)
{
    return buf;
}

static u32 rf_buf_size(void *buf)
{
    struct videobuf_buffer *b = container_of(buf, struct videobuf_buffer, data);

    return b->len;
}

static void rf_buf_stream_finish(struct rf_fh *fh, void *buf)
{
    struct videobuf_buffer *b = container_of(buf, struct videobuf_buffer, data);

    videobuf_stream_finish(&fh->video_q, b);
}


static int rf_dev_reqbufs(void *_fh, struct uvc_reqbufs *b)
{
    struct rf_fh *fh = (struct rf_fh *)_fh;


    return videobuf_reqbufs(&fh->video_q, (struct video_reqbufs *)b);
}

static int rf_dev_qbuf(void *_fh, struct video_buffer *b)
{
    struct rf_fh *fh = (struct rf_fh *)_fh;

    return videobuf_qbuf(&fh->video_q, b);
}


static int rf_dev_dqbuf(void *_fh, struct video_buffer *b)
{
    struct rf_fh *fh = (struct rf_fh *)_fh;

    return videobuf_dqbuf(&fh->video_q, b);
}

static int rf_stream_on(void *_fh, int index)
{
    int err;
    u8 channel = 0;
    struct rf_fh *fh = (struct rf_fh *)_fh;

    os_sem_pend(&fh->sem, 0);
    fh->eof = 1;
    fh->drop_frame = 0;

    videobuf_streamon(&fh->video_q, (u8 *)&channel);

    if (fh->streamon) {
        os_sem_post(&fh->sem);
        return 0;
    }

    fh->streamon++;
    os_sem_post(&fh->sem);
    return err;
}

static int rf_stream_off(void *_fh, int index)
{
    int err;
    struct rf_fh *fh = (struct rf_fh *)_fh;
    u32 time = jiffies + msecs_to_jiffies(10);

    os_sem_pend(&fh->sem, 0);
    if (--fh->streamon) {
        os_sem_post(&fh->sem);
        return 0;
    }


    fh->streamoff = 1;
    while (fh->streamoff) {
        if (time_after(jiffies, time)) {
            break;
        }
    }

    if (fh->buf) {
        rf_buf_free(fh, fh->buf);
        fh->buf = 0;
        fh->b_offset = 0;
    }
    err = videobuf_streamoff(&fh->video_q, 0);

    os_sem_post(&fh->sem);
    return err;

}


static int a7196_gpio_interrupt_enable(u8 event)
{
    if (event > 17) {
        return -1;
    }
    WKUP_CON0 |= BIT(event);
    return 0;
}
static int a7196_gpio_interrupt_disable(u8 event)
{
    if (event > 17) {
        return -1;
    }
    WKUP_CON0 &= ~BIT(event);
    return 0;
}

static void rf_isr(void *priv)
{
    struct _a7196_config *para = priv;
    if (WKUP_CON3 & BIT(0)) {
        WKUP_CON2 |= BIT(0);
        if (para->mode == 2) {
            os_sem_post(&para->tx_sem);
        } else {
            os_sem_post(&para->rx_sem);
        }
    }
}

static void rf_irq_config(struct _a7196_config *para)
{

    IOMC2 &= ~(0x7f);
    /* IOMC2 |=  0x1A;  //PORTB_10 */
    IOMC2 |=  0x17;  //PORTB_07
    /* IOMC2 |=  0x16;  //PORTB_06 */

    WKUP_CON1 |= BIT(0);//down edge
    WKUP_CON2 |= BIT(0);//clear pending
    WKUP_CON0 |= BIT(0);//ENABLE
    port_wakeup_hdl = port_wakeup_reg(rf_isr, para);
}


int a7196_receive_frame(void)
{
    u32 receive_len, ret_len, frame_len = 0;
    u8 *frame_data;
    u8 temp = 0 ;
    u16 crc_16, Rcrc_16;
    int ret;
    u16 err;
    struct rf_frm_tag frame_info = {0};
    struct _cmd_pack_head cmd_pack_head = {0};
    struct _data_pack_head data_pack_head = {0};
    u8 tmp;


    /* StrobeCmd(CMD_RFR); // */
    /* Rf_ReadBuf(FIFO_REG, rec_buf, BUFFER_LEN); */
    /* a7196_gpio_interrupt_enable(0); */
    memset(rec_buf, 0, BUFFER_LEN);
    os_sem_set(&__this->rx_sem, 0);
    /* StrobeCmd(CMD_RFR); //reset rx FIFO pointer */
start_r:
    StrobeCmd(CMD_RX); //entry rx
    err = os_sem_pend(&__this->rx_sem, 100);
    if (err != OS_NO_ERR) {
        /* put_u8hex(A7196_ReadReg(MODECTRL_REG));  */
        /* put_u8hex(A7196_ReadReg(MODE_REG));  */
        put_u8hex(A7196_ReadReg(RSSI_REG));
        puts("\n--\n");
        /* goto start_r;	 */
        return 1;
    }
    puts("O\n");
    Rf_ReadBuf(FIFO_REG, rec_buf, BUFFER_LEN);
    put_buf(rec_buf, 10);
    memcpy(&Rcrc_16, &rec_buf[BUFFER_LEN - 2], 2);
    crc_16 = CRC16(rec_buf, BUFFER_LEN - 2);
    if (crc_16 != Rcrc_16) {
        /* puts("C1\n"); */
        return 0;
        /* goto start_r; */
    }
    if (rec_buf[0] != CMD_ID || rec_buf[1] != CMD_START) {
        /* puts("NS\n"); */
        goto start_r;
    }

    memcpy(&cmd_pack_head, rec_buf, sizeof(struct _cmd_pack_head));
    memcpy(&frame_info, &rec_buf[sizeof(struct _cmd_pack_head)], sizeof(struct rf_frm_tag));
    frame_len = frame_info.frm_len;
    receive_len = 0;
    /* frame_data = rf_buf_malloc(__this->fb, frame_len); */
    put_u32hex(frame_info.frm_seq);
    return 0;
}

int	a7196_send_frame(u8 *frame_data, u32 len)
{

    static u16 i, cnt, packet_len, f_seq = 0, p_seq = 0;
    u8 remain, ret;
    u16 err;
    u32 send_len = 0;
    u16 crc_16;
    struct _data_pack_head data_pack_head = {0};
    struct _cmd_pack_head  cmd_pack_head = {0};
    struct rf_frm_tag frame_info = {0};

#if 1
    memcpy(send_buf, PN9_Tab, 64);
    StrobeCmd(CMD_TFR); //
    os_sem_set(&__this->tx_sem, 0);
    Rf_WriteBuf(FIFO_REG, send_buf, BUFFER_LEN);
    StrobeCmd(CMD_TX); //entry tx & transmit
    err = os_sem_pend(&__this->tx_sem, 10);
    if (err != OS_NO_ERR) {
        puts("E");
        return 1;
    }
    puts("K");
    return 0;


#endif

    /* StrobeCmd(CMD_STBY); */
    packet_len = BUFFER_LEN - sizeof(struct _data_pack_head) - 2;

    //填充起始命令  帧头信息
    cmd_pack_head.code = CMD_ID;
    cmd_pack_head.cmd_num = CMD_START;
    cmd_pack_head.p_len  = sizeof(struct rf_frm_tag);
    frame_info.frm_seq = (++f_seq);
    frame_info.frm_len = len;
    frame_info.type = 0;
    frame_info.crc = CRC16(frame_data, len);
    memset(send_buf, 11, BUFFER_LEN);
    memcpy(send_buf, &cmd_pack_head, sizeof(struct _cmd_pack_head));//命令信息头
    memcpy(&send_buf[sizeof(struct _cmd_pack_head)], &frame_info, sizeof(struct rf_frm_tag));//命令参数是
    /* memcpy(send_buf,PN9_Tab,64); */
    crc_16 = CRC16(send_buf, BUFFER_LEN - 2);
    memcpy(&send_buf[BUFFER_LEN - 2], &crc_16, 2);//crc

    /* a7196_gpio_interrupt_enable(0); */
    StrobeCmd(CMD_TFR); //
    os_sem_set(&__this->tx_sem, 0);
    Rf_WriteBuf(FIFO_REG, send_buf, BUFFER_LEN);
    StrobeCmd(CMD_TX); //entry tx & transmit
#if 1
    err = os_sem_pend(&__this->tx_sem, 10);
    if (err != OS_NO_ERR) {
        /* StrobeCmd(CMD_STBY); */
        puts("E");
        return 1;
    }
#endif
    /* printf("0x%x,0x%x \n",packet_len,BUFFER_LEN); */
    puts("K");
    /* put_u8hex(A7196_ReadReg(MODE_REG));  */
    return 0;
}

static int get_lbuf_data(void)
{
    int err = 0;
    static u8 i = 0;
    struct rf_videobuf *p_lbu = NULL;
    p_lbu = lbuf_pop(__this->plbuf, 1);
    if (p_lbu) {
        if (i % 1 == 0) {
            a7196_send_frame(p_lbu->data, p_lbu->len);
        }
        i++;
        lbuf_free(__this->plbuf, p_lbu);
        err = 1;
        /* puts("K"); */
    }
    return err;
}
static void a7196_task(void *p)
{

    struct _a7196_config *rf_para = p;
    while (1) {
        if (rf_para->mode == 2) {
            if (get_lbuf_data()) {
                /* os_time_dly(1); */
            } else {
                os_time_dly(1);
            }
        } else if (rf_para->mode == 1) {
            if (((struct rf_fh *)rf_para->fb)->streamon) {
                if (a7196_receive_frame() == 0) {
                    /* os_time_dly(1); */
                }
            } else {
                os_time_dly(1);
            }
        }
    }
}




static int a7196_close(struct device *device)
{
    if (__this->mode == 2) {
        lbuf_clear(__this->plbuf);
        if (__this->data_buf) {
            free(__this->data_buf);
        }
    } else {
        videobuf_queue_release(&((struct rf_fh *)__this->fb)->video_q);
        if (__this->fb) {
            free(__this->fb);
        }
    }
    puts("\n a7196 close \n");
    __this->mode = 0;
    return 0;
}


static int a7196_ioctl(struct device *device, u32 cmd, u32 arg)
{

    struct rf_fh *fh = (struct rf_fh *)device->private_data;
    u8 ch;
    int ret = 0;
    switch (cmd) {
    case RFIOC_REQBUFS:
        ret = rf_dev_reqbufs(fh, (struct rf_reqbufs *)arg);
        break;
    case RFIOC_DQBUF:
        ret = rf_dev_dqbuf(fh, (struct video_buffer *)arg);
        break;
    case RFIOC_QBUF:
        ret = rf_dev_qbuf(fh, (struct video_buffer *)arg);
        break;
    case RFIOC_STREAM_ON:
        ret = rf_stream_on(fh, arg);
        break;
    case RFIOC_STREAM_OFF:
        ret = rf_stream_off(fh, arg);
        break;
    case RFIOC_SET_CH:
        ch = *(u8 *)arg;
        setCH(ch);
        break;
    default:
        break;
    }
    return ret;
}
static int a7196_write(struct device *dev, void *buf, u32 len, u32 addr)
{
    struct rf_videobuf *p = NULL;
    if (__this->mode == 2) { //发送模式
        p = lbuf_alloc(__this->plbuf, len);

        if (!p) {
            printf("\n lbuf alloc err \n");
            return 0;
        }
        p->len = len;
        memcpy(p->data, buf, len);
        lbuf_push(__this->plbuf, p, 1);
    } else {
        puts("\n receive mode \n");
    }
    return 0;
}

static int  a7196_open(const struct dev_node *node,  struct device **device, void *arg)
{
    u8 work_mode = (u8)arg;
    int err;
    struct rf_fh *fh = NULL;
    struct rf_videobuf *p_lbuf = NULL;

    if (!__this->spi_fd) {
        __this->spi_fd = dev_open("spi2", 0);
        if (!__this->spi_fd) {
            printf("\n a7196 spi dev open err\n");
            return -EINVAL;
        }
        /* spi = __this->spi_fd; */
    }
    if (__this->mode) {
        if (__this->mode != ((work_mode) ? 2 : 1)) {
            printf("\n a7196 had config mode %d \n", __this->mode);
            return -EINVAL;
        }
    } else {
        __this->mode = (work_mode) ? 2 : 1;

        if (__this->mode == 2) { //发送模式
            __this->data_buf =	malloc(2 * 1024 * 1024);
            if (!__this->data_buf) {
                printf("malloc err \n");
                return -EINVAL;
            }
            /* INIT_LIST_HEAD(&__this->wlist_head); */
            __this->plbuf =  lbuf_init(__this->data_buf, 2 * 1024 * 1024, 32, sizeof(struct rf_videobuf));
            if (!__this->plbuf) {
                return -EINVAL;
            }
            switch_TXRX(10);
            puts("\n lbuf init sucess 2\n");
            initRF(); //init RF
            StrobeCmd(CMD_STBY);
            setCH(150);
            set_a7196_fifo();//fifo length upto 512 bytes
        } else {

            switch_TXRX(0);
            initRF(); //init RF
            printf("\n--func=%s\n", __FUNCTION__);
            StrobeCmd(CMD_STBY);
            setCH(150);
            set_a7196_fifo();//fifo length upto 512 bytes
            fh = (struct rf_fh *)zalloc(sizeof(*fh));
            videobuf_queue_init(&fh->video_q, 32);
            fh->open = 1;
            os_sem_create(&fh->sem, 0);
            os_sem_post(&fh->sem);
            *device = &fh->device;
            (*device)->private_data = fh;

        }
        printf("\n work  mode: %x\n", __this->mode);
        gpio_set_pull_up(A7196_TX_SW_PORT, 1);
        gpio_set_pull_up(A7196_RX_SW_PORT, 1);
        gpio_set_pull_down(A7196_TX_SW_PORT, 0);
        gpio_set_pull_down(A7196_RX_SW_PORT, 0);
        os_sem_create(&__this->tx_sem, 0);
        os_sem_create(&__this->rx_sem, 0);
        rf_irq_config(__this);
        __this->fb = fh;
        err = task_create(a7196_task, __this, "rf_task");

    }

    puts("\n a7196 open succes \n");
    return 0;
}



static int  a7196_init(const struct dev_node *node, void *_data)
{

    struct rf_platform_data *pdata = NULL;
    pdata = (struct rf_platform_data *) _data;
    memset(__this, 0x0, sizeof(*__this));
    if (pdata) {
        __this->height = pdata->height;
        __this->width  = pdata->width;
    }
    puts("\n a7196_init \n");
    return 0;
}


static bool a7196_online(const struct dev_node *node)
{

    /* return false; */
    return true;
}


const struct device_operations a7196_dev_ops = {
    .online = a7196_online,
    .init = a7196_init,
    .open = a7196_open,
    .write = a7196_write,
    .ioctl = a7196_ioctl,
    .close = a7196_close,
};

REGISTER_RF_DEVICE(rf_dev) = {
    .name = "a7196",
    .ops = &a7196_dev_ops,
};

