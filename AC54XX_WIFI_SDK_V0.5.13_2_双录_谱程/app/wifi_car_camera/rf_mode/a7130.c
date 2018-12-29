#include "a7130.h"
#include "device/rf_dev.h"
#include "asm/crc16.h"

#include "circular_buf.h"
#include "app_config.h"

#ifdef CONFIG_BOARD_BBM_SENDER_BOARD
#define A7130_RX_SW_PORT  IO_PORTG_08
#define A7130_TX_SW_PORT  IO_PORTG_09
#else
#define A7130_RX_SW_PORT  IO_PORTB_11
#define A7130_TX_SW_PORT  IO_PORTB_12
#endif

#pragma pack(1)
struct rf_frm_head {
    u32 mark;
    u32 frm_len;
    u32 frm_seq;//seq自加
    u8 type;//视频类型
    u16  p_len;
    u16  p_seq;
    u8 end_mark;
    u16 crc;
};
#pragma pack()

#pragma pack(1)
struct rf_frm_head2 {
    u16 mark;
    u32 frm_len;
    u16 frm_seq;//seq自加
    u8 type;//视频类型
    u16  p_len;
    u16  p_seq;
    u16 crc;
};
#pragma pack()

struct a7130_config {
    u8 online;
    u8 mode;
    u8 state;
    u16 width;
    u16 height;
    OS_SEM rx_sem;
    OS_SEM rx_ex_sem;
    OS_SEM tx_sem;
    OS_SEM tx_ex_sem;
    spinlock_t lock;
    cbuffer_t cbuf_data_head;
    u8 *p_cbuf_data;
    struct list_head wlist_head;
    struct lbuff_head  *plbuf;
    struct rf_fh *fb;
    /* struct rf_frm_head frame_head; */
    /* struct rf_frm_head2 frame_head2; */
    struct rf_frm_tag frame_info;
    u8 *data_buf;
    void *spi_fd;
    void *fd;

};
static struct a7130_config rf_cfg;
#define __this  (&rf_cfg)
static u8 rece_buf[10];
static u8 page_head[64];
static void *port_wakeup_hdl;
static void *port_wakeup_hdl2;

#define CBUF_DATA_LEN  1024*1024
//extern fifo
#if 0
#define FIFO_LEN   0x100
#define FIFO_LEN_L 0xff
#define FIFO_LEN_H 0x00
#else
#define FIFO_LEN   0x40//64
#define FIFO_LEN_L 0x3F
#define FIFO_LEN_H 0x00 //
#endif



#define CMD_LEN   FIFO_LEN
static u8 fifo_buf[FIFO_LEN];
static u8 cmd_buf[CMD_LEN];
static u8 ack_buf[CMD_LEN];




static void StrobeCmd(u8 cmd);

u32 rf_buf_free_space(struct rf_fh *fh)
{
    return videobuf_stream_free_space(&fh->video_q);
}

void *rf_buf_malloc(struct rf_fh *fh, u32 size)
{
    struct videobuf_buffer *b;

    b = videobuf_stream_alloc(&fh->video_q, size);
    if (!b) {
        return NULL;
    }
    b->len = size;

//	printf("video_buf: %x\n", b, b->data);

    return b->data;
}

void *rf_buf_realloc(struct rf_fh *fh, void *buf, int size)
{
    struct videobuf_buffer *b = container_of(buf, struct videobuf_buffer, data);

    b->len = size;
    videobuf_stream_realloc(&fh->video_q, b, size);

    return b->data;
}


void rf_buf_free(struct rf_fh *fh, void *buf)
{
    struct videobuf_buffer *b = container_of(buf, struct videobuf_buffer, data);

    ASSERT(buf != NULL, "rf_buf_free\n");

    videobuf_stream_free(&fh->video_q, b);
}


void *rf_buf_ptr(void *buf)
{
    return buf;
}

u32 rf_buf_size(void *buf)
{
    struct videobuf_buffer *b = container_of(buf, struct videobuf_buffer, data);

    return b->len;
}

void rf_buf_stream_finish(struct rf_fh *fh, void *buf)
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



int get_txrx_status()
{
    return gpio_read(A7130_GIO1_PORT);
}
void set_gio1(void)
{
    gpio_direction_input(A7130_GIO1_PORT);
}

void a7130_spi_cs_low(void)
{
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 0);
}
void a7130_spi_cs_high(void)
{
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SET_CS, 1);
}

static int a7130_gpio_interrupt_enable(u8 event)
{
    if (event > 17) {
        return -1;
    }
    WKUP_CON0 |= BIT(event);
    return 0;
}
static int a7130_gpio_interrupt_disable(u8 event)
{
    if (event > 17) {
        return -1;
    }
    WKUP_CON0 &= ~BIT(event);
    return 0;
}
static int a7130_gpio_interrupt_clear(u8 event)
{
    if (event > 17) {
        return -1;
    }
    WKUP_CON2 |= BIT(event);
    return 0;
}
int a7130_gpio_check_interrupt(u8 event)
{
    if (event > 17) {
        return -1;
    }
    if (BIT(event)&WKUP_CON3) {
        return 1;
    }
    return 0;
}

void a7130_wtr_int_isr(void *priv)
{
    struct a7130_config *para = priv;
    if (a7130_gpio_check_interrupt(0)) {
        a7130_gpio_interrupt_clear(0);
        /* a7130_gpio_interrupt_disable(0); */
        if (para->mode == 2) {
            os_sem_post(&para->tx_sem);
        } else {
            os_sem_post(&para->rx_sem);
        }
    }
}
void a7130_cko_int_isr(void *priv)
{
    struct a7130_config *para = priv;
    if (a7130_gpio_check_interrupt(1)) {
        a7130_gpio_interrupt_clear(1);
        /* a7130_gpio_interrupt_disable(1); */
        if (para->mode == 2) {
            os_sem_post(&para->tx_ex_sem);
        } else {
            os_sem_post(&para->rx_ex_sem);
        }
    }
}

