/*
 * Advanced Student Management System
 * C Mini Project Source Code
 *
 * Converted from the supplied Java Student Management System document.
 *
 * Features:
 * - Full CRUD Operations with Unique ID Validation
 * - Department & Semester Categorization
 * - Multi-criteria Searching (by ID, Name, or Department)
 * - Sorting by Marks descending or Name alphabetically
 * - Statistical Analytics
 * - Audit Trail / Activity Logging
 * - Persistent Storage
 * - CSV Export
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_STUDENTS 1000
#define MAX_NAME 100
#define MAX_DEPT 50
#define MAX_EMAIL 120
#define MAX_CONTACT 30
#define MAX_LOGS 2000
#define MAX_LOG 250

#define DATA_FILE "sms_database.dat"
#define CSV_FILE "students_report.csv"

typedef struct {
    int id;
    char name[MAX_NAME];
    char department[MAX_DEPT];
    int semester;
    double marks;
    char email[MAX_EMAIL];
    char contactNumber[MAX_CONTACT];
    char lastModified[32];
} Student;

Student students[MAX_STUDENTS];
int studentCount = 0;

char auditLogs[MAX_LOGS][MAX_LOG];
int auditCount = 0;

/* ---------- Utility Functions ---------- */

void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void readLine(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    if (fgets(buffer, (int)size, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }
    buffer[strcspn(buffer, "\n")] = '\0';
}

int readInt(const char *prompt) {
    int value;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &value) == 1) {
            clearInputBuffer();
            return value;
        }
        printf("[Error] Please enter a valid integer.\n");
        clearInputBuffer();
    }
}

double readDouble(const char *prompt) {
    double value;
    while (1) {
        printf("%s", prompt);
        if (scanf("%lf", &value) == 1) {
            clearInputBuffer();
            return value;
        }
        printf("[Error] Please enter a valid number.\n");
        clearInputBuffer();
    }
}

void updateTimestamp(Student *s) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    if (tm_info != NULL) {
        strftime(s->lastModified, sizeof(s->lastModified),
                 "%Y-%m-%d %H:%M:%S", tm_info);
    }
}

void logActivity(const char *message) {
    if (auditCount >= MAX_LOGS) return;

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[32];

    if (tm_info != NULL) {
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    } else {
        strcpy(timestamp, "Unknown time");
    }

    snprintf(auditLogs[auditCount], MAX_LOG,
             "[%s] %s", timestamp, message);
    auditCount++;
}

/* ---------- Student Functions ---------- */

const char *calculateGrade(double marks) {
    if (marks >= 90) return "A+ (Outstanding)";
    if (marks >= 80) return "A (Excellent)";
    if (marks >= 70) return "B (Good)";
    if (marks >= 60) return "C (Average)";
    if (marks >= 50) return "D (Pass)";
    return "F (Fail)";
}

int findStudentById(int id) {
    for (int i = 0; i < studentCount; i++) {
        if (students[i].id == id)
            return i;
    }
    return -1;
}

void displayStudent(const Student *s) {
    printf("\nID                   : %d\n", s->id);
    printf("Name                 : %s\n", s->name);
    printf("Department           : %s (Sem %d)\n",
           s->department, s->semester);
    printf("Marks                : %.2f / 100\n", s->marks);
    printf("Grade                : %s\n", calculateGrade(s->marks));
    printf("Email                : %s\n", s->email);
    printf("Contact              : %s\n", s->contactNumber);
    printf("Last Updated         : %s\n", s->lastModified);
}

