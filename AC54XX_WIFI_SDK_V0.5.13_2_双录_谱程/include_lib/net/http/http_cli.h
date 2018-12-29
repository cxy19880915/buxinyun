#ifndef _HTTP_CLI_H_
#define _HTTP_CLI_H_

typedef enum {
    HERROR_REDIRECT		        = 1,
    HERROR_OK			        = 0,
    HERROR_MEM			        = -1,
    HERROR_HEADER		        = -2,
    HERROR_RESPOND		        = -3,
    HERROR_SOCK			        = -4,
    HERROR_CALLBACK		        = -5,
    HERROR_UNKNOWN		        = -6,
    HERROR_PARAM		        = -7,
    HERROR_REDIRECT_DEEP		= -8,
} httpin_error;

typedef enum {
    HTTPIN_HEADER,
    HTTPIN_PROGRESS,
    HTTPIN_FINISHED,
    HTTPIN_ABORT,
    HTTPIN_ERROR,
    HTTPIN_NON_BLOCK,
} httpin_status;

/****************************************************/

typedef long (*httpcli_cb)(void *httpcli_ctx, void *buf, unsigned int size, void *priv, httpin_status status);

typedef struct httpcli_ctx {
    //user need init
    const char *url;
    int lowRange;
    int highRange;
    httpcli_cb cb;
    void *priv;
    char *post_data;
    int timeout_millsec;
    char *data_format;
    //user get
    int content_length;

    //priv
    int exit_flag;
    int req_exit_flag;
} httpcli_ctx;


/****************************************************/

httpin_error httpcli_get(httpcli_ctx *ctx);
httpin_error httpcli_post(httpcli_ctx *ctx);
void httpcli_cancel(httpcli_ctx *ctx);

httpin_error httpscli_get(httpcli_ctx *ctx);
httpin_error httpscli_post(httpcli_ctx *ctx);
void httpscli_cancel(httpcli_ctx *ctx);


long httpcli_content_length(httpcli_ctx *ctx);

#endif  //_HTTP_CLI_H_
