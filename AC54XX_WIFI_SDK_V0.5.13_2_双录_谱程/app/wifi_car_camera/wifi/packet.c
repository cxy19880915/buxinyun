#include "server/packet.h"
int get_mov_media_info(struct __packet_info *pinfo)
{
    char buf[64];
    u32  nlen = 0;
    struct vfs_attr attr;
    fget_attrs(pinfo->fd, &attr);

    pinfo->time.tm_year = attr.crt_time.year;
    pinfo->time.tm_mon  = attr.crt_time.month;
    pinfo->time.tm_mday = attr.crt_time.day;
    pinfo->time.tm_hour = attr.crt_time.hour;
    pinfo->time.tm_min  = attr.crt_time.min;
    pinfo->time.tm_sec  = attr.crt_time.sec;
#if 1
    printf("start=%d-%d-%d-%d-%d-%d\n"
           , pinfo->time.tm_year
           , pinfo->time.tm_mon
           , pinfo->time.tm_mday
           , pinfo->time.tm_hour
           , pinfo->time.tm_min
           , pinfo->time.tm_sec);
#endif
    if (strstr(pinfo->file_name, get_rec_path_1())) {
        fget_name(pinfo->fd, (u8 *)pinfo->file_name + strlen(get_rec_path_1()), sizeof(pinfo->file_name) - strlen(get_rec_path_1()));
    } else if (strstr(pinfo->file_name, get_rec_path_2())) {
        fget_name(pinfo->fd, (u8 *)pinfo->file_name + strlen(get_rec_path_2()), sizeof(pinfo->file_name) - strlen(get_rec_path_2()));

    }
    /* printf("pinfo->file_name=%s\n", pinfo->file_name); */

    read_time_scale_dur(pinfo->fd, &pinfo->info);
    read_height_and_length(pinfo->fd, &pinfo->info);
    read_stts(pinfo->fd, &pinfo->info);
    pinfo->info.sample_rate = get_audio_sample_rate(pinfo->fd);
    if (pinfo->info.sample_rate == -1) {
        return -1;
    }


    if (is_has_audio(pinfo->fd)) {
        u32 count = get_audio_sample_count(pinfo->fd);
        if (count == -1) {
            return -1;
        }

        pinfo->info.audio_chunk_num = get_audio_chunk_offset_entry(pinfo->fd);
        if (pinfo->info.audio_chunk_num == -1) {
            return -1;
        }
        pinfo->info.audio_block_size = (count * 2) / pinfo->info.audio_chunk_num;
        /* printf("audio block size -> %d\n", pinfo->info.audio_block_size); */
        /* printf("audio size  -> %d\n", (count * 2) / pinfo->info.audio_chunk_num); */
        pinfo->info.audio_stco_tab = get_audio_chunk_table(pinfo->fd, &pinfo->info);
        if (pinfo->info.audio_stco_tab == NULL) {
            return -1;
        }
    }

    /* printf("weight:%d  height :%d  durition:%d \n", pinfo->info.length >> 16, pinfo->info.height >> 16, pinfo->info.durition / pinfo->info.scale); */
    //get table

    pinfo->info.stco_tab = get_chunk_index_table(pinfo->fd, &pinfo->info);

    pinfo->info.stsz_tab = get_sample_index_table(pinfo->fd, &pinfo->info);
    if (pinfo->info.stco_tab == NULL && pinfo->info.stsz_tab == NULL) {
        printf(" pinfo->info.stco_tab == NULL\n\n");
        return -1;
    }

    return 0;
}


