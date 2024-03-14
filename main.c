#include <stdlib.h>
#include <stdio.h>
#include <time.h>

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

    char* buffer = malloc(1 << 16);
    size_t capacity = 1 << 16;
    size_t length = 0;
    do {
        capacity += 1 << 16;
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

        Lexer lexer = create_lexer(line, "<stdin>");
        Term* term = parse_line(&lexer, &env, &strong, &strict);
        if (term != NULL) {
            size_t count = 0;
            clock_t start = clock();
            term = eval_term(term, NULL, env, 1, strong, strict, &count);
            clock_t end = clock();

            printf("|> reductions: %zu\n", count);
            printf("|> time:       %lfs\n", (double)(end - start) / (double)CLOCKS_PER_SEC);
            printf("|> term:       ");
            print_term(term);
            free_term(term);
        }

        free(line);
    }

    clear_history();
}

void test_repl(Env* env, int strong, int strict) {
    for (;;) {
        char line[256] = {0};
        printf("[λ]: ");
        fgets(line, 256, stdin);
        if (*line == '\0') break;

        Lexer lexer = create_lexer(line, "<stdin>");
        Term* term = parse_line(&lexer, &env, &strong, &strict);
        if (term != NULL) {
            size_t count = 0;
            clock_t start = clock();
            term = eval_term(term, NULL, env, 1, strong, strict, &count);
            clock_t end = clock();

            printf("|> reductions: %zu\n", count);
            printf("|> time:       %lfs\n", (double)(end - start) / (double)CLOCKS_PER_SEC);
            printf("|> term:       ");
            print_term(term);
            free_term(term);
        }
    }
}

int main(int argc, char* argv[]) {
    Env* env = NULL;

    for (int i = 1; i < argc; i++) {
        char* source = slurp(argv[i]);
        Lexer lexer = create_lexer(source, argv[i]);
        parse_definition_list(&lexer, &env);
        free(source);
    }

    repl(env, 1, 1);

    while (env != NULL) {
        Env* next = env->next;
        free_term(env->term);
        free(env->symbol);
        free(env);

        env = next;
    }

    return 0;
}
