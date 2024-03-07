CFLAGS = -std=c99 -pedantic -Wall -Wextra
LDFLAGS = -lreadline

all:
	$(CC) $(CFLAGS) main.c -o lambda-repl $(LDFLAGS)

clean:
	rm -f lambda-repl
