//#include "common.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <crypt.h>

bool student_operation_handler(int connFD);
bool viewAllCourses(int connFD, char* loggedInStudentID);
bool enrollStudentInCourse(int connFD, char* loggedInStudentID);
bool dropStudentFromCourse(int connFD, char* loggedInStudentID);
bool viewEnrolledCourses(int connFD, char* loggedInStudentID);
void changePassword(int connFD, char* loggedInStudentID);


//--------------------------------STUDENT OPERATIONS------------------------------
bool student_operation_handler(int connFD)
{
    char loggedInStudentID[15]; // Store the logged-in student's ID
    bzero(loggedInStudentID, sizeof(loggedInStudentID));

    if (login_handler(3, connFD, NULL, loggedInStudentID, NULL)) // Pass NULL for faculty
    {
        ssize_t writeBytes, readBytes;            // Number of bytes read from / written to the client
        char readBuffer[1000], writeBuffer[1000]; // A buffer used for reading & writing to the client
        bzero(writeBuffer, sizeof(writeBuffer));
        bzero(readBuffer, sizeof(readBuffer));
        strcpy(writeBuffer, "\n----------------------------------------------------------\nWelcome Student! Login successful! \n----------------------------------------------------------\n");
        
        Label:
        
        while (1)
        {
        
            strcat(writeBuffer, "\n\n------------------------Student Menu-------------------------\n");
            strcat(writeBuffer, "\n1. View All Courses\n2. Enroll Course\n3. Drop Course\n4. View Enrolled Courses Details\n5. Change Password \n6. Logout and Exit \n");
            
           
            
            strcat(writeBuffer, "\n------------------------------------------------------------\nEnter your choice : ");
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing STUDENT_MENU to client!");
                return false;
            }
            bzero(writeBuffer, sizeof(writeBuffer));

            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error while reading client's choice for STUDENT_MENU");
                return false;
            }

            int choice = atoi(readBuffer);
            if(choice<1 || choice>7){
               strcat(writeBuffer,"Wrong choice");
               writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
               if (writeBytes == -1)
               {
                   perror("Error while writing to client!");
                   goto Label;
               }
            }
            
            switch (choice)
            {
            case 1:
                viewAllCourses(connFD, loggedInStudentID);
                break;
	
            case 2:
                enrollStudentInCourse(connFD, loggedInStudentID);
                break;
                
            case 3:
                dropStudentFromCourse(connFD, loggedInStudentID);
                break;

            case 4:
                viewEnrolledCourses(connFD, loggedInStudentID);
                break;

            case 5:
            	 changePassword(connFD, loggedInStudentID);	
            	 break;
            	 	
            case 6:
            	 writeBytes = write(connFD, "\nLogging you out!$", strlen("\nLogging you out!$"));
                 return false;
	    
            /*default:
                writeBytes = write(connFD, "\nLogging you out!$", strlen("\nLogging you out!$"));
            return false; */
            }
        }
    }
    else
    {
        // STUDENT LOGIN FAILED
        printf("Unauthorized Student\n");
        return false;
    }
    
    goto Label;
    
    return true;
}



//--------------------------------------VIEW ALL COURSES-----------------------------------
bool viewAllCourses(int connFD, char* loggedInStudentID) {
    ssize_t writeBytes;
    char writeBuffer[1000];

    int courseFileDescriptor;
    struct Course course;

    courseFileDescriptor = open(COURSE_FILE, O_RDONLY);
    if (courseFileDescriptor == -1) {
        // Course File doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "\n*No courses available*\n");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1) {
            perror("Error while writing COURSE_DOESNT_EXIST message to the client!");
            return false;
        }
        return false;
    }

    bzero(writeBuffer, sizeof(writeBuffer));
    strcpy(writeBuffer, "Course List:\n");

    while (read(courseFileDescriptor, &course, sizeof(struct Course)) > 0) {
        // Check if the course has been wiped off
        if (course.id[0] == '\0') {
            // Skip this course
            continue;
        }
        
        // Append course information to the writeBuffer
        char courseInfo[100];
        snprintf(courseInfo, sizeof(courseInfo), "\tName: %s\n\tCourse ID: %s\n\tInstructor: %s\n\tSeats Available: %d\n\n", course.name, course.id, course.instructor, course.seats_available);
        strcat(writeBuffer, courseInfo);
    }

    strcat(writeBuffer, "\nYou'll now be redirected to the main menu...");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error writing course info to the client!");
        return false;
    }

    return true;
}


