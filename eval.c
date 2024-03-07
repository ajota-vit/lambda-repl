#include "eval.h"

#define id(T) ((T)->var.id)
#define symbol(T) ((T)->lam.symbol)
#define body(T) ((T)->lam.body)
#define left(T) ((T)->app.left)
#define right(T) ((T)->app.right)