void addStudent(void) {
    if (studentCount >= MAX_STUDENTS) {
        printf("[Error] Student database is full.\n");
        return;
    }

    printf("\n--- Add New Student Record ---\n");

    int id = readInt("Enter Unique Student ID: ");

    if (findStudentById(id) != -1) {
        printf("[Error] Student ID already exists in the system database!\n");
        return;
    }

    Student *s = &students[studentCount];

    s->id = id;
    readLine("Enter Full Name: ", s->name, sizeof(s->name));
    readLine("Enter Department (e.g., CSE, ECE, MECH): ",
              s->department, sizeof(s->department));

    for (size_t i = 0; s->department[i] != '\0'; i++)
        s->department[i] = (char)toupper((unsigned char)s->department[i]);

    s->semester = readInt("Enter Current Semester (1-8): ");

    if (s->semester < 1 || s->semester > 8) {
        printf("[Warning] Invalid semester. Setting semester to 1.\n");
        s->semester = 1;
    }

    s->marks = readDouble("Enter Total Marks (0.0 - 100.0): ");

    if (s->marks < 0 || s->marks > 100) {
        printf("[Error] Marks must be between 0 and 100. Record creation aborted.\n");
        return;
    }

    readLine("Enter Email Address: ", s->email, sizeof(s->email));
    readLine("Enter Contact Number: ", s->contactNumber,
             sizeof(s->contactNumber));

    updateTimestamp(s);
    studentCount++;

    saveData();

    char logMessage[MAX_LOG];
    snprintf(logMessage, sizeof(logMessage),
             "Added student: ID %d (%s)", id, s->name);
    logActivity(logMessage);

    printf("[Success] Student record created and persisted successfully!\n");
}

void viewAllStudents(void) {
    if (studentCount == 0) {
        printf("\n[Info] No student records found.\n");
        return;
    }

    printf("\n--- All Student Records ---\n");

    for (int i = 0; i < studentCount; i++) {
        printf("\nStudent %d of %d", i + 1, studentCount);
        displayStudent(&students[i]);
    }
}

void searchStudentMenu(void) {
    if (studentCount == 0) {
        printf("\n[Info] No student records found.\n");
        return;
    }

    int choice = readInt(
        "\nSearch by:\n"
        "1. ID\n"
        "2. Name\n"
        "3. Department\n"
        "Select option: "
    );

    if (choice == 1) {
        int id = readInt("Enter Student ID: ");
        int index = findStudentById(id);

        if (index == -1)
            printf("[Info] Student not found.\n");
        else
            displayStudent(&students[index]);

    } else if (choice == 2 || choice == 3) {
        char query[100];
        readLine(choice == 2 ? "Enter Name: " : "Enter Department: ",
                 query, sizeof(query));

        int found = 0;

        for (int i = 0; i < studentCount; i++) {
            int match = 0;

            if (choice == 2) {
                char nameCopy[MAX_NAME];
                char queryCopy[100];

                strncpy(nameCopy, students[i].name, sizeof(nameCopy));
                nameCopy[sizeof(nameCopy) - 1] = '\0';
                strncpy(queryCopy, query, sizeof(queryCopy));
                queryCopy[sizeof(queryCopy) - 1] = '\0';

                for (size_t j = 0; nameCopy[j]; j++)
                    nameCopy[j] =
                        (char)tolower((unsigned char)nameCopy[j]);

                for (size_t j = 0; queryCopy[j]; j++)
                    queryCopy[j] =
                        (char)tolower((unsigned char)queryCopy[j]);

                match = strstr(nameCopy, queryCopy) != NULL;
            } else {
                match = strcasecmp(students[i].department, query) == 0;
            }

            if (match) {
                displayStudent(&students[i]);
                found = 1;
            }
        }

        if (!found)
            printf("[Info] No matching students found.\n");
    } else {
        printf("[Error] Invalid search option.\n");
    }
}

void updateStudent(void) {
    int id = readInt("\nEnter Student ID to update: ");
    int index = findStudentById(id);

    if (index == -1) {
        printf("[Error] Student not found.\n");
        return;
    }

    Student *s = &students[index];

    printf("\n--- Update Student Details ---\n");
    displayStudent(s);

    readLine("\nEnter New Name: ", s->name, sizeof(s->name));
    readLine("Enter New Department: ",
             s->department, sizeof(s->department));

    for (size_t i = 0; s->department[i] != '\0'; i++)
        s->department[i] = (char)toupper((unsigned char)s->department[i]);

    s->semester = readInt("Enter New Semester (1-8): ");
    s->marks = readDouble("Enter New Marks (0.0 - 100.0): ");

    if (s->marks < 0 || s->marks > 100) {
        printf("[Error] Marks must be between 0 and 100.\n");
        return;
    }

    readLine("Enter New Email: ", s->email, sizeof(s->email));
    readLine("Enter New Contact Number: ",
             s->contactNumber, sizeof(s->contactNumber));

    updateTimestamp(s);
    saveData();

    char logMessage[MAX_LOG];
    snprintf(logMessage, sizeof(logMessage),
             "Updated student: ID %d (%s)", id, s->name);
    logActivity(logMessage);

    printf("[Success] Student record updated successfully!\n");
}