static int a7130_wtr_interrupt_register(struct a7130_config *para)
{
    IOMC2 &= ~(0x7f);
#ifdef CONFIG_BOARD_BBM_SENDER_BOARD
    IOMC2 |= (0x17);   //PORTB_7
#else
    /* IOMC2 |=  0x1B;  //PORTB_11 */
    IOMC2 |= (0x17);   //PORTB_7
#endif
    WKUP_CON1 |= BIT(0);//事件0 下降沿
    /* WKUP_CON1 &= ~BIT(0); */
    a7130_gpio_interrupt_clear(0);
    port_wakeup_hdl = (void *)port_wakeup_reg(a7130_wtr_int_isr, para);
    return 0;
}
static int a7130_cko_interrupt_register(struct a7130_config *para)
{
    IOMC2 &= ~(0x7f00);
#ifdef CONFIG_BOARD_BBM_SENDER_BOARD
    IOMC2 |=  0x1600;  //PORTB_06
#else
    IOMC2 |=  0x1600;  //PORTB_6
    /* IOMC2 |= (0x1700);   //PORTB_7 */
#endif
    /* WKUP_CON1 |= BIT(1);//事件1 下降沿 */
    WKUP_CON1 &= ~BIT(1);
    a7130_gpio_interrupt_clear(1);
    port_wakeup_hdl2 = (void *)port_wakeup_reg(a7130_cko_int_isr, para);
    return 0;
}



void A7130_WriteReg(u8 addr, u8 dataByte)
{
    a7130_spi_cs_low();
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, addr);
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, dataByte);
    a7130_spi_cs_high();
}
u8 A7130_ReadReg(u8 addr)
{
    u8 rdata = 0;
    a7130_spi_cs_low();
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, addr | 0x40);
    /* dev_ioctl(__this->spi_fd,IOCTL_SPI_READ_BYTE,(u32)&rdata); */
    dev_read(__this->spi_fd, &rdata, 1);
    a7130_spi_cs_high();
    return rdata;
}
/*********************************************************************
** A7130_ReadReg_Page
*********************************************************************/
u8 A7130_ReadReg_Page(u8 addr, u8 page)
{
    u8 tmp;

    A7130_WriteReg(RFANALOG_REG, A7130Config[RFANALOG_REG] | PageTab[page]);//page select
    tmp = A7130_ReadReg(addr);
    return tmp;
}

/*********************************************************************
** A7130_WriteReg_Page
*********************************************************************/
void A7130_WriteReg_Page(u8 addr, u8 wbyte, u8 page)
{
    A7130_WriteReg(RFANALOG_REG, A7130Config[RFANALOG_REG] | PageTab[page]);//page select
    A7130_WriteReg(addr, wbyte);
}

void a7130_writeID(void)
{
    u8 i;
    u8 id_code[8];
    a7130_spi_cs_low();
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, IDCODE_REG);
    dev_write(__this->spi_fd, ID_Tab, sizeof(ID_Tab));
    a7130_spi_cs_high();

    a7130_spi_cs_low();
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, IDCODE_REG | 0x40);
    dev_read(__this->spi_fd, id_code, 8);
    a7130_spi_cs_high();
    for (i = 0; i < 8; i++) {
        printf(" %x ;", id_code[i]);
    }
}

static void StrobeCmd(u8 cmd)
{
    u8 i;
    u8  cmd_ = 0;
    int err;
#if 1
    a7130_spi_cs_low();
    err = dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, cmd);
    a7130_spi_cs_high();
#endif
}

static void CHGroupCal(u8 ch)
{
    u8 tmp;
    u8 vb, vbcf;
    u8 deva, adev;

    A7130_WriteReg(PLL1_REG, ch);

    A7130_WriteReg(CALIBRATION_REG, 0x0C);
    do {
        tmp = A7130_ReadReg(CALIBRATION_REG);
        tmp &= 0x0C;
    } while (tmp);

    //for check
    tmp = A7130_ReadReg(VCOCAL1_REG);
    vb = tmp & 0x07;
    vbcf = (tmp >> 3) & 0x01;

    tmp = A7130_ReadReg(VCODEVCAL1_REG);
    deva = tmp;

    tmp = A7130_ReadReg(VCODEVCAL2_REG);
    adev = tmp;

    if (vbcf) {
        /* Err_State();//error */
        printf("\n calibration err 3\n");
    }
}

void SetCH(u8 ch)
{
    A7130_WriteReg(PLL1_REG, ch); //RF freq = RFbase + (CH_Step * ch)
}

/*********************************************************************
** calibration
*********************************************************************/
void a7130_cal(void)
{
    u8 tmp, loopCnt;
    u8 fb, fbcf, fcd, vcb, vccf, rhc, rlc;

    StrobeCmd(CMD_PLL); //calibration @PLL state

    //calibration IF procedure
    loopCnt = 0;
    /* A7130_WriteReg(RXGAIN3_REG, 0x3D); */
    A7130_WriteReg(RXGAIN3_REG, 0x1D);
    while (1) {
        A7130_WriteReg(CALIBRATION_REG, 0x02);
        do {
            tmp = A7130_ReadReg(CALIBRATION_REG);
            /* printf("\n CALIBRATION_REG1 %x \n", tmp); */
            tmp &= 0x02;
            /* printf("\n CALIBRATION_REG2 %x \n",tmp); */
        } while (tmp);

        tmp = A7130_ReadReg(IFCAL1_REG);
        fb = tmp & 0x0F;
        printf("\n IFCAL1_REG %x \n", fb);
        if (fb > 3 && fb < 9) {
            break;
        }

        loopCnt++;
        if (loopCnt >= 100) {
            break;
        }
    }
    A7130_WriteReg(RXGAIN3_REG, 0x3D);

    if (loopCnt >= 10) {
        /* Err_State(); */
        printf("\n calibration err 1\n");
    }

    //calibration RSSI, VCC procedure
    A7130_WriteReg(CALIBRATION_REG, 0x11);
    do {
        tmp = A7130_ReadReg(CALIBRATION_REG);
        tmp &= 0x11;
    } while (tmp);

    //calibration VBC,VDC procedure
    CHGroupCal(30); //calibrate channel group Bank I
    CHGroupCal(90); //calibrate channel group Bank II
    CHGroupCal(150); //calibrate channel group Bank III
    StrobeCmd(CMD_STBY); //return to STBY state

    //for check
    tmp = A7130_ReadReg(IFCAL1_REG);
    fb = tmp & 0x0F;
    fbcf = (tmp >> 4) & 0x01;

    tmp = A7130_ReadReg(IFCAL2_REG);
    fcd = tmp & 0x1F;

    tmp = A7130_ReadReg(VCOCCAL_REG) & 0x1F;
    vcb = tmp & 0x0F;
    vccf = (tmp >> 4) & 0x01;

    rhc = A7130_ReadReg(RXGAIN2_REG);
    rlc = A7130_ReadReg(RXGAIN3_REG);

    if (fbcf || vccf) {
        /* Err_State(); */
        printf("\n calibration err 2\n");
    }
}

