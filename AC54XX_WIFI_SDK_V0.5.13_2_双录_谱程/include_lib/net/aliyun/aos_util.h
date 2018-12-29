#ifndef __AOS_UTIL_H__
#define __AOS_UTIL_H__

#include "typedef.h"

/** size of the SHA1 DIGEST */
#define APR_SHA1_DIGESTSIZE 20

/**
 * Define the Magic String prefix that identifies a password as being
 * hashed using our algorithm.
 */
#define APR_SHA1PW_ID "{SHA}"

/** length of the SHA Password */
#define APR_SHA1PW_IDLEN 5


extern u32 aos_base64_encode(const unsigned char *in, u32 inLen, char *out);

extern void HMAC_SHA1(unsigned char hmac[20], const unsigned char *key, u32 key_len,
                      const unsigned char *message, u32 message_len);

extern u32 oss_add_time_stamp(char *date);

#endif
