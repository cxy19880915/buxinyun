#include "pan2418.h"
#include "device/rf_dev.h"

#include "circular_buf.h"
#include "style_jl02.h"
#include "ui/ui_text.h"
#include "action.h"
#define ENHANCE_MODE 0


#define ADD_PALNA   1


struct _p2418_config {
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
    cbuffer_t cbuf_data_head;
    u8 *p_cbuf_data;
    struct list_head wlist_head;
    struct lbuff_head  *plbuf;
    struct rf_fh *fb;
    u8 *data_buf;
    void *spi_fd;
    void *fd;

};
static struct _p2418_config rf_cfg;
#define __this  (&rf_cfg)


#define CBUF_DATA_LEN  1024*1024
static void *port_wakeup_hdl;
const u8 tx_rx_addr[5] = {0xC1, 0xC2, 0xC3, 0xC4, 0xCC};
#define BUFFER_LEN  512
static u8 rec_buf[BUFFER_LEN];
static u8 send_buf[BUFFER_LEN];

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

static void *spi = NULL;

static void Rf_WriteBuf(u8 addr, u8 *buf, u32 len)
{
    int err = 0;
    dev_ioctl(spi, IOCTL_SPI_SET_CS, 0);
    dev_ioctl(spi, IOCTL_SPI_SEND_BYTE, addr);
    err = dev_write(spi, buf, len);
    dev_ioctl(spi, IOCTL_SPI_SET_CS, 1);
}


void Rf_WriteReg(u8 addr, u8 dataByte)
{
    dev_ioctl(spi, IOCTL_SPI_SET_CS, 0);
    dev_ioctl(spi, IOCTL_SPI_SEND_BYTE, addr);
    dev_write(spi, &dataByte, 1);
    /* dev_ioctl(spi, IOCTL_SPI_SEND_BYTE, dataByte); */
    dev_ioctl(spi, IOCTL_SPI_SET_CS, 1);
}

void Rf_ReadReg(u8 addr, u8 *rdata)
{
    dev_ioctl(spi, IOCTL_SPI_SET_CS, 0);
    dev_ioctl(spi, IOCTL_SPI_SEND_BYTE, addr);
    dev_ioctl(spi, IOCTL_SPI_READ_BYTE, (u32)rdata);
    /* dev_read(spi, rdata, 1); */
    dev_ioctl(spi, IOCTL_SPI_SET_CS, 1);
}

void Rf_ReadBuf(u8 addr, u8 *rdata, u32 len)
{
    dev_ioctl(spi, IOCTL_SPI_SET_CS, 0);
    dev_ioctl(spi, IOCTL_SPI_SEND_BYTE, addr);
    dev_read(spi, rdata, len);
    dev_ioctl(spi, IOCTL_SPI_SET_CS, 1);
}
#define	Rf_set_ce()	(Rf_WriteReg(CE_FSPI_ON, 0))
#define	Rf_clr_ce()	(Rf_WriteReg(CE_FSPI_OFF, 0))

static void Rf_set_mode(u8 mode)
{
    if (mode) { //tx
        Rf_WriteReg(W_REGISTER + CONFIG,  0x8e);
        /* Rf_clr_ce(); */
    } else { //rx
        Rf_WriteReg(W_REGISTER + CONFIG,  0x8f);
        delay_us(10000);
        /* Rf_set_ce(); */
    }
}

static void Rf_set_fifo_size(u8 channel, u16 size)
{
    u8 fifo_size[2];

    if (size < 1 || size > 512) {
        return;
    }

    if (channel > 5) {
        return;
    }

    fifo_size[0] = (u8)size;
    fifo_size[1] = size >> 8;
    switch (channel) {
    case 0:
        Rf_WriteBuf(W_REGISTER + RX_PW_P0, fifo_size, 2);
        break;
    case 1:
        Rf_WriteBuf(W_REGISTER + RX_PW_P1, fifo_size, 2);
        break;
    case 2:
        Rf_WriteBuf(W_REGISTER + RX_PW_P2, fifo_size, 2);
        break;
    case 3:
        Rf_WriteBuf(W_REGISTER + RX_PW_P3, fifo_size, 2);
        break;
    case 4:
        Rf_WriteBuf(W_REGISTER + RX_PW_P4, fifo_size, 2);
        break;
    case 5:
        Rf_WriteBuf(W_REGISTER + RX_PW_P5, fifo_size, 2);
        break;
    default:
        Rf_WriteBuf(W_REGISTER + RX_PW_P0, fifo_size, 2);
        break;
    }
}
static void Rf_set_channel(u8 ch)
{
    Rf_WriteReg(W_REGISTER + RF_CH, ch);
}

void Rf_clr_tx_fifo(void)
{
    Rf_WriteReg(FLUSH_TX, 0);
}

void Rf_clr_rx_fifo(void)
{
    Rf_WriteReg(FLUSH_RX, 0);
}

void Rf_clr_status(void)
{
    Rf_WriteReg(W_REGISTER + STATUS, 0x70);
}

u8 Rf_get_status(void)
{
    u8 data = 0;
    Rf_ReadReg(STATUS, &data);
    return data;
}
u8 Rf_get_fifo_status(void)
{
    u8 data = 0;
    Rf_ReadReg(FIFO_STATUS, &data);
    return data;
}

void Rf_transfer(u8 *tx_buf, u16 tx_len)
{
    Rf_WriteBuf(W_TX_PAYLOAD, tx_buf, tx_len);
    Rf_set_ce();
}