//----------------------------------ENROLL COURSE--------------------------------------
bool enrollStudentInCourse(int connFD, char* loggedInStudentID) {
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[10000];
    char tempBuffer_courseID[15];

    struct Student student;
    struct Course course;
    int studentFileDescriptor, courseFileDescriptor;

    studentFileDescriptor = open(STUDENT_FILE, O_RDWR);
    if (studentFileDescriptor == -1) {
        perror("Error while opening the student file!");
        return false;
    }

    courseFileDescriptor = open(COURSE_FILE, O_RDWR);
    if (courseFileDescriptor == -1) {
        perror("Error while opening the course file!");
        close(studentFileDescriptor);  // Close the student file descriptor
        return false;
    }
    
    bzero(writeBuffer, sizeof(writeBuffer));
    bzero(readBuffer, sizeof(readBuffer));
    
    strcpy(writeBuffer, "Enter the course ID you want to enroll in:");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing message to the client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    
    if (readBytes == -1)
    {
        perror("Error reading course ID from client!");
        return false;
    }
    
    strcpy(tempBuffer_courseID,readBuffer);
    bzero(writeBuffer, sizeof(writeBuffer));
    bzero(readBuffer, sizeof(readBuffer));
    
    // Find the student and course based on IDs
    while (read(studentFileDescriptor, &student, sizeof(struct Student)) > 0) {
        if (strcmp(student.id, loggedInStudentID) == 0) {
            break;
        }
    }
    
    while (read(courseFileDescriptor, &course, sizeof(struct Course)) > 0) {
        if (strcmp(course.id, tempBuffer_courseID) == 0) {
            break;
        }
    }
    
    if (strcmp(student.id, loggedInStudentID) != 0 || strcmp(course.id, tempBuffer_courseID) != 0) {
        // Student or course not found
        close(studentFileDescriptor);
        close(courseFileDescriptor);
        return false;
    }
    
    if (student.numEnrolledCourses >= MAX_ENROLLED_COURSES) {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "You are already enrolled in the maximum number of courses.");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        close(studentFileDescriptor);
        close(courseFileDescriptor);
        if (writeBytes == -1) {
            perror("Error writing message to the client!");
        }
        return false;
    }
    
    for (int i = 0; i < student.numEnrolledCourses; i++) {
        if (strcmp(student.enrolledCourses[i], tempBuffer_courseID) == 0) {
            bzero(writeBuffer, sizeof(writeBuffer));
            strcpy(writeBuffer, "You are already enrolled in this course.");
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            close(studentFileDescriptor);
            close(courseFileDescriptor);
            if (writeBytes == -1) {
                perror("Error writing message to the client!");
            }
            return false;
        }
    }
    
    if (course.seats_available == 0) {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "No available seats in the course.");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        close(studentFileDescriptor);
        close(courseFileDescriptor);
        if (writeBytes == -1) {
            perror("Error writing message to the client!");
        }
        return false;
    }
    
    // Enroll the student in the course
    strncpy(student.enrolledCourses[student.numEnrolledCourses], tempBuffer_courseID, sizeof(student.enrolledCourses[0]));
    student.numEnrolledCourses++;
    course.seats_available--;
    
    // Write the updated student and course data back to their respective files
    lseek(studentFileDescriptor, -sizeof(struct Student), SEEK_CUR);
    write(studentFileDescriptor, &student, sizeof(struct Student));
    
    lseek(courseFileDescriptor, -sizeof(struct Course), SEEK_CUR);
    write(courseFileDescriptor, &course, sizeof(struct Course));
    
    // Send success message to the client
    bzero(writeBuffer, sizeof(writeBuffer));
    strcpy(writeBuffer, "Enrollment successful.");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    
    close(studentFileDescriptor);
    close(courseFileDescriptor);
    
    if (writeBytes == -1) {
        perror("Error writing message to the client!");
        return false;
    }
    
    return true;
}



