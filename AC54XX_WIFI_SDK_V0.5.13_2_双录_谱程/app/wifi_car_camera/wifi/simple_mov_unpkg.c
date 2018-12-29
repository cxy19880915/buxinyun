#include "generic/typedef.h"
#include "fs/fs.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "server/simple_mov_unpkg.h"

#define UNPKG_ERR() do {printf("%s  %d\n",__func__,__LINE__);return -1;}while(0)

int seek_to_moov(FILE *fp)
{
    int offset_moov = 0;
    if (fseek(fp, 0x1C, SEEK_SET) == false) {
        UNPKG_ERR();
    }
    //fread(&offset_moov,1,4,fp);
    if (fread(fp, &offset_moov, 4) != sizeof(u32)) {
        UNPKG_ERR();
    }
    if (fseek(fp, 0x1C + htonl(offset_moov), SEEK_SET) == false) {
        UNPKG_ERR();

    }
    return 0x1C + htonl(offset_moov);

}

int seek_to_stsz(FILE *fp)
{
    u32 size;
    int total_offset;
    total_offset = seek_to_moov(fp);
    if (total_offset == -1) {
        UNPKG_ERR();
    }
    //跳过stts   0x18
//跳过stsd
    total_offset += 0x1C5;
    if (fseek(fp, total_offset, SEEK_SET) == false) {
        UNPKG_ERR();
    }

    if (sizeof(u32) != fread(fp, &size, 4)) {
        UNPKG_ERR();
    }


    size = htonl(size);
    total_offset += (size + 0x18);
    if (false == fseek(fp, total_offset, SEEK_SET)) {
        UNPKG_ERR();
    }

    //跳过stss
    fread(fp, &size, 4);
    size = htonl(size);
    total_offset += (size);
    if (false == fseek(fp, total_offset, SEEK_SET)) {
        UNPKG_ERR();
    }
    //跳过stsc
    if (sizeof(u32) != fread(fp, &size, 4)) {
        UNPKG_ERR();
    }
    size = htonl(size);
    total_offset += (size);
    return total_offset;
}

int get_audio_sample_count(FILE *fp)
{
    int total_offset;
    u32 size;
    total_offset = seek_to_moov(fp);
    if (total_offset == -1) {
        UNPKG_ERR();
    }
    total_offset += 116;
    if (false == fseek(fp, total_offset, SEEK_SET)) {
        UNPKG_ERR();
    }
    if (sizeof(u32) != fread(fp, &size, 4)) {
        UNPKG_ERR();
    }
    size = htonl(size);

    total_offset += (size  + 0x199 + 0x10);
    if (false == fseek(fp, total_offset, SEEK_SET)) {
        UNPKG_ERR();
    }
    if (sizeof(u32) != fread(fp, &size, 4)) {
        UNPKG_ERR();
    }
    size = htonl(size);

    /* printf("music stts size -> %d\n", size); */

    return size;
}


int get_audio_sample_rate(FILE *fp)
{
    int total_offset;
    u32 size;
    total_offset = seek_to_moov(fp);
    if (total_offset == -1) {
        UNPKG_ERR();
    }
    total_offset += 116;
    if (false == fseek(fp, total_offset, SEEK_SET)) {
        UNPKG_ERR();
    }
    if (sizeof(u32) != fread(fp, &size, 4)) {
        UNPKG_ERR();
    }
    size = htonl(size);
    total_offset += (size  + 0xA4);
    if (false == fseek(fp, total_offset, SEEK_SET)) {
        UNPKG_ERR();
    }
    if (sizeof(u32) != fread(fp, &size, 4)) {
        UNPKG_ERR();
    }
    size = htonl(size);

    /* printf("sample rate-> %d\n", size); */

    return size;
}



