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
#include <pthread.h>
#include <math.h>
#include "GradeServer.h"

struct query_queue{
char *query; struct query_queue* next;};

struct TAs_struct{
char *id; char *password;};

struct students_struct {
char id[10]; char password[COMMAND_MAX]; int grade; struct students_struct *next;};

query_queue *query_head = NULL;
students_struct *students_head = NULL;
int client_fd;
int number_of_TAs; int number_of_students;
TAs_struct *TAs;
pthread_mutex_t mutex;
pthread_cond_t cond;

int main(int argc, char **argv){
    int port = (int) atoi(argv[1]);

    signal(SIGINT, signal_handler);
    signal(SIGKILL, signal_handler);

    FILE *TAs_file = fopen("assistants.txt", "r");
    FILE *students_file = fopen("students.txt", "r");

    char *line = NULL;  size_t len = 0;
    number_of_TAs = 0; number_of_students = 0;
    while (getline(&line, &len, TAs_file) != -1) {
        number_of_TAs++;}
    while (getline(&line, &len, students_file) != -1) {
        number_of_students++;}

    rewind(TAs_file);
    TAs = (TAs_struct*)malloc(number_of_TAs*sizeof(TAs_struct));
    line=NULL; char *p;
    for (int i=0; i<number_of_TAs; i++){
        getline(&line, &len, TAs_file);
        *(line+9)='\0';
        TAs[i].id=line; TAs[i].password=line+10;
        line = NULL;
        p=strchr(TAs[i].password,'\n'); if (p!=NULL) {*p='\0';}
    }

    rewind(students_file);
    line=NULL;
    for (int i=0; i<number_of_students; i++){
        getline(&line, &len, students_file);
        *(line+9)='\0';
        p=strchr(line+10,'\n'); if (p!=NULL) {*p='\0';}
        add_student(line, line+10, 0);
        line = NULL;
    }

    char *buf = (char*) malloc(COMMAND_MAX*sizeof(char));
    int server_fd = tcp_establish(port);

    pthread_t threads[NUMBER_OF_THREADS];
    for (int i=0; i<NUMBER_OF_THREADS; i++){
        pthread_create(&threads[i], NULL, handle_queries,(void*) &client_fd);
    }
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    for(;;) {
        client_fd= accept(server_fd, NULL, NULL);
        read(client_fd, buf, COMMAND_MAX);
        query_enqueue(buf);
    }
    return 0;
}

void *handle_queries(void *void_client_fd){
    int *client_fd = (int*) void_client_fd;

    char ta_str[] = "TA"; char student_str[]="student";
    char delimiter[2]=" ";

    char *query = (char*) malloc(COMMAND_MAX*sizeof(char));
    char *read_command = (char*) malloc(COMMAND_MAX*sizeof(char));
    char *read_id = (char*) malloc(10*sizeof(char));
    char *arg_3 = (char*) malloc(COMMAND_MAX*sizeof(char));
    students_struct* student = (students_struct*) malloc(sizeof(students_struct));
    char *invalid = "invalid id";

    for(;;) {
        query = query_dequeue();

        read_command= strtok(query, delimiter);
        read_id=strtok(NULL,delimiter);
        arg_3=strtok(NULL,delimiter);


        if (strcmp(read_command,"Login")==0) {
            student = in_students(read_id);
            if (in_TAs(TAs, number_of_TAs, read_id, arg_3)){
                write(*client_fd, ta_str, strlen(ta_str)+1);
            }else if (student!=NULL){
                if (strcmp(student->password, arg_3)==0){
                    write(*client_fd, student_str, strlen(student_str)+1);
                }
            }else {write(*client_fd, NULL, 1);}

        } else if (strcmp(read_command,"ReadGrade")==0){
            student = in_students(read_id);
            if (student!=NULL){
                char *grade = (char*) malloc(4*sizeof(char));
                sprintf(grade, "%d", student->grade);
                write(*client_fd, grade, strlen(grade)+1);
            }else{
                 write(*client_fd, invalid, strlen(invalid)+1);
            }

        } else if (strcmp(read_command,"UpdateGrade")==0){
            student = in_students(read_id);
            int grade = (int) atoi(arg_3);
            if (student!=NULL){
                student->grade = grade;
            }else{
                add_student(read_id, NULL, grade);
                number_of_students++;
                }

        }else if (strcmp(read_command,"GradeList")==0){
            int list_size = 16*number_of_students;
            char* grade_list = (char*) malloc(list_size*sizeof(char));
            students_struct *iter = (students_struct*) malloc(sizeof(students_struct));
            iter = students_head;
            while (iter!=NULL){
                char* add_to_list = (char*) malloc(16*sizeof(char));
                sprintf(add_to_list, "%s: %d\n", iter->id, iter->grade);
                strcat(grade_list, add_to_list);
                iter=iter->next;
            }
            char *list_size_char = (char*) malloc(100*sizeof(char));
            sprintf(list_size_char, "%ld", strlen(grade_list)+1);
            write(*client_fd, list_size_char, strlen(list_size_char) + 1);
            sleep(0.5);
            write(*client_fd, grade_list, strlen(grade_list)+1);
        }
    }return NULL;
}