void a7130_conf(void)
{
    u8 i;

    //0x00 mode register, for reset
    //0x05 fifo data register
    //0x06 id code register
    //0x3F USID register, read only
    //0x36 key data, 16 bytes
    //0x3D FCB register,4 bytes

    for (i = 0x01; i <= 0x04; i++) {
        A7130_WriteReg(i, A7130Config[i]);
    }

    for (i = 0x07; i <= 0x29; i++) {
        A7130_WriteReg(i, A7130Config[i]);
    }

    for (i = 0; i <= 7; i++) { //0x2A DAS
        A7130_WriteReg_Page(0x2A, A7130_Addr2A_Config[i], i);
    }

    for (i = 0x2B; i <= 0x35; i++) {
        A7130_WriteReg(i, A7130Config[i]);
    }

    A7130_WriteReg(0x37, A7130Config[0x37]);

    for (i = 0; i <= 4; i++) { //0x38 ROM
        A7130_WriteReg_Page(0x38, A7130_Addr38_Config[i], i);
    }

    for (i = 0x39; i <= 0x3C; i++) {
        A7130_WriteReg(i, A7130Config[i]);
    }

    A7130_WriteReg(0x3E, A7130Config[0x3E]);
}
void a7130_reset(void)
{
    A7130_WriteReg(MODE_REG, 0x00); //reset RF chip
}

static void initRF(void)
{
    a7130_spi_cs_high();
    a7130_reset(); //reset A7105 RF chip
    a7130_writeID(); //write ID code
    a7130_conf(); //config A7105 chip
    a7130_cal(); //calibration IF,VCO,VCOC
}
void txrx_switch(u8 type)
{
    if (type) { //发送
        gpio_direction_output(A7130_RX_SW_PORT, 1);
        gpio_direction_output(A7130_TX_SW_PORT, 0);

    } else { //收
        gpio_direction_output(A7130_RX_SW_PORT, 0);
        gpio_direction_output(A7130_TX_SW_PORT, 1);

    }
}
static void set_RF_fifo(void)
{
    int err;
    /* u8 cmd_fifo[3] = {FIFO1_REG, 0x3f, 0x00}; */
    u8 cmd_fifo[3] = {FIFO1_REG, FIFO_LEN_L, FIFO_LEN_H};
    a7130_spi_cs_low();
    err = dev_write(__this->spi_fd, cmd_fifo, 3);
    a7130_spi_cs_high();
}

void set_fifo_len(u16 len)
{
    int err;
    /* u8 cmd_fifo[3] = {FIFO1_REG, 0x3f, 0x00}; */
    u8 len_h, len_l;
    len_l = (u8)(0x00FF & (len - 1));
    len_h = (u8)(len - 1) >> 8;

    u8 cmd_fifo[3] = {FIFO1_REG, len_l, len_h};
    a7130_spi_cs_low();
    err = dev_write(__this->spi_fd, cmd_fifo, 3);
    a7130_spi_cs_high();
}


u32 a7130_read_basefifo(u8 *data)
{
    u32 err, re_len = 0;
    u8 cnt, cmd, remain_l, i = 0;
    cnt = FIFO_LEN / 48;
    remain_l = FIFO_LEN % 48;
    cmd = FIFO_REG | 0x40; //address 0x05, bit cmd=0, r/w=1
    a7130_gpio_interrupt_disable(0);
    a7130_gpio_interrupt_disable(1);
    os_sem_set(&__this->rx_sem, 0);
    os_sem_set(&__this->rx_ex_sem, 0);

    a7130_gpio_interrupt_enable(1);
    a7130_gpio_interrupt_enable(0);
    StrobeCmd(CMD_RFR); //reset rx FIFO pointer
    StrobeCmd(CMD_RX); //entry rx

    err = os_sem_pend(&__this->rx_sem, 5);
    if (err != OS_NO_ERR) {
        return re_len;
    }
    a7130_spi_cs_low();
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, cmd);
    /* dev_read(__this->spi_fd, &data[48*2], FIFO_LEN- 96); */
    dev_read(__this->spi_fd, &data[i * 48], FIFO_LEN);
    a7130_spi_cs_high();

    return FIFO_LEN;
}
u32 a7130_read_exfifo(u8 *data)
{
    u32 err, re_len = 0;
    u8 cnt, cmd, remain_l, i;
    cnt = FIFO_LEN / 48;
    remain_l = FIFO_LEN % 48;
    cmd = FIFO_REG | 0x40; //address 0x05, bit cmd=0, r/w=1
    a7130_gpio_interrupt_disable(0);
    a7130_gpio_interrupt_disable(1);
    os_sem_set(&__this->rx_sem, 0);
    os_sem_set(&__this->rx_ex_sem, 0);

    a7130_gpio_interrupt_enable(1);
    a7130_gpio_interrupt_enable(0);
    StrobeCmd(CMD_RFR); //reset rx FIFO pointer
    StrobeCmd(CMD_RX); //entry rx
    for (i = 0; i < cnt; i++) {
        err = os_sem_pend(&__this->rx_ex_sem, 20);
        if (err != OS_NO_ERR) {
            return re_len;
        }
        /* a7130_gpio_interrupt_enable(1); */
        a7130_spi_cs_low();
        dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, cmd);
        dev_read(__this->spi_fd, &data[i * 48], 48);
        a7130_spi_cs_high();
        re_len += 48;
    }
    err = os_sem_pend(&__this->rx_sem, 10);
    if (err != OS_NO_ERR) {
        return re_len;
    }
    a7130_spi_cs_low();
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, cmd);
    dev_read(__this->spi_fd, &data[i * 48], remain_l);
    a7130_spi_cs_high();
    re_len += remain_l;

    return re_len;
}



u8 write_packet_seg(u8 *p, u16 len)
{
    u8 cmd;
    int err = 0;
    cmd = FIFO_REG; //send address 0x05, bit cmd=0, r/w=0
    a7130_spi_cs_low();
    err = dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, cmd);
    if (err < 0) {
        printf("\n spi send cmd err \n");
        return 1;
    }
    err =  dev_write(__this->spi_fd, p, len);
    if (err < 0) {
        printf("\n spi send data  err \n");
        return 1;
    }
    a7130_spi_cs_high();
    return 0;
}