static void Rf_init(u8 type)
{
    u8  tx_addr_buf_tmp[5];
    u8  bb_cal_data[]    = {0x02, 0x0a, 0x3c, 0x3c, 0x64, 0x00, 0x0e, 0x00, 0x20, 0xa2, 0x50, 0x00};
    /* u8  bb_cal_data[]    = {0x02,0x0a,0x3c,0x3c,0x64,0x00,0x0e,0x00,0x20,0xa2 ,0x11,0x00};  */
    u8  rf_cal_data[]    = {0xf2, 0xff, 0x6d, 0x3f, 0xa0};
    u8  rf_cal2_data[]   = {0x8c, 0x83, 0xbb, 0xae, 0xff, 0xd0, 0x80, 0x10, 0x21};
    u8  dem_cal2_data[]  = {0x0b, 0xdf, 0x02, 0xaa, 0xaa, 0x00};
    u8  dem_cal_data[]   = {0x00, 0x41, 0x7c, 0x1f, 0x00, 0x02, 0x00, 0x20, 0x1e};
    u8  pa_ramp_ctl_data[] = {0x21, 0x43, 0x65, 0x07, 0x21, 0x43, 0x10, 0x32, 0x43, 0x65, 0x10, 0x87, 0x54, 0x32, 0xf8, 0xFF, 0xFF, 0x07, 0x0c, 0x07};
#if(FAST_MODE_EN == 1)
    {
        dem_cal_data[0] |= 0x08;
    }
#endif

#if(FEC_EN == 1)
    {
        dem_cal_data[0] |= 0x02;
    }
#endif

#if(SCRAMBLE_EN == 1)
    {
        dem_cal_data[0] |= 0x01;
    }
#endif

#if(ADD_PALNA == 1)
    if (type) {
        bb_cal_data[0] |= 0xc0; //收
    } else {
        bb_cal_data[0] |= 0xa0;//发
    }
#endif

    Rf_WriteReg(RST_FSPI, 0x5A);	//soft rst
    Rf_WriteReg(RST_FSPI, 0xA5);
    delay(100);
    Rf_WriteReg(W_REGISTER + EN_RXADDR, 0x01);
    Rf_WriteReg(W_REGISTER + SETUP_AW, 0x03);
    Rf_set_channel(DEFAULT_CHANNEL);
    Rf_set_fifo_size(0, PAYLOAD_WIDTH);

#if  1
    Rf_WriteBuf(W_REGISTER + TX_ADDR, (u8 *)tx_rx_addr, sizeof(tx_rx_addr));
#endif
    Rf_WriteBuf(W_REGISTER + RX_ADDR_P0, (u8 *)tx_rx_addr, sizeof(tx_rx_addr));
    Rf_ReadBuf(RX_ADDR_P0, tx_addr_buf_tmp, sizeof(tx_rx_addr));//读出写入的数据进行校验
    put_buf(tx_addr_buf_tmp, 5);
    Rf_WriteBuf(W_REGISTER + BB_CAL,    bb_cal_data,  sizeof(bb_cal_data));

#if(ADD_PALNA == 1)
    Rf_WriteBuf(W_REGISTER + PA_RAMP_CTL,    pa_ramp_ctl_data,  sizeof(pa_ramp_ctl_data));
#endif
    Rf_WriteBuf(W_REGISTER + RF_CAL2,   rf_cal2_data, sizeof(rf_cal2_data));
    Rf_WriteBuf(W_REGISTER + DEM_CAL,   dem_cal_data, sizeof(dem_cal_data));
    Rf_WriteBuf(W_REGISTER + RF_CAL,    rf_cal_data,  sizeof(rf_cal_data));
    Rf_WriteBuf(W_REGISTER + DEM_CAL2,  dem_cal2_data, sizeof(dem_cal2_data));
    Rf_WriteReg(W_REGISTER + DYNPD, 0x00);
    Rf_WriteReg(W_REGISTER + RF_SETUP,  DATA_RATE);	//6Mbps
    Rf_WriteReg(ACTIVATE, 0x73);
#if ENHANCE_MODE
    Rf_WriteReg(W_REGISTER + FEATURE, 0x20);
    Rf_WriteReg(W_REGISTER + SETUP_RETR, 0x24); //重发次数 bit0-3 自动传输延时bit4-7 0000 250us
    Rf_WriteReg(W_REGISTER + EN_AA, 0x01);//ENABLE PIPE 0 ACK
#else
    Rf_WriteReg(W_REGISTER + FEATURE, 0x20);
    Rf_WriteReg(W_REGISTER + SETUP_RETR, 0x00);
    Rf_WriteReg(W_REGISTER + EN_AA, 0x00);
#endif
}

static void rf_isr(void *priv)
{
    struct _p2418_config *para = priv;
    if (WKUP_CON3 & BIT(0)) {
        WKUP_CON2 |= BIT(0);
        if (para->mode == 2) {
            os_sem_post(&para->tx_sem);
        } else {
            os_sem_post(&para->rx_sem);
        }
    }
}
static void rf_irq_config(struct _p2418_config *para)
{

    IOMC2 &= ~(0x7f);
    IOMC2 |=  0x1A;  //PORTB_10

    WKUP_CON1 |= BIT(0);//down edge
    WKUP_CON2 |= BIT(0);//clear pending
    WKUP_CON0 |= BIT(0);//DISABLE
    port_wakeup_hdl = port_wakeup_reg(rf_isr, para);
}

void test_send(void)
{
    static u16 f_seq = 0;
    u16 err;
    struct _cmd_pack_head  cmd_pack_head = {0};
    cmd_pack_head.code = CMD_ID;
    cmd_pack_head.cmd_num = CMD_START;
    cmd_pack_head.p_len  = sizeof(struct rf_frm_tag);
    /* cmd_pack_head.crc = (++f_seq); */
    memset(send_buf, 1, BUFFER_LEN);
    memcpy(send_buf, &cmd_pack_head, sizeof(struct _cmd_pack_head));
    Rf_clr_status();
    Rf_clr_ce();
    Rf_transfer(send_buf, BUFFER_LEN);
    err = os_sem_pend(&__this->tx_sem, 2);
    if (err != OS_NO_ERR) {
        /* return 1; */
        printf("\nsend err\n");
    }
    puts("F");
}

