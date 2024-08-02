//#include "common.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <crypt.h>

bool faculty_operation_handler(int connFD);
bool viewAllOfferingCourses(int connFD, char* loggedInFacultyID);
bool addCourse(int connFD, char* loggedInFacultyID, char* loggedInFacultyName);
bool removeCourse(int connFD, char* loggedInFacultyID);
bool modifyCourse(int connFD, char* loggedInFacultyID);
void changePassword_F(int connFD, char* loggedInFacultyID);


//--------------------------------FACULTY OPERATIONS-------------------------------
bool faculty_operation_handler(int connFD)
{
    char loggedInFacultyID[15]; // Store the logged-in faculty's ID
    char loggedInFacultyName[100];
    bzero(loggedInFacultyID, sizeof(loggedInFacultyID));
    bzero(loggedInFacultyName, sizeof(loggedInFacultyName));
    if(login_handler(2, connFD, loggedInFacultyID, NULL, loggedInFacultyName)) // Pass NULL for student
    {
        ssize_t writeBytes, readBytes;            // Number of bytes read from / written to the client
        char readBuffer[1000], writeBuffer[1000], tempBuffer[1000]; // A buffer used for reading & writing to the client
        bzero(writeBuffer, sizeof(writeBuffer));
        bzero(readBuffer, sizeof(readBuffer));
        strcpy(writeBuffer, "\n----------------------------------------------------------\nWelcome Faculty! Login successful! \n----------------------------------------------------------\n");
        
        Label:
        
        while (1)
        {
        
            strcat(writeBuffer, "\n\n------------------------Faculty Menu-------------------------\n");
            strcat(writeBuffer, "\n1. View Offering Courses\n2. Add New Course\n3. Remove Course from Catalog\n4. Update Course Details\n5. Change Password\n6. Logout and Exit\n");
           
            strcat(writeBuffer, "\n------------------------------------------------------------\nEnter your choice : ");
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing FACULTY_MENU to client!");
                return false;
            }
            bzero(writeBuffer, sizeof(writeBuffer));

            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error while reading client's choice for FACULTY_MENU");
                return false;
            }

            int choice = atoi(readBuffer);
            if(choice<1 || choice>6){
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
                viewAllOfferingCourses(connFD, loggedInFacultyID);
                break;
	
            case 2:
                addCourse(connFD, loggedInFacultyID, loggedInFacultyName);
                break;
                
            case 3:
                removeCourse(connFD, loggedInFacultyID);
                break;

            case 4:
                modifyCourse(connFD, loggedInFacultyID);
                break;

            case 5:
                changePassword_F(connFD, loggedInFacultyID);
                break;
	    case 6:
                writeBytes = write(connFD, "\nLogging you out!$", strlen("\nLogging you out!$"));
                return false;
            
            /*default:
                writeBytes = write(connFD, "\nLogging you out!$", strlen("\nLogging you out!$"));
                return false;*/
            }
        }
    }
    else
    {
        // FACULTY LOGIN FAILED
        printf("Unauthorized Faculty\n");
        return false;
    }
    
    goto Label;
    
    return true;    
}



//----------------------------VIEW COURSE OFFERINGS------------------------
bool viewAllOfferingCourses(int connFD, char* loggedInFacultyID) {
    ssize_t writeBytes,readBytes;
    char writeBuffer[1000],readBuffer[1000];

    int courseFileDescriptor;
    struct Course course;

    courseFileDescriptor = open(COURSE_FILE, O_RDWR);
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

    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer)); 
    strcpy(writeBuffer, "Course List:\n");

    while (read(courseFileDescriptor, &course, sizeof(struct Course)) > 0) {
    if(strcmp(loggedInFacultyID,course.facultyid)==0){
        // Append course information to the writeBuffer
        char courseInfo[100];
        snprintf(courseInfo, sizeof(courseInfo), "\tID: %s\n\tName: %s\n\tInstructor: %s\n\tSeats Available: %d\n\n", course.id, course.name, course.instructor, course.seats_available);
        strcat(writeBuffer, courseInfo);
    }
    }

    strcat(writeBuffer, "\nYou'll now be redirected to the main menu...");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error writing course info to the client!");
        return false;
    }
    return true;
}


