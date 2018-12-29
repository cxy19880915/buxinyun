#ifndef __OSS_OBJECT_H__
#define __OSS_OBJECT_H__

#include "typedef.h"

#define ALIYUN_DBUG_INFO_ON	1		/*!< Aliyun debug information on/off */

#define ALIYUN_MBEDTLS_DEBUG_LEVEL 0

#define FILE_TYPE_HTLM "text/html"
#define FILE_TYPE_CSS "text/css"
#define FILE_TYPE_XML "text/xml"
#define FILE_TYPE_GIF "image/gif"
#define FILE_TYPE_JPEG "image/jpeg"
#define FILE_TYPE_JS "application/x-javascript"
#define FILE_TYPE_ATOM "application/atom+xml"
#define FILE_TYPE_RSS "application/rss+xml"
#define FILE_TYPE_MML "text/mathml"
#define FILE_TYPE_TXT "text/plain"
#define FILE_TYPE_JAD "text/vnd.sun.j2me.app-descriptor"
#define FILE_TYPE_WML "text/vnd.wap.wml"
#define FILE_TYPE_HTC "text/x-component"
#define FILE_TYPE_PNG "image/png"
#define FILE_TYPE_TIF "image/tiff"
#define FILE_TYPE_WBMP "image/vnd.wap.wbmp"
#define FILE_TYPE_ICO "image/x-icon"
#define FILE_TYPE_JNG "image/x-jng"
#define FILE_TYPE_BMP "image/x-ms-bmp"
#define FILE_TYPE_SVG "image/svg+xml"
#define FILE_TYPE_WEBP "image/webp"
#define FILE_TYPE_JAR "application/java-archive"
#define FILE_TYPE_HQX "application/mac-binhex40"
#define FILE_TYPE_DOC "application/msword"
#define FILE_TYPE_PDF "application/pdf"
#define FILE_TYPE_PS "application/postscript"
#define FILE_TYPE_RTF "application/rtf"
#define FILE_TYPE_XLS "application/vnd.ms-excel"
#define FILE_TYPE_PPT "application/vnd.ms-powerpoint"
#define FILE_TYPE_VMLC "application/vnd.wap.wmlc"
#define FILE_TYPE_KML "application/vnd.google-earth.kml+xml"
#define FILE_TYPE_KMZ "application/vnd.google-earth.kmz"
#define FILE_TYPE_7Z "application/x-7z-compressed"
#define FILE_TYPE_CCO "application/x-cocoa"
#define FILE_TYPE_JARDIFF "application/x-java-archive-diff"
#define FILE_TYPE_JNLP "application/x-java-jnlp-file"
#define FILE_TYPE_RUN "application/x-makeself"
#define FILE_TYPE_PL "application/x-perl"
#define FILE_TYPE_PRC "application/x-pilot"
#define FILE_TYPE_RAR "application/x-rar-compressed"
#define FILE_TYPE_RPM "application/x-redhat-package-manager"
#define FILE_TYPE_SEA "application/x-sea"
#define FILE_TYPE_SWF "application/x-shockwave-flash"
#define FILE_TYPE_SIT "application/x-stuffit"
#define FILE_TYPE_TCL "application/x-tcl"
#define FILE_TYPE_PEM "application/x-x509-ca-cert"
#define FILE_TYPE_XPI "application/x-xpinstall"
#define FILE_TYPE_XHTLM "application/xhtml+xml"
#define FILE_TYPE_ZIP "application/zip"
#define FILE_TYPE_WGZ "application/x-nokia-widget"
#define FILE_TYPE_BIN "application/octet-stream"
#define FILE_TYPE_MIDI "audio/midi"
#define FILE_TYPE_MP3 "audio/mpeg"
#define FILE_TYPE_OGG "audio/ogg"
#define FILE_TYPE_M4A "audio/x-m4a"
#define FILE_TYPE_RA "audio/x-realaudio"
#define FILE_TYPE_3GPP "video/3gpp"
#define FILE_TYPE_MP4 "video/mp4"
#define FILE_TYPE_MPEG "video/mpeg"
#define FILE_TYPE_MOV "video/quicktime"
#define FILE_TYPE_WEBM "video/webm"
#define FILE_TYPE_FLV "video/x-flv"
#define FILE_TYPE_M4V "video/x-m4v"
#define FILE_TYPE_MNG "video/x-mng"
#define FILE_TYPE_ASF "video/x-ms-asf"
#define FILE_TYPE_WMV "video/x-ms-wmv"
#define FILE_TYPE_AVI "video/x-msvideo"
#define FILE_TYPE_TS "video/MP2T"
#define FILE_TYPE_M3U8 "application/x-mpegURL"
#define FILE_TYPE_APK "application/vnd.android.package-archive"

enum OBJECT_TYPE {
    NORMAL,
    APPENDABLE,
};


extern void *new_oss_object(void);
extern void free_oss_object(void *oss_object);
extern int oss_object_set_https_ops(void *oss_object, const char *remote_host, const char *remote_port, u32 timeout_ms);

extern int oss_set_object_name(void *oss_object, const char *oject_name);
extern int oss_set_access_key_id(void *oss_object, const char *access_key_id);
extern int oss_set_access_key_secret(void *oss_object, const char *access_key_secret);
extern int oss_set_access_token(void *oss_object, const char *access_token);
extern int oss_set_bucket_name(void *oss_object, const char *bucket_name);
extern void oss_set_file_type(void *oss_object, const char *file_type);
extern enum OBJECT_TYPE oss_get_object_type(void *oss_object);
extern u32 oss_get_object_resp_content_len(void *oss_object);
extern int oss_set_bucket_filter(void *oss_object, const char *prefix, const char *delimiter, const char *marker, u16 maxkey);

extern int oss_head_object(void *oss_object);
extern int oss_delete_object(void *oss_object);

extern int oss_put_object_request_start(void *oss_object, unsigned int content_len);
extern int oss_put_object_from_buffer(void *oss_object, const void *buf, unsigned int buf_len);
extern int oss_put_object_request_end(void *oss_object);

extern int oss_append_object_request_start(void *oss_object, unsigned int content_len);
extern int oss_append_object_from_buffer(void *oss_object, const void *buf, unsigned int buf_len);
extern int oss_append_object_request_end(void *oss_object);

extern int oss_get_object_request(void *oss_object);
extern int oss_get_object_to_buffer(void *oss_object, void *buf, unsigned int buf_len);

extern int oss_get_bucket_request(void *oss_object);

extern int oss_object_lock(void *oss_object);
extern int oss_object_unlock(void *oss_object);

#endif