int	p2418_send_frame_withack(u8 *frame_data, u32 len)//enhance mode
{
    u8 temp = 0 ;
    static u16 i, cnt, packet_len, f_seq = 0, p_seq = 0;
    u8 remain, ret;
    u16 err;
    u32 send_len = 0;
    u16 crc_16;
    struct _data_pack_head data_pack_head = {0};
    struct _cmd_pack_head  cmd_pack_head = {0};
    struct rf_frm_tag frame_info = {0};

    packet_len = BUFFER_LEN - sizeof(struct _data_pack_head) - 2;

    cmd_pack_head.code = CMD_ID;
    cmd_pack_head.cmd_num = CMD_START;
    cmd_pack_head.p_len  = sizeof(struct rf_frm_tag);
    frame_info.frm_seq = (++f_seq);
    frame_info.frm_len = len;
    frame_info.type = 0;
    frame_info.crc = CRC16(frame_data, len);
    memset(send_buf, 1, BUFFER_LEN);
    memcpy(send_buf, &cmd_pack_head, sizeof(struct _cmd_pack_head));
    memcpy(&send_buf[sizeof(struct _cmd_pack_head)], &frame_info, sizeof(struct rf_frm_tag));

    crc_16 = CRC16(send_buf, BUFFER_LEN - 2);
    memcpy(&send_buf[BUFFER_LEN - 2], &crc_16, 2);
    Rf_set_mode(1);
    os_sem_set(&__this->tx_sem, 0);
    Rf_clr_tx_fifo();
    Rf_clr_status();
    Rf_clr_ce();
    Rf_transfer(send_buf, BUFFER_LEN);
#if 1
    err = os_sem_pend(&__this->tx_sem, 0);
    if (err != OS_NO_ERR) {
        return 1;
    }
#endif

    Rf_ReadReg(STATUS, &temp);
    if (temp & BIT(4)) { //重发未成功中断
        /* if(!(temp & BIT(5))){//重发未成功中断 */
        printf("\n111111 no ack %x  \n", temp);

        Rf_set_mode(0);
        Rf_clr_status();
        Rf_clr_rx_fifo();
        err = os_sem_pend(&__this->tx_sem, 10);
        if (err != OS_NO_ERR) {
            return 1;
        }
        Rf_ReadBuf(R_RX_PAYLOAD, rec_buf, PAYLOAD_WIDTH);//
        put_buf(rec_buf, 8);
        return 1;
    }
    puts("A");
    cnt = len / packet_len;
    remain = len % packet_len;
    data_pack_head.code = DATA_ID;
    for (i = 0; i < cnt; i++) {
        Rf_clr_status();
        Rf_clr_ce();
        data_pack_head.p_seq = i;
        data_pack_head.p_len = packet_len;
        /* data_pack_head.crc = CRC16(&frame_data[i*packet_len],packet_len); */
        memcpy(send_buf, &data_pack_head, sizeof(struct _data_pack_head));
        memcpy(&send_buf[sizeof(struct _data_pack_head)], &frame_data[i * packet_len], packet_len);
        crc_16 = CRC16(send_buf, BUFFER_LEN - 2);
        memcpy(&send_buf[BUFFER_LEN - 2], &crc_16, 2);
        Rf_transfer(send_buf, BUFFER_LEN);
        err = os_sem_pend(&__this->tx_sem, 2);
        if (err != OS_NO_ERR) {
            return 1;
        }
        Rf_ReadReg(R_REGISTER | STATUS, &temp);
        if (!(temp & BIT(5))) { //重发未成功中断
            /* if(temp & BIT(4)){//重发未成功中断 */
            printf("\n i:%d no ack %x \n", i, temp);
            /* return 1; */
        }
    }
#if 1
    if (remain) {
        Rf_clr_status();
        Rf_clr_ce();
        data_pack_head.p_seq = i;
        data_pack_head.p_len = remain;
        /* data_pack_head.crc = CRC16(&frame_data[i*packet_len],remain); */
        memcpy(send_buf, &data_pack_head, sizeof(struct _data_pack_head));
        memcpy(&send_buf[sizeof(struct _data_pack_head)], &frame_data[i * packet_len], remain);
        crc_16 = CRC16(send_buf, BUFFER_LEN - 2);
        memcpy(&send_buf[BUFFER_LEN - 2], &crc_16, 2);
        Rf_transfer(send_buf, BUFFER_LEN);
    }
    err = os_sem_pend(&__this->tx_sem, 2);
    if (err != OS_NO_ERR) {
        return 1;
    }
    Rf_ReadReg(R_REGISTER | STATUS, &temp);
    /* if(temp & BIT(4)){//重发未成功中断	 */
    if (!(temp & BIT(5))) { //重发未成功中断
        printf("\n333333333 no ack %x \n", temp);
        /* return 1; */
    }
#endif
    puts("K");



    return 0;
}
#if 0
/*******************
 * S server:CMD_START + DATA +CMD_CHECK +     R              + (resend DATA)
 *
 * R server:R            R         R       S[CMD_RESEND]
 ** ****************/
int	p2418_send_frame3(u8 *frame_data, u32 len)//双向通讯模式
{

}

