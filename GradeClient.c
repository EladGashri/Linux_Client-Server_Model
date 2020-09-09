#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <memory.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <ctype.h>
#include "GradeClient.h"

struct user{
char id[10]; bool login; enum type user_type;};

int main(int argc, char **argv){
    char *hostname = argv[1];
    int port = (int) atoi(argv[2]);
    int pipe_fd[2]; pipe(pipe_fd);
    char delimiter[2]=" ";

    if (fork()==0){  /* command line interpeter*/
        close(pipe_fd[READ]);
        for (;;) {
            char *command = (char*) malloc(sizeof(char)*COMMAND_MAX);
            printf("> "); fgets(command, COMMAND_MAX, stdin);
            char *p = strchr(command,'\n');
            if (p!=NULL) {*p='\0';}
            if (strcmp(command,"Exit")==0 || strcmp(command,"Logout")==0 || strcmp(command,"ReadGrade")==0 || strcmp(command,"GradeList")==0){
                write(pipe_fd[WRITE], command, strlen(command)+1);
                if (strcmp(command,"Exit")==0) {break;}
            }else{
                char *copy_command = (char*) malloc(strlen(command)*sizeof(char));
                strcpy(copy_command,command);
                char *command_no_args = (char*) malloc(COMMAND_MAX*sizeof(char));
                char *arg_1 = (char*) malloc(COMMAND_MAX*sizeof(char));
                char *arg_2 = (char*) malloc(COMMAND_MAX*sizeof(char));
                char *arg_3 = (char*) malloc(COMMAND_MAX*sizeof(char));
                command_no_args=strtok(command, delimiter); arg_1 = strtok(NULL, delimiter);
                arg_2 = strtok(NULL, delimiter); arg_3 = strtok(NULL, delimiter);

                if (strcmp(command_no_args, "Login")==0 || strcmp(command_no_args, "UpdateGrade")==0){
                    if (arg_1==NULL || strlen(arg_1)!=9 || arg_2==NULL || arg_3!=NULL) {printf("Wrong Input\n"); continue;}
                    write(pipe_fd[WRITE], copy_command, strlen(copy_command)+1);

                }else if (strcmp(command_no_args, "ReadGrade")==0){
                    if (arg_1==NULL || strlen(arg_1)!=9 || arg_2!=NULL){printf("Wrong Input\n"); continue;}
                    write(pipe_fd[WRITE], copy_command, strlen(copy_command)+1);

                }else {printf("Wrong Input\n");}
            }
        sleep(1);
        }

    }else { /* communication process */
        close(pipe_fd[WRITE]);
        user *current_user = (user*) malloc(sizeof(user));
        current_user->login = false;
        char read_command[COMMAND_MAX];
        for (;;){
            read(pipe_fd[READ], read_command, COMMAND_MAX);

            if (strcmp(read_command,"Exit")==0){
                current_user->login = false;
                break;

            }else if (strcmp(read_command,"Logout")==0){
                if (current_user->login==false){printf("Not logged in\n"); continue;}
                else {printf("Good bye %s\n", current_user->id);
                current_user->login = false;}

            }else {
                int socket_fd = tcp_connect(hostname, port);
                char buf[COMMAND_MAX];

                char *copy_command = (char*) malloc(strlen(read_command)*sizeof(char));
                strcpy(copy_command,read_command);

                char *command_no_args = (char*) malloc(COMMAND_MAX*sizeof(char));
                char *id = (char*) malloc(10*sizeof(char));
                command_no_args=strtok(read_command, delimiter);
                id=strtok(NULL, delimiter);

                if (strcmp(command_no_args,"Login")==0){
                    if (current_user->login){printf("Wrong Input\n"); continue;}
                    write(socket_fd, copy_command, strlen(copy_command)+1);
                    read(socket_fd, buf, COMMAND_MAX);
                    if (strcmp(buf,"TA")==0){
                        printf("Welcome TA %s\n", id);
                        current_user->user_type = TA;
                    }else if (strcmp(buf,"student")==0){
                        printf("Welcome Student %s\n",id);
                        current_user->user_type = STUDENT;
                    }else {printf("Wrong user information\n"); continue;}
                    strcpy((char*) (current_user->id), id);
                    current_user->login = true;
                    bzero(buf,COMMAND_MAX);

                }else if (current_user->login==false){printf("Not logged in\n"); continue;}

                if (strcmp(command_no_args,"ReadGrade")==0){
                    if (current_user->user_type==STUDENT){
                        if (id!=NULL) {printf("Action not allowed\n"); continue;}
                        char *write_to_socket = strcat(strcat(command_no_args, " "), current_user->id);
                        write(socket_fd, write_to_socket, strlen(write_to_socket)+1);
                        read(socket_fd, buf, COMMAND_MAX);

                    }else if (current_user->user_type==TA){
                        if (id==NULL){ printf("Missing argument\n"); continue;}
                        write(socket_fd, copy_command, strlen(copy_command)+1);
                        read(socket_fd, buf, COMMAND_MAX);
                    } printf("%s\n",buf);
                    bzero(buf,COMMAND_MAX);;

                }else if (strcmp(command_no_args,"GradeList")==0){
                    if (current_user->user_type==STUDENT){ printf("Action not allowed\n"); continue;}
                    write(socket_fd, command_no_args, strlen(command_no_args)+1);
                    char *list_size = malloc(100*sizeof(char));
                    read(socket_fd, list_size, 6);
                    char* grade_list = (char*) malloc((int) atoi(list_size)*sizeof(char));
                    int socket_fd_2 = tcp_connect(hostname, port);
                    read(socket_fd_2, grade_list, (int) atoi(list_size)+1);
                    printf("%s", grade_list);
                    close(socket_fd_2);

                }else if (strcmp(command_no_args,"UpdateGrade")==0){
                    if (current_user->user_type==STUDENT){ printf("Action not allowed\n"); continue;}
                    write(socket_fd, copy_command, strlen(copy_command)+1);
                }
            }
        }
    }
    return 0;
}

int tcp_connect(const char* host, int port){
    int clifd;
    struct addrinfo *a = alloc_tcp_addr(host, port, 0);
    clifd= socket( a->ai_family, a->ai_socktype, a->ai_protocol);
    connect( clifd, a->ai_addr,a->ai_addrlen);
    freeaddrinfo( a );
    return clifd;
}

struct addrinfo* alloc_tcp_addr (const char *host, int port, int flags){
    int err; struct addrinfo hint, *a; char ps [16];
    snprintf(ps , sizeof(ps), "%hu", port);
    memset(&hint, 0, sizeof(hint));
    hint.ai_flags= flags;
    hint.ai_family= AF_UNSPEC;
    hint.ai_socktype= SOCK_STREAM;
    hint.ai_protocol= IPPROTO_TCP;
    if( (err = getaddrinfo(host, ps, &hint, &a)) != 0 ) {
        fprintf(stderr,"%s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }
    return a;
}
