@echo off
REM Build AutoDiffX on Windows (MinGW gcc + WinFlexBison)
win_bison -d parser.y
win_flex --wincompat lexer.l
gcc -Wall -o autodiffx.exe parser.tab.c lex.yy.c ast.c semantic.c diff.c optimize.c main.c
echo Done. Run: autodiffx.exe ^< tests\valid.txt
