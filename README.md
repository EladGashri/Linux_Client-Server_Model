Programming a multi-threaded server application that handles multiple users' queries and a multi-process user application that allows users to login, read and update server data. 

Applications communicate using a TCP socket connection.

C programming in Linux.

There are two kinds of clients:

● A teaching assistant (TA): can read and modify grades.

● A student: can only read his grade.

Users can send one of the following commands:

● Login id password

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
