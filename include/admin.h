#include "common.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <crypt.h>

bool admin_operation_handler(int connFD);
bool addStudent(int connFD);
bool viewStudentDetails(int connFD);
bool modifyStudent(int connFD);
bool addFaculty(int connFD);
bool viewFacultyDetails(int connFD);
bool activateStudent(int connFD);
bool blockStudent(int connFD);
bool modifyFaculty(int connFD);

//------------------------------ADMIN OPERATIONS------------------------------
bool admin_operation_handler(int connFD)
{
    if (login_handler(1, connFD, NULL, NULL, NULL))
    {
        ssize_t writeBytes, readBytes;            // Number of bytes read from / written to the client
        char readBuffer[1000], writeBuffer[1000]; // A buffer used for reading & writing to the client
        bzero(writeBuffer, sizeof(writeBuffer));
        bzero(readBuffer, sizeof(readBuffer));
        strcpy(writeBuffer, "\n----------------------------------------------------------\nWelcome Admin! Login successful! \n----------------------------------------------------------\n");
        
        Label:
        
        while (1)
        {
        
            strcat(writeBuffer, "\n\n------------------------Admin Menu-------------------------\n");
            strcat(writeBuffer, "\n1. Add Student\n2. View Student Details\n3. Modify Student Information\n4. Add Faculty\n5. View Faculty Details\n6. Activate Student \n7. Block Student\n8. Modify Faculty Details\n9. Logout and Exit \n");           
            strcat(writeBuffer, "\n------------------------------------------------------------\nEnter your choice : ");
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing ADMIN_MENU to client!");
                return false;
            }
            bzero(writeBuffer, sizeof(writeBuffer));

            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error while reading client's choice for ADMIN_MENU");
                return false;
            }

            int choice = atoi(readBuffer);
            if(choice<1 || choice>9){
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
                addStudent(connFD);
                break;
	
            case 2:
                viewStudentDetails(connFD);
                break;
                
            case 3:
                modifyStudent(connFD);
                break;

            case 4:
                addFaculty(connFD);
                break;

            case 5:
                viewFacultyDetails(connFD);
                break;
            
            case 6:
            	 activateStudent(connFD);	
            	 break;
            
            case 7:
            	 blockStudent(connFD);
		 break;
	    
	    case 8:
	    	 modifyFaculty(connFD);
	    	 break;
	    
	    case 9:
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
        // ADMIN LOGIN FAILED
        printf("Unauthorized Admin\n");
        return false;
    }
    
    goto Label;
    
    return true;
}



//-------------------------------ADD STUDENT------------------------------
bool addStudent(int connFD) {
      struct Student newStudent, previousStudent;
      ssize_t readBytes, writeBytes;
      char readBuffer[1000], writeBuffer[1000];
      bzero(readBuffer, sizeof(readBuffer));
      bzero(writeBuffer, sizeof(writeBuffer));
      
      int studentFileDescriptor = open(STUDENT_FILE, O_RDWR);
      if(studentFileDescriptor==-1){
      perror("Error while opening student file");
        return false;
      }
     
      sprintf(writeBuffer, "Enter the student's name: ");
      writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
      if (writeBytes == -1) {
        perror("Error writing message to client!");
        return false;
    }
     bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading student name from client!");
        return false;
    }

     strcpy(newStudent.name, readBuffer);
     bzero(readBuffer, sizeof(readBuffer));
     bzero(writeBuffer, sizeof(writeBuffer));
     
     sprintf(writeBuffer, "Enter the gender: ");
      writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
      if (writeBytes == -1) {
        perror("Error writing message to client!");
        return false;
    }
     bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading student gender from client!");
        return false;
    }

    strcpy(newStudent.gender, readBuffer);
    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));
     
     sprintf(writeBuffer, "Enter the student's age: ");
     writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
     if (writeBytes == -1) {
        perror("Error writing message to client!");
        return false;
     }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading student age from client!");
        return false;
    }

     int studentAge = atoi(readBuffer);
     newStudent.age = studentAge;
    
      int offset = lseek(studentFileDescriptor, 0, SEEK_END);
      if(offset==0){ 
      strcpy(newStudent.id, "MT2023001");
      }else{
      // Increment the student ID
         int offset1 = lseek(studentFileDescriptor, -sizeof(struct Student), SEEK_END);
     readBytes = read(studentFileDescriptor, &previousStudent, sizeof(struct Student));
      if (offset1 == -1) {
            perror("Error seeking to the last Student record!");
            return false;
        }
        char numericalPart[4];
        int numericalValue = atoi(previousStudent.id + 6);
        numericalValue++;
        sprintf(numericalPart, "%03d", numericalValue);
        strcpy(newStudent.id, "MT2023");
        strcat(newStudent.id, numericalPart);
      }
      newStudent.isActive=true;
      initializeStudent(&newStudent); // Call the initialization function
      strcpy(newStudent.password,"iiitb@");
      strcat(newStudent.password,newStudent.name);
    writeBytes = write(studentFileDescriptor, &newStudent, sizeof(struct Student));
    if (writeBytes == -1) {
        perror("Error while writing Student record to file!");
        return false;
    }

    close(studentFileDescriptor);
    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "The newly created student's ID is: %s", newStudent.id);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    return true;
}

