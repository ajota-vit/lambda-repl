CFLAGS = -std=c99 -pedantic -Wall -Wextra
LDFLAGS = -lreadline

all:
	$(CC) $(CFLAGS) main.c eval.c parse.c -o lambda-repl $(LDFLAGS)

clean:
	rm -f lambda-repl
