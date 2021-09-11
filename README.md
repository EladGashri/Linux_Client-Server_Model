Implemented a multi-threaded server application that handles multiple users' queries simultaneously and a multi-process user application that allows users to login, read and update server data. 

Applications communicate using a TCP socket.

C programming in Linux.

There are two kinds of clients: a student and a teaching assistant (TA).

Users can send one of the following commands:

● Login id password.

● ReadGrade: Print the grade of the logged in student. The operation can be done
only by a student.

● ReadGrade id: Print the grade of the specified id. The operation can be done
only by a TA.

● GradeList: Print a list of all the grades in the server. The operation can be done
only by a TA. 

● UpdateGrade id grade : Update The grade of id in the server. The operation can be done only by a
TA.

● Logout.

● Exit.
