# Lambda Repl

```
$ ./lambda-repl
[λ]: Two = \f.\x.f (f x)
[λ]: Two Two
|> reductions: 5
|> time:       0.000069s 
|> term:       λx. λx'. x (x (x (x x')))
```

## Introduction

`lambda-repl` accepts a list of files with lambda definitions.\
the default reduction strategy is call-by-value and the default normalisation is normal form. this can be change with the following commands:\
- `[λ]: !strong` => 'normal from' normalisation
- `[λ]: !weak  ` => 'weak head normal form' normalisation
- `[λ]: !strict` => 'call-by-value' evaluation
- `[λ]: !lazy  ` => 'call-by-name' evaluation

## Syntax

- `\[var]. [λ-term] ` => abstraction
- `[λ-term] [λ-term]` => application
- `[var] = [λ-term] ` => definition

## Compilation

requires gnu readline

```
make
```