u8 write_packet_base(u8 *p, u16 len)//enable EAK EAR
{
    int err;
    u8 cnt_page, last_page_len, i;
    if (len != (FIFO_LEN - 16)) {
        printf("\n writ packet len more than FIFO_LEN  \n");
        return 2;
    }
    /* StrobeCmd(CMD_STBY); */
    a7130_gpio_interrupt_disable(0);
    a7130_gpio_interrupt_disable(1);
    os_sem_set(&__this->tx_sem, 0);
    os_sem_set(&__this->tx_ex_sem, 0);

    a7130_gpio_interrupt_enable(0);
    a7130_gpio_interrupt_enable(1);
    memcpy(&page_head[16], p, 48);
// send page head
    StrobeCmd(CMD_TFR);
    write_packet_seg(page_head, 64);
    StrobeCmd(CMD_TX);
#if 0
    err = os_sem_pend(&__this->tx_ex_sem, 5);
    if (err != OS_NO_ERR) {
        return 2;
    }
    write_packet_seg(&p[48], 64);
#endif

#if 1
    err = os_sem_pend(&__this->tx_sem, 10);
    if (err != OS_NO_ERR) {
        return 1;
    }
#endif
    return 0;
}
u8 write_packet_EAK(u8 *p, u16 len)//enable EAK EAR
{
    int err;
    u8 cnt_page, last_page_len, i;
    if (len != (FIFO_LEN - 16)) {
        printf("\n writ packet len more than FIFO_LEN  \n");
        return 0;
    }
    cnt_page = len / 48;
    last_page_len = len % 48;

    StrobeCmd(CMD_STBY);
    a7130_gpio_interrupt_disable(0);
    a7130_gpio_interrupt_disable(1);
    os_sem_set(&__this->tx_sem, 0);
    os_sem_set(&__this->tx_ex_sem, 0);

    a7130_gpio_interrupt_enable(0);
    a7130_gpio_interrupt_enable(1);


#if 0
    StrobeCmd(CMD_TFR);
    write_packet_seg(page_head, 16);
    write_packet_seg(p, len);
    StrobeCmd(CMD_TX);

    err = os_sem_pend(&__this->tx_sem, 10);
    if (err != OS_NO_ERR) {
        return 1;
    }

#else
    memcpy(&page_head[16], p, 48);
// send page head
    StrobeCmd(CMD_TFR);
    write_packet_seg(page_head, 64);
    StrobeCmd(CMD_TX);
// send data by seg
    for (i = 1; i < (cnt_page); i++) {
        err = os_sem_pend(&__this->tx_ex_sem, 5);
        if (err != OS_NO_ERR) {
            return i + 1;
        }
        write_packet_seg(&p[i * 48], 48);
    }
    if (last_page_len) {
        err = os_sem_pend(&__this->tx_ex_sem, 1);
        if (err != OS_NO_ERR) {
            /* StrobeCmd(CMD_TFR); */
            return i + 1;
        }
        write_packet_seg(&p[(i + 1) * 48], last_page_len);
    }
#if 1
    err = os_sem_pend(&__this->tx_sem, 10);
    if (err != OS_NO_ERR) {
        return 1;
    }
#endif


#endif
    return 0;

}
u8 write_packet2(u8 *p, u16 len)
{
    int err;
    u8 cnt_page, last_page_len, i;
    if (len > FIFO_LEN && len < 48) {
        printf("\n writ packet len more than FIFO_LEN  \n");
        return 0;
    }
    cnt_page = len / 48;
    last_page_len = len % 48;
    StrobeCmd(CMD_STBY);
    a7130_gpio_interrupt_disable(0);
    a7130_gpio_interrupt_disable(1);
    os_sem_set(&__this->tx_sem, 0);
    os_sem_set(&__this->tx_ex_sem, 0);

    a7130_gpio_interrupt_enable(0);
    a7130_gpio_interrupt_enable(1);
    memcpy(&page_head[16], p, 48);
// send page head
    StrobeCmd(CMD_TFR);
    write_packet_seg(page_head, 64);
    StrobeCmd(CMD_TX);
// send data by seg
    for (i = 1; i < (cnt_page); i++) {
        err = os_sem_pend(&__this->tx_ex_sem, 1);
        if (err != OS_NO_ERR) {
            return i + 1;
        }
        write_packet_seg(&p[i * 48], 48);
    }
    if (last_page_len) {
        err = os_sem_pend(&__this->tx_ex_sem, 1);
        if (err != OS_NO_ERR) {
            /* StrobeCmd(CMD_TFR); */
            return i + 1;
        }
        write_packet_seg(&p[(i + 1) * 48], last_page_len);
    }
#if 1
    err = os_sem_pend(&__this->tx_sem, 1);
    if (err != OS_NO_ERR) {
        return 1;
    }
#endif
    return 0;
}
u8 write_packet(u8 *p, u16 len)
{
    int err;
    u8 cnt_page, last_page_len, i;
    if (len > FIFO_LEN) {
        printf("\n writ packet len more than FIFO_LEN  \n");
        return 0;
    }
    cnt_page = len / 48;
    last_page_len = len % 48;
    StrobeCmd(CMD_STBY);
    a7130_gpio_interrupt_disable(0);
    a7130_gpio_interrupt_disable(1);
    os_sem_set(&__this->tx_sem, 0);
    os_sem_set(&__this->tx_ex_sem, 0);

    a7130_gpio_interrupt_enable(0);
    a7130_gpio_interrupt_enable(1);
// send page head
    StrobeCmd(CMD_TFR);
    write_packet_seg(page_head, 64);
    StrobeCmd(CMD_TX);
// send data by seg
    if (len <= 48) {
        err = os_sem_pend(&__this->tx_ex_sem, 3);
        if (err != OS_NO_ERR) {
            return 1;
        }
        write_packet_seg(p, len);
    } else {
        for (i = 0; i < (cnt_page - 1); i++) {
            err = os_sem_pend(&__this->tx_ex_sem, 3);
            if (err != OS_NO_ERR) {
                /* StrobeCmd(CMD_TFR); */
                return i + 3;
            }
            /* a7130_gpio_interrupt_enable(1); */
            write_packet_seg(&p[i * 48], 48);
        }
        if (last_page_len) {
            err = os_sem_pend(&__this->tx_ex_sem, 3);
            if (err != OS_NO_ERR) {
                /* StrobeCmd(CMD_TFR); */
                return i + 3;
            }
            write_packet_seg(&p[i * 48], 48);
            err = os_sem_pend(&__this->tx_ex_sem, 3);
            if (err != OS_NO_ERR) {
                /* StrobeCmd(CMD_TFR); */
                return i + 4;
            }
            write_packet_seg(&p[(i + 1) * 48], last_page_len);
        } else {
            err = os_sem_pend(&__this->tx_ex_sem, 3);
            if (err != OS_NO_ERR) {
                /* StrobeCmd(CMD_TFR); */
                return i + 3;
            }
            write_packet_seg(&p[i * 48], 48);
        }
    }
#if 1
    err = os_sem_pend(&__this->tx_sem, 4);
    if (err != OS_NO_ERR) {
        /* StrobeCmd(CMD_TFR); */
        return 2;
    }
#endif
    return 0;
}


