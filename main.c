#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define NAME "rsc" // short for run script conveniently
#define VERSION "1.0.0"

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "-v") == 0) {
        printf("%s version %s\n", NAME, VERSION);
        return 0;
    }

    DIR *d;
    struct dirent *dir;
    d = opendir("./scripts");
    int count = 1;

    if (d) {
        printf("Available scripts:\n");
        while ((dir = readdir(d)) != NULL) {
            if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
                printf("%d) %s\n", count++, dir->d_name);
            }
        }
        closedir(d);

        int choice;
        printf("Enter script number: ");
        scanf("%d", &choice);

        if (choice > 0 && choice < count) {
            d = opendir("./scripts");
            int i = 1;
            while ((dir = readdir(d)) != NULL) {
                if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
                    if (i == choice) {
                        // I'm gonna assume scripts are executable, if they're not... why? I mean they're scripts bro, chmod them I'm not writing c for that
                        char command[100];
                        snprintf(command, sizeof(command), "./scripts/%s", dir->d_name);
                        execlp("/bin/bash", "bash", command, (char *)NULL);
                        break;
                    }
                    i++;
                }
            }
        closedir(d);
    } else {
            printf("Invalid choice.\n");
    }
    } else {
        printf("Error opening scripts directory.\n");
}

    return 0;
}