#endif
int	p2418_send_frame2(u8 *frame_data, u32 len)//乒乓模式
{

    static u16 i, remain, cnt, packet_len, f_seq = 0, p_seq = 0;
    u8 ret;
    u16 err;
    u32 send_len = 0;
    u16 crc_16;
    struct _data_pack_head data_pack_head = {0};
    struct _cmd_pack_head  cmd_pack_head = {0};
    struct rf_frm_tag frame_info = {0};

    if (len == 512) { //空帧
        /* puts("len err \n");	 */
        /* printf("len %d \n",len); */
        return 0;
    } else {
        /* put_u32hex(len);	 */
    }
    packet_len = BUFFER_LEN - sizeof(struct _data_pack_head) - 2;

    cmd_pack_head.code = CMD_ID;
    cmd_pack_head.cmd_num = CMD_START;
    cmd_pack_head.p_len  = sizeof(struct rf_frm_tag);
    frame_info.frm_seq = (++f_seq);
    frame_info.frm_len = len;
    frame_info.type = 0;
    frame_info.crc = CRC16(frame_data, len);
    /* cmd_pack_head.crc = CRC16(&frame_info,sizeof(struct rf_frm_tag));// */
    memset(send_buf, 0x12, BUFFER_LEN);
    memcpy(send_buf, &cmd_pack_head, sizeof(struct _cmd_pack_head));
    memcpy(&send_buf[sizeof(struct _cmd_pack_head)], &frame_info, sizeof(struct rf_frm_tag));

    crc_16 = CRC16(send_buf, BUFFER_LEN - 2);
    memcpy(&send_buf[BUFFER_LEN - 2], &crc_16, 2);

    /* Rf_clr_tx_fifo(); */
    Rf_clr_ce();
    Rf_clr_status();
    os_sem_set(&__this->tx_sem, 0);
    /* Rf_transfer(send_buf, BUFFER_LEN); */
    Rf_WriteBuf(W_TX_PAYLOAD, send_buf, BUFFER_LEN);
#if 0
    err = os_sem_pend(&__this->tx_sem, 2);
    if (err != OS_NO_ERR) {
        return 1;
    }
#endif
    /* return 0; 	 */
    cnt = len / packet_len;
    remain = len % packet_len;
    data_pack_head.code = DATA_ID;
    i = 0;
    data_pack_head.p_seq = 0;
    data_pack_head.p_len = packet_len;
    /* data_pack_head.crc = CRC16(&frame_data[i*packet_len],packet_len); */
    /* memset(send_buf, 1, BUFFER_LEN); */
    memcpy(send_buf, &data_pack_head, sizeof(struct _data_pack_head));
    memcpy(&send_buf[sizeof(struct _data_pack_head)], frame_data, packet_len);
    /* memcpy(&send_buf[sizeof(struct _data_pack_head)], &frame_data[i * packet_len], packet_len); */
    crc_16 = CRC16(send_buf, BUFFER_LEN - 2);
    memcpy(&send_buf[BUFFER_LEN - 2], &crc_16, 2);
    /* Rf_transfer(send_buf, BUFFER_LEN); */
    Rf_WriteBuf(W_TX_PAYLOAD, send_buf, BUFFER_LEN);
    Rf_set_ce();

    for (i = 1; i < cnt; i++) {
        err = os_sem_pend(&__this->tx_sem, 2);
        if (err != OS_NO_ERR) {
            /* return 1; */
            goto err;
        }

        Rf_clr_status();
        data_pack_head.p_seq = i;
        data_pack_head.p_len = packet_len;
        memcpy(send_buf, &data_pack_head, sizeof(struct _data_pack_head));
        memcpy(&send_buf[sizeof(struct _data_pack_head)], &frame_data[i * packet_len], packet_len);
        crc_16 = CRC16(send_buf, BUFFER_LEN - 2);
        memcpy(&send_buf[BUFFER_LEN - 2], &crc_16, 2);
        /* Rf_transfer(send_buf, BUFFER_LEN); */
        Rf_WriteBuf(W_TX_PAYLOAD, send_buf, BUFFER_LEN);
        /* Rf_set_ce(); */
    }

#if 1
    if (remain) {
        err = os_sem_pend(&__this->tx_sem, 2);
        if (err != OS_NO_ERR) {
            /* return 1; */
            goto err;
        }
        Rf_clr_status();
        data_pack_head.p_seq = i;
        data_pack_head.p_len = remain;
        /* data_pack_head.crc = CRC16(&frame_data[i*packet_len],remain); */
        memcpy(send_buf, &data_pack_head, sizeof(struct _data_pack_head));
        memcpy(&send_buf[sizeof(struct _data_pack_head)], &frame_data[i * packet_len], remain);
        crc_16 = CRC16(send_buf, BUFFER_LEN - 2);
        memcpy(&send_buf[BUFFER_LEN - 2], &crc_16, 2);
        /* Rf_transfer(send_buf, BUFFER_LEN); */
        Rf_WriteBuf(W_TX_PAYLOAD, send_buf, BUFFER_LEN);
        err = os_sem_pend(&__this->tx_sem, 2);
        if (err != OS_NO_ERR) {
            puts("L1");
            /* return 1; */
            goto err;
        }
        Rf_clr_status();
    } else {
#if 1
        err = os_sem_pend(&__this->tx_sem, 2);
        if (err != OS_NO_ERR) {
            puts("L2");
            /* return 1; */
            goto err;
        }
        Rf_clr_status();
#endif
    }
#if 10
    err = os_sem_pend(&__this->tx_sem, 2);
    if (err != OS_NO_ERR) {
        if (remain)
            /* puts("L3"); */
        {
            put_u16hex(cnt);
        }
        /* put_u16hex(remain); */
        else {
            puts("L4\n");
        }
        /* return 1; */
        goto err;
    }
    Rf_clr_status();
#endif
#endif
    /* puts("K"); */
    return 0;
err:
    /* puts("E\n"); */
    return 1;
}