static u8 write_cmd(u8 *_cmd, u8 len)
{
    int err;
    u8 i;
#if 10
    StrobeCmd(CMD_TFR);
    write_packet_seg(_cmd, len);
    StrobeCmd(CMD_TX);
    err = os_sem_pend(&__this->tx_sem, 2);
    if (err != OS_NO_ERR) {
        return 1;
    }
#else
    StrobeCmd(CMD_TFR);
    write_packet_seg(_cmd, 64);
    StrobeCmd(CMD_TX);
    err = os_sem_pend(&__this->tx_ex_sem, 2);
    if (err != OS_NO_ERR) {
        return 1;
    }

    for (i = 0; i < 3; i++) {
        write_packet_seg(&_cmd[64 + i * 48], 48);
        err = os_sem_pend(&__this->tx_ex_sem, 2);
        if (err != OS_NO_ERR) {
            return i + 2;
        }
    }

    write_packet_seg(&_cmd[64 + i * 48], 48);

    err = os_sem_pend(&__this->tx_sem, 2);
    if (err != OS_NO_ERR) {
        return i + 2;
    }
#endif
    return 0;
}


// 等待命令回复
static int waiting_cmd(u8 type)
{
    int err;
    u8 cmd, i;
    cmd = FIFO_REG | 0x40; //address 0x05, bit cmd=0, r/w=1
    /* set_fifo_len(CMD_LEN); */
    a7130_gpio_interrupt_disable(0);
    a7130_gpio_interrupt_disable(1);
    os_sem_set(&__this->rx_sem, 0);
    os_sem_set(&__this->rx_ex_sem, 0);

    a7130_gpio_interrupt_enable(1);
    a7130_gpio_interrupt_enable(0);
    StrobeCmd(CMD_RFR);
    StrobeCmd(CMD_RX);

#if 10
    err = os_sem_pend(&__this->rx_sem, 100);
    if (err != OS_NO_ERR) {
        return 1;
    }
    a7130_spi_cs_low();
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, cmd);
    dev_read(__this->spi_fd, cmd_buf, CMD_LEN);
    a7130_spi_cs_high();
    /* put_buf(cmd_buf,CMD_LEN); */
#else
    for (i = 0; i < 5; i++) {
        err = os_sem_pend(&__this->rx_ex_sem, 20);
        if (err != OS_NO_ERR) {
            return 1;
        }
        a7130_spi_cs_low();
        dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, cmd);
        dev_read(__this->spi_fd, &cmd_buf[i * 48], 48);
        a7130_spi_cs_high();
    }

    err = os_sem_pend(&__this->rx_sem, 10);
    if (err != OS_NO_ERR) {
        return 3;
    }
    a7130_spi_cs_low();
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, cmd);
    dev_read(__this->spi_fd, &cmd_buf[i * 48], 16);
    a7130_spi_cs_high();


#endif


    if (cmd_buf[0] == CMD_ID && cmd_buf[1] == type) {
        return 0;
    }

    return 2;
}
int waiting_cmd2(u8 type)//ex fifo
{
    int err;
    u8 cmd;
    cmd = FIFO_REG | 0x40; //address 0x05, bit cmd=0, r/w=1
    /* set_fifo_len(CMD_LEN); */
    a7130_gpio_interrupt_disable(0);
    a7130_gpio_interrupt_disable(1);
    os_sem_set(&__this->rx_sem, 0);
    os_sem_set(&__this->rx_ex_sem, 0);

    a7130_gpio_interrupt_enable(1);
    a7130_gpio_interrupt_enable(0);
    StrobeCmd(CMD_RFR);
    StrobeCmd(CMD_RX);

    err = os_sem_pend(&__this->rx_ex_sem, 20);
    if (err != OS_NO_ERR) {
        return 1;
    }
    a7130_spi_cs_low();
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, cmd);
    dev_read(__this->spi_fd, cmd_buf, 48);
    a7130_spi_cs_high();
    err = os_sem_pend(&__this->rx_ex_sem, 20);
    if (err != OS_NO_ERR) {
        return 2;
    }
    a7130_spi_cs_low();
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, cmd);
    dev_read(__this->spi_fd, &cmd_buf[48], 48);
    a7130_spi_cs_high();

    err = os_sem_pend(&__this->rx_sem, 10);
    if (err != OS_NO_ERR) {
        return 3;
    }
    a7130_spi_cs_low();
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, cmd);
    dev_read(__this->spi_fd, &cmd_buf[96], CMD_LEN - 96);
    a7130_spi_cs_high();
    /* put_buf(cmd_buf,CMD_LEN); */
    if (cmd_buf[0] == CMD_ID && cmd_buf[1] == type) {
        return 0;
    }

    return 4;
}

// 等待命令回复
int waiting_ack(u8 type)
{
    int err;
    u8 cmd;
    cmd = FIFO_REG | 0x40; //address 0x05, bit cmd=0, r/w=1
    set_fifo_len(CMD_LEN);
    a7130_gpio_interrupt_disable(0);
    a7130_gpio_interrupt_disable(1);
    os_sem_set(&__this->rx_sem, 0);
    os_sem_set(&__this->rx_ex_sem, 0);

    a7130_gpio_interrupt_enable(1);
    a7130_gpio_interrupt_enable(0);
    StrobeCmd(CMD_RFR);
    StrobeCmd(CMD_RX);
    err = os_sem_pend(&__this->rx_sem, 10);
    if (err != OS_NO_ERR) {
        return 1;
    }
    a7130_spi_cs_low();
    dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, cmd);
    dev_read(__this->spi_fd, ack_buf, CMD_LEN);
    a7130_spi_cs_high();
    if (ack_buf[0] == CMD_ID && ack_buf[1] == CMD_ACK && ack_buf[3] == type) {

        return 0;
    }
    put_buf(ack_buf, CMD_LEN);
    return 2;
}

