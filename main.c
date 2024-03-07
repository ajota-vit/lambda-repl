#include <stdlib.h>
#include <stdio.h>

#include "eval.h"
#include "parse.h"

#include <readline/readline.h>
#include <readline/history.h>

const char* slurp(const char* path) {
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "TODO: 1");
        exit(1);
    }

    char* buffer = malloc(4096);
    size_t capacity = 4096;
    size_t length = 0;
    do {
        capacity *= 2;
        buffer = realloc(buffer, capacity);
        length += fread(&buffer[length], 1, capacity - length, file);
    } while (length == capacity);
    fclose(file);

    buffer = realloc(buffer, length+1);
    buffer[length] = '\0';

    return buffer;
}

void repl(void) {
    Env* env = NULL;
    int strong = 1;
    int strict = 1;

    using_history();
    stifle_history(1024);

    for (;;) {
        char* line = readline("[λ]: ");
        if (line == NULL) break;
        add_history(line);
    }

    clear_history();
}

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        printf(slurp(argv[i]));
    }

    repl();

    return 0;
}
