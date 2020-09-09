all: GradeClient GradeServer

GradeClient: GradeClient.c
	gcc -o GradeClient GradeClient.c

GradeServer: GradeServer.c
	gcc -o GradeServer GradeServer.c -pthread 