int	a7130_send_frame4(u8 *frame_data, u32 len)// add custom ACK
{
    static u16 i, cnt, packet_len, f_seq = 0, p_seq = 0;
    u8 err, remain, ret;
    u32 send_len = 0;
    u16 crc_16;
    if (len == 512) { //空帧
        /* puts("len err \n");	 */
        /* printf("len %d \n",len); */
        return 0;
    } else {
        /* put_u32hex(len);	 */
    }
    packet_len = FIFO_LEN - sizeof(struct _data_pack_head) - 2;

    struct _data_pack_head data_pack_head = {0};
    struct _cmd_pack_head  cmd_pack_head = {0};
    struct rf_frm_tag frame_info = {0};
    cmd_pack_head.code = CMD_ID;
    cmd_pack_head.cmd_num = CMD_START;
    cmd_pack_head.p_len  = sizeof(struct rf_frm_tag);
    frame_info.frm_seq = (++f_seq);
    frame_info.frm_len = len;
    frame_info.type = 0;
    frame_info.crc = CRC16(frame_data, len);

    memcpy(fifo_buf, &cmd_pack_head, sizeof(struct _cmd_pack_head));
    memcpy(&fifo_buf[sizeof(struct _cmd_pack_head)], &frame_info, sizeof(struct rf_frm_tag));

    crc_16 = CRC16(fifo_buf, FIFO_LEN - 2);
    memcpy(&fifo_buf[FIFO_LEN - 2], &crc_16, 2);//crc

#if 1
    a7130_gpio_interrupt_disable(0);
    a7130_gpio_interrupt_disable(1);
    os_sem_set(&__this->tx_sem, 0);
    os_sem_set(&__this->tx_ex_sem, 0);

    a7130_gpio_interrupt_enable(0);
    a7130_gpio_interrupt_enable(1);
#endif

    ret = write_cmd(fifo_buf, FIFO_LEN);
    if (ret) {
        /* printf("\n ret:%d \n", ret); */
        puts("C");
        return 1;
    }
    cnt = len / packet_len;
    remain = len % packet_len;
    data_pack_head.code = DATA_ID;
    for (i = 0; i < cnt; i++) {
        data_pack_head.p_seq = i;
        data_pack_head.p_len = packet_len;
        memcpy(fifo_buf, &data_pack_head, sizeof(struct _data_pack_head));
        memcpy(&fifo_buf[sizeof(struct _data_pack_head)], &frame_data[i * packet_len], packet_len);

        crc_16 = CRC16(fifo_buf, FIFO_LEN - 2);
        memcpy(&fifo_buf[FIFO_LEN - 2], &crc_16, 2);

        ret = write_cmd(fifo_buf, FIFO_LEN);
        if (ret) {
            printf("\n 0x%x,0x%x 0x%x\n", cnt, i, ret);
            return 1;
        }
    }
    if (remain) {
        data_pack_head.p_seq = i;
        data_pack_head.p_len = remain;
        memcpy(fifo_buf, &data_pack_head, sizeof(struct _data_pack_head));
        memcpy(&fifo_buf[sizeof(struct _data_pack_head)], &frame_data[i * packet_len], remain);
        crc_16 = CRC16(fifo_buf, FIFO_LEN - 2);
        memcpy(&fifo_buf[FIFO_LEN - 2], &crc_16, 2);

        ret = write_cmd(fifo_buf, FIFO_LEN);
        if (ret) {
            puts("L");
            return 1;
        }
    }
    /* puts("K"); */
    return 0;
}

static u8 a7130_receive_data(void)
{
    u32 wlen, ret_len;
    u16 crc_16, Rcrc_16;
    int ret, err;
    u8 cmd;
    cmd = FIFO_REG | 0x40; //address 0x05, bit cmd=0, r/w=1
    u16 count_sum = 0, err_sums = 0;
    a7130_gpio_interrupt_disable(0);
    a7130_gpio_interrupt_disable(1);
    os_sem_set(&__this->rx_sem, 0);
    os_sem_set(&__this->rx_ex_sem, 0);

    a7130_gpio_interrupt_enable(1);
    a7130_gpio_interrupt_enable(0);
    StrobeCmd(CMD_RFR);
    StrobeCmd(CMD_RX);

    while (1) {
#if 10
        err = os_sem_pend(&__this->rx_sem, 2);
        if (err != OS_NO_ERR) {
            return 1;
        }
        a7130_spi_cs_low();
        dev_ioctl(__this->spi_fd, IOCTL_SPI_SEND_BYTE, cmd);
        dev_read(__this->spi_fd, cmd_buf, CMD_LEN);
        a7130_spi_cs_high();
        StrobeCmd(CMD_RX);
#else

        ret_len =  a7130_read_exfifo(cmd_buf);
        if (ret_len != FIFO_LEN) {
            puts("a\n");
        }
#endif
        memcpy(&Rcrc_16, &cmd_buf[FIFO_LEN - 2], 2);
        crc_16 = CRC16(cmd_buf, FIFO_LEN - 2);
        if (crc_16 != Rcrc_16) {
            /* puts("C1"); */
            /* goto err0; */
            err_sums++;
        } else {
            wlen = cbuf_write(&__this->cbuf_data_head, cmd_buf, CMD_LEN);
        }
        if (count_sum >= 5000) {
            /* put_u16hex(err_sums); */
            count_sum = 0;
            err_sums = 0;
        }
    }
}
static u32 read_cbuf_data(void *buf, u32 len)
{
    u32 rlen;

    rlen = cbuf_read(&__this->cbuf_data_head, buf, len);

    return rlen;
}