int	p2418_send_frame(u8 *frame_data, u32 len)//
{

    static u16 i, remain, cnt, packet_len, f_seq = 0, p_seq = 0;
    u8  ret;
    u16 err;
    u32 send_len = 0;
    u16 crc_16;
    struct _data_pack_head data_pack_head = {0};
    struct _cmd_pack_head  cmd_pack_head = {0};
    struct rf_frm_tag frame_info = {0};



    packet_len = BUFFER_LEN - sizeof(struct _data_pack_head) - 2;

    cmd_pack_head.code = CMD_ID;
    cmd_pack_head.cmd_num = CMD_START;
    cmd_pack_head.p_len  = sizeof(struct rf_frm_tag);
    frame_info.frm_seq = (++f_seq);
    frame_info.frm_len = len;
    frame_info.type = 0;
    frame_info.crc = CRC16(frame_data, len);
    /* cmd_pack_head.crc = CRC16(&frame_info,sizeof(struct rf_frm_tag));// */
    memset(send_buf, 1, BUFFER_LEN);
    memcpy(send_buf, &cmd_pack_head, sizeof(struct _cmd_pack_head));
    memcpy(&send_buf[sizeof(struct _cmd_pack_head)], &frame_info, sizeof(struct rf_frm_tag));

    /* crc_16 = CRC16(send_buf,sizeof(struct _cmd_pack_head)+ sizeof(struct rf_frm_tag)); */
    /* memcpy(&send_buf[sizeof(struct _cmd_pack_head)+sizeof(struct rf_frm_tag)],&crc_16,2); */
    crc_16 = CRC16(send_buf, BUFFER_LEN - 2);
    memcpy(&send_buf[BUFFER_LEN - 2], &crc_16, 2);

    os_sem_set(&__this->tx_sem, 0);
    Rf_clr_tx_fifo();
    Rf_clr_status();
    Rf_clr_ce();
    Rf_transfer(send_buf, BUFFER_LEN);
#if 1
    err = os_sem_pend(&__this->tx_sem, 2);
    if (err != OS_NO_ERR) {
        return 1;
    }
#endif
    /* return 0; 	 */
    cnt = len / packet_len;
    remain = len % packet_len;
    data_pack_head.code = DATA_ID;
    for (i = 0; i < cnt; i++) {

        Rf_clr_status();
        Rf_clr_ce();
        data_pack_head.p_seq = i;
        data_pack_head.p_len = packet_len;
        /* data_pack_head.crc = CRC16(&frame_data[i*packet_len],packet_len); */
        memcpy(send_buf, &data_pack_head, sizeof(struct _data_pack_head));
        memcpy(&send_buf[sizeof(struct _data_pack_head)], &frame_data[i * packet_len], packet_len);
        crc_16 = CRC16(send_buf, BUFFER_LEN - 2);
        memcpy(&send_buf[BUFFER_LEN - 2], &crc_16, 2);
        Rf_transfer(send_buf, BUFFER_LEN);
        err = os_sem_pend(&__this->tx_sem, 2);
        if (err != OS_NO_ERR) {
            return 1;
        }
        /* os_sem_set(&__this->tx_sem, 0); */
    }

#if 1
    if (remain) {
        /* err = os_sem_pend(&__this->tx_sem, 2); */
        /* if (err != OS_NO_ERR) { */
        /* return 1; */
        /* } */
        Rf_clr_status();
        Rf_clr_ce();
        data_pack_head.p_seq = i;
        data_pack_head.p_len = remain;
        /* data_pack_head.crc = CRC16(&frame_data[i*packet_len],remain); */
        memcpy(send_buf, &data_pack_head, sizeof(struct _data_pack_head));
        memcpy(&send_buf[sizeof(struct _data_pack_head)], &frame_data[i * packet_len], remain);
        crc_16 = CRC16(send_buf, BUFFER_LEN - 2);
        memcpy(&send_buf[BUFFER_LEN - 2], &crc_16, 2);
        Rf_transfer(send_buf, BUFFER_LEN);
        err = os_sem_pend(&__this->tx_sem, 2);
        if (err != OS_NO_ERR) {
            return 1;
        }
    }
#endif
    puts("K");
    return 0;
}


