#include <stdlib.h>
#include <stdio.h>

#include "eval.h"
#include "parse.h"

#include <readline/readline.h>
#include <readline/history.h>

char* slurp(const char* path) {
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

void repl(Env* env, int strong, int strict) {
    using_history();
    stifle_history(1024);

    for (;;) {
        char* line = readline("[λ]: ");
        if (line == NULL) break;
        add_history(line);

        Lexer lexer = create_lexer(line);
        Term* term = parse_line(&lexer, &env, &strong, &strict);
        if (term != NULL) {
            print_term(term);
        }
    }

    clear_history();
}

int main(int argc, char* argv[]) {
    Env* env = NULL;

    for (int i = 1; i < argc; i++) {
        char* source = slurp(argv[i]);
        Lexer lexer = create_lexer(source);
        parse_definition_list(&lexer, &env);
        free(source);
    }

    repl(env, 1, 1);

    return 0;
}