FILE *play_next(struct __packet_info *pinfo)
{
    char name[32];
    char tmp_name[32];
    FILE *tmp_fd = NULL;
    u8 invalid = 0;
    fget_name(pinfo->fd, (u8 *)name, 32);
    if (strstr(pinfo->file_name, get_rec_path_1())) {
        pinfo->fs = fscan(get_rec_path_1(), "-tMOV -st");
    } else if (strstr(pinfo->file_name, get_rec_path_2())) {
        pinfo->fs = fscan(get_rec_path_2(), "-tMOV -st");
    }

    fclose(pinfo->fd);

    while (1) {
        if (tmp_fd == NULL) {
            tmp_fd = fselect(pinfo->fs, FSEL_FIRST_FILE, 0);
        } else {
            tmp_fd = fselect(pinfo->fs, FSEL_NEXT_FILE, 0);
        }
        if (tmp_fd == NULL) {
            break;
        }

        fget_name(tmp_fd, (u8 *)tmp_name, 32);
        printf("tmp_name->%s name->%s\n", tmp_name, name);
        if (!strcmp(tmp_name, name) && !invalid) {
            if (tmp_fd != NULL) {
                fclose(tmp_fd);
            }
            tmp_fd = fselect(pinfo->fs, FSEL_NEXT_FILE, 0);
            if (tmp_fd == NULL) {
                break;
            }
            if (!is_vaild_mov_file(tmp_fd)) {
                puts("invalid  mov file\n");
                invalid = 1;
                continue;
            }
            break;
        }
        if (invalid) {
            tmp_fd = fselect(pinfo->fs, FSEL_NEXT_FILE, 0);
            if (tmp_fd == NULL) {
                break;
            } else {
                fclose(tmp_fd);
            }
            if (!is_vaild_mov_file(tmp_fd)) {
                puts("invalid  mov file\n");

                invalid = 1;
                continue;
            }
            break;


        }

        fclose(tmp_fd);
    }

    fget_name(tmp_fd, (u8 *)tmp_name, 32);
    printf("PLAYBACK_NEXT_FILE:%s\n", tmp_name);
    fscan_release(pinfo->fs);
    return tmp_fd;

}
FILE *play_prev(struct __packet_info *pinfo)
{
//#define CONFIG_REC_PATH_1       CONFIG_STORAGE_PATH"/C/DCIM/1/"
    char name[32];
    char tmp_name[32];
    FILE *tmp_fd = NULL;
    u32 invalid = 0;
    fget_name(pinfo->fd, (u8 *)name, 32);
    if (strstr(pinfo->file_name, get_rec_path_1())) {
        pinfo->fs = fscan(get_rec_path_1(), "-tMOV  -st");
    } else if (strstr(pinfo->file_name, get_rec_path_2())) {
        pinfo->fs = fscan(get_rec_path_2(), "-tMOV  -st");
    }

    fclose(pinfo->fd);

    while (1) {
        if (tmp_fd == NULL) {
            tmp_fd = fselect(pinfo->fs, FSEL_LAST_FILE, 0);
        } else {
            tmp_fd = fselect(pinfo->fs, FSEL_PREV_FILE, 0);
        }


        if (tmp_fd == NULL) {
            break;
        }

        fget_name(tmp_fd, (u8 *)tmp_name, 32);
        /* printf("tmp_name -> %s \n", tmp_name); */
        if (!strcmp(tmp_name, name) && !invalid) {
            tmp_fd = fselect(pinfo->fs, FSEL_PREV_FILE, 0);
            if (tmp_fd == NULL) {
                break;
            }
            if (!is_vaild_mov_file(tmp_fd)) {
                fclose(tmp_fd);
                continue;
            }
            break;
        }

        if (invalid) {
            tmp_fd = fselect(pinfo->fs, FSEL_NEXT_FILE, 0);
            if (tmp_fd == NULL) {
                break;
            }
            if (!is_vaild_mov_file(tmp_fd)) {
                puts("invalid  mov file\n");

                invalid = 1;
                continue;
            }
            break;


        }

        fclose(tmp_fd);
    }

    fscan_release(pinfo->fs);
    return tmp_fd;

}

