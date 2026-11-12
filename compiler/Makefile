# Build:  make        Run tests:  make test        Clean:  make clean

autodiffx: lexer.l parser.y ast.h ast.c semantic.c diff.c optimize.c main.c
	bison -d parser.y
	flex lexer.l
	gcc -Wall -o autodiffx parser.tab.c lex.yy.c ast.c semantic.c diff.c optimize.c main.c

test: autodiffx
	./autodiffx < tests/valid.txt
	./autodiffx < tests/errors.txt

clean:
	rm -f autodiffx autodiffx.exe parser.tab.c parser.tab.h lex.yy.c
