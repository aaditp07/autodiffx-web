CC      = gcc
CFLAGS  = -Wall -Wextra -O2
LDFLAGS = -lm
TARGET  = autodiffx

SRCS = parser.tab.c lex.yy.c ast.c semantic.c diff.c optimize.c main.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): parser.tab.c lex.yy.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

parser.tab.c parser.tab.h: parser.y
	bison -d parser.y

lex.yy.c: lexer.l parser.tab.h
	flex lexer.l

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) parser.tab.c parser.tab.h lex.yy.c *.exe

test: $(TARGET)
	./$(TARGET) < tests/valid.txt
	./$(TARGET) < tests/multi.txt
	./$(TARGET) < tests/trig.txt
	./$(TARGET) -d y < tests/partial.txt

.PHONY: all clean test
