# AutoDiffX v2 — Symbolic Differentiation Compiler

> A research-grade mini-compiler for **symbolic differentiation** of mathematical expressions, built entirely in **Flex + Bison + C** — no external math libraries or CAS systems.

```
Input  :  sin(x^2) + ln(x) * x^3
∂/∂x   :  cos(x^2)*2*x + (1/x)*x^3 + ln(x)*3*x^2
Optimized:  2*x*cos(x^2) + x^2 + 3*x^2*ln(x)
```

---

## Features

| Feature | v1 | v2 |
|---|:---:|:---:|
| Polynomial differentiation | ✓ | ✓ |
| Multi-variable (x, y, z, …) | — | ✓ |
| Partial derivatives (∂f/∂y) | — | ✓ |
| Transcendental functions (sin, cos, tan, ln, exp) | — | ✓ |
| Chain rule | — | ✓ |
| Like-term collection (3x² + 2x² → 5x²) | — | ✓ |
| Constant sub-expression evaluation | partial | ✓ |
| Division-by-zero (symbolic, e.g. x/(2-2)) | — | ✓ |
| Memory management (free_node) | — | ✓ |
| `-d <var>` CLI flag | — | ✓ |

---

## Compiler Architecture

```
Source text
    │
    ▼
┌─────────────────────────────────────────────────────────────────────┐
│  Phase 1 · Lexical Analysis  (lexer.l + Flex)                       │
│  Tokens: NUMBER  ID  FUNC  + - * / ^ ( )                            │
│  FUNC recognized: sin cos tan ln exp                                 │
└───────────────────────────────┬─────────────────────────────────────┘
                                │ token stream
                                ▼
┌─────────────────────────────────────────────────────────────────────┐
│  Phase 2 · Syntax Analysis  (parser.y + Bison)                      │
│  Grammar: LALR(1), handles operator precedence and associativity     │
│  Builds: Abstract Syntax Tree (AST)                                  │
└───────────────────────────────┬─────────────────────────────────────┘
                                │ AST
                                ▼
┌─────────────────────────────────────────────────────────────────────┐
│  Phase 3 · AST  (ast.c / ast.h)                                     │
│  Nodes: NUM VAR ADD SUB MUL DIV POW NEG FUNC                        │
│  Helpers: copy_node, eval_const, has_var, is_const_wrt              │
└───────────────────────────────┬─────────────────────────────────────┘
                                │ AST
                                ▼
┌─────────────────────────────────────────────────────────────────────┐
│  Phase 4 · Semantic Analysis  (semantic.c)                          │
│  Checks: single-letter vars · division-by-zero · ln domain          │
│          variable-in-exponent restriction                            │
└───────────────────────────────┬─────────────────────────────────────┘
                                │ validated AST
                                ▼
┌─────────────────────────────────────────────────────────────────────┐
│  Phase 5 · Differentiation  (diff.c)                                │
│  Rules: sum · difference · product · quotient · power               │
│         sin · cos · tan · ln · exp  (all with chain rule)           │
│  Partial diff: d(y)/dx = 0,  d(x)/dx = 1                           │
└───────────────────────────────┬─────────────────────────────────────┘
                                │ derivative AST
                                ▼
┌─────────────────────────────────────────────────────────────────────┐
│  Phase 6 · Optimization  (optimize.c)                               │
│  - Constant folding  (arithmetic + trig/log/exp with numeric args)  │
│  - Identity elimination  (×1, +0, ^1, ^0 …)                        │
│  - Like-term collection  (3x² + 2x² → 5x²)                         │
│  - Multi-pass fixed-point simplification                             │
└───────────────────────────────┬─────────────────────────────────────┘
                                │
                                ▼
                     Printed result + freed memory
```

---

## Grammar (BNF)

```bnf
program  ::= line*
line     ::= expr '\n' | '\n'

expr     ::= expr '+' expr
           | expr '-' expr
           | expr '*' expr
           | expr '/' expr
           | expr '^' expr
           | '-' expr
           | '(' expr ')'
           | FUNC '(' expr ')'
           | NUMBER
           | ID

FUNC     ::= 'sin' | 'cos' | 'tan' | 'ln' | 'exp'
NUMBER   ::= [0-9]+ ('.' [0-9]+)?
ID       ::= [a-zA-Z]   (single letter; multi-letter rejected by semantic analysis)
```

Operator precedence (Bison `%left` / `%right` declarations):

| Level | Operators | Associativity |
|---|---|---|
| 1 (lowest) | `+`  `-` | left |
| 2 | `*`  `/` | left |
| 3 | unary `-` | right |
| 4 (highest) | `^` | right |

---

## Differentiation Rules