//------------------------------------------VIEW STUDENT DETAILS---------------------------------------
bool viewStudentDetails(int connFD) {
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000], tempBuff[15];

    struct Student student;
    int studentFileDescriptor;
    struct Course course;

    // Prompt the user to enter the student ID.
    sprintf(writeBuffer, "Enter the student ID of the student you're searching for: ");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error writing message to client!");
        return false;
    }
    
    // Read the student ID from the client.
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading student ID from client!");
        return false;
    }

    strcpy(tempBuff, readBuffer);
    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));

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
        return false;
    }

    bool studentFound = false;
    while (read(studentFileDescriptor, &student, sizeof(struct Student)) > 0) {
        if (strcmp(student.id, tempBuff) == 0) {
            studentFound = true;
            break;
        }
    }

    if (studentFound) {
        bzero(writeBuffer, sizeof(writeBuffer));
        sprintf(writeBuffer, "Student Details - \n\tID: %s\n\tName: %s\n\tAge: %d\n\tGender: %s\n\n\tEnrolled Courses:\n", student.id, student.name, student.age, student.gender);
        // Now, we'll iterate through the enrolled courses and retrieve their names.
        for (int i = 0; i < student.numEnrolledCourses; i++) {
            // Open the course file and search for the course by ID.
            int courseFileDescriptor = open(COURSE_FILE, O_RDONLY); 
            while (read(courseFileDescriptor, &course, sizeof(struct Course)) > 0) {
                if (strcmp(course.id, student.enrolledCourses[i]) == 0) {
                    strcat(writeBuffer, "\t");
                    strcat(writeBuffer, course.name);
                    strcat(writeBuffer, "\n");
                    break;
                }
            }
            close(courseFileDescriptor);
        }

        strcat(writeBuffer, "\nYou'll now be redirected to the main menu...");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1) {
            perror("Error writing student info to the client!");
            return false;
        }
        return true;
    } else {
        // Student record doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "\n**No student could be found for the given ID**\n");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1) {
            perror("Error while writing STUDENT_ID_DOESNT_EXIST message to the client!");
            return false;
        }
    }

    return true;
}


