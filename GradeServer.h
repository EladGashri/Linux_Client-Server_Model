#ifndef GRADESERVER_H
#define GRADESERVER_H

#include <stdbool.h>

#define NUMBER_OF_THREADS 5
#define HOST_NAME_MAX 100
#define COMMAND_MAX 256

typedef struct query_queue query_queue;

typedef struct TAs_struct TAs_struct;

typedef struct students_struct students_struct;

void add_student(char *id, char* password, int grade);
void *handle_queries(void *void_client_fd);
void query_enqueue(char *buf);
char* query_dequeue();
void signal_handler();
bool in_TAs(TAs_struct *TAs, int number_of_TAs, char *id, char *password);
students_struct *in_students(char *id);
struct addrinfo* alloc_tcp_addr (const char *host, int port, int flags);
int tcp_establish(int port);

#endif
