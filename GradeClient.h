#ifndef GRADECLIENT_H
#define GRADECLIENT_H

typedef struct user user;

enum type{STUDENT, TA};

struct addrinfo* alloc_tcp_addr (const char *host, int port, int flags);
int tcp_connect(const char* host, int port);

#define HOST_NAME_MAX 100
#define COMMAND_MAX 256
#define READ  0
#define WRITE 1

#endif