void deleteStudent(void) {
    int id = readInt("\nEnter Student ID to delete: ");
    int index = findStudentById(id);

    if (index == -1) {
        printf("[Error] Student not found.\n");
        return;
    }

    char name[MAX_NAME];
    strcpy(name, students[index].name);

    for (int i = index; i < studentCount - 1; i++)
        students[i] = students[i + 1];

    studentCount--;
    saveData();

    char logMessage[MAX_LOG];
    snprintf(logMessage, sizeof(logMessage),
             "Deleted student: ID %d (%s)", id, name);
    logActivity(logMessage);

    printf("[Success] Student record deleted successfully!\n");
}

/* ---------- Sorting ---------- */

int compareMarksDescending(const void *a, const void *b) {
    const Student *s1 = (const Student *)a;
    const Student *s2 = (const Student *)b;

    if (s1->marks < s2->marks) return 1;
    if (s1->marks > s2->marks) return -1;
    return 0;
}

int compareNamesAscending(const void *a, const void *b) {
    const Student *s1 = (const Student *)a;
    const Student *s2 = (const Student *)b;
    return strcasecmp(s1->name, s2->name);
}

void sortStudentsMenu(void) {
    if (studentCount == 0) {
        printf("\n[Info] No student records found.\n");
        return;
    }

    int choice = readInt(
        "\nSort by:\n"
        "1. Marks descending\n"
        "2. Name alphabetically\n"
        "Select option: "
    );

    if (choice == 1) {
        qsort(students, studentCount, sizeof(Student),
              compareMarksDescending);
    } else if (choice == 2) {
        qsort(students, studentCount, sizeof(Student),
              compareNamesAscending);
    } else {
        printf("[Error] Invalid sorting option.\n");
        return;
    }

    printf("\n[Success] Records sorted successfully.\n");
    viewAllStudents();
}

/* ---------- Analytics ---------- */

void showAnalytics(void) {
    if (studentCount == 0) {
        printf("\n[Info] No student records available for analytics.\n");
        return;
    }

    double total = 0;
    int highest = 0;
    int lowest = 0;
    int gradeCounts[6] = {0};

    for (int i = 0; i < studentCount; i++) {
        total += students[i].marks;

        if (students[i].marks > students[highest].marks)
            highest = i;

        if (students[i].marks < students[lowest].marks)
            lowest = i;

        if (students[i].marks >= 90) gradeCounts[0]++;
        else if (students[i].marks >= 80) gradeCounts[1]++;
        else if (students[i].marks >= 70) gradeCounts[2]++;
        else if (students[i].marks >= 60) gradeCounts[3]++;
        else if (students[i].marks >= 50) gradeCounts[4]++;
        else gradeCounts[5]++;
    }

    printf("\n--- Class Performance Analytics ---\n");
    printf("Total Students Enrolled                 : %d\n", studentCount);
    printf("Class Average Marks                     : %.2f / 100\n",
           total / studentCount);
    printf("Top Performer                           : %s (ID: %d) with %.2f marks\n",
           students[highest].name, students[highest].id,
           students[highest].marks);
    printf("Lowest Performer                        : %s (ID: %d) with %.2f marks\n",
           students[lowest].name, students[lowest].id,
           students[lowest].marks);

    printf("\nGrade Distribution:\n");
    printf("A+ (90-100) : %d\n", gradeCounts[0]);
    printf("A  (80-89)  : %d\n", gradeCounts[1]);
    printf("B  (70-79)  : %d\n", gradeCounts[2]);
    printf("C  (60-69)  : %d\n", gradeCounts[3]);
    printf("D  (50-59)  : %d\n", gradeCounts[4]);
    printf("F  (0-49)   : %d\n", gradeCounts[5]);
}

/* ---------- CSV Export ---------- */

void escapeCsv(const char *src, char *dest, size_t size) {
    int needsQuotes = strchr(src, ',') != NULL || strchr(src, '"') != NULL;
    size_t pos = 0;

    if (needsQuotes && pos < size - 1)
        dest[pos++] = '"';

    for (size_t i = 0; src[i] != '\0' && pos < size - 2; i++) {
        if (src[i] == '"' && needsQuotes) {
            if (pos < size - 2) dest[pos++] = '"';
            if (pos < size - 2) dest[pos++] = '"';
        } else {
            dest[pos++] = src[i];
        }
    }

    if (needsQuotes && pos < size - 1)
        dest[pos++] = '"';

    dest[pos] = '\0';
}

