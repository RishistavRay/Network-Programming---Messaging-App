#include "helper.h"

char** split_input(char* line) {
    char** words = malloc(sizeof(char*) * MAX_LINE_LENGTH);
    char* token;
    int i = 0;

    token = strtok(line, " ");
    while (token != NULL) {
        words[i++] = token;
        token = strtok(NULL, " ");
    }

    words[i] = NULL; // Mark end of array with NULL pointer

    return words;
}
