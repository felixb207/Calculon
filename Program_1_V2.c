// PC-Arduino Calculator
// Program 1: User Interface on Windows
// Author: Felix Bohdal
// Last Change: 09.07.2025
// Code was improved and Bug-fixed with the Help of Chat-GPT 4o


/*
Changelog 10.07.2025

1. display Response Funktion wurde um die neuen Errors erweitert und zeigt geeignete Hilfestellungen.

2. Get User input wurde so erweitert, dass eingabe über der länge vom Buffer (100) erkannt werden und ein Error ausgegeben wird und dieser nicht an Arduino gesendet wird

 */


#include <windows.h>  // Windows API functions for serial communication
#include <stdio.h>    // Standard input/output functions
#include <string.h>   // Functions for string manipulation
#include <time.h>     // Functions for time and inactivity management


HANDLE serialHandle;       // Handle for the serial port connection
FILE *logFile = NULL;      // File pointer for the log file
time_t lastActivityTime;   // Time of the last user action (used for inactivity detection)


// Displays Start- and Helptext
void show_help_text() {
    printf(">>> PC-Arduino Calculator <<<\n");
    printf("Do Calculations in the format: [number] [operator] [number] (e.g., 12 + 5)\n");
    printf("Calculator will terminate with command \"exit\", or after 5 minutes of inactivity\n\n");
}

//starting Serial Connection with Arduino
int connect_serial() {
    // Opening Port (Port must be adapted to correct Arduino Port)
    serialHandle = CreateFile(
        "\\\\.\\COM11", GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, 0, NULL
    );

    if (serialHandle == INVALID_HANDLE_VALUE)
        return 0;  // Connection failed

    // Configure communication parameters
    DCB serialParams = { 0 };
    serialParams.DCBlength = sizeof(serialParams);

    if (!GetCommState(serialHandle, &serialParams))
        return 0;  // Failed to get current serial parameters

    // Set serial communication settings
    serialParams.BaudRate = CBR_9600;    // Baudrate must be the same as on Arduino
    serialParams.ByteSize = 8;           // 8 bits per byte
    serialParams.StopBits = ONESTOPBIT;  // One stop bit
    serialParams.Parity   = NOPARITY;    // No parity bit

    if (!SetCommState(serialHandle, &serialParams))
        return 0;  // Failed to apply settings

    // Set  timeouts
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(serialHandle, &timeouts))
        return 0;  // Failed to set timeouts


    return 1;  // Connection Successfull
}

// Reads a string from user input, checks if under 100 characters
void get_user_input(char *inputBuffer, size_t bufferSize) {
    printf("Enter operation (or 'exit'): ");  // Prompt the user for input

    if (fgets(inputBuffer, bufferSize, stdin) != NULL) {
        size_t len = strlen(inputBuffer);

        // Check if newline character is present (i.e., entire input fits into buffer)
        if (len > 0 && inputBuffer[len - 1] == '\n') {
            inputBuffer[len - 1] = '\0'; // Remove the newline character
        } else {
            // Newline not found => input was longer than (bufferSize - 1)
            int c;
            while ((c = getchar()) != '\n' && c != EOF); // Discard remaining characters in input stream

            printf("ERROR: Input too long (max %zu characters). Please try again.\n", bufferSize - 1);
            inputBuffer[0] = '\0'; // Clear buffer, so main can detect invalid input if needed
        }

        lastActivityTime = time(NULL); // Reset inactivity timer
    }
}


// Sends the user's input string over the serial connection
void send_data(const char *data) {
    DWORD bytesWritten;
    WriteFile(serialHandle, data, strlen(data), &bytesWritten, NULL); // Write input data
    WriteFile(serialHandle, "\n", 1, &bytesWritten, NULL);            // Send newline to signal end of message
}

// Receives a response string from the Arduino
void receive_data(char *responseBuffer, size_t bufferSize) {
    DWORD bytesRead;
    BOOL result = ReadFile(serialHandle, responseBuffer, bufferSize - 1, &bytesRead, NULL);

    if (!result || bytesRead == 0) {
        // If reading failed or nothing was read, return an error string
        perror("Receiving error");
        strcpy(responseBuffer, "Receiving error");
        return;
    }

    responseBuffer[bytesRead] = '\0'; // Properly null-terminate the received string
}

// Displays the received response or an error message + help which error occured and what to do different
void display_response(const char *response) {
    if (strncmp(response, "ERROR_DIV_0", strlen("ERROR_DIV_0")) == 0) {
        printf("Error: Division by zero is not allowed.\n");
    } else if (strncmp(response, "ERROR_INVALID_INPUT", strlen("ERROR_INVALID_INPUT")) == 0) {
        printf("Error: Invalid input format.\n");
        show_help_text();
    } else if (strncmp(response, "ERROR_BUFFER_FULL", strlen("ERROR_BUFFER_FULL")) == 0) {
        printf("Error: Input buffer full on Arduino side.\n");
    } else if (strncmp(response, "ERROR", 5) == 0) {
        // Fallback für unbekannte Errors mit Prefix "ERROR"
        printf("Unknown error received: %s\n", response);
    } else {
        // Arduino returned a valid result
        printf("Result: %s\n", response);
    }
}

// Creates or opens the log file for appending new entries
// All Operations and different Instances shall be logged to the same file with Date and Time
void create_log_file() {
    logFile = fopen("calculator_log.txt", "a"); // Open in append mode
    if (!logFile)
        perror("Error creating log file");
}

// Writes a user action and Arduino response into the log file
void write_to_log(const char *input, const char *response) {
    if (logFile) {
        time_t now = time(NULL);
        struct tm *timeInfo = localtime(&now); // Convert time to local time format
        fprintf(logFile, "[%02d.%02d.%d %02d:%02d:%02d] Input: %s | Response: %s\n",
            timeInfo->tm_mday, timeInfo->tm_mon + 1, timeInfo->tm_year + 1900,
            timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec,
            input, response);  // Write a full timestamped entry
        fflush(logFile);        // Flush to file
    }
}

// Prints an error message to standard error output
void handle_error(const char *errorMessage) {
    fprintf(stderr, "ERROR: %s\n", errorMessage);
}

// Checks if user has been inactive for over 5 minutes
int check_inactivity() {
    return difftime(time(NULL), lastActivityTime) > 300; // 300 seconds = 5 minutes
}

// --- Main function ---
int main() {
    show_help_text(); // Welcome message and user guide

    if (!connect_serial()) {
        // Exit if connection failed
        handle_error("Failed to connect to Arduino.");
        return 1;
    }

    create_log_file();        // Prepare to log interactions
    lastActivityTime = time(NULL); // Initialize activity timer

    while (1) {
        if (check_inactivity()) {
            // Auto-terminate after 5 minutes of inactivity
            printf("No activity for 5 minutes. Exiting.\n");
            break;
        }

        char input[100]; // Buffer for user input
        get_user_input(input, sizeof(input)); // Ask for a new operation

        // Check if the user typed "exit"
        if (strcmp(input, "exit") == 0) {
            printf("Exiting program.\n");
            break;
        }
        // Only send to Arduino if Input is shorter than 100
        if (strcmp(input, "\0") != 0) {
            send_data(input); // Send operation to Arduino

            char response[100]; // Buffer for Arduino response
            receive_data(response, sizeof(response)); // Get response

            display_response(response); // print result
            write_to_log(input, response); // Save interaction to log file
        }
    }

    CloseHandle(serialHandle); //properly close Serial Handle
    if (logFile) fclose(logFile); // Close log file if opened

    return 0;
}
