/*************************************************************************
	> File Name: include_lib/upgrade/ecc_rs_api.h
	> Author:
	> Mail:
	> Created Time: Tue 23 May 2017 04:59:33 PM HKT
 ************************************************************************/

#ifndef _UPGRADE_ECC_RS_API_H
#define _UPGRADE_ECC_RS_API_H

#ifdef __cplusplus
extern "C" {

#endif
//#pragma comment(lib,"rs_v11.lib")

typedef struct ecc_rs {
    unsigned char *data_ptr;
    int data_len;
    unsigned short *check_ptr;
    unsigned char *out_ptr;
} ECC_RS_STRU;


typedef struct ecc_rs_api {
    unsigned int (*need_buf)(int max_err);    //最大纠错数
    void (*initialize_ecc)(unsigned char *ptr, int max_err);    //初始化
    void (*encode_data)(unsigned char *ptr, struct ecc_rs *ecc_rs);  //计算校验信息
    int (*decode_data)(unsigned char *ptr, struct ecc_rs *ecc_rs);  //计算校验信息
} ECC_RS_API;

extern struct ecc_rs_api  *get_ecc_rs_api();

#ifdef __cplusplus
}
#endif

#endif
