# AutoDiffX: A Mini-Compiler for Symbolic Differentiation and Expression Optimization

PCD Innovative Assignment. It is written in **Flex + Bison + C only**: no Python and no math library.

```
Input     : x^3 + 2*x^2 + 5*x
Optimized : 3*x^2 + 4*x + 5
```

## Files

| File | Phase | Job |
|---|---|---|
| `lexer.l` | 1. Lexical analysis | Turns characters into tokens, reports bad characters |
| `parser.y` | 2. Syntax analysis | Grammar + operator precedence, builds the AST |
| `ast.h`, `ast.c` | 3. AST | Tree node, create nodes, print tree, print expression |
| `semantic.c` | 4. Semantic analysis | Only `x` allowed, no `x` in exponent, no division by 0 |
| `diff.c` | 5. Differentiation | Sum, difference, product, quotient, power rules |
| `optimize.c` | 6. Optimization | Constant folding, identity elimination, zero simplification |
| `main.c` | 7. Output | Runs the phases and prints the results |

## Build and run

Linux / WSL:
```bash
sudo apt install flex bison gcc make
make
./autodiffx < tests/valid.txt      # run the test file
./autodiffx                        # type expressions, Ctrl+D to finish
```

Windows (MinGW + WinFlexBison): run `build.bat`, then `autodiffx.exe < tests\valid.txt`

Manual build:
```bash
bison -d parser.y
flex lexer.l
gcc -Wall -o autodiffx parser.tab.c lex.yy.c ast.c semantic.c diff.c optimize.c main.c
```

## How each phase works

**1. Lexical analysis (`lexer.l`)**: the scanner recognizes these tokens:

| Pattern | Token |
|---|---|
| `3`, `2.5` | `NUMBER` |
| `x`, `y`, … | `ID` |
| `+ - * / ^ ( )` | `PLUS MINUS MUL DIV POW LPAREN RPAREN` |
| Any other character | lexical error |

**2. Syntax analysis (`parser.y`)**: this is the grammar:
```
expr : expr + expr | expr - expr | expr * expr | expr / expr | expr ^ expr
     | - expr | ( expr ) | NUMBER | ID
```
Precedence goes from lowest to highest:
- `+ -` (left associative)
- `* /` (left associative)
- unary `-`
- `^` (right associative, so `2^3^2 = 2^(3^2)`)

**3. AST (`ast.c`)**: each grammar rule creates a node. For `x^3 + 2*x^2 + 5*x`:
```
+
    +
        ^
            VAR x
            NUM 3
        *
            NUM 2
            ^
                VAR x
                NUM 2
    *
        NUM 5
        VAR x
```

**4. Semantic analysis (`semantic.c`)**: the program reports an error for:
- any variable other than `x` (`y^2`)
- `x` in an exponent (`x^x`)
- division by zero (`x / 0`)

**5. Differentiation (`diff.c`)**:

| Rule | Formula |
|---|---|
| Constant | d(c) = 0 |
| Variable | d(x) = 1 |
| Sum / Difference | d(u ± v) = u' ± v' |
| Product | d(u·v) = u'·v + u·v' |
| Quotient | d(u/v) = (u'·v − u·v') / v² |
| Power | d(uⁿ) = n · uⁿ⁻¹ · u' |

**6. Optimization (`optimize.c`)**: the children are simplified first, then these rules are applied:

| Group | Examples |
|---|---|
| Constant folding | `3 - 1 → 2`, `2^3 → 8`, `2*(2*x) → 4*x` |
| Identity elimination | `e + 0`, `e - 0`, `e * 1`, `e / 1`, `e ^ 1 → e` |
| Zero simplification | `e * 0 → 0`, `0 / e → 0`, `0 - e → -e`, `e ^ 0 → 1` |

Example of the derivative before and after optimization:
```
Derivative : 3*x^(3 - 1)*1 + 0*x^2 + 2*2*x^(2 - 1)*1 + 0*x + 5*1
Optimized  : 3*x^2 + 4*x + 5
```

## Results

| Input | Optimized derivative |
|---|---|
| `x^3 + 2*x^2 + 5*x` | `3*x^2 + 4*x + 5` |
| `-x^2 - 3*x + 10` | `-2*x - 3` |
| `x^2 * (x + 1)` | `2*x*(x + 1) + x^2` |
| `1 / x` | `-1/x^2` |
| `(2*x + 1)^3` | `6*(2*x + 1)^2` |
| `x^-2` | `-2*x^(-3)` |

## Error handling

| Input | Message |
|---|---|
| `x @ 2` | Lexical error: unexpected character '@' |
| `x^3 + * 2` | Syntax error: syntax error near '*' |
| `(x + 1` | Syntax error: syntax error at end of line |
| `y^2 + x` | Semantic error: unknown variable 'y' (only x is allowed) |
| `x^x` | Semantic error: x in an exponent is not supported |
| `x / 0` | Semantic error: division by zero |

## Limitations

- **No expansion or like-term collection.** The optimizer does not expand brackets or combine like terms. So `(x+1)/(x-1)` gives `(x - 1 - (x + 1))/(x - 1)^2`. That is correct, but it is not reduced to `-2/(x - 1)^2`.
- **Division by zero is only caught for a literal `0`.** So `x/(2-2)` is not caught.
- **Memory is not freed.** This is fine for a small program that runs and exits, and it keeps the code short.
