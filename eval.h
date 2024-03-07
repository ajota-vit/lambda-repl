#ifndef EVAL_H_
#define EVAL_H_

typedef enum {
    VAR,
    LAM,
    APP,
} Kind;

typedef union Term {
    Kind kind;
    struct {
        const char* symbol;
        union Term* id;
    } var;
    struct {
        const char* symbol;
        union Term* body;
    } lam;
    struct {
        Kind kind;
        union Term* left;
        union Term* right;
    } app;
} Term;

typedef struct Map {
    struct Map* next;
    Term* key;
    Term* term;
} Map;

typedef struct Env {
    struct Env* next;
    const char* symbol;
    Term* term;
} Env;

#endif