int send_video_packet(struct __packet_info  *pinfo, u32 i)
{
    /* puts("send_video_packet\n"); */
    int ret;
    u32 tmp ;
    char *buffer = NULL;
    u32 tmp2;
    u32 start_code = 0x01000000;
    struct frm_head *frame_head = (struct frm_head *)pinfo->data;
    memset(frame_head, 0, sizeof(struct frm_head));
    int sample_size = get_sample_size(pinfo->info.stsz_tab, i);
    if (sample_size == -1) {
        return -1;
    }
    int sample_offset = get_chunk_offset(pinfo->info.stco_tab, i);
    if (sample_offset == -1) {
        return -1;
    }
    if (sample_size > pinfo->len) {
        printf("I frame size:0x%x   offset:0x%x buffer size:%d\n", sample_size, sample_offset, pinfo->len);
        puts("I frame is too large\n");
        while (1);
    }
    fseek(pinfo->fd, sample_offset, SEEK_SET);

    frame_head->type = (pinfo->type | pinfo->fast_play_mask);
//   frame_head->type = H264_TYPE_VIDEO;
    frame_head->seq = i;

    /* printf("video seq=%d\n", frame_head->seq); */
    frame_head->timestamp = 90000 / 30;


    ret = fread(pinfo->fd, pinfo->data + sizeof(struct frm_head), sample_size);
    if (ret <= 0) {
        return ret;
    }
    frame_head->frm_sz = ret;
#if 0
    tmp = pinfo->data + sizeof(struct frm_head);

    if (*((char *)(tmp + 4)) == 0x67) {
        memcpy(tmp, &start_code, 4);
        tmp += 14;
        memcpy(&tmp2, tmp, 4);

        if (lwip_htonl(tmp2) != 0x00000004) {
            tmp -= 1;
        }
        memcpy(tmp, &start_code, 4);
        tmp += 8;
        memcpy(tmp, &start_code, 4);
    } else {
        memcpy(tmp, &start_code, 4);
    }
#else

    buffer = (char *)pinfo->data + sizeof(struct frm_head);

    if (*((char *)(buffer + 4)) == 0x67) {
        //处理PPS帧和SPS帧 I帧
        memcpy(&tmp, buffer, 4);
        tmp = htonl(tmp);
        memcpy(buffer, &start_code, 4);
        memcpy(&tmp2, buffer + tmp + 4, 4);
        tmp2 = htonl(tmp2);
        memcpy(buffer + tmp + 4, &start_code, 4);
        memcpy(buffer + tmp + tmp2 + 8, &start_code, 4);
        //printf("tmp %d  tmp2 %d\n",tmp,tmp2);

    } else {
        //	   处理P帧
        memcpy(buffer, &start_code, 4);
    }


#endif


    /* put_buf(pinfo->data + 20, 32); */
    ret = sock_send(pinfo->sock, (char *)pinfo->data, ret + sizeof(struct frm_head), 0);

    return ret;
}


int send_audio_packet(struct __packet_info *pinfo, u32 j)
{
    /* puts("send_audio_packet\n"); */
    int ret;
    static u32 tmp = 0;
    static u32 tmp2 = 0;
    struct frm_head *frame_head = (struct frm_head *)pinfo->data;

    int audio_offset = get_audio_chunk_offset(pinfo->info.audio_stco_tab, j);
    if (audio_offset == -1) {
        return -1;
    }
    if (false == fseek(pinfo->fd, audio_offset, SEEK_SET)) {
        return -1;
    }

    tmp = frame_head->seq;
    frame_head->type = PCM_TYPE_AUDIO;
    frame_head->frm_sz = pinfo->info.audio_block_size;
    ret = fread(pinfo->fd, pinfo->data + sizeof(struct frm_head), pinfo->info.audio_block_size);
    if (ret <= 0) {
        return ret;
    }


    ret = sock_send(pinfo->sock, (char *)pinfo->data, ret + sizeof(struct frm_head), 0);
    tmp2++;
    if (tmp2 == pinfo->info.audio_chunk_num) {
        tmp = 0;
    }
    return ret;

}