//数据组帧
static void rf_data_process_task(void *p)
{
    u8 *frame_data = NULL;
    u16 crc_16, Rcrc_16;
    u32 receive_len, ret_len, frame_len = 0;
    struct rf_frm_tag frame_info = {0};
    struct _cmd_pack_head cmd_pack_head = {0};
    struct _data_pack_head data_pack_head = {0};

    //for test
    static u16 pre_seq = 0;
    static u16 count_seq = 0;
    static frame_lost = 0;
    while (1) {

        if (read_cbuf_data(fifo_buf, FIFO_LEN) != FIFO_LEN) {
            os_time_dly(10);
            continue;
        }
        /* puts("\n read cbuf succes \n"); */
        if (frame_data) {
            memcpy(&data_pack_head, fifo_buf, sizeof(struct _data_pack_head));
            if ((data_pack_head.code == DATA_ID)
                && data_pack_head.p_len < (FIFO_LEN - 2)
               ) {
                if ((receive_len + data_pack_head.p_len) <= frame_len) {
                    memcpy(&frame_data[receive_len], &fifo_buf[sizeof(struct _data_pack_head)], data_pack_head.p_len);
                } else {
                    rf_buf_free(__this->fb, frame_data);
                    frame_data = NULL;
                    continue;
                }
                receive_len += data_pack_head.p_len;
                if (receive_len == frame_len) {
                    crc_16 = CRC16(frame_data, frame_len);
                    if (crc_16 == frame_info.crc) {
                        if (pre_seq == 0) {
                            pre_seq = frame_info.frm_seq;
                            count_seq = frame_info.frm_seq;
                        } else {
                            if (frame_info.frm_seq > (pre_seq + 1)) {
                                frame_lost = 1;
                                put_u16hex(frame_info.frm_seq - pre_seq);
                            }
                            pre_seq = frame_info.frm_seq;
                            if (frame_info.frm_seq >= (count_seq + 30)) {
                                count_seq = frame_info.frm_seq;
                                puts("\n---\n");
                            }
                            if (frame_info.frm_seq < count_seq) {
                                count_seq = frame_info.frm_seq;
                            }
                        }
#if 0
                        rf_buf_stream_finish(__this->fb, frame_data);
                        frame_data = NULL;
#else
// I/P帧丢帧处理
                        if (*(frame_data + 4) == 0x67) {
                            frame_lost  = 0;
                        }
                        if (frame_lost == 0) {
                            rf_buf_stream_finish(__this->fb, frame_data);
                            frame_data = NULL;
                        }
#endif
                    } else {
                        puts("\n F CRC ERR\n");
                        rf_buf_free(__this->fb, frame_data);
                        frame_data = NULL;
                    }

                }

            } else {
                rf_buf_free(__this->fb, frame_data);
                frame_data = NULL;
                if (data_pack_head.code == CMD_ID) {
                    if (fifo_buf[0] != CMD_ID || fifo_buf[1] != CMD_START) {
                        continue;
                    }
                    memcpy(&cmd_pack_head, fifo_buf, sizeof(struct _cmd_pack_head));
                    memcpy(&frame_info, &fifo_buf[sizeof(struct _cmd_pack_head)], sizeof(struct rf_frm_tag));
                    frame_len = frame_info.frm_len;
                    receive_len = 0;
                    frame_data = rf_buf_malloc(__this->fb, frame_len);
                }
            }
        } else {
            if (fifo_buf[0] != CMD_ID || fifo_buf[1] != CMD_START) {
                continue;
            }
            memcpy(&cmd_pack_head, fifo_buf, sizeof(struct _cmd_pack_head));
            memcpy(&frame_info, &fifo_buf[sizeof(struct _cmd_pack_head)], sizeof(struct rf_frm_tag));
            frame_len = frame_info.frm_len;
            receive_len = 0;
            frame_data = rf_buf_malloc(__this->fb, frame_len);
            /* puts("A\n"); */
        }
    }
}


int get_lbuf_data(void)
{
    int err = 0;
    static u8 i = 0;
#if 1
    struct rf_videobuf *p_lbu = NULL;
    p_lbu = lbuf_pop(__this->plbuf, 1);
    if (p_lbu) {
        if (i % 1 == 0) {
            /* put_buf(p_lbu->data,4); */
            a7130_send_frame4(p_lbu->data, p_lbu->len);
            /* os_time_dly(1); */
        }
        i++;
        lbuf_free(__this->plbuf, p_lbu);
        err = 1;
    }
#endif
    return err;
}

int a7130_receive_frame4(void)
{
    u32 receive_len, ret_len, frame_len = 0;
    u8 *frame_data;
    u16 crc_16, Rcrc_16;
    int ret;
    struct rf_frm_tag frame_info = {0};
    struct _cmd_pack_head cmd_pack_head = {0};
    struct _data_pack_head data_pack_head = {0};
    if (waiting_cmd(CMD_START)) {
        goto err0;
    }
re_start:
    memcpy(&cmd_pack_head, cmd_buf, sizeof(struct _cmd_pack_head));
    memcpy(&frame_info, &cmd_buf[sizeof(struct _cmd_pack_head)], sizeof(struct rf_frm_tag));

    memcpy(&Rcrc_16, &cmd_buf[FIFO_LEN - 2], 2);
    crc_16 = CRC16(cmd_buf, FIFO_LEN - 2);
    if (crc_16 != Rcrc_16) {
        puts("C1");
        goto err0;
    }

    frame_len = frame_info.frm_len;
    receive_len = 0;
    frame_data = rf_buf_malloc(__this->fb, frame_len);
    if (frame_data == NULL) {
        puts("\n lbuf malloc err \n");
        goto err0;
    }
    while (receive_len < frame_len) {
        ret_len = a7130_read_basefifo(fifo_buf);
        /* ret_len =  a7130_read_exfifo(fifo_buf); */
        memcpy(&Rcrc_16, &fifo_buf[FIFO_LEN - 2], 2);
        crc_16 = CRC16(fifo_buf, FIFO_LEN - 2);
        if (crc_16 != Rcrc_16) {
            puts("C2\n");
            goto err1;
        }

        if (ret_len == FIFO_LEN) {
            /* if (ret_len >= 64) { */
            memcpy(&data_pack_head, fifo_buf, sizeof(struct _data_pack_head));
            if ((data_pack_head.code == DATA_ID)
                && data_pack_head.p_len <= ret_len
               ) {
                if (1) {
                    memcpy(&frame_data[receive_len], &fifo_buf[sizeof(struct _data_pack_head)], data_pack_head.p_len);
                    receive_len += data_pack_head.p_len;
                }
            } else {
                if (data_pack_head.code == CMD_ID) {
                    if (frame_data) {
                        rf_buf_free(__this->fb, frame_data);
                        frame_data = NULL;
                    }
                    puts("R\n");
                    memcpy(cmd_buf, fifo_buf, FIFO_LEN);
                    goto re_start;
                }
                puts("D1");
                goto err1;
            }
        } else {
            puts("D2");
            goto err1;
        }

    }
    if (receive_len == frame_len) {
        crc_16 = CRC16(frame_data, frame_len);
        if (crc_16 == frame_info.crc) {
            rf_buf_stream_finish(__this->fb, frame_data);
            frame_data = NULL;
            /* puts("O"); */
        } else {
            puts("\n F CRC ERR\n");
            goto err1;
        }
    } else {
        puts("\n F LEN  ERR\n");
        goto err1;
    }


    return 1;
err1:
    if (frame_data) {
        rf_buf_free(__this->fb, frame_data);
    }
err0:
    return 0;
}