//---------------------------------------ADD COURSE---------------------------------
bool addCourse(int connFD, char* loggedInFacultyID, char* loggedInFacultyName) {
      struct Course newCourse, previousCourse;
      ssize_t readBytes, writeBytes;
      char readBuffer[1000], writeBuffer[1000];
      bzero(readBuffer, sizeof(readBuffer));
      bzero(writeBuffer, sizeof(writeBuffer));
      
      int courseFileDescriptor = open(COURSE_FILE, O_RDWR);
      if(courseFileDescriptor==-1){
      perror("Error while opening course file");
        return false;
      }
     
      sprintf(writeBuffer, "Enter the course's name: ");
      writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
      if (writeBytes == -1) {
        perror("Error writing message to client!");
        return false;
    }
     bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading course name from client!");
        return false;
    }

    strcpy(newCourse.name, readBuffer);
    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));
    
    /*sprintf(writeBuffer, "Enter the course instructor's name: ");
      writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
      if (writeBytes == -1) {
        perror("Error writing message to client!");
        return false;
    }
     bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading instructor name from client!");
        return false;
    }

    strcpy(newCourse.instructor, readBuffer);
    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer)); */
    
    strcpy(newCourse.instructor, loggedInFacultyName);
    strcpy(newCourse.facultyid, loggedInFacultyID); 
             
    sprintf(writeBuffer, "Enter the seats offered: ");
      writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
      if (writeBytes == -1) {
        perror("Error writing message to client!");
        return false;
    }
     bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading seats offered from client!");
        return false;
    }
    int seats=atoi(readBuffer);
    newCourse.seats_offered= seats;
    newCourse.seats_available= seats;
    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));
    
      int offset = lseek(courseFileDescriptor, 0, SEEK_END);
      if(offset==0){ 
      strcpy(newCourse.id, "CS01");
      }else{
      // Increment the course ID
         int offset1 = lseek(courseFileDescriptor, -sizeof(struct Course), SEEK_END);
     readBytes = read(courseFileDescriptor, &previousCourse, sizeof(struct Course));
      if (offset1 == -1) {
            perror("Error seeking to the last Course record!");
            return false;
        }
        char numericalPart[4];
        int numericalValue = atoi(previousCourse.id + 2);
        numericalValue++;
        sprintf(numericalPart, "%02d", numericalValue);
        strcpy(newCourse.id, "CS");
        strcat(newCourse.id, numericalPart);
      }
     
    writeBytes = write(courseFileDescriptor, &newCourse, sizeof(struct Course));
    if (writeBytes == -1) {
        perror("Error while writing Course record to file!");
        return false;
    }

    close(courseFileDescriptor);
    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "The newly created course's ID is: %s", newCourse.id);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    return true;
}


