#ifndef __OSS_AUTH_H__
#define __OSS_AUTH_H__

#include "typedef.h"

extern void oss_sign_headers(char b64_buf[32],
                             const char *signstr,
                             u32 signstr_len,
                             const char *access_key_secret,
                             u32 access_key_secret_len);

#endif