int get_audio_chunk_offset_entry(FILE *fp)
{
    int total_offset;
    u32 size;
    total_offset = seek_to_moov(fp);
    if (total_offset == -1) {
        UNPKG_ERR();
    }
    total_offset += 116;
    if (false == fseek(fp, total_offset, SEEK_SET)) {
        UNPKG_ERR();
    }

    if (sizeof(u32) != fread(fp, &size, 4)) {
        UNPKG_ERR();
    }
    size = htonl(size);

    total_offset += (size  + 0x1E1 + 12);
    if (false == fseek(fp, total_offset, SEEK_SET)) {
        UNPKG_ERR();
    }
    if (sizeof(u32) != fread(fp, &size, 4)) {
        UNPKG_ERR();
    }
    size = htonl(size);

    /* printf("music stco size -> %d\n", size); */

    return size;
}
int seek_to_audio_stco(FILE *fp)
{
    int total_offset;
    u32 size;
    total_offset = seek_to_moov(fp);
    if (total_offset == -1) {
        UNPKG_ERR();
    }
    total_offset += 116;
    if (false == fseek(fp, total_offset, SEEK_SET)) {
        UNPKG_ERR();
    }

    if (sizeof(u32) != fread(fp, &size, 4)) {
        UNPKG_ERR();
    }
    size = htonl(size);

    total_offset += (size  + 0x1E1);
    return total_offset;
}
u32 *get_audio_chunk_table(FILE *fp, struct __mov_unpkg_info *info)
{
    u32 *audio_stco_tab = NULL;
    u32 size = 0;
    int ret;
    int offset = 0;
    offset = seek_to_audio_stco(fp);
    if (offset == -1) {
        printf("%s  %d\n", __func__, __LINE__);
        return NULL;
    }
    if (false == fseek(fp, offset + 12, SEEK_SET)) {
        printf("%s  %d\n", __func__, __LINE__);
        return NULL;

    }
    if (sizeof(u32) != fread(fp, &size, 4)) {
        printf("%s  %d\n", __func__, __LINE__);
        return NULL;
    }
    size = htonl(size);

    /* printf("size_count-> %d\n", size); */
    audio_stco_tab = malloc(size * sizeof(u32));
    if (audio_stco_tab == NULL) {
        return NULL;
    }

    ret = fread(fp, audio_stco_tab, size * sizeof(u32));
    if (ret != size * sizeof(u32)) {
        free(audio_stco_tab);
        return NULL;
    }
    return audio_stco_tab;
}
int get_audio_chunk_offset(u32 *audio_stco_tab, u32 count)
{
    if (audio_stco_tab == NULL) {
        return -1;
    }
    return htonl(audio_stco_tab[count]);
}

int seek_to_stco(FILE *fp)
{
    u32 size;
    int total_offset;
    total_offset = seek_to_moov(fp);
    if (total_offset == -1) {
        UNPKG_ERR();
    }
    //跳过stts   0x18
//跳过stsd
    total_offset += 0x1C5;
    if (false  == fseek(fp, total_offset, SEEK_SET)) {
        UNPKG_ERR();
    }
    if (sizeof(u32) != fread(fp, &size, 4)) {
        UNPKG_ERR();
    }
    size = htonl(size);
    total_offset += (size  + 0x18);
    if (false == fseek(fp, total_offset, SEEK_SET)) {
        UNPKG_ERR();
    }


    //跳过stss
    if (sizeof(u32) != fread(fp, &size, 4)) {
        UNPKG_ERR();
    }
    size = htonl(size);
    total_offset += size ;
    if (false == fseek(fp, total_offset, SEEK_SET)) {
        UNPKG_ERR();
    }
    //跳过stsc
    if (sizeof(u32) != fread(fp, &size, 4)) {
        UNPKG_ERR();
    }
    size = htonl(size);
    total_offset += size ;
    if (false == fseek(fp, total_offset, SEEK_SET)) {
        UNPKG_ERR();
    }
    //跳过stsz
    if (sizeof(u32) != fread(fp, &size, 4)) {
        UNPKG_ERR();
    }
    size = htonl(size);

    total_offset += size ;

    return total_offset;
}


u32 *get_sample_index_table(FILE *fp, struct __mov_unpkg_info *info)
{
    u32 *stsz_tab = NULL;
    u32 sample_count = 0;
    int ret;
    int offset = 0;
    offset = seek_to_stsz(fp);
    if (offset == -1) {
        printf("%s  %d\n", __func__, __LINE__);
        return NULL;
    }
    if (false == fseek(fp, 16 + offset, SEEK_SET)) {
        printf("%s   %d\n", __func__, __LINE__);
        return NULL;
    }
    if (sizeof(u32) != fread(fp, &sample_count, 4)) {
        printf("%s   %d\n", __func__, __LINE__);
        return NULL;

    }
    sample_count = htonl(sample_count);
    stsz_tab = malloc(sample_count * sizeof(u32));
    if (stsz_tab == NULL) {
        return NULL;
    }
    ret = fread(fp, stsz_tab, sample_count * sizeof(u32));
    if (ret != sample_count * sizeof(u32)) {
        free(stsz_tab);
        return NULL;
    }
    return stsz_tab;
}

int is_vaild_mov_file(FILE *fp)
{
    int result1 = 0;
    int result2 = 0;
    u32 size = 0;
    int offset = 0;
    char moov[9] = {0};
    offset = seek_to_moov(fp);
    fread(fp, moov, 8);
    if (strcmp(moov + 4, "moov")) {
        result1 = 1;
    }
    fseek(fp, offset + 0x1BD + 24, SEEK_SET);

    fread(fp, moov, 8);

    if (memcmp(moov + 4, "avc", 3)) {
        result2 = 1;
    }

    if (result1 || result2) {
        return 0;
    } else {
        return 1;
    }


}