void exportToCsv(void) {
    FILE *file = fopen(CSV_FILE, "w");

    if (!file) {
        printf("[Error] Could not create CSV file.\n");
        return;
    }

    fprintf(file,
            "ID,Name,Department,Semester,Marks,Grade,Email,Contact,Last Modified\n");

    for (int i = 0; i < studentCount; i++) {
        char name[220], dept[120], email[240], contact[100];

        escapeCsv(students[i].name, name, sizeof(name));
        escapeCsv(students[i].department, dept, sizeof(dept));
        escapeCsv(students[i].email, email, sizeof(email));
        escapeCsv(students[i].contactNumber, contact, sizeof(contact));

        fprintf(file, "%d,%s,%s,%d,%.2f,\"%s\",%s,%s,%s\n",
                students[i].id, name, dept, students[i].semester,
                students[i].marks, calculateGrade(students[i].marks),
                email, contact, students[i].lastModified);
    }

    fclose(file);

    logActivity("Exported student records to CSV.");
    printf("[Success] Report exported to %s\n", CSV_FILE);
}

/* ---------- Persistence ---------- */

void saveData(void) {
    FILE *file = fopen(DATA_FILE, "wb");

    if (!file) {
        printf("[Error] Could not save database.\n");
        return;
    }

    fwrite(&studentCount, sizeof(studentCount), 1, file);
    fwrite(students, sizeof(Student), studentCount, file);

    fclose(file);
}

void loadData(void) {
    FILE *file = fopen(DATA_FILE, "rb");

    if (!file) {
        studentCount = 0;
        return;
    }

    if (fread(&studentCount, sizeof(studentCount), 1, file) != 1) {
        studentCount = 0;
        fclose(file);
        return;
    }

    if (studentCount < 0 || studentCount > MAX_STUDENTS) {
        studentCount = 0;
        fclose(file);
        return;
    }

    fread(students, sizeof(Student), studentCount, file);
    fclose(file);
}

/* ---------- Audit Logs ---------- */

void viewAuditLogs(void) {
    printf("\n--- System Audit Trails ---\n");

    if (auditCount == 0) {
        printf("[Info] No audit records available.\n");
        return;
    }

    for (int i = 0; i < auditCount; i++)
        printf("%s\n", auditLogs[i]);
}

/* ---------- Main Menu ---------- */

void showMainMenu(void) {
    printf("\n=============================================\n");
    printf(" ADVANCED STUDENT MANAGEMENT SYSTEM (SMS)\n");
    printf("=============================================\n");
    printf(" 1. Add New Student Record\n");
    printf(" 2. View All Student Records\n");
    printf(" 3. Advanced Search (ID, Name, or Department)\n");
    printf(" 4. Update Student Details\n");
    printf(" 5. Delete Student Record\n");
    printf(" 6. Sort and Display Records\n");
    printf(" 7. View Class Performance Analytics\n");
    printf(" 8. Export Report to CSV Format\n");
    printf(" 9. View System Audit Trails\n");
    printf("10. Save & Exit Application\n");
    printf("=============================================\n");
}

int main(void) {
    loadData();
    logActivity("Application started.");

    int choice;

    do {
        showMainMenu();
        choice = readInt("Select an option (1-10): ");

        switch (choice) {
            case 1:
                addStudent();
                break;
            case 2:
                viewAllStudents();
                break;
            case 3:
                searchStudentMenu();
                break;
            case 4:
                updateStudent();
                break;
            case 5:
                deleteStudent();
                break;
            case 6:
                sortStudentsMenu();
                break;
            case 7:
                showAnalytics();
                break;
            case 8:
                exportToCsv();
                break;
            case 9:
                viewAuditLogs();
                break;
            case 10:
                saveData();
                logActivity("Application closed normally.");
                printf("\n[System] Saving data & shutting down. Have a great day!\n");
                break;
            default:
                printf("\n[Error] Invalid choice! Please enter a number between 1 and 10.\n");
        }
    } while (choice != 10);

    return 0;
}