//-----------------------------------------DROP COURSE---------------------------------------
bool dropStudentFromCourse(int connFD, char* loggedInStudentID) {
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[10000];
    char tempBuffer_courseID[15];

    struct Student student;
    struct Course course;
    int studentFileDescriptor, courseFileDescriptor;

    studentFileDescriptor = open(STUDENT_FILE, O_RDWR);
    if (studentFileDescriptor == -1) {
        perror("Error while opening the student file!");
        return false;
    }

    courseFileDescriptor = open(COURSE_FILE, O_RDWR);
    if (courseFileDescriptor == -1) {
        perror("Error while opening the course file!");
        close(studentFileDescriptor);
        return false;
    }

    bzero(writeBuffer, sizeof(writeBuffer));
    bzero(readBuffer, sizeof(readBuffer));

    strcpy(writeBuffer, "Enter the course ID you want to drop:");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error writing message to the client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading course ID from client!");
        return false;
    }
    strcpy(tempBuffer_courseID, readBuffer);
    bzero(writeBuffer, sizeof(writeBuffer));
    bzero(readBuffer, sizeof(readBuffer));

    // Find the student and course based on IDs
    while (read(studentFileDescriptor, &student, sizeof(struct Student)) > 0) {
        if (strcmp(student.id, loggedInStudentID) == 0) {
            break;
        }
    }

    while (read(courseFileDescriptor, &course, sizeof(struct Course)) > 0) {
        if (strcmp(course.id, tempBuffer_courseID) == 0) {
            break;
        }
    }

    if (strcmp(student.id, loggedInStudentID) != 0 || strcmp(course.id, tempBuffer_courseID) != 0) {
        // Student or course not found
        close(studentFileDescriptor);
        close(courseFileDescriptor);
        return false;
    }

    // Check if the student is enrolled in the course
    bool foundCourse = false;
    int courseIndex = -1;
    for (int i = 0; i < student.numEnrolledCourses; i++) {
        if (strcmp(student.enrolledCourses[i], tempBuffer_courseID) == 0) {
            foundCourse = true;
            courseIndex = i;
            break;
        }
    }

    if (!foundCourse) {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "You are not enrolled in this course.");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        close(studentFileDescriptor);
        close(courseFileDescriptor);
        if (writeBytes == -1) {
            perror("Error writing message to the client!");
        }
        return false;
    }

    // Remove the course from the student's enrollment
    for (int i = courseIndex; i < student.numEnrolledCourses - 1; i++) {
        strcpy(student.enrolledCourses[i], student.enrolledCourses[i + 1]);
    }
    student.numEnrolledCourses--;

    // Increase the available seats for the course
    course.seats_available++;

    // Write the updated student and course data back to their respective files
    lseek(studentFileDescriptor, -sizeof(struct Student), SEEK_CUR);
    write(studentFileDescriptor, &student, sizeof(struct Student));

    lseek(courseFileDescriptor, -sizeof(struct Course), SEEK_CUR);
    write(courseFileDescriptor, &course, sizeof(struct Course));

    // Send success message to the client
    bzero(writeBuffer, sizeof(writeBuffer));
    strcpy(writeBuffer, "Course dropped successfully.");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));

    close(studentFileDescriptor);
    close(courseFileDescriptor);

    if (writeBytes == -1) {
        perror("Error writing message to the client!");
        return false;
    }

    return true;
}


//----------------------------------------VIEW ENROLLED COURSES----------------------------------------
bool viewEnrolledCourses(int connFD, char* loggedInStudentID) {
    ssize_t writeBytes, readBytes;
    char writeBuffer[1000], readBuffer[1000];
    struct Student student;
    struct Course course;
    int studentFileDescriptor, courseFileDescriptor;

    studentFileDescriptor = open(STUDENT_FILE, O_RDONLY);
    if (studentFileDescriptor == -1) {
        perror("Error while opening the student file!");
        return false; 
    }

    courseFileDescriptor = open(COURSE_FILE, O_RDONLY);
    if (courseFileDescriptor == -1) {
        perror("Error while opening the course file!");
        close(studentFileDescriptor);
        return false;
    }
    
    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));
    
    // Find the student based on the provided student ID
    while (read(studentFileDescriptor, &student, sizeof(struct Student)) > 0) {
        if (strcmp(student.id, loggedInStudentID) == 0) {
            break;
        }
    }
    
    if (strcmp(student.id, loggedInStudentID) != 0) {
        // Student not found
        close(studentFileDescriptor);
        close(courseFileDescriptor);
        return false;
    }
    
    strcpy(writeBuffer, "\nEnrolled Courses:\n");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));
    
    for (int i = 0; i < student.numEnrolledCourses; i++) {
        lseek(courseFileDescriptor, 0, SEEK_SET); // Rewind the course file
    
        while (read(courseFileDescriptor, &course, sizeof(struct Course)) > 0) {
            if (strcmp(course.id, student.enrolledCourses[i]) == 0) {
                snprintf(writeBuffer, sizeof(writeBuffer), "Course ID: %s\nCourse Name: %s\nInstructor: %s\n\n",
                    course.id, course.name, course.instructor);
                writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            }
        }
    }

    close(studentFileDescriptor);
    close(courseFileDescriptor);
    return true;
}


