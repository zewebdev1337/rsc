#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#define NAME "rsc" // short for run script conveniently
#define VERSION "1.0.3"
#define MAX_ARGS 64 // Maximum number of arguments allowed for a script + program name + script path + NULL

// Compare two strings for sorting
int compare_strings(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

// check if a string contains only digits
int is_valid_number(const char *str) {
    if (str == NULL || *str == '\0') { // Handle empty or NULL string
        return 0;
    }
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) {
            return 0; // Not a digit
        }
    }
    return 1; // All characters are digits
}

// Helper to free script names array
void free_script_names(char **names, int count) {
    if (names) {
        for (int i = 0; i < count; i++) {
            free(names[i]); // Free each duplicated string
        }
        free(names); // Free the array
    }
}

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "-v") == 0) {
        printf("%s version %s\n", NAME, VERSION);
        return 0;
    }

    DIR *d;
    struct dirent *dir;
    d = opendir("./scripts");
    int count = 0;
    char **script_names = NULL; // Initialize to NULL

    if (!d) {
        perror("Error opening scripts directory './scripts'");
        return 1;
    }

    // First pass: count the number of scripts
    errno = 0; // Reset errno before readdir loop
    while ((dir = readdir(d)) != NULL) {
        // Skip hidden files/dirs and . & ..
        if (dir->d_name[0] != '.' && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
            count++;
        }
    }
    if (errno != 0 && dir == NULL) {
        perror("Error reading scripts directory");
        closedir(d);
        return 1;
    }

    // Handle case with no scripts
    if (count == 0) {
        printf("No scripts found in ./scripts directory.\n");
        closedir(d);
        return 0;
    }

    // Allocate memory for script names
    script_names = malloc(count * sizeof(char *));
    if (!script_names) {
        perror("Failed to allocate memory for script names");
        closedir(d);
        return 1;
    }
    // Initialize pointers to NULL in case of error during population
    for (int i = 0; i < count; i++) {
        script_names[i] = NULL;
    }

    rewinddir(d); // Go back to the beginning of the directory stream

    // Second pass: store the script names
    int i = 0;
    errno = 0; // Reset errno
    while ((dir = readdir(d)) != NULL && i < count) {
        if (dir->d_name[0] != '.' && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
            script_names[i] = strdup(dir->d_name);
            if (!script_names[i]) {
                perror("Failed to duplicate script name");
                free_script_names(script_names, i); // Free already allocated names
                closedir(d);
                return 1;
            }
            i++;
        }
    }
    if (errno != 0 && dir == NULL) {
        perror("Error reading scripts directory (second pass)");
        free_script_names(script_names, i); // Free potentially partially filled array
        closedir(d);
        return 1;
    }
    closedir(d); // Close directory handle


    // Sort the script names alphabetically
    qsort(script_names, count, sizeof(char *), compare_strings);

    // Handle single script case
    if (count == 1) {
        printf("Only one script found. Running `%s`...\n", script_names[0]);

        char chmod_command[256];
        snprintf(chmod_command, sizeof(chmod_command), "chmod +x ./scripts/%s", script_names[0]);
        if (system(chmod_command) != 0) {
             fprintf(stderr, "Warning: Failed to make script executable (chmod). Attempting to run anyway.\n");
        }


        char script_path[256];
        snprintf(script_path, sizeof(script_path), "./scripts/%s", script_names[0]);

        // Arguments for execvp: program, path_to_script, NULL
        char *exec_args[] = {"/bin/bash", script_path, NULL};

        printf("Executing: %s %s\n", exec_args[0], exec_args[1]);
        execvp(exec_args[0], exec_args);

        // If execvp returns, an error occurred
        perror("execvp failed");
        free_script_names(script_names, count);
        return 1; // Exit indicating error
    }

    // Display the sorted scripts (multiple scripts case)
    printf("Available scripts:\n");
    for (i = 0; i < count; i++) {
        printf("%d) %s\n", i + 1, script_names[i]);
    }

    // Input validation loop
    int choice = 0;
    char input[512]; // Buffer size for arguments
    char *script_args_str = NULL; // To store pointer to the start of arguments part

    while (1) {
        printf("Enter script number (or number,arg1,arg2,...): ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            if (feof(stdin)) {
                printf("\nEOF detected. Exiting.\n");
                free_script_names(script_names, count);
                return 0; // Graceful exit on EOF
            }
            perror("fgets failed");
            free_script_names(script_names, count);
            return 1; // Error reading input
        }

        // Remove newline character
        input[strcspn(input, "\n")] = '\0';

        // Find the first comma
        char *comma_pos = strchr(input, ',');
        char num_part[16]; // Buffer for the number part

        if (comma_pos != NULL) {
            // Comma found, split the string
            size_t num_len = comma_pos - input;
            if (num_len == 0) { // Comma at the beginning, invalid
                 printf("Invalid input: Missing script number before comma.\n");
                 continue;
            }
            if (num_len >= sizeof(num_part)) { // Number part too long
                 printf("Invalid input: Script number too long.\n");
                 continue;
            }
            strncpy(num_part, input, num_len);
            num_part[num_len] = '\0';
            script_args_str = comma_pos + 1; // Point to the character after the comma
        } else {
            // No comma, the whole input should be the number
            strncpy(num_part, input, sizeof(num_part) - 1);
            num_part[sizeof(num_part) - 1] = '\0'; // Ensure null termination
            script_args_str = NULL; // No arguments passed
        }

        // Validate the number part
        if (is_valid_number(num_part)) {
            choice = atoi(num_part);
            if (choice >= 1 && choice <= count) {
                break; // Valid choice, exit the loop
            } else {
                 printf("Invalid input: Script number '%d' out of range (1-%d).\n", choice, count);
            }
        } else {
             printf("Invalid input: '%s' is not a valid script number.\n", num_part);
        }
         printf("Please enter a number between 1 and %d, optionally followed by ',arg1,arg2,...'.\n", count);
    }

    // Prepare for Execution
    const char *selected_script = script_names[choice - 1];

    // Make the script executable
    char chmod_command[256];
    snprintf(chmod_command, sizeof(chmod_command), "chmod +x \"./scripts/%s\"", selected_script); // Added quotes for safety
    if (system(chmod_command) != 0) {
        fprintf(stderr, "Warning: Failed to make script executable (chmod). Attempting to run anyway.\n");
    }

    // Prepare arguments for execvp
    char script_path[256];
    snprintf(script_path, sizeof(script_path), "./scripts/%s", selected_script);

    char *exec_args[MAX_ARGS]; // Array to hold args for execvp
    int arg_count = 0;

    exec_args[arg_count++] = "/bin/bash";  // Arg 0: program name (interpreter)
    exec_args[arg_count++] = script_path; // Arg 1: script to execute

    // Parse and add script arguments if they exist
    if (script_args_str != NULL && *script_args_str != '\0') {
        // Important: strtok modifies the string it parses.
        // We are modifying the `input` buffer here via script_args_str pointer.
        char *token = strtok(script_args_str, ",");
        while (token != NULL && arg_count < MAX_ARGS - 1) { // Leave space for NULL terminator
            // Optional: Trim whitespace from token here if needed
            exec_args[arg_count++] = token;
            token = strtok(NULL, ",");
        }
        if (token != NULL) {
             fprintf(stderr, "Warning: Too many arguments provided (max %d allowed). Ignoring excess arguments.\n", MAX_ARGS - 3); // -3 for bash, script, NULL
        }
    }

    exec_args[arg_count] = NULL; // execvp requires a NULL terminated array

    // Execute the script
    printf("Executing:");
    for(int k=0; k < arg_count; ++k) {
        // Basic quoting for display if arg contains spaces (won't handle complex cases)
        if (strchr(exec_args[k], ' ') != NULL) {
            printf(" \"%s\"", exec_args[k]);
        } else {
            printf(" %s", exec_args[k]);
        }
    }
    printf("\n");
    fflush(stdout); // Ensure the "Executing" line is printed before exec replaces the process

    execvp(exec_args[0], exec_args);

    // execvp only returns on error
    perror("execvp failed");
    free_script_names(script_names, count);
    return 1; // Indicate execution error
}
