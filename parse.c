#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

#include "eval.h"
#include "parse.h"

Lexer create_lexer(const char* source, const char* name) {
    Lexer lexer;
    lexer.name = name;
    lexer.source = source;
    lexer.current = source;
    return lexer;
}

void parser_error(Lexer* lexer, const char* message) {
    size_t line = 1;
    size_t column = 1;

    for (const char* current = lexer->source; current < lexer->current; current++) {
        column += 1;

        if (*current == '\n') {
            line += 1;
            column = 1;
        }
    }

    fprintf(stderr, "[%s:%zu:%zu] %s\n", lexer->name, line, column, message);
}

void parse_whitespace(Lexer* lexer) {
    while (isspace(*lexer->current) && *lexer->current != '\n' && *lexer->current != '\f' && *lexer->current != '\0') {
        lexer->current += 1;
    }
}

int parse_newline(Lexer* lexer) {
    parse_whitespace(lexer);
    return *lexer->current == '\n' || *lexer->current == '\f' || *lexer->current == '\0';
}

int parse_eof(Lexer* lexer) {
    while (isspace(*lexer->current)) {
        lexer->current += 1;
    }
    return *lexer->current == '\0';
}

int next_symbol(Lexer* lexer, const char* symbol) {
    parse_whitespace(lexer);
    size_t i = 0;
    for (i = 0; symbol[i] != '\0'; i++) {
        if (lexer->current[i] != symbol[i]) return 0; 
    }
    lexer->current += i;
    return 1;
}

int peek_symbol(Lexer* lexer, const char* symbol) {
    parse_whitespace(lexer);
    size_t i = 0;
    for (i = 0; symbol[i] != '\0'; i++) {
        if (lexer->current[i] != symbol[i]) return 0; 
    }
    return 1;
}

char* parse_symbol(Lexer* lexer) {
    parse_whitespace(lexer);
    if (!isalpha(*lexer->current) && *lexer->current != '_') return NULL;

    size_t len = 0;
    while (isalnum(lexer->current[len]) || lexer->current[len] == '_') {
        len += 1;
    }

    char* symbol = malloc(len+1);
    for (size_t i = 0; i < len; i++)
        symbol[i] = lexer->current[i];
    symbol[len] = '\0';
    lexer->current += len;

    return symbol;
}

Term* parse_term(Lexer* lexer);

Term* parse_lam(Lexer* lexer) {
    if (next_symbol(lexer, "\\") == 0) {
        parser_error(lexer, "missing '\\'");
        return NULL;
    }

    char* parm = parse_symbol(lexer);
    if (parm == NULL) {
        parser_error(lexer, "missing parameter");
        return NULL;
    }

    if (next_symbol(lexer, ".") == 0) {
        free(parm);

        parser_error(lexer, "missing '.'");
        return NULL;
    }

    Term* body = parse_term(lexer);
    if (body == NULL) {
        free(parm);

        parser_error(lexer, "missing lambda body");
        return NULL;
    }

    return term_lam(parm, body);
}

Term* parse_term(Lexer* lexer) {
    Term* head = NULL;

    for (;;) {
        Term* node = NULL;

        if (peek_symbol(lexer, "(")) {
            if (next_symbol(lexer, "(") == 0) {
                if (head != NULL) free_term(head);

                parser_error(lexer, "missing '('");
                return NULL;
            }
            node = parse_term(lexer);
            if (node == NULL) {
                if (head != NULL) free_term(head);

                parser_error(lexer, "missing contents");
                return NULL;
            }
            if (next_symbol(lexer, ")") == 0) {
                if (head != NULL) free_term(head);
                if (node != NULL) free_term(head);

                parser_error(lexer, "missing ')'");
                return NULL;
            }
        } else if (peek_symbol(lexer, "\\")) {
            node = parse_lam(lexer);
            if (node == NULL) {
                if (head != NULL) free_term(head);
                return NULL;
            }
        } else {
            char* symbol = parse_symbol(lexer);
            if (symbol != NULL) {
                node = term_var(symbol, NULL);
            }
        }

        if (node == NULL)
            return head;

        if (head == NULL) {
            head = node;
        } else {
            head = term_app(head, node);
        }
    }
}

void parse_definition_list(Lexer* lexer, Env** env) {
    while (!parse_eof(lexer)) {
        char* symbol = parse_symbol(lexer);
        if (symbol == NULL) {
            parser_error(lexer, "definition missing variable");
            return;
        }
        if (next_symbol(lexer, "=") == 0) {
            free(symbol);

            parser_error(lexer, "definition missing '='");
            return;
        }
        Term* term = parse_term(lexer);
        if (term == NULL) {
            free(symbol);

            parser_error(lexer, "definition missing '='");
            return;
        }
        if (parse_newline(lexer) == 0) {
            free(symbol);
            free_term(term);

            parser_error(lexer, "invalid definition formatting");
            return;
        }
        bind_term(term, NULL);

        Env* new = malloc(sizeof(Env));
        new->next = *env;
        new->symbol = symbol;
        new->term = term;
        *env = new;
    }
}

Term* parse_line(Lexer* lexer, Env** env, int* strong, int* strict) {
    while (peek_symbol(lexer, "!")) {
        next_symbol(lexer, "!");
        const char* option = parse_symbol(lexer);
        if (option == NULL) {
            parser_error(lexer, "missing option");
            return NULL;
        } else if (strcmp(option, "strong") == 0) {
            *strong = 1;
        } else if (strcmp(option, "weak") == 0) {
            *strong = 0;
        } else if (strcmp(option, "strict") == 0) {
            *strict = 1;
        } else if (strcmp(option, "lazy") == 0) {
            *strict = 0;
        } else {
            parser_error(lexer, "unkown option");
            return NULL;
        }
    }

    const char* backup = lexer->current; // this needs to be improved
    char* symbol = parse_symbol(lexer);
    if (symbol == NULL) {
        Term* term = parse_term(lexer);
        if (term == NULL) return NULL;
        bind_term(term, NULL);
        if (parse_newline(lexer) == 0) {
            free_term(term);

            parser_error(lexer, "invalid line formatting");
            return NULL;
        }
        return term;
    }

    if (!peek_symbol(lexer, "=")) {
        lexer->current = backup;
        Term* term = parse_term(lexer);
        if (term == NULL) return NULL;
        bind_term(term, NULL);
        if (parse_newline(lexer) == 0) {
            free_term(term);

            parser_error(lexer, "invalid line formatting");
            return NULL;
        }
        return term;
    }

    if (next_symbol(lexer, "=") == 0) {
        free(symbol);

        parser_error(lexer, "line missing '='");
        return NULL;
    }
    Term* term = parse_term(lexer);
    if (term == NULL) {
        free(symbol);

        parser_error(lexer, "line missing term");
        return NULL;
    }
    bind_term(term, NULL);
    if (parse_newline(lexer) == 0) {
        free(symbol);
        free_term(term);

        parser_error(lexer, "invalid line formatting");
        return NULL;
    }

    Env* new = malloc(sizeof(Env));
    new->next = *env;
    new->symbol = symbol;
    new->term = term;
    *env = new;
    
    return NULL;
}