| Expression | Rule | Derivative |
|---|---|---|
| c (constant) | — | 0 |
| x (target var) | — | 1 |
| y (other var) | — | 0 |
| u + v | Sum rule | u′ + v′ |
| u − v | Difference rule | u′ − v′ |
| u · v | Product rule | u′v + uv′ |
| u / v | Quotient rule | (u′v − uv′) / v² |
| u^n (n const) | Power rule | n · u^(n−1) · u′ |
| sin(u) | Chain rule | cos(u) · u′ |
| cos(u) | Chain rule | −sin(u) · u′ |
| tan(u) | Chain rule | u′ / cos²(u) |
| ln(u) | Chain rule | u′ / u |
| exp(u) | Chain rule | exp(u) · u′ |

---

## Build and Run

### Linux / macOS / WSL

```bash
sudo apt install flex bison gcc make   # Debian/Ubuntu
make
./autodiffx                            # interactive mode
./autodiffx < tests/valid.txt          # batch from file
./autodiffx -d y < tests/partial.txt  # partial w.r.t. y
```

### Windows (MinGW + WinFlexBison)

```bat
build.bat
autodiffx.exe < tests\valid.txt
autodiffx.exe -d y < tests\partial.txt
```

### Manual build

```bash
bison  -d parser.y
flex   lexer.l
gcc -Wall -O2 -o autodiffx parser.tab.c lex.yy.c ast.c semantic.c diff.c optimize.c main.c -lm
```

---

## Example Output

```
Input      : sin(x^2) + x * ln(x)
Tokens     : FUNC(sin) LPAREN ID(x) POW NUMBER(2) RPAREN PLUS ID(x) MUL FUNC(ln) LPAREN ID(x) RPAREN
AST        :
    +
        FUNC sin
            ^
                VAR x
                NUM 2
        *
            VAR x
            FUNC ln
                VAR x
Semantic   : OK
Expression : sin(x^2) + x*ln(x)
Simplified : sin(x^2) + x*ln(x)
d/dx       : cos(x^2)*2*x*1 + 1*ln(x) + x*(1/x)*1
Optimized  : 2*x*cos(x^2) + ln(x) + 1
```

---

## Results Table

| Input | d/dx (Optimized) |
|---|---|
| `x^3 + 2*x^2 + 5*x` | `3*x^2 + 4*x + 5` |
| `-x^2 - 3*x + 10` | `-2*x - 3` |
| `1 / x` | `-1/x^2` |
| `sin(x)` | `cos(x)` |
| `cos(2*x + 1)` | `-2*sin(2*x + 1)` |
| `exp(x^2)` | `2*x*exp(x^2)` |
| `ln(x)` | `1/x` |
| `x^2 + y^2` (wrt x) | `2*x` |
| `x^2 + y^2` (wrt y) | `2*y` |
| `3*x^2 + 2*x^2` | `10*x` *(like terms combined)* |

---

## Error Handling

| Input | Phase | Message |
|---|---|---|
| `x @ 2` | Lexical | `Lexical error: unexpected character '@'` |
| `x^3 + * 2` | Syntax | `Syntax error: syntax error near '*'` |
| `y2 + x` | Semantic | `Semantic error: 'y2' is not a valid variable` |
| `x / 0` | Semantic | `Semantic error: division by zero` |
| `x / (2 - 2)` | Semantic | `Semantic error: division by zero (denominator evaluates to 0)` |
| `ln(-1)` | Semantic | `Semantic error: ln(-1) is undefined` |

---

## Complexity Analysis

| Phase | Time | Space |
|---|---|---|
| Lexing | O(n) | O(n) tokens |
| Parsing (LALR) | O(n) | O(n) stack depth |
| Semantic check | O(n) | O(h) call stack |
| Differentiation | O(n) → O(n²) worst | O(n²) derivative tree |
| Simplification | O(n · k) passes | O(n) |

> n = expression length, h = AST height, k = number of simplification passes (bounded at 8)

---

## Known Limitations

1. **No logarithmic differentiation** — `x^x` is rejected (exponent contains the variable).
2. **No like-term expansion across parentheses** — `(x+1)^2` stays as-is; it is not expanded to `x^2+2x+1`.
3. **No simplification of trig identities beyond constant folding** — `sin²(x)+cos²(x)` is not reduced to `1` symbolically.
4. **Division-by-zero is detected only for constant denominators** — `1/(x-x)` is not caught.

---

## Project Structure

```
compiler/
├── lexer.l        Phase 1 — Flex scanner
├── parser.y       Phase 2 — Bison LALR(1) parser
├── ast.h          Shared type definitions and function prototypes
├── ast.c          Phase 3 — AST construction, printing, memory
├── semantic.c     Phase 4 — Semantic analysis
├── diff.c         Phase 5 — Symbolic differentiation
├── optimize.c     Phase 6 — Constant folding + like-term collection
├── main.c         Driver — CLI parsing, pipeline orchestration
├── Makefile
└── tests/
    ├── valid.txt       Polynomial expressions
    ├── trig.txt        Transcendental functions
    ├── multi.txt       Multi-variable expressions
    ├── partial.txt     Partial differentiation (use -d y)
    └── errors.txt      Expected error cases
```

---

## License

MIT © 2024 — Aadit Pithava
