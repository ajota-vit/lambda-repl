#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

#include "eval.h"
#include "parse.h"

Lexer create_lexer(const char* source) {
    Lexer lexer;
    lexer.current = source;
    return lexer;
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
    if (!isalpha(*lexer->current)) return NULL;

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
    assert(next_symbol(lexer, "\\"));

    char* parm = parse_symbol(lexer);
    assert(parm != NULL);

    assert(next_symbol(lexer, "."));

    Term* body = parse_term(lexer);
    assert(body != NULL);

    return term_lam(parm, body);
}

Term* parse_term(Lexer* lexer) {
    Term* head = NULL;

    for (;;) {

        Term* node = NULL;

        if (peek_symbol(lexer, "(")) {
            assert(next_symbol(lexer, "("));
            node = parse_term(lexer);
            assert(next_symbol(lexer, ")"));
        } else if (peek_symbol(lexer, "\\")) {
            node = parse_lam(lexer);
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
        assert(next_symbol(lexer, "="));
        Term* term = parse_term(lexer);
        assert(term != NULL);
        bind_term(term, NULL);
        assert(parse_newline(lexer));

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
            assert(0 && "no option");
        } else if (strcmp(option, "strong") == 0) {
            *strong = 1;
        } else if (strcmp(option, "weak") == 0) {
            *strong = 0;
        } else if (strcmp(option, "strict") == 0) {
            *strict = 1;
        } else if (strcmp(option, "lazy") == 0) {
            *strict = 0;
        } else {
            assert(0 && "unkown option");
        }
    }

    char* symbol = parse_symbol(lexer);
    if (symbol == NULL) {
        Term* term = parse_term(lexer);
        if (term != NULL) bind_term(term, NULL);
        assert(parse_newline(lexer));
        return term;
    }

    if (!peek_symbol(lexer, "=")) {
        Term* term = parse_term(lexer);
        if (term != NULL) {
            term = term_app(term_var(symbol, NULL), term);
        } else {
            term = term_var(symbol, NULL);
        }
        bind_term(term, NULL);
        assert(parse_newline(lexer));
        return term;
    }

    assert(next_symbol(lexer, "="));
    Term* term = parse_term(lexer);
    assert(term != NULL);
    bind_term(term, NULL);
    assert(parse_newline(lexer));

    Env* new = malloc(sizeof(Env));
    new->next = *env;
    new->symbol = symbol;
    new->term = term;
    *env = new;
    
    return NULL;
}