void query_enqueue(char *buf){
    pthread_mutex_lock(&mutex);
    query_queue *new_query=malloc(sizeof(query_queue));
    new_query->query = buf;
    new_query->next = NULL;
    if (query_head==NULL){
        query_head = new_query;
    }else{
        query_queue *query_tail = query_head;
        while(query_tail->next!=NULL){
            query_tail=query_tail->next;
        }query_tail->next=new_query;
    }
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

char* query_dequeue(){
    pthread_mutex_lock(&mutex);
    while (query_head==NULL){
        pthread_cond_wait(&cond, &mutex);
    }
    char *return_val = query_head->query;
    query_head = query_head->next;
    pthread_mutex_unlock(&mutex);
    return return_val;
}

void add_student(char *id, char* password, int grade){
    students_struct *added_student = (students_struct*) malloc(sizeof(students_struct));
    strcpy(added_student->id, id);
    if (password!=NULL){
        strcpy(added_student->password, password);
    }
    added_student->grade = grade;
    if (students_head == NULL){
        added_student->next=NULL;
        students_head = added_student;
    }else if ((int) atoi(added_student->id) > (int) atoi(students_head->id)){
        added_student->next=students_head;
        students_head = added_student;
    }else{
        students_struct *iter;
        iter = students_head;
        while (iter->next!=NULL && (int) atoi(added_student->id) < (int) atoi(iter->next->id)){
            iter=iter->next;
        } added_student->next = iter->next;
        iter->next = added_student;
    }
}

void signal_handler(){
    close(client_fd);
    exit(0);
}

bool in_TAs(TAs_struct *TAs, int number_of_TAs, char *id, char *password){
    for (int i=0; i<number_of_TAs; TAs++, i++){
        if (strcmp(TAs->id,id)==0 && strcmp(TAs->password,password)==0){return true;}
    }return false;
}

students_struct *in_students(char *id){
    students_struct *find_student = students_head;
    while (find_student!=NULL && (int) atoi(id) <= (int) atoi(find_student->id)){
        if (strcmp(find_student->id, id)==0){
            return find_student;
        }else{
            find_student = find_student->next;
        }
    }
    return NULL;
}

int tcp_establish(int port){
    int srvfd;
    struct addrinfo *a =alloc_tcp_addr(NULL/*host*/, port, AI_PASSIVE);
    srvfd= socket( a->ai_family, a->ai_socktype, a->ai_protocol);
    bind( srvfd, a->ai_addr,a->ai_addrlen);
    listen( srvfd, 5);
    freeaddrinfo( a );
    return srvfd;
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
