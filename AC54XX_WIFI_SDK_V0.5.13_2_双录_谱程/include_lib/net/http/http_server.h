#ifndef  __HTTP_SERVER_H__
#define  __HTTP_SERVER_H__

int http_get_server_init(unsigned short port);

void http_get_server_discpnnect_cli(struct sockaddr_in *dst_addr);

void http_get_server_uninit(void);

void http_cli_thread_kill(struct http_cli_t *cli);

#endif  /*HTTP_SERVER_H*/