int is_has_audio(FILE *fp)
{
    int total_offset;
    char buf[9] = {0};
    u32 tark_len;
    total_offset = seek_to_moov(fp);

    total_offset += (108 + 8);
    fseek(fp, total_offset, SEEK_SET);

    fread(fp, &tark_len, 4);
    tark_len = htonl(tark_len);
    total_offset += (tark_len);
    fseek(fp, total_offset, SEEK_SET);

    fread(fp, buf, 8);

    /* printf("trak_len->0x%x  name->%s\n",tark_len,buf+4); */
    if (!memcmp(buf + 4, "trak", 4)) {
        return 1;
    } else {
        return 0;
    }


}


int get_sample_size(u32 *stsz_table, u32 count)
{
    if (stsz_table == NULL) {
        return -1;
    }
    return htonl(stsz_table[count]);
}



u32 *get_chunk_index_table(FILE *fp, struct __mov_unpkg_info *info)
{
    u32 *stco_tab = NULL;
    char buf[12];
    int ret;
    u32 chunk_count = 0;
    int offset = 0;
    offset = seek_to_stco(fp);
    if (offset == -1) {
        printf("%s   %d\n", __func__, __LINE__);
        return NULL;

    }
    if (false == fseek(fp, offset + 12, SEEK_SET)) {
        printf("%s   %d\n", __func__, __LINE__);
        return NULL;

    }
    if (sizeof(u32) != fread(fp, &chunk_count, 4)) {
        printf("%s   %d\n", __func__, __LINE__);
        return NULL;

    }

    chunk_count = htonl(chunk_count);

    info->video_sample_count = chunk_count;

    stco_tab = malloc(chunk_count * sizeof(u32));
    if (stco_tab == NULL) {
        return NULL;
    }

    ret = fread(fp, stco_tab, chunk_count * sizeof(u32));
    if (ret != chunk_count * sizeof(u32)) {
        free(stco_tab);
        return NULL;
    }
    return stco_tab;
}

int get_chunk_offset(u32 *stco_tab, u32 count)
{
    if (stco_tab == NULL) {
        return -1;
    }
    return htonl(stco_tab[count]);
}
int read_stts(FILE *fp, struct __mov_unpkg_info *info)
{
    u32 size = 0;
    int offset = 0;
    offset = seek_to_moov(fp);
    if (offset == -1) {
        UNPKG_ERR();
    }
//跳过stsd
    offset += 0x1C5;
    if (false == fseek(fp, offset, SEEK_SET)) {
        UNPKG_ERR();
    }
    if (sizeof(u32) != fread(fp, &size, 4)) {
        UNPKG_ERR();
    }
    size = htonl(size);
    offset += (size);
    if (false == fseek(fp, offset + 20, SEEK_SET)) {
        UNPKG_ERR();
    }

    if (sizeof(u32) != fread(fp, &info->sample_duration, 4)) {
        UNPKG_ERR();
    }
    info->sample_duration = htonl(info->sample_duration);
    return 0;
    /* printf("sample_duration -> %d\n", info->sample_duration); */
}
int read_time_scale_dur(FILE *file_fp, struct __mov_unpkg_info *info)
{
    int offset = 0;
    offset = seek_to_moov(file_fp);
    if (offset == -1) {
        UNPKG_ERR();
    }
    if (false == fseek(file_fp, offset + 280, SEEK_SET)) {
        UNPKG_ERR();
    }
    if (sizeof(u32) != fread(file_fp, &info->scale, 4)) {
        UNPKG_ERR();
    }
    info->scale = htonl(info->scale);
    if (sizeof(u32) != fread(file_fp, &info->durition, 4)) {
        UNPKG_ERR();
    }
    info->durition = htonl(info->durition);
    return 0;
    /* printf("scale : %d     durition : %d \n", info->scale, info->durition); */

}

int read_height_and_length(FILE *file_fp, struct __mov_unpkg_info *info)
{
    int offset = 0;
    offset = seek_to_moov(file_fp);
    if (offset == -1) {
        UNPKG_ERR();
    }
    if (false == fseek(file_fp, offset + 8 + 108 + 92, SEEK_SET)) {
        UNPKG_ERR();
    }
    if (sizeof(u32) != fread(file_fp, &info->length, 4)) {
        UNPKG_ERR();
    }
    info->length = htonl(info->length);

    if (sizeof(u32) != fread(file_fp, &info->height, 4)) {
        UNPKG_ERR();
    }
    info->height = htonl(info->height);
    return 0;
    /* printf("length : %d     height : %d \n", info->length >> 16, info->height >> 16); */

}