void incr_date_time(struct tm *tm_time, int incr)
{
    do {
        if (incr >= 60) {
            tm_time->tm_sec += 60;
            incr -= 60;
        } else {
            tm_time->tm_sec += incr;
            incr = 0;
        }

        if (tm_time->tm_sec > 59) {
            tm_time->tm_sec -= 60;
            tm_time->tm_min += 1;

            if (tm_time->tm_min > 59) {
                tm_time->tm_min -= 60;
                tm_time->tm_hour += 1;

                if (tm_time->tm_hour > 23) {
                    tm_time->tm_hour -= 24;
                    tm_time->tm_mday += 1;

                    if (tm_time->tm_mday > cal_days(tm_time->tm_year, tm_time->tm_mon)) {
                        tm_time->tm_mday = 1;
                        tm_time->tm_mon += 1;

                        if (tm_time->tm_mon > 12) {
                            tm_time->tm_mon = 1;
                            ++tm_time->tm_year;
                        }
                    }
                }
            }
        }
    } while (incr);
}
u32 conver_date_time(struct tm *tm_time)
{
    u32 date_time;

    date_time = tm_time->tm_sec;
    date_time |= tm_time->tm_min << 6;
    date_time |= tm_time->tm_hour << 12;
    date_time |= tm_time->tm_mday << 17;
    date_time |= tm_time->tm_mon << 22;
    date_time |= (tm_time->tm_year - 2015) << 26;

    return date_time;
}

int send_media_packet(struct __packet_info *pinfo)
{
    /* puts("send_media_packet\n"); */
    int ret = 0;
    char name[32] = {0};

    struct frm_head *frame_head = (struct frm_head *)pinfo->data;
    read_height_and_length(pinfo->fd, &pinfo->info);
    memset(frame_head, 0, sizeof(struct frm_head) + sizeof(struct media_info));
    //send media info packet
    frame_head->type = MEDIA_INFO_TYPE;
    frame_head->seq = 0;


    struct media_info *media = (struct media_info *)(pinfo->data + sizeof(struct frm_head));
    media->length = pinfo->info.length >> 16;
    media->height = pinfo->info.height >> 16;
    media->fps    = pinfo->info.scale / pinfo->info.sample_duration;
    media->audio_rate   = pinfo->info.sample_rate;

    media->dur_time = pinfo->info.durition / pinfo->info.scale;



    printf("=====media->dur_time=%d\n", media->dur_time);
    strcpy(media->filename, pinfo->file_name);
    /* strcpy(pinfo->data + sizeof(struct frm_head) + sizeof(struct media_info),pinfo->file_name); */
    size_t len = sizeof(struct frm_head) + sizeof(struct media_info)  + strlen(pinfo->file_name);
    frame_head->frm_sz = len - sizeof(struct frm_head);

    ret = sock_send(pinfo->sock, (char *)pinfo->data, len, 0);

    return ret;
}
int send_date_packet(struct __packet_info *pinfo, u32 msec)
{
    int ret = 0;
    u32 date = 0;
    struct tm tm_time;
    struct frm_head *frame_head = (struct frm_head *)pinfo->data;
    frame_head->type = DATE_TIME_TYPE;

    frame_head->frm_sz = 4;
#if 1
    printf("xdt=%d-%d-%d-%d-%d-%d\n"
           , pinfo->time.tm_year
           , pinfo->time.tm_mon
           , pinfo->time.tm_mday
           , pinfo->time.tm_hour
           , pinfo->time.tm_min
           , pinfo->time.tm_sec);
#endif

    incr_date_time(&pinfo->time, msec);
    date = conver_date_time(&pinfo->time);

    memcpy(pinfo->data + sizeof(struct frm_head), &date, frame_head->frm_sz);

    ret = sock_send(pinfo->sock, (char *)pinfo->data, sizeof(struct frm_head) + 4, 0);
    return ret;
}
int send_end_packet(struct __packet_info *pinfo)
{
    puts("send_end_packet\n");
    int ret = 0;
    struct frm_head *frame_head = (struct frm_head *)pinfo->data;
    memset(frame_head, 0, sizeof(struct frm_head));

    if (pinfo->fd == NULL) {
        puts("PLAY_OVER_TYPE | LAST_FREG_MAKER\n");
        frame_head->type = (PLAY_OVER_TYPE | LAST_FREG_MAKER);
    } else {

        puts("PLAY_OVER_TYPE\n");
        frame_head->type = PLAY_OVER_TYPE;
    }

    ret = sock_send(pinfo->sock, (char *)pinfo->data, sizeof(struct frm_head), 0);
    return ret;

}
int update_data(struct __packet_info *pinfo)
{

    if (pinfo->info.stco_tab != NULL) {
        free(pinfo->info.stco_tab);
        pinfo->info.stco_tab = NULL;
    }

    if (pinfo->info.stsz_tab != NULL) {
        free(pinfo->info.stsz_tab);
        pinfo->info.stsz_tab = NULL;
    }

    if (pinfo->info.audio_stco_tab != NULL) {
        free(pinfo->info.audio_stco_tab);
        pinfo->info.audio_stco_tab = NULL;
    }
#if 0
    if (pinfo->fd != NULL) {
        fclose(pinfo->fd);
        pinfo->fd = NULL;

    }
#endif

    return get_mov_media_info(pinfo);
}