//------------------------------------REMOVE COURSE FROM CATALOG--------------------------------------
bool removeCourse(int connFD, char* loggedInFacultyID) {
    struct Course courseToRemove;
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];
    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));

    int courseFileDescriptor = open(COURSE_FILE, O_RDWR);
    if (courseFileDescriptor == -1) {
        perror("Error while opening course file");
        return false;
    }

    sprintf(writeBuffer, "Enter the course ID to remove: ");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error writing message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading course ID from client!");
        return false;
    }

    char courseIDToRemove[50];
    strcpy(courseIDToRemove, readBuffer);

    // Search for the course in the catalog
    int offset = lseek(courseFileDescriptor, 0, SEEK_SET);
    while (read(courseFileDescriptor, &courseToRemove, sizeof(struct Course)) > 0) {
        if (strcmp(courseToRemove.id, courseIDToRemove) == 0) {
            // Course found in the catalog, so we can remove it
            break;
        }
    }

    if (strcmp(courseToRemove.id, courseIDToRemove) != 0) {
        // Course not found in the catalog
        close(courseFileDescriptor);
        return false;
    }

    // Remove the course from the catalog
    lseek(courseFileDescriptor, -sizeof(struct Course), SEEK_CUR);
    struct Course emptyCourse;
    memset(&emptyCourse, 0, sizeof(struct Course));
    write(courseFileDescriptor, &emptyCourse, sizeof(struct Course));

    // Close the course catalog file
    close(courseFileDescriptor);

    // Now, iterate through the student file to remove the course from their enrolled courses
    int studentFileDescriptor = open(STUDENT_FILE, O_RDWR);
    if (studentFileDescriptor == -1) {
        perror("Error while opening student file");
        return false;
    }

    struct Student student;
    while (read(studentFileDescriptor, &student, sizeof(struct Student)) > 0) {
        for (int i = 0; i < student.numEnrolledCourses; i++) {
            if (strcmp(student.enrolledCourses[i], courseIDToRemove) == 0) {
                // Found the course in a student's list of enrolled courses, so remove it
                for (int j = i; j < student.numEnrolledCourses - 1; j++) {
                    strcpy(student.enrolledCourses[j], student.enrolledCourses[j + 1]);
                }
                strcpy(student.enrolledCourses[student.numEnrolledCourses - 1], "");
                student.numEnrolledCourses--;
                // Update the student record in the file
                lseek(studentFileDescriptor, -sizeof(struct Student), SEEK_CUR);
                write(studentFileDescriptor, &student, sizeof(struct Student));
                break;
            }
        }
    }

    sprintf(writeBuffer, "Course with ID %s has been successfully removed from the catalog.\n", courseIDToRemove);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));

    // Close the student file
    close(studentFileDescriptor);

    return true;
}


//------------------------------------UPDATE COURSE DETAILS--------------------------------
bool modifyCourse(int connFD, char* loggedInFacultyID) {
    struct Course modifiedCourse;
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000], tempBuff[15];
    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));

    int courseFileDescriptor = open(COURSE_FILE, O_RDWR);
    if (courseFileDescriptor == -1) {
        perror("Error while opening course file");
        return false;
    }

    bzero(writeBuffer, sizeof(writeBuffer));

    sprintf(writeBuffer, "Enter the ID of the course whose information you want to edit: ");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error writing message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading course id from client!");
        return false;
    }

    strcpy(tempBuff, readBuffer);
    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));

    // Find the course record by its ID
    int found = 0;
    while (read(courseFileDescriptor, &modifiedCourse, sizeof(struct Course)) > 0) {
        if (strcmp(modifiedCourse.id, tempBuff) == 0) {
            found = 1;
            break;
        }
    }

    if (!found) {
        close(courseFileDescriptor);
        sprintf(writeBuffer, "Course with ID %s not found.", tempBuff);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        return false;
    }

    // Ask the user what they want to modify
    sprintf(writeBuffer, "Select the field you want to modify:\n1. Name\n2. Instructor\n3. Exit\nEnter the option (1/2/3): ");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error writing message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading user choice from client!");
        return false;
    }

    int choice = atoi(readBuffer);
    switch (choice) {
        case 1:
            // Modify the name
            sprintf(writeBuffer, "Enter the new name (or press Enter to keep the current name): ");
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1) {
                perror("Error writing message to client!");
                return false;
            }

            bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1) {
                perror("Error reading course name from client!");
                return false;
            }

            if (strlen(readBuffer) > 1) {
                strcpy(modifiedCourse.name, readBuffer);
            }
            break;

        case 2:
            // Modify the instructor
            sprintf(writeBuffer, "Enter the new instructor (or press Enter to keep the current instructor): ");
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1) {
                perror("Error writing message to client!");
                return false;
            }

            bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1) {
                perror("Error reading course instructor from client!");
                return false;
            }

            if (strlen(readBuffer) > 1) {
                strcpy(modifiedCourse.instructor, readBuffer);
            }
            break;

        case 3:
            // Exit without modification
            close(courseFileDescriptor);
            return false;

        default:
            // Invalid choice
            sprintf(writeBuffer, "Invalid option. Please enter a valid option (1/2/3/4).");
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            return false;
    }

    // Update the modified course record in the file
    lseek(courseFileDescriptor, -sizeof(struct Course), SEEK_CUR);
    writeBytes = write(courseFileDescriptor, &modifiedCourse, sizeof(struct Course));
    if (writeBytes == -1) {
        perror("Error while writing modified Course record to file!");
        return false;
    }

    close(courseFileDescriptor);
    sprintf(writeBuffer, "Course with ID %s has been successfully modified.", tempBuff);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    return true;
}
    
