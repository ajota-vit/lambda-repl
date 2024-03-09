#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "eval.h"

#define id(T) ((T)->var.id)
#define symbol(T) ((T)->lam.symbol)
#define body(T) ((T)->lam.body)
#define left(T) ((T)->app.left)
#define right(T) ((T)->app.right)

Term* term_var(const char* symbol, Term* id) {
    Term* term = malloc(sizeof(Term));
    *term = (Term){.var = {VAR, symbol, id}};
    return term;
}

Term* term_lam(const char* symbol, Term* body) {
    Term* term = malloc(sizeof(Term));
    *term = (Term){.lam = {LAM, symbol, body}};
    return term;
}

Term* term_app(Term* left, Term* right) {
    Term* term = malloc(sizeof(Term));
    *term = (Term){.app = {APP, left, right}};
    return term;
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
