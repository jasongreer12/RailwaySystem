#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_MAX 256
#define ITEM_COUNT_MAX 64
#define ITEM_CHAR_MAX 64

// Call this function with the mode "train" or "intersection"
void parseFile(const char *mode) {
    char filepath[64];

    // Loads file and path as string to filepath based on mode
    if (strcmp(mode, "train") == 0) {
        sprintf(filepath, "text_files/trains.txt");
    } else if (strcmp(mode, "intersection") == 0) {
        sprintf(filepath, "text_files/intersections.txt");
    } else {
        fprintf(stderr, "Invalid mode: %s\n", mode);
        return;
    }

    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    char line[LINE_MAX];

    while (fgets(line, sizeof(line), file)) {
        // Remove newline character at end (if any)
        line[strcspn(line, "\n")] = '\0';

        // Split at the colon
        char *key = strtok(line, ":");
        char *values = strtok(NULL, ":");

        if (key && values) {
            printf("Key: %s\n", key);

            // Split values by commas
            char *token = strtok(values, ",");
            while (token != NULL) {
                printf("  Value: %s\n", token);
                token = strtok(NULL, ",");
            }
        }
    }

    fclose(file);
}


// Testing
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <mode>\n", argv[0]);
        return EXIT_FAILURE;
    }

    parseFile(argv[1]);

    return EXIT_SUCCESS;
}