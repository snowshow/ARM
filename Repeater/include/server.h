#ifndef _SERVER_H
#define _SERVER_H

int create_tcp_server(const char* hostname, const char* servname);
int create_socket_stream(const char* hostname, const char* servname);

#endif