//------------------------------------------CHANGE PASSWORD------------------------------------------
void changePassword(int connFD, char* loggedInStudentID){
        ssize_t readBytes, writeBytes;
        char readBuffer[1000], writeBuffer[1000],studentID[15],newPassword[20],oldPassword[20];

        struct Student student;
        int studentFileDescriptor;
        studentFileDescriptor = open(STUDENT_FILE, O_RDWR);
        writeBytes = write(connFD, "Enter the your ID : ", strlen("Enter the your ID : "));
        if (writeBytes == -1) {
            perror("Error while writing GET_STUDENT_ID message to the client!");
            return ;
        }

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1) {
            perror("Error getting student ID from the client!");
            return;
        }

        strcpy(studentID , readBuffer);
        Retry:
        bzero(readBuffer, sizeof(readBuffer));
        bzero(writeBuffer, sizeof(writeBuffer));
        
        writeBytes = write(connFD, "Enter the old password: ", strlen("Enter the old password: "));
        if (writeBytes == -1) {
            perror("Error while writing STUDENT_OLDPassword message to the client!");
            return ;
        }

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1) {
            perror("Error getting oldpassword from the client!");
            return;
        }
        strcpy(oldPassword , readBuffer);
          
        if (studentFileDescriptor == -1) {
           // Student File doesn't exist
           bzero(writeBuffer, sizeof(writeBuffer));
           strcpy(writeBuffer, "\n**No student could be found for the given ID**\n");

           writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
           if (writeBytes == -1) {
               perror("Error while writing STUDENT_ID_DOESNT_EXIST message to the client!");
               return;
           }
        }
        bool userFound = false;
        while (read(studentFileDescriptor, &student, sizeof(struct Student)) > 0) {
           if (strcmp(student.id, studentID) == 0) {
               userFound = true;
               break;
           }
        }
          
        if(userFound){  
           if (strcmp(oldPassword,student.password) == 0){  //if password entered by user matches the password stored in file student
           
           bzero(readBuffer, sizeof(readBuffer));
           bzero(writeBuffer, sizeof(writeBuffer));
        
           writeBytes = write(connFD, "Enter the new password: ", strlen("Enter the new password: "));
	   if (writeBytes == -1) {
		    perror("Error while writing newPassword message to the client!");
		    return ;
	   }

	   bzero(readBuffer, sizeof(readBuffer));
	   readBytes = read(connFD, readBuffer, sizeof(readBuffer));
	   if (readBytes == -1) {
		    perror("Error getting newpassword from the client!");
		    return;
	   }
	   strcpy(student.password, readBuffer);
		 
           int offset = lseek(studentFileDescriptor,-sizeof(struct Student), SEEK_CUR);
           if (offset == -1)
	   {
		perror("Error while seeking to required student record!");
		return ;
	   }
	    
	   writeBytes = write(studentFileDescriptor, &student, sizeof(struct Student));
	   if (writeBytes == -1)
	   {
		perror("Error while updating student Password info into file");
	   }
	   close(studentFileDescriptor);
	   writeBytes = write(connFD, "\n\nThe required modification was successfully made!\nYou'll now be redirected to the main menu!", strlen("\n\nThe required modification was successfully made!\nYou'll now be redirected to the main menu!")); 
           }     
           else{        //if password does not match
               bzero(writeBuffer, sizeof(writeBuffer));
               writeBytes = write(connFD, "Entered password does not match: ", strlen("Entered password does not match: "));
	       if (writeBytes == -1) {
		    perror("Error while writing password error msg to the client!");
		    return ;
	       }
	   
           goto Retry;
           }   
          
        }//userFound if
}