static int get_lbuf_data(void)
{
    int err = 0;
    static u8 i = 0;
#if 1
    static  struct rf_videobuf *p_lbu = NULL;
    p_lbu = lbuf_pop(__this->plbuf, 1);
    if (p_lbu) {
        if (i % 1 == 0) {
#if ENHANCE_MODE
            p2418_send_frame_withack(p_lbu->data, p_lbu->len);
#else
            /* p2418_send_frame(p_lbu->data, p_lbu->len); */
            p2418_send_frame2(p_lbu->data, p_lbu->len);
            /* test_send(); */
#endif
        }
        i++;
        lbuf_free(__this->plbuf, p_lbu);
        p_lbu = NULL;
        err = 1;
    } else {

        /* p_lbu = lbuf_pop(__this->plbuf, 1); */
    }
#endif
    return err;
}
int p2418_receive_frame(void)
{
    u32 receive_len, ret_len, frame_len = 0;
    u8 *frame_data;
    u8 temp = 0 ;
    u16 crc_16, Rcrc_16;
    int ret, err;
    struct rf_frm_tag frame_info = {0};
    struct _cmd_pack_head cmd_pack_head = {0};
    struct _data_pack_head data_pack_head = {0};

    /* Rf_set_mode(0); */
    Rf_clr_rx_fifo();
    os_sem_set(&__this->rx_sem, 0);
    Rf_clr_status();

start_r:
    err = os_sem_pend(&__this->rx_sem, 8);
    if (err != OS_NO_ERR) {
        /* Rf_ReadReg(R_REGISTER | FIFO_STATUS,&temp); */
        /* put_u8hex(temp); */
        /* put_u8hex(Rf_get_status()); */
        puts("\n--\n");
        /* goto start_r;	 */
        return 1;
    }

    Rf_ReadBuf(R_RX_PAYLOAD, rec_buf, PAYLOAD_WIDTH);//
    Rf_clr_status();

re_start:
    memcpy(&Rcrc_16, &rec_buf[BUFFER_LEN - 2], 2);
    crc_16 = CRC16(rec_buf, BUFFER_LEN - 2);
    if (crc_16 != Rcrc_16) {
        puts("C1\n");
        goto start_r;
    }
    if (rec_buf[0] != CMD_ID || rec_buf[1] != CMD_START) {
        /* puts("C11\n"); */
        /* Rf_clr_status(); */
        goto start_r;
    }
    memcpy(&cmd_pack_head, rec_buf, sizeof(struct _cmd_pack_head));
    memcpy(&frame_info, &rec_buf[sizeof(struct _cmd_pack_head)], sizeof(struct rf_frm_tag));
    //crc 校验
    /* if(cmd_pack_head.crc != CRC16(&frame_info,sizeof(struct rf_frm_tag))	){ */
    /* printf("\n frame crc err \n");	 */
    /* goto err0;	 */
    /* } */
    frame_len = frame_info.frm_len;
    receive_len = 0;
    frame_data = rf_buf_malloc(__this->fb, frame_len);

    while (receive_len < frame_len) {
        err = os_sem_pend(&__this->rx_sem, 3);
        if (err != OS_NO_ERR) {
            /* return 1; */
            puts("T\n");
            goto err1;
        }
        Rf_ReadBuf(R_RX_PAYLOAD, rec_buf, PAYLOAD_WIDTH);//
        Rf_clr_status();
        memcpy(&Rcrc_16, &rec_buf[BUFFER_LEN - 2], 2);
        crc_16 = CRC16(rec_buf, BUFFER_LEN - 2);
        if (crc_16 != Rcrc_16) {
            puts("C2\n");
            goto err1;
        }

        memcpy(&data_pack_head, rec_buf, sizeof(struct _data_pack_head));
        if ((data_pack_head.code == DATA_ID)
            && data_pack_head.p_len < (BUFFER_LEN - 2)
            /* && (data_pack_head.crc == CRC16(&rec_buf[sizeof(struct _data_pack_head)],data_pack_head.p_len))  */
           ) {
            if ((receive_len + data_pack_head.p_len) <= frame_len) {
                memcpy(&frame_data[receive_len], &rec_buf[sizeof(struct _data_pack_head)], data_pack_head.p_len);
            }
            receive_len += data_pack_head.p_len;

        } else {
            if (data_pack_head.code == CMD_ID) {
                if (frame_data) {
                    rf_buf_free(__this->fb, frame_data);
                    frame_data = NULL;
                }
                goto re_start;
            } else {
                puts("C3\n");
                put_u8hex(data_pack_head.code);
                goto err1;
            }
        }
    }

    if (receive_len == frame_len) {
        crc_16 = CRC16(frame_data, frame_len);
        if (crc_16 == frame_info.crc) {
            rf_buf_stream_finish(__this->fb, frame_data);
            frame_data = NULL;

        } else {
            puts("\n F CRC ERR\n");
            goto err1;
        }
    } else {
        puts("\n F LEN  ERR\n");
        goto err1;
    }

    put_u32hex(frame_info.frm_seq);
    return 1;
err1:
    if (frame_data) {
        rf_buf_free(__this->fb, frame_data);
        frame_data = NULL;
    }
    /* goto start_r;	 */
err0:
    return 0;
}
int p2418_receive_frame2(void)
{
    static u8 analy_arrsy[22] = {0};
    static u8 frame_recnt = 0;
    static u16 pre_seq = 0;
    u8 i;
    u16 fram_sum;
    u32 receive_len, ret_len, frame_len = 0;
    u8 *frame_data;
    u8 temp = 0 ;
    u8 temp_pseq = 0;
    u16 crc_16, Rcrc_16;
    int ret, err;
    struct rf_frm_tag frame_info = {0};
    struct _cmd_pack_head cmd_pack_head = {0};
    struct _data_pack_head data_pack_head = {0};

    static u8 mark_frame = 0;
    Rf_clr_rx_fifo();
    os_sem_set(&__this->rx_sem, 0);
    Rf_clr_status();

start_r:
    err = os_sem_pend(&__this->rx_sem, 100);
    if (err != OS_NO_ERR) {
        return 2;
    }

    Rf_ReadBuf(R_RX_PAYLOAD, rec_buf, PAYLOAD_WIDTH);//
    Rf_clr_status();
re_start:
    memcpy(&Rcrc_16, &rec_buf[BUFFER_LEN - 2], 2);
    crc_16 = CRC16(rec_buf, BUFFER_LEN - 2);
    if (crc_16 != Rcrc_16) {
        puts("C1\n");
        goto start_r;
    }
    if (rec_buf[0] != CMD_ID || rec_buf[1] != CMD_START) {
        /* puts("s\n"); */
        goto start_r;
        /* goto err0;	 */
    }
    memcpy(&cmd_pack_head, rec_buf, sizeof(struct _cmd_pack_head));
    memcpy(&frame_info, &rec_buf[sizeof(struct _cmd_pack_head)], sizeof(struct rf_frm_tag));
    frame_len = frame_info.frm_len;
    receive_len = 0;
    frame_data = rf_buf_malloc(__this->fb, frame_len);
    while (receive_len < frame_len) {
        err = os_sem_pend(&__this->rx_sem, 10);
        if (err != OS_NO_ERR) {
            puts("T\n");
            goto err1;
        }
        Rf_ReadBuf(R_RX_PAYLOAD, rec_buf, PAYLOAD_WIDTH);//
        Rf_clr_status();
        memcpy(&Rcrc_16, &rec_buf[BUFFER_LEN - 2], 2);
        crc_16 = CRC16(rec_buf, BUFFER_LEN - 2);
        if (crc_16 != Rcrc_16) {
            puts("C2\n");
            goto err1;
        }

        memcpy(&data_pack_head, rec_buf, sizeof(struct _data_pack_head));
        if ((data_pack_head.code == DATA_ID)
            && data_pack_head.p_len < (BUFFER_LEN - 2)
            /* && (data_pack_head.crc == CRC16(&rec_buf[sizeof(struct _data_pack_head)],data_pack_head.p_len))  */
           ) {

            if ((receive_len + data_pack_head.p_len) <= frame_len) {
                memcpy(&frame_data[receive_len], &rec_buf[sizeof(struct _data_pack_head)], data_pack_head.p_len);
            } else {
                goto err1;
            }
            receive_len += data_pack_head.p_len;
            if (temp_pseq < data_pack_head.p_seq) {
                temp = (data_pack_head.p_seq - temp_pseq);
                if (temp > 1) {
                    put_u8hex(temp);
                    /* put_u8hex(temp_pseq); */
                    /* put_u16hex(data_pack_head.p_len); */
                }
            }
            temp_pseq = data_pack_head.p_seq;

        } else {
            if (data_pack_head.code == CMD_ID) {
                if (frame_data) {
                    rf_buf_free(__this->fb, frame_data);
                    frame_data = NULL;
                }
                /* put_u32hex(frame_len-receive_len); */
                puts("R\n");
                temp_pseq = 0;
                goto re_start;
            } else {
                puts("C3\n");
                put_u8hex(data_pack_head.code);
                goto err1;
            }
        }
    }

    if (receive_len == frame_len) {
        crc_16 = CRC16(frame_data, frame_len);
        if (crc_16 == frame_info.crc) {
            rf_buf_stream_finish(__this->fb, frame_data);
            frame_data = NULL;
        } else {
            puts("\n F CRC ERR\n");
            goto err1;
        }
    } else {
        puts("\n F LEN  ERR\n");
        goto err1;
    }
    mark_frame++;
    if (mark_frame == 10) {
        mark_frame = 0;
        puts("\n-----------\n");
    }
#if 0
    // for show receive frame num
    if (frame_recnt == 0) {
        frame_recnt++;
        pre_seq = frame_info.frm_seq;
    } else {
        if (frame_info.frm_seq > pre_seq) {
            analy_arrsy[frame_recnt] = frame_info.frm_seq - pre_seq;
        } else {
            analy_arrsy[frame_recnt] = 1;
        }
        frame_recnt++;
        if (frame_recnt > 21) {
            frame_recnt = 21;
            for (i = 0; i < frame_recnt; i++) {
                analy_arrsy[i] = analy_arrsy[i + 1];
            }
        }
        for (i = frame_recnt - 1; i > 0; i--) {
            fram_sum += analy_arrsy[i];
            if (fram_sum > 20);
            break;
        }

        /* video_rec_post_msg("HlightOn");//前照灯显示 */
    }
#endif

    if (pre_seq == 0) {
        pre_seq = frame_info.frm_seq;
    } else {
        if (frame_info.frm_seq > (pre_seq + 1)) {
            put_u16hex(frame_info.frm_seq - pre_seq);
        }
        /* else */
        pre_seq = frame_info.frm_seq;
    }
    /* put_u32hex(frame_info.frm_seq); */
    return 1;
err1:
    if (frame_data) {
        rf_buf_free(__this->fb, frame_data);
        frame_data = NULL;
    }
err0:
    return 0;
}

