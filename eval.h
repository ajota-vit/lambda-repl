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
        char* symbol;
        union Term* id;
    } var;
    struct {
        Kind kind;
        char* symbol;
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
    char* symbol;
    Term* term;
} Env;

Term* term_var(char* symbol, Term* id);
Term* term_lam(char* symbol, Term* body);
Term* term_app(Term* left, Term* right);
void free_term(Term* term);
void print_term(Term* term);
void bind_term(Term* term, Env* env);
Term* eval_term(Term* term, Map* map, Env* env, int eval, int strong, int strict, size_t* count);

#endif
