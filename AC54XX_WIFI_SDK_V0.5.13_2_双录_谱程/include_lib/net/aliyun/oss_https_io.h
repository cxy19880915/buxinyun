#ifndef __OSS_HTTPS_IO_H__
#define __OSS_HTTPS_IO_H__

#include "ssl.h"
#include "certs.h"
#include "net.h"
#include "x509_crt.h"
#include "pk.h"
#include "entropy.h"
#include "ctr_drbg.h"
#include "debug.h"

#define LF     (char) 10
#define CR     (char) 13
#define CRLF   "\x0d\x0a"
#define LAST_CHUNK   "0\r\n\r\n"


#define HTTP_RESP_200  "200 OK"
#define HTTP_RESP_404  "404 Not Found"
#define HTTP_RESP_204  "204 No Content"
#define HTTP_RESP_304  "304 Not Modified"
#define HTTP_RESP_403  "403 Forbidden"

#define HTTP_GET "GET"
#define HTTP_HEAD "HEAD"
#define HTTP_PUT "PUT"
#define HTTP_POST "POST"
#define HTTP_DELETE "DELETE"

#define AOS_MAX_URI_LEN 2048
#define AOS_MAX_RESP_LEN 2048

#define AOS_HTTP_PREFIX   "http://"
#define AOS_HTTPS_PREFIX  "https://"
#define AOS_RTMP_PREFIX   "rtmp://"
#define AOS_VERSION    "3.2.1"
#define AOS_VER        "aliyun-sdk-c/" AOS_VERSION

#define OSS_CANNONICALIZED_HEADER_PREFIX "x-oss-"
#define OSS_CANNONICALIZED_HEADER_DATE "x-oss-date"
#define OSS_CANNONICALIZED_HEADER_ACL "x-oss-acl"
#define OSS_CANNONICALIZED_HEADER_COPY_SOURCE "x-oss-copy-source"
#define OSS_CONTENT_MD5 "Content-MD5"
#define OSS_CONTENT_TYPE "Content-Type"
#define OSS_CONTENT_LENGTH "Content-Length"
#define OSS_DATE "Date"
#define OSS_AUTHORIZATION "Authorization"
#define OSS_ACCESSKEYID "OSSAccessKeyId"
#define OSS_EXPECT "Expect"
#define OSS_EXPIRES "Expires"
#define OSS_SIGNATURE "Signature"
#define OSS_ACL "acl"
#define OSS_PREFIX "prefix"
#define OSS_DELIMITER "delimiter"
#define OSS_MARKER "marker"
#define OSS_MAX_KEYS "max-keys"
#define OSS_UPLOADS "uploads"
#define OSS_UPLOAD_ID "uploadId"
#define OSS_MAX_PARTS "max-parts"
#define OSS_PART_NUMBER_MARKER "part-number-marker"
#define OSS_KEY_MARKER "key-marker"
#define OSS_UPLOAD_ID_MARKER "upload-id-marker"
#define OSS_MAX_UPLOADS "max-uploads"
#define OSS_PARTNUMBER "partNumber"
#define OSS_APPEND "append"
#define OSS_POSITION "position"
#define OSS_MULTIPART_CONTENT_TYPE "application/x-www-form-urlencoded"
#define OSS_COPY_SOURCE "x-oss-copy-source"
#define OSS_COPY_SOURCE_RANGE "x-oss-copy-source-range"
#define OSS_STS_SECURITY_TOKEN "x-oss-security-token"
#define OSS_REPLACE_OBJECT_META "x-oss-replace-object-meta"
#define OSS_OBJECT_TYPE "x-oss-object-type"
#define OSS_NEXT_APPEND_POSITION "x-oss-next-append-position"
#define OSS_HASH_CRC64_ECMA "x-oss-hash-crc64ecma"
#define OSS_CALLBACK "x-oss-callback"
#define OSS_PROCESS "x-oss-process"
#define OSS_LIFECYCLE "lifecycle"
#define OSS_DELETE "delete"
#define OSS_YES "yes"
#define OSS_OBJECT_TYPE_NORMAL "Normal"
#define OSS_OBJECT_TYPE_APPENDABLE "Appendable"
#define OSS_LIVE_CHANNEL "live"
#define OSS_LIVE_CHANNEL_STATUS "status"
#define OSS_COMP "comp"
#define OSS_LIVE_CHANNEL_STAT "stat"
#define OSS_LIVE_CHANNEL_HISTORY "history"
#define OSS_LIVE_CHANNEL_VOD "vod"
#define OSS_LIVE_CHANNEL_START_TIME "startTime"
#define OSS_LIVE_CHANNEL_END_TIME "endTime"
#define OSS_PLAY_LIST_NAME "playlistName"
#define LIVE_CHANNEL_STATUS_DISABLED "disabled"
#define LIVE_CHANNEL_STATUS_ENABLED "enabled"
#define LIVE_CHANNEL_STATUS_IDLE "idle"
#define LIVE_CHANNEL_STATUS_LIVE "live"
#define LIVE_CHANNEL_DEFAULT_TYPE "HLS"
#define LIVE_CHANNEL_DEFAULT_PLAYLIST "playlist.m3u8"
#define LIVE_CHANNEL_DEFAULT_FRAG_DURATION  5
#define LIVE_CHANNEL_DEFAULT_FRAG_COUNT  3
#define OSS_MAX_PART_NUM  10000
#define OSS_PER_RET_NUM  1000
#define MAX_SUFFIX_LEN  1024


struct https_io_options {
    mbedtls_net_context net_context;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;
    mbedtls_ssl_session saved_session;
    char *remote_host;
    const char *remote_port;
    u32 timeout_ms;
    u16 remote_host_len;
    char session_flag;
    char connected_flag;
    u32 resp_content_len;
    u32 resp_beyond_len;
    char resp_beyond_buf[AOS_MAX_RESP_LEN];
};

extern void oss_https_request_disconnect(struct https_io_options *https_io_options);
extern int oss_https_request_connect(struct https_io_options *https_io_options);
extern int oss_https_write_request(struct https_io_options *https_io_options, const void *buf, u32 len);
extern int oss_https_read_response(struct https_io_options *https_io_options, void *buf, u32 len);
extern int oss_https_read_content(struct https_io_options *https_io_options, void *buf, u32 len);
extern int oss_https_select(struct https_io_options *https_io_options, char *write_able, char *read_able);


#endif