//------------------------------------MODIFY STUDENT DETAILS------------------------------------
bool modifyStudent(int connFD) {
    struct Student modifiedStudent;
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000], tempBuff[15];
    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));

    int studentFileDescriptor = open(STUDENT_FILE, O_RDWR);
    if (studentFileDescriptor == -1) {
        perror("Error while opening student file");
        return false;
    }
    
    bzero(writeBuffer, sizeof(writeBuffer));
    
    sprintf(writeBuffer, "Enter the ID of the student whose information you want to edit: ");
      writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
      if (writeBytes == -1) {
        perror("Error writing message to client!");
        return false;
    }
     bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading student id from client!");
        return false;
    }

    strcpy(tempBuff, readBuffer);
    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));

    // Find the student record by their ID
    int found = 0;
    while (read(studentFileDescriptor, &modifiedStudent, sizeof(struct Student)) > 0) {
        if (strcmp(modifiedStudent.id, tempBuff) == 0) {
            found = 1;
            break;
        }
    }

    if (!found) {
        close(studentFileDescriptor);
        sprintf(writeBuffer, "Student with ID %s not found.", tempBuff);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        return false;
    }
    
    // Ask the user what they want to modify
    sprintf(writeBuffer, "Select the field you want to modify:\n1. Name\n2. Age\n3. Gender\n4. Exit\nEnter the option (1/2/3/4): ");
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
                perror("Error reading student name from client!");
                return false;
            }

            if (strlen(readBuffer) > 1) {
                strcpy(modifiedStudent.name, readBuffer);
            }
            break;
            
        case 2:
            // Modify the age
            sprintf(writeBuffer, "Enter the new age: ");
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1) {
               perror("Error writing message to client!");
               return false;
            }

            bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1) {
               perror("Error reading student age from client!");
               return false;
            }

            int newAge = atoi(readBuffer);
            modifiedStudent.age = newAge;
            break;
            
         case 3:
            // Modify the gender
            sprintf(writeBuffer, "Enter the gender: ");
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1) {
                perror("Error writing message to client!");
                return false;
            }

            bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1) {
                perror("Error reading student gender from client!");
                return false;
            }

            if (strlen(readBuffer) > 1) {
                strcpy(modifiedStudent.gender, readBuffer);
            }
            break;   
            
          case 4:
            // Exit without modification
            close(studentFileDescriptor);
            return false;

          default:
            // Invalid choice
            sprintf(writeBuffer, "Invalid option. Please enter a valid option (1/2/3/4).");
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            return false;
    }

    // Update the modified student record in the file
    lseek(studentFileDescriptor, -sizeof(struct Student), SEEK_CUR);
    writeBytes = write(studentFileDescriptor, &modifiedStudent, sizeof(struct Student));
    if (writeBytes == -1) {
        perror("Error while writing modified Student record to file!");
        return false;
    }

    close(studentFileDescriptor);
    sprintf(writeBuffer, "Student with ID %s has been successfully modified.", tempBuff);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    return true;
}



//------------------------------------ADD FACULTY---------------------------------------
bool addFaculty(int connFD) {
struct Faculty newFaculty, previousFaculty;
      ssize_t readBytes, writeBytes;
      char readBuffer[1000], writeBuffer[1000];
      bzero(readBuffer, sizeof(readBuffer));
      bzero(writeBuffer, sizeof(writeBuffer));
      
      int facultyFileDescriptor = open(FACULTY_FILE, O_RDWR);
      if(facultyFileDescriptor==-1){
      perror("Error while opening faculty file");
        return false;
      }
     
      sprintf(writeBuffer, "Enter the faculty's name: ");
      writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
      if (writeBytes == -1) {
        perror("Error writing message to client!");
        return false;
    }
     bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading faculty name from client!");
        return false;
    }
    strcpy(newFaculty.name, readBuffer);
    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "Enter the faculty's department: ");
      writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
      if (writeBytes == -1) {
        perror("Error writing message to client!");
        return false;
    }
     bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading faculty department from client!");
        return false;
    }

    strcpy(newFaculty.department, readBuffer);
    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "Enter the faculty's age: ");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error writing message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading faculty age from client!");
        return false;
    }

    int facultyAge = atoi(readBuffer);
     newFaculty.age = facultyAge;
    
      int offset = lseek(facultyFileDescriptor, 0, SEEK_END);
      if(offset==0){ 
      strcpy(newFaculty.id, "FAC001");
      }else{
      // Increment the faculty ID
         int offset1 = lseek(facultyFileDescriptor, -sizeof(struct Faculty), SEEK_END);
     readBytes = read(facultyFileDescriptor, &previousFaculty, sizeof(struct Faculty));
      if (offset1 == -1) {
            perror("Error seeking to the last Faculty record!");
            return false;
        }
        char numericalPart[4];
        int numericalValue = atoi(previousFaculty.id + 3);
        numericalValue++;
        sprintf(numericalPart, "%03d", numericalValue);
        strcpy(newFaculty.id, "FAC");
        strcat(newFaculty.id, numericalPart);
      }
      strcpy(newFaculty.password,"iiitb@");
      strcat(newFaculty.password,newFaculty.name);
    writeBytes = write(facultyFileDescriptor, &newFaculty, sizeof(struct Faculty));
    if (writeBytes == -1) {
        perror("Error while writing Faculty record to file!");
        return false;
    }

    close(facultyFileDescriptor);
    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "The newly created faculty's ID is: %s", newFaculty.id);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    return true;
    }