#define CRC_ENABLE  0




static void a7130_task0(void *p)
{
    int msg[32];
    struct a7130_config *rf_para = p;
    u32 err;

    while (1) {
        if (rf_para->mode == 2) {
            if (get_lbuf_data()) {
                /* puts("OK"); */
                /* os_time_dly(1);	 */
            } else {
                /* puts("NF"); */
                /* os_time_dly(1);	 */
            }
        } else if (rf_para->mode == 1) {
            if (((struct rf_fh *)rf_para->fb)->streamon) {
                /* if (a7130_receive_frame4() == 0) { */
                if (a7130_receive_data() == 0) {
                    /* os_time_dly(1); */
                }
            } else {
                /* os_time_dly(1); */
            }
        }
        /* os_time_dly(1); */
    }
}



static bool a7130_dev_online(const struct dev_node *node)
{
    int err = 0;
    int id;

    /* return false; */

    return true;
}

int  a7130_init(const struct dev_node *node, void *_data)
{

    struct rf_platform_data *pdata = NULL;
    pdata = (struct rf_platform_data *) _data;
    memset(__this, 0x0, sizeof(*__this));
    if (pdata) {
        __this->height = pdata->height;
        __this->width  = pdata->width;
    }
    puts("\n a7130_init \n");
    return 0;
}

int  a7130_open(const struct dev_node *node,  struct device **device, void *arg)
{
    u8 work_mode = (u8)arg;
    int err;
    struct rf_fh *fh = NULL;
    struct rf_videobuf *p_lbuf = NULL;

    if (!__this->spi_fd) {
        __this->spi_fd = dev_open("spi2", 0);
        if (!__this->spi_fd) {
            printf("\n a7130 spi dev open err\n");
            return -EINVAL;
        }
    }

    if (__this->mode) {
        if (__this->mode != ((work_mode) ? 2 : 1)) {
            printf("\n a7130 had config mode %d \n", __this->mode);
            return -EINVAL;
        }
    } else {
        __this->mode = (work_mode) ? 2 : 1;

        if (__this->mode == 2) { //发送模式
            __this->data_buf =	malloc(2 * 1024 * 1024);
            /* __this->data_buf =	malloc(1*1*256); */
            if (!__this->data_buf) {
                printf("malloc err \n");
                return -EINVAL;
            }
            INIT_LIST_HEAD(&__this->wlist_head);
            __this->plbuf =  lbuf_init(__this->data_buf, 2 * 1024 * 1024, 32, sizeof(struct rf_videobuf));
            /* __this->plbuf =  lbuf_init(__this->data_buf,1*256,32,sizeof(struct rf_videobuf)); */
            if (__this->plbuf) {
                puts("\n lbuf init sucess\n");
                puts("\n lbuf init sucess\n");
            }
            txrx_switch(10);
            printf("\n work send mode: %x,", __this->mode);
            initRF(); //init RF
            StrobeCmd(CMD_STBY);
            SetCH(100);
            set_RF_fifo();//fifo length upto 4K bytes
            /* set_fifo_len(CMD_LEN); */

        } else { //接收模式

            printf("\n work receive mode: %x\n", __this->mode);
            initRF(); //init RF
            StrobeCmd(CMD_STBY);
            SetCH(100);
            set_RF_fifo();//fifo length upto 4K bytes
            /* set_fifo_len(CMD_LEN); */

            fh = (struct rf_fh *)zalloc(sizeof(*fh));
            videobuf_queue_init(&fh->video_q, 32);
            fh->open = 1;
            os_sem_create(&fh->sem, 0);
            os_sem_post(&fh->sem);
            *device = &fh->device;
            (*device)->private_data = fh;
            txrx_switch(0);
            __this->p_cbuf_data =  malloc(CBUF_DATA_LEN);
            if (!__this->p_cbuf_data) {
                printf("malloc err \n");
                return -EINVAL;
            }
            cbuf_init(&__this->cbuf_data_head, __this->p_cbuf_data, CBUF_DATA_LEN);
            err = task_create(rf_data_process_task, __this, "rf_data_process");
        }
        gpio_set_pull_up(A7130_TX_SW_PORT, 1);
        gpio_set_pull_up(A7130_RX_SW_PORT, 1);
        gpio_set_pull_down(A7130_TX_SW_PORT, 0);
        gpio_set_pull_down(A7130_RX_SW_PORT, 0);
        puts("\n a7130 open succes 2\n");
        /* A7130_WriteReg(CODE1_REG, 0x17);//ENABLE crc */
        __this->fb = fh;
        os_sem_create(&__this->tx_sem, 0);
        os_sem_create(&__this->rx_sem, 0);
        os_sem_create(&__this->tx_ex_sem, 0);
        os_sem_create(&__this->rx_ex_sem, 0);
        a7130_wtr_interrupt_register(__this);
        a7130_cko_interrupt_register(__this);
        err = task_create(a7130_task0, __this, "rf_task");
    }
    puts("\n a7130 open succes \n");
    return 0;
err1:
    return -EINVAL;
}

int a7130_ioctl(struct device *device, u32 cmd, u32 arg)
{

    u8 ch;
    struct rf_fh *fh = (struct rf_fh *)device->private_data;
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
        SetCH(ch);
        break;
    default:
        break;
    }
    return ret;
}

static int a7130_write(struct device *dev, void *buf, u32 len, u32 addr)
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


static int a7130_close(struct device *device)
{
    if (__this->mode == 2) {
        lbuf_clear(__this->plbuf);
        if (__this->data_buf) {
            free(__this->data_buf);
        }
    } else {
        /* os_sem_pend(&-((struct rf_fh *)__this->fb)>sem, 0); */
        videobuf_queue_release(&((struct rf_fh *)__this->fb)->video_q);
        if (__this->fb) {
            free(__this->fb);
        }
    }
    puts("\n a7130 close \n");
    __this->mode = 0;
    return 0;
}

const struct device_operations a7130_dev_ops = {
    .online = a7130_dev_online,
    .init = a7130_init,
    .open = a7130_open,
    .write = a7130_write,
    .ioctl = a7130_ioctl,
    .close = a7130_close,
};

REGISTER_RF_DEVICE(rf_dev) = {
    .name = "a7130",
    .ops = &a7130_dev_ops,
};