/*********************************GPS*************************************/
#define MOV_GPS_BUF_LEN			60*10*512
#define MOV_FILE_GPS_HEAD_ADDR	0X70
static int gps_data_addr = 0;//GPS文件地址
static int gps_data_len = 0;//GPS数据长度
static short gps_packet_seq = 0;//GPS数据包SEQ
static u8 *gps_data_buf = NULL;//RAM GPS数据地址
static int last_sec = 0; //用在快进
#define GPS_ERROR()	 {printf("\nerror : func : %s line %d \n\n",__func__,__LINE__);return -1;}

static int gps_get_data(char *src, int src_len, char *buf, int buf_len)
{
    static int addr = 0;
    int len = 0;
    char *pstart, *pend, *pbuf;
    addr = gps_data_addr;
    if (addr >= src_len) {
        addr = 0;
        return -1;
    }
    pend = src + addr;
    pstart = strstr(pend, "$GPRMC");
    if (pstart) {
        pend = strstr(pstart, "\r\n");
        if (pend) {
            pbuf = strstr(pend, "$GPGGA");
            if (pbuf) {
                pbuf = strstr(pbuf + strlen("\r\n"), "\r\n");
            }
            if (pbuf) {
                pend = pbuf;
            }
            len = pend - pstart + strlen("\r\n");

            pbuf = strstr(pend, "$GPGSA");
            if (pbuf) {
                pbuf = strstr(pbuf + strlen("\r\n"), "\r\n");
            }
            if (pbuf) {
                pend = pbuf;
            }
            len = pend - pstart + strlen("\r\n");
            if (buf_len < len) {
                len = 0;
                goto exit;
            }
            memcpy(buf, pstart, len);
        }
    }
exit:
    return len;
}

static int send_no_gps_media(struct __packet_info *pinfo)
{
    int ret = 0;
    int len = 0;
    char buf[32] = "NO GPS DATA\r\n";
    struct frm_head *frame_head = (struct frm_head *)pinfo->data;

    len = strlen(buf);
    frame_head->type = NO_GPS_DATA_TYPE;
    frame_head->frm_sz = len;
    frame_head->seq = 0;
    memcpy(pinfo->data + sizeof(struct frm_head), buf, len);
    printf("%s\n", buf);

    ret = sock_send(pinfo->sock, pinfo->data, sizeof(struct frm_head) + len, 0);
    if (ret != sizeof(struct frm_head) + len) {
        printf("gps media send err!!!\n\n");
        return ret;
    }
    return 0;
}

//data_len:<0无数据，>0有数据
static int send_gps_media(struct __packet_info *pinfo, int data_len)
{
    int ret = 0;
    int len = 0;
    char buf[32] = "GPS DATA\r\n";
    struct frm_head *frame_head = (struct frm_head *)pinfo->data;

    if (data_len > 0) {
        len = strlen(buf);
        frame_head->type = GPS_INFO_TYPE;
        frame_head->frm_sz = len;
        frame_head->seq = 0;
        memcpy(pinfo->data + sizeof(struct frm_head), buf, len);
        ret = sock_send(pinfo->sock, pinfo->data, sizeof(struct frm_head) + len, 0);
        if (ret != sizeof(struct frm_head) + len) {
            printf("gps media send err!!!\n\n");
            return ret;
        }
    } else {
        frame_head->type = GPS_INFO_TYPE;
        frame_head->frm_sz = 0;
        frame_head->seq = 0;
        ret = sock_send(pinfo->sock, pinfo->data, sizeof(struct frm_head), 0);
        if (ret != sizeof(struct frm_head)) {
            printf("gps media send err!!!\n\n");
            return ret;
        }
    }
    return 0;
}

