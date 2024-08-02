#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <crypt.h>
#include "constant.h"
#include "admin-credentials.h"


//-------------------definition of function-----------------------------------
bool login_handler(int usertype, int connFD, char* loggedInFacultyID, char* loggedInStudentID, char* loggedInFacultyName);

struct Student {
    char id[10];
    char name[100];
    int age;
    char gender[10];
    bool isActive;
    char password[20];
    char enrolledCourses[MAX_ENROLLED_COURSES][15];  // Array to store enrolled course IDs
    int numEnrolledCourses;  // Number of enrolled courses
    };

// Create a function to initialize a Student object
void initializeStudent(struct Student *student) {
    student->numEnrolledCourses = 0;
}

struct Course{
      char id[50];
      char name[100];
      char instructor[100];
      int seats_offered;
      int seats_available;
      char facultyid[15];
      };


 struct Faculty {
    char id[50];
    char name[100];
    char department[100];
    int age;
    char password[20];
};

//-----------------------login approval--------------------------------------
bool login_handler(int usertype, int connFD, char* loggedInFacultyID, char* loggedInStudentID, char* loggedInFacultyName)
{
    ssize_t readBytes, writeBytes;            // Number of bytes written to / read from the socket
    char readBuffer[1000], writeBuffer[1000]; // Buffer for reading from / writing to the client
    char tempBuffer[1000];
    

    struct Student student;
    struct Faculty faculty;
    int facultyFileDescriptor, studentFileDescriptor, cid;
    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));

    // Get login message for respective user type
    if (usertype==1)//If user is admin
        strcpy(writeBuffer, "\nEnter your credentials to Login:");
    else if (usertype==2)//If user is admin
        strcpy(writeBuffer, "\nWelcome Faculty!!Enter your credentials to Login:");    
    else 
        strcpy(writeBuffer, "Welcome Student!\n\nEnter your credentials to login to your account:");

    // Append the request for LOGIN ID message
    strcat(writeBuffer, "\n");
    //strcat(writeBuffer, LOGIN_ID);

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing message to the client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading login ID from client!");
        return false;
    }

    bool userFound = false;
    
    if (usertype==1) {
        int x = (strchr(readBuffer, '-') == NULL) ? 0 : 1;
    if(x == 0 && strcmp(readBuffer, ADMIN_LOGIN_ID) != 0)
    {
        return false;
    }
        if (strcmp(readBuffer, ADMIN_LOGIN_ID) == 0)
            userFound = true;      
    }
    
    else if(usertype==2){
     bzero(tempBuffer, sizeof(tempBuffer));
     strcpy(tempBuffer, readBuffer);
    facultyFileDescriptor = open(FACULTY_FILE, O_RDONLY);
    if (facultyFileDescriptor == -1) {
        // Student File doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "\n**No faculty file be found for the given ID**\n");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1) {
            perror("Error while writing  message to the client!");
            return false;
        }
    }
      bool facultyFound = false;
     while (read(facultyFileDescriptor, &faculty, sizeof(struct Faculty)) > 0) {
        if (strcmp(faculty.id, tempBuffer) == 0) {
            userFound = true;
            strcpy(loggedInFacultyID, faculty.id); // Store the logged-in faculty's ID
            strcpy(loggedInFacultyName, faculty.name); // Store the logged-in faculty's Name
            break;
        }
    }
     bzero(tempBuffer, sizeof(tempBuffer));
    }
    else if(usertype==3){
    strcpy(tempBuffer, readBuffer);
    studentFileDescriptor = open(STUDENT_FILE, O_RDONLY);
    if (studentFileDescriptor == -1) {
        // Student File doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "\n**No student could be found for the given ID**\n");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1) {
            perror("Error while writing STUDENT_ID_DOESNT_EXIST message to the client!");
            return false;
        }
     
    }
      bool studentFound = false;
     while (read(studentFileDescriptor, &student, sizeof(struct Student)) > 0) {
        if (strcmp(student.id, tempBuffer) == 0) {
          if (student.isActive) {
            userFound = true;
            strcpy(loggedInStudentID, student.id); // Store the logged-in student's ID
          } else {
            // The student is not active
                    bzero(writeBuffer, sizeof(writeBuffer));
                    strcpy(writeBuffer, "\n**Your account is not active. Please contact the administrator**\n");
                    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
                    close(studentFileDescriptor);
                    return false;
                } 
            break;
        }
      }
      bzero(tempBuffer, sizeof(tempBuffer));
    }
    if (userFound)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        writeBytes = write(connFD, PASSWORD, strlen(PASSWORD));
        if (writeBytes == -1)
        {
            perror("Error writing PASSWORD message to client!");
            return false;
        }

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == 0)
        {
            perror("Error reading password from the client!");
            return false;
        }

        
        char hashedPassword[1000];
        strcpy(hashedPassword, readBuffer);
        if (usertype==1)
        {    
           
            if (strcmp(hashedPassword,ADMIN_PASSWORD) == 0)
                return true;
        }
        if(usertype==2){
           if (strcmp(hashedPassword,faculty.password) == 0)
                return true;
         }
        
         if (usertype==3)
        {
            if (strcmp(hashedPassword,student.password) == 0)
                return true;
        }
       

        bzero(writeBuffer, sizeof(writeBuffer));
        writeBytes = write(connFD, INVALID_PASSWORD, strlen(INVALID_PASSWORD));
    }
    else
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        writeBytes = write(connFD, INVALID_LOGIN, strlen(INVALID_LOGIN));
    }

    return false;
}
