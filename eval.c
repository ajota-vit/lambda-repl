#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "eval.h"

#define id(T) ((T)->var.id)
#define symbol(T) ((T)->lam.symbol)
#define body(T) ((T)->lam.body)
#define left(T) ((T)->app.left)
#define right(T) ((T)->app.right)

char* copy_string(const char* string) {
    size_t len = strlen(string);
    char* copy = malloc(len+1);
    memcpy(copy, string, len+1);
    return copy;
}

Term* term_var(char* symbol, Term* id) {
    Term* term = malloc(sizeof(Term));
    *term = (Term){.var = {VAR, symbol, id}};
    return term;
}

Term* term_lam(char* symbol, Term* body) {
    Term* term = malloc(sizeof(Term));
    *term = (Term){.lam = {LAM, symbol, body}};
    return term;
}

Term* term_app(Term* left, Term* right) {
    Term* term = malloc(sizeof(Term));
    *term = (Term){.app = {APP, left, right}};
    return term;
}

void free_term(Term* term) {
    if (term->kind == VAR) {
        free(symbol(term));
    } else if (term->kind == LAM) {
        free(symbol(term));
        free_term(body(term));
    } else if (term->kind == APP) {
        free_term(left(term));
        free_term(right(term));
    }
    free(term);
}

Term* copy_term(Term* term, Map* map) {
    if (term->kind == VAR) {
        while (map != NULL) {
            if (map->key == id(term)) return term_var(copy_string(symbol(term)), map->term);
            map = map->next;
        }

        return term_var(copy_string(symbol(term)), id(term));
    } else if (term->kind == LAM) {
        Term* lam = term_lam(copy_string(symbol(term)), NULL);
        Map new = {map, term, lam};
        body(lam) = copy_term(body(term), &new);
        return lam;
    } else if (term->kind == APP) {
        return term_app(
            copy_term(left(term), map),
            copy_term(right(term), map)
        );
    }

    return NULL;
}

void print_term_impl(Term* term, Env* env, int lam_braces, int app_braces) {
	if (term->kind == VAR) {
        size_t count = 0;
        while (env != NULL) {
            if (strcmp(env->symbol, symbol(term)) == 0) count += 1;
            if (env->term == id(term)) count = 0;
            env = env->next;
        }

        printf("%s", symbol(term));
        for (size_t i = 0; i < count; i++) printf("'");
	} else if (term->kind == LAM) {
		Env new = {env, symbol(term), term};

        size_t count = 0;
        while (env != NULL) {
            if (strcmp(env->symbol, symbol(term)) == 0) count += 1;
            env = env->next;
        }

		if (lam_braces) printf("(");
		printf("λ");
        printf("%s", symbol(term));
        for (size_t i = 0; i < count; i++) printf("'");
        printf(". ");
		print_term_impl(body(term), &new, 0, 0);
		if (lam_braces) printf(")");
	} else if (term->kind == APP) {
		if (app_braces) printf("(");
		print_term_impl(left(term), env, 1, 0);
		printf(" ");
		print_term_impl(right(term), env, lam_braces, 1);
		if (app_braces) printf(")");
	}
}

void print_term(Term* term) {
    print_term_impl(term, NULL, 0, 0);
    printf("\n");
}

void bind_term(Term* term, Env* env) {
    if (term->kind == VAR) {
        while (env != NULL && strcmp(env->symbol, symbol(term)) != 0) {
            env = env->next;
        }

        if (env != NULL) {
            id(term) = env->term;
        } else {
            id(term) = NULL;
        }
    } else if (term->kind == LAM) {
        Env new = {env, symbol(term), term};
        bind_term(body(term), &new);
    } else if (term->kind == APP) {
        bind_term(left(term), env);
        bind_term(right(term), env);
    }
}

Term* eval_term(Term* term, Map* map, Env* env, int eval, int strong, int strict, size_t* count) {
    for (;;) {

        if (term->kind == VAR) {
            Map* current = map;
            while (current != NULL) {
                if (current->key == id(term)) {
                    free_term(term);
                    term = copy_term(current->term, NULL);
                    break;
                }

                current = current->next;
            }
        }

        while (term->kind == VAR && id(term) == NULL) {
            Env* current = env;
            while (current != NULL && strcmp(current->symbol, symbol(term)) != 0) {
                current = current->next;
            }
            if (current == NULL) {
                break;
            }
            free_term(term);
            term = copy_term(current->term, NULL);
        }

        if (term->kind == LAM) body(term) = eval_term(body(term), map, env, eval && strong, strong, strict, count);
        if (term->kind != APP) return term;

        left(term) = eval_term(left(term), map, env, eval, strong, strict, count);
        right(term) = eval_term(right(term), map, env, eval && strict, strong, strict, count);
        if (!eval || left(term)->kind != LAM) return term;
        *count += 1;

        Map new = {map, left(term), right(term)};
        Term* temp = eval_term(body(left(term)), &new, env, 0, strong, strict, count);

        free_term(right(term));
        free(symbol(left(term)));
        free(left(term));
        free(term);

        term = temp;
    }

    return term;
}