int find_gps_data(struct __packet_info *pinfo)
{
    int current_addr;
    int err;
    last_sec = 0;
    if (pinfo->fd == NULL) {
        GPS_ERROR();
    }
    if (fseek(pinfo->fd, MOV_FILE_GPS_HEAD_ADDR, SEEK_SET) == FALSE) {
        GPS_ERROR();
    }
    if (fread(pinfo->fd, &gps_data_addr, sizeof(gps_data_addr)) != sizeof(gps_data_addr)) {
        GPS_ERROR();
    }
    gps_data_addr = ntohl(gps_data_addr);
    if (gps_data_addr == 0) {
        fseek(pinfo->fd, 0, SEEK_SET);
        send_no_gps_media(pinfo);
        return -1;
    }
    if (fseek(pinfo->fd, gps_data_addr, SEEK_SET) == FALSE) {
        GPS_ERROR();
    }
    if (fread(pinfo->fd, &gps_data_len, sizeof(gps_data_len)) != sizeof(gps_data_len)) {
        GPS_ERROR();
    }
    gps_data_len = ntohl(gps_data_len);
    if (gps_data_len == 0) {
        fseek(pinfo->fd, 0, SEEK_SET);
        send_no_gps_media(pinfo);
        return -1;
    }
    if (gps_data_buf == NULL) {
        gps_data_buf = malloc(MOV_GPS_BUF_LEN);
        if (!gps_data_buf) {
            fseek(pinfo->fd, 0, SEEK_SET);
            GPS_ERROR();
        }
        memset(gps_data_buf, 0, MOV_GPS_BUF_LEN);
    } else {
        memset(gps_data_buf, 0, MOV_GPS_BUF_LEN);
    }
    err = fread(pinfo->fd, gps_data_buf, gps_data_len);
    if (err != gps_data_len) {
        fseek(pinfo->fd, 0, SEEK_SET);
        GPS_ERROR();
    }
    gps_data_addr = 0;
    gps_packet_seq = 0;
    if (fseek(pinfo->fd, 0, SEEK_SET) == FALSE) {
        GPS_ERROR();
    }
    printf("MOV GPS data find succes\n\n");
    return 0;
}


int send_gps_data_packet(struct __packet_info *pinfo)
{
    int ret;
    int i;
    int len;
    int current_addr;
    int timer;
    char fast_mode;
    char send_buf[512] = {0};
    struct frm_head *frame_head = (struct frm_head *)pinfo->data;

    if (frame_head == NULL || gps_data_len == 0) {
        /*printf("gps head or data len 0 !!!\n\n");*/
        return -1;
    }
    timer = pinfo->time.tm_hour * 3600 + pinfo->time.tm_min * 60 + pinfo->time.tm_sec;
    if (!last_sec) {
        fast_mode = 1;
        last_sec  = timer;
    } else {
        fast_mode = timer - last_sec;
        last_sec  = timer;
    }
    for (; fast_mode >= 1; fast_mode--) {
        len = gps_get_data(gps_data_buf, gps_data_len, send_buf, sizeof(send_buf));
        if (len <= 0) {
            if (len < 0) {
                printf("GPS data read end \n");
            } else {
                printf("GPS data read err \n");
            }
            return -1;
        }
        gps_data_addr += len;
        gps_packet_seq++;
        frame_head->type = GPS_INFO_TYPE;
        frame_head->frm_sz = len;
        frame_head->seq = gps_packet_seq;

        memcpy(pinfo->data + sizeof(struct frm_head), send_buf, len);
        ret = sock_send(pinfo->sock, pinfo->data, sizeof(struct frm_head) + len, 0);
        if (ret != (sizeof(struct frm_head) + len)) {
            printf("gps data send err !!!\n\n");
            return ret;
        }
    }
    printf("MOV GPS data send succes , len : %d \n\n", len);
    return ret;
}

int unfind_gps_data(void)
{
    if (gps_data_buf != NULL) {
        free(gps_data_buf);
    }
    gps_data_buf = NULL;
    gps_data_len = 0;
    gps_data_addr = 0;
    gps_packet_seq = 0;
    return 0;
}
/******************************************************************/


