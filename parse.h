#ifndef PARSE_H_
#define PARSE_H_

#include "eval.h"

typedef struct {
    const char* current;
} Lexer;

Lexer create_lexer(const char* source);
void parse_definition_list(Lexer* lexer, Env** env);
Term* parse_line(Lexer* lexer, Env** env, int* strong, int* strict);

#endif
