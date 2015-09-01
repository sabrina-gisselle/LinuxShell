all: lex.yy.c y.tab.c y.tab.h
		cc lex.yy.c y.tab.c helper.c shell.c -o shell

y.tab.h: y.tab.c

y.tab.c: parser.y
		yacc -dv parser.y

lex.yy.c: scanner.l
		lex scanner.l

clean:
	rm -rf lex.yy.c y.tab.c y.tab.h shell
