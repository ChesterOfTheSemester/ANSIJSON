#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* readFile(const char* filename)
{
    FILE    *file;
    int     c;
    long    char_count = 0;
    char    *buffer = NULL,
            *file_content;

    // Open the file
    file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    // Initialize a dynamic buffer with an initial size
    size_t buffer_capacity = 1024;
    buffer = (char*)malloc(buffer_capacity);
    if (!buffer) {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }

    while ((c = fgetc(file)) != EOF) {
        if (char_count >= buffer_capacity) {
            // Increase buffer size as needed
            buffer_capacity += 1024;
            char* new_buffer = (char*)realloc(buffer, buffer_capacity);
            if (!new_buffer) {
                perror("Error reallocating memory");
                free(buffer);
                fclose(file);
                return NULL;
            }
            buffer = new_buffer;
        }
        buffer[char_count++] = (char)c;
    }

    fclose(file);

    // Allocate memory for the final content and copy data
    file_content = (char*)malloc(char_count + 1);
    if (!file_content) {
        perror("Error allocating memory");
        free(buffer);
        return NULL;
    }
    memcpy(file_content, buffer, char_count);
    file_content[char_count] = '\0';

    free(buffer);

    return file_content;
}