//-----------------------------------VIEW FACULTY DETAILS--------------------------------------
bool viewFacultyDetails(int connFD) {
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000], tempBuff[15];

    struct Faculty faculty;
    struct Course course;
    int facultyFD, courseFD;

    // Prompt the user to enter the faculty ID.
    sprintf(writeBuffer, "Enter the faculty ID of the faculty you're searching for: ");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error writing message to client!");
        return false;
    }
    
    // Read the faculty ID from the client.
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading student ID from client!");
        return false;
    }

    strcpy(tempBuff, readBuffer);
    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));

    facultyFD = open(FACULTY_FILE, O_RDONLY);
    if (facultyFD == -1) {
        // faculty File doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "\n**No faculty could be found for the given ID**\n");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1) {
            perror("Error while writing faculty_ID_DOESNT_EXIST message to the client!");
            return false;
        }
       
        return false;
    }

    bool facultyFound = false;
    while (read(facultyFD, &faculty, sizeof(struct Faculty)) > 0) {
        if (strcmp(faculty.id, tempBuff) == 0) {
            facultyFound = true;
            break;
        }
    }

    if (facultyFound) {
        bzero(writeBuffer, sizeof(writeBuffer));
        sprintf(writeBuffer, "Faculty Details - \n\tID: %s\n\tName: %s\n\tAge: %d", faculty.id, faculty.name, faculty.age);
        strcat(writeBuffer, "\n\nCourses Taught:\n");

        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1) {
            perror("Error writing faculty info to the client!");
            return false;
        }

        courseFD = open(COURSE_FILE, O_RDONLY);
        if (courseFD == -1) {
            // course File doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "\n**No course could be found for the given ID**\n");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1) {
            perror("Error while writing COURSE_DOESNT_EXIST message to the client!");
            return false;
        }
        return false;
        }
        else {
            while (read(courseFD, &course, sizeof(struct Course)) > 0) {
                if (strcmp(course.facultyid, tempBuff) == 0) {
                    bzero(writeBuffer, sizeof(writeBuffer));
                    sprintf(writeBuffer, "\t- %s\n", course.name);
                    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
                    bzero(writeBuffer, sizeof(writeBuffer));
                }
            }
        }
        
        strcat(writeBuffer, "\nYou'll now be redirected to the main menu...");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1) {
            perror("Error writing student info to the client!");
            return false;
        }
        return true;
    } else {
        // Faculty record doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "\n**No faculty could be found for the given ID**\n");
 
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1) {
            perror("Error while writing faculty_ID_DOESNT_EXIST message to the client!");
            return false;
        }
    }
    return true;
}


//-------------------------------------ACTIVATE STUDENT------------------------------------
bool activateStudent(int connFD) {
    struct Student student;
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];
    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));

    int studentFileDescriptor = open(STUDENT_FILE, O_RDWR);
    if (studentFileDescriptor == -1) {
        perror("Error while opening student file");
        return false;
    }

    sprintf(writeBuffer, "Enter the student's ID to activate: ");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error writing message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading student ID from client!");
        return false;
    }

    bool studentFound = false;
    while (read(studentFileDescriptor, &student, sizeof(struct Student)) > 0) {
        if (strcmp(student.id, readBuffer) == 0) {
            if (!student.isActive) {
                studentFound = true;
                // Update the isActive status to true (activate the student)
                student.isActive = true;
                // Seek back to the beginning of the record and write the updated student data
                int offset = lseek(studentFileDescriptor, -sizeof(struct Student), SEEK_CUR);
                if (offset == -1) {
                    perror("Error seeking to the student record!");
                    return false;
                }
                writeBytes = write(studentFileDescriptor, &student, sizeof(struct Student));
                if (writeBytes == -1) {
                    perror("Error while updating student record!");
                    return false;
                }
                break;
            } else {
                // The student is already active
                bzero(writeBuffer, sizeof(writeBuffer));
                strcpy(writeBuffer, "Student is already active.");
                writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
                return false;
            }
        }
    }

    if (!studentFound) {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "Student with the provided ID not found.");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        return false;
    }

    close(studentFileDescriptor);
    bzero(writeBuffer, sizeof(writeBuffer));
    strcpy(writeBuffer, "Student has been successfully activated.");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    return true;
}


