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
        Kind kind;
        const char* symbol;
        union Term* id;
    } var;
    struct {
        Kind kind;
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

Term* term_var(const char* symbol, Term* id);
Term* term_lam(const char* symbol, Term* body);
Term* term_app(Term* left, Term* right);
void bind_term(Term* term, Env* env);
void print_term(Term* term);

#endif