//--------------------------------CHANGE PASSWORD--------------------------------------    
void changePassword_F(int connFD, char* loggedInFacultyID){
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000],facultyID[15],newPassword[20],oldPassword[20];

    struct Faculty faculty;
    int facultyFileDescriptor;
    facultyFileDescriptor = open(FACULTY_FILE, O_RDWR);
     writeBytes = write(connFD, "Enter the your ID : ", strlen("Enter the your ID : "));
        if (writeBytes == -1) {
            perror("Error while writing GET_FACULTY_ID message to the client!");
            return ;
        }

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1) {
            perror("Error getting faculty ID from the client!");
            return;
        }

        strcpy(facultyID , readBuffer);
        Retry:
        bzero(readBuffer, sizeof(readBuffer));
        bzero(writeBuffer, sizeof(writeBuffer));
        
           writeBytes = write(connFD, "Enter the old password: ", strlen("Enter the old password: "));
        if (writeBytes == -1) {
            perror("Error while writing FACULTY_OLDPassword message to the client!");
            return ;
        }

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1) {
            perror("Error getting oldpassword from the client!");
            return;
        }
          strcpy(oldPassword , readBuffer);
          
           if (facultyFileDescriptor == -1) {
        // Student File doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "\n**No student could be found for the given ID**\n");
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1) {
            perror("Error while writing STUDENT_ID_DOESNT_EXIST message to the client!");
            return;
        }
    }
      bool userFound = false;
     while (read(facultyFileDescriptor, &faculty, sizeof(struct Faculty)) > 0) {
        if (strcmp(faculty.id, facultyID) == 0) {
            userFound = true;   
            break;
        }
    }
          
          if(userFound){
              if (strcmp(oldPassword,faculty.password) == 0){  //if password entered by user matches the password stored in file student

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
	      strcpy(faculty.password , readBuffer);
          
              int offset = lseek(facultyFileDescriptor,-sizeof(struct Faculty), SEEK_CUR);
              if (offset == -1)
	      {
		perror("Error while seeking to required student record!");
		return ;
	      }
	    
	      writeBytes = write(facultyFileDescriptor, &faculty, sizeof(struct Faculty));
	      if (writeBytes == -1)
	      {
		perror("Error while updating student Password info into file");
	      }
	      close(facultyFileDescriptor);
	      writeBytes = write(connFD, "\n\nThe required modification was successfully made!\nYou'll now be redirected to the main menu!", strlen("\n\nThe required modification was successfully made!\nYou'll now be redirected to the main menu!"));
	      }     
              else{  //if password does not match
                  bzero(writeBuffer, sizeof(writeBuffer));
                  writeBytes = write(connFD, "Entered password does not match: \n", strlen("Entered password does not match: \n"));
	          if (writeBytes == -1) {
		     perror("Error while writing password error msg to the client!");
		     return ;
	          }
	   
              goto Retry;
              }    
          }//userFound if
        
          return true;
          }