static u8 rf_receive_data(void)
{
    u32 wlen;
    u16 crc_16, Rcrc_16;
    int ret, err;
    Rf_clr_rx_fifo();
    os_sem_set(&__this->rx_sem, 0);
    Rf_clr_status();
    u16 count_sum = 0, err_sums = 0;
    while (1) {
        err = os_sem_pend(&__this->rx_sem, 10);
        if (err != OS_NO_ERR) {
            /* put_u16hex(err_sums); */
            return 2;
        }
        count_sum++;
        Rf_ReadBuf(R_RX_PAYLOAD, rec_buf, PAYLOAD_WIDTH);//
        Rf_clr_status();
        memcpy(&Rcrc_16, &rec_buf[BUFFER_LEN - 2], 2);
        crc_16 = CRC16(rec_buf, BUFFER_LEN - 2);
        if (crc_16 != Rcrc_16) {
            /* puts("CRC\n"); */
            err_sums++;
        } else {
            wlen = cbuf_write(&__this->cbuf_data_head, rec_buf, BUFFER_LEN);
            /* printf("cbuf write %d \n",wlen); */
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


static void pan2418_task0(void *p)
{
    int msg[32];
    struct _p2418_config *rf_para = p;
    u32 err;
    Rf_clr_rx_fifo();
    Rf_clr_tx_fifo();
    Rf_clr_status();
    if (rf_para->mode == 2) {
        Rf_clr_ce();
    } else {

        Rf_set_ce();
    }
    while (1) {
        if (rf_para->mode == 2) {
            if (get_lbuf_data()) {
            } else {
                os_time_dly(1);
            }
        } else if (rf_para->mode == 1) {
            if (((struct rf_fh *)rf_para->fb)->streamon) {
                if (p2418_receive_frame() == 0) {
                    /* os_time_dly(1); */
                }
            } else {
                /* os_time_dly(1); */
            }
        }
    }
}

static void pan2418_task(void *p)
{
    int msg[32];
    struct _p2418_config *rf_para = p;
    u32 err;
    static u8 streamon_mark = 0;
    Rf_clr_status();
    if (rf_para->mode == 2) {
        Rf_clr_ce();
        os_sem_set(&__this->tx_sem, 0);
        Rf_clr_tx_fifo();
        Rf_clr_status();
        while (1) {

            if (get_lbuf_data()) {

                /* os_time_dly(1); */
            } else {
                os_time_dly(1);
            }
        }
    } else {
        /* Rf_clr_rx_fifo(); */
        while (1) {
            if (((struct rf_fh *)rf_para->fb)->streamon) {
                if (streamon_mark == 0) {
                    streamon_mark = 1;
                    Rf_clr_ce();
                    Rf_clr_rx_fifo();
                    os_sem_set(&__this->rx_sem, 0);
                    Rf_clr_status();
                    Rf_set_ce();
                }
#if 0
                if (p2418_receive_frame2() == 2) {
                    streamon_mark = 0;
                }
#else
                if (rf_receive_data()) {
                    /* printf("\n--func=%s\n", __FUNCTION__);	 */
                    os_time_dly(1);
                }
#endif
            } else {
                if (streamon_mark) {
                    streamon_mark = 0;
                    Rf_clr_ce();
                    Rf_clr_status();
                }
                os_time_dly(1);
            }
        }
    }

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

        if (read_cbuf_data(send_buf, BUFFER_LEN) != BUFFER_LEN) {
            os_time_dly(1);
            continue;
        }
        /* puts("\n read cbuf succes \n"); */
        if (frame_data) {
            memcpy(&data_pack_head, send_buf, sizeof(struct _data_pack_head));
            if ((data_pack_head.code == DATA_ID)
                && data_pack_head.p_len < (BUFFER_LEN - 2)
               ) {
                if ((receive_len + data_pack_head.p_len) <= frame_len) {
                    memcpy(&frame_data[receive_len], &send_buf[sizeof(struct _data_pack_head)], data_pack_head.p_len);
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

#if 0 // I/P帧丢帧处理
                        rf_buf_stream_finish(__this->fb, frame_data);
                        frame_data = NULL;
#else
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
                    if (send_buf[0] != CMD_ID || send_buf[1] != CMD_START) {
                        continue;
                    }
                    memcpy(&cmd_pack_head, send_buf, sizeof(struct _cmd_pack_head));
                    memcpy(&frame_info, &send_buf[sizeof(struct _cmd_pack_head)], sizeof(struct rf_frm_tag));
                    frame_len = frame_info.frm_len;
                    receive_len = 0;
                    frame_data = rf_buf_malloc(__this->fb, frame_len);
                }
            }
        } else {
            if (send_buf[0] != CMD_ID || send_buf[1] != CMD_START) {
                continue;
            }
            memcpy(&cmd_pack_head, send_buf, sizeof(struct _cmd_pack_head));
            memcpy(&frame_info, &send_buf[sizeof(struct _cmd_pack_head)], sizeof(struct rf_frm_tag));
            frame_len = frame_info.frm_len;
            receive_len = 0;
            frame_data = rf_buf_malloc(__this->fb, frame_len);
            /* puts("A\n"); */
        }
    }
}

static bool p2418_dev_online(const struct dev_node *node)
{
    return true;
}

static int  p2418_init(const struct dev_node *node, void *_data)
{

    struct rf_platform_data *pdata = NULL;
    pdata = (struct rf_platform_data *) _data;
    memset(__this, 0x0, sizeof(*__this));
    if (pdata) {
        __this->height = pdata->height;
        __this->width  = pdata->width;
    }
    puts("\n pan2418_init \n");
    return 0;
}

static int  p2418_open(const struct dev_node *node,  struct device **device, void *arg)
{
    u8 work_mode = (u8)arg;
    int err;
    struct rf_fh *fh = NULL;
    struct rf_videobuf *p_lbuf = NULL;

    if (!__this->spi_fd) {
        __this->spi_fd = dev_open("spi2", 0);
        if (!__this->spi_fd) {
            printf("\n p2418 spi dev open err\n");
            return -EINVAL;
        }
        spi = __this->spi_fd;
    }
    if (__this->mode) {
        if (__this->mode != ((work_mode) ? 2 : 1)) {
            printf("\n pan2418 had config mode %d \n", __this->mode);
            return -EINVAL;
        }
    } else {

        __this->mode = (work_mode) ? 2 : 1;
        if (work_mode) { //发送模式
            __this->data_buf =	malloc(2 * 1024 * 1024);
            if (!__this->data_buf) {
                printf("malloc err \n");
                return -EINVAL;
            }
            /* INIT_LIST_HEAD(&__this->wlist_head); */
            __this->plbuf =  lbuf_init(__this->data_buf, 2 * 1024 * 1024, 32, sizeof(struct rf_videobuf));
            if (__this->plbuf) {
                puts("\n lbuf init sucess\n");
            }
            Rf_init(0);
            Rf_set_mode(1);
            /* rf_irq_config(); */
        } else {
            fh = (struct rf_fh *)zalloc(sizeof(*fh));
            videobuf_queue_init(&fh->video_q, 32);
            fh->open = 1;
            os_sem_create(&fh->sem, 0);
            os_sem_post(&fh->sem);
            *device = &fh->device;
            (*device)->private_data = fh;
            Rf_init(1);
            Rf_set_mode(0);
            __this->p_cbuf_data =  malloc(CBUF_DATA_LEN);
            if (!__this->p_cbuf_data) {
                printf("malloc err \n");
                return -EINVAL;
            }
            cbuf_init(&__this-> cbuf_data_head, __this->p_cbuf_data, CBUF_DATA_LEN);
            err = task_create(rf_data_process_task, __this, "rf_data_process");
        }
        os_sem_create(&__this->tx_sem, 0);
        os_sem_create(&__this->rx_sem, 0);
        __this->fb = fh;
        /* err = task_create(pan2418_task0, __this, "rf_task"); */
        err = task_create(pan2418_task, __this, "rf_task");
        rf_irq_config(__this);
    }
    puts("\n pan2418 open succes \n");
    return 0;

}

static int p2418_write(struct device *dev, void *buf, u32 len, u32 addr)
{
    static u8 mark = 0;
    struct rf_videobuf *p = NULL;
    if (mark) {
        /* return 0;	 */
    }
    if (__this->mode == 2) { //发送模式
        p = lbuf_alloc(__this->plbuf, len);

        if (!p) {
            printf("\n lbuf alloc err \n");
            return 0;
        }
        p->len = len;
        memcpy(p->data, buf, len);
        lbuf_push(__this->plbuf, p, 1);
        mark = 1;
    } else {
        puts("\n receive mode \n");
        puts("\n receive mode \n");
    }
    return 0;
}
static int p2418_ioctl(struct device *device, u32 cmd, u32 arg)
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
        Rf_set_channel(ch);
        break;
    default:
        break;
    }
    return ret;
}
static int p2418_close(struct device *device)
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
    puts("\n p2410 close \n");
    __this->mode = 0;
    return 0;
}

const struct device_operations pan2418_dev_ops = {
    .online = p2418_dev_online,
    .init = p2418_init,
    .open = p2418_open,
    .write = p2418_write,
    .ioctl = p2418_ioctl,
    .close = p2418_close,
};

REGISTER_RF_DEVICE(rf_dev) = {
    .name = "pan2418",
    .ops = &pan2418_dev_ops,
};