//-------------------------------------BLOCK STUDENT------------------------------------
bool blockStudent(int connFD) {
    struct Student student;
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];
    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));

    int studentFileDescriptor = open(STUDENT_FILE, O_RDWR);
    if (studentFileDescriptor == -1) {
        perror("Error while opening student file");
        return false;
    }

    sprintf(writeBuffer, "Enter the student's ID to block: ");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error writing message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading student ID from client!");
        return false;
    }

    bool studentFound = false;
    while (read(studentFileDescriptor, &student, sizeof(struct Student)) > 0) {
        if (strcmp(student.id, readBuffer) == 0) {
            if (student.isActive) {
                studentFound = true;
                // Update the isActive status to false
                student.isActive = false;
                // Seek back to the beginning of the record and write the updated student data
                int offset = lseek(studentFileDescriptor, -sizeof(struct Student), SEEK_CUR);
                if (offset == -1) {
                    perror("Error seeking to the student record!");
                    return false;
                }
                writeBytes = write(studentFileDescriptor, &student, sizeof(struct Student));
                if (writeBytes == -1) {
                    perror("Error while updating student record!");
                    return false;
                }
                break;
            } else {
                // The student is already blocked
                bzero(writeBuffer, sizeof(writeBuffer));
                strcpy(writeBuffer, "Student is already blocked.");
                writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
                return false;
            }
        }
    }

    if (!studentFound) {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "Student with the provided ID not found.");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        return false;
    }

    close(studentFileDescriptor);
    bzero(writeBuffer, sizeof(writeBuffer));
    strcpy(writeBuffer, "Student has been successfully blocked.");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    return true;
}
	

//------------------------------------MODIFY FACULTY DETAILS------------------------------------
bool modifyFaculty(int connFD) {
    struct Faculty modifiedFaculty;
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000], tempBuff[15];
    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));

    int facultyFileDescriptor = open(FACULTY_FILE, O_RDWR); // Assuming FACULTY_FILE is the file containing faculty records
    if (facultyFileDescriptor == -1) {
        perror("Error while opening faculty file");
        return false;
    }
    
    bzero(writeBuffer, sizeof(writeBuffer));
    
    sprintf(writeBuffer, "Enter the ID of the faculty whose information you want to edit: ");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error writing message to client!");
        return false;
    }
    
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading faculty id from client!");
        return false;
    }

    strcpy(tempBuff, readBuffer);
    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));

    // Find the faculty record by their ID
    int found = 0;
    while (read(facultyFileDescriptor, &modifiedFaculty, sizeof(struct Faculty)) > 0) {
        if (strcmp(modifiedFaculty.id, tempBuff) == 0) {
            found = 1;
            break;
        }
    }

    if (!found) {
        close(facultyFileDescriptor);
        sprintf(writeBuffer, "Faculty with ID %s not found.", tempBuff);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        return false;
    }
    
    // Ask the user what they want to modify
    sprintf(writeBuffer, "Select the field you want to modify:\n1. Name\n2. Age\n3. Department\n4. Exit\nEnter the option (1/2/3/4): ");
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
                perror("Error reading faculty name from client!");
                return false;
            }

            if (strlen(readBuffer) > 1) {
                strcpy(modifiedFaculty.name, readBuffer);
            }
            break;
            
        case 2:
            // Modify the age
            sprintf(writeBuffer, "Enter the new age: ");
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1) {
                perror("Error writing message to client!");
                return false;
            }

            bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1) {
                perror("Error reading faculty age from client!");
                return false;
            }

            int newAge = atoi(readBuffer);
            modifiedFaculty.age = newAge;
            break;
            
        case 3:
            // Modify the department
            sprintf(writeBuffer, "Enter the new department (or press Enter to keep the current department): ");
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1) {
                perror("Error writing message to client!");
                return false;
            }

            bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1) {
                perror("Error reading faculty department from client!");
                return false;
            }

            if (strlen(readBuffer) > 1) {
                strcpy(modifiedFaculty.department, readBuffer);
            }
            break;   
            
        case 4:
            // Exit without modification
            close(facultyFileDescriptor);
            return false;

        default:
            // Invalid choice
            sprintf(writeBuffer, "Invalid option. Please enter a valid option (1/2/3/4).");
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            return false;
    }

    // Update the modified faculty record in the file
    lseek(facultyFileDescriptor, -sizeof(struct Faculty), SEEK_CUR);
    writeBytes = write(facultyFileDescriptor, &modifiedFaculty, sizeof(struct Faculty));
    if (writeBytes == -1) {
        perror("Error while writing modified Faculty record to file!");
        return false;
    }

    close(facultyFileDescriptor);
    sprintf(writeBuffer, "Faculty with ID %s has been successfully modified.", tempBuff);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    return true;
}
