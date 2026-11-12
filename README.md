# AutodiffX Web

A full-stack web application for **AutodiffX** — a mini-compiler for symbolic differentiation built with Flex + Bison + C.

Enter a polynomial expression in `x`, and the compiler walks through every phase:
1. Lexical analysis → tokens
2. Syntax analysis → AST
3. Semantic checks
4. Differentiation
5. Optimization / simplification

## Stack

| Layer | Tech |
|---|---|
| Compiler | Flex + Bison + C (pre-built `autodiffx.exe`) |
| Backend | Node.js + Express |
| Frontend | React 18 + Vite |

## Quick Start

**Prerequisites:** Node.js 18+, Windows (compiler binary is `.exe`)

```bash
# 1. Install backend deps
cd backend
npm install
npm start          # → http://localhost:3001

# 2. Install frontend deps (new terminal)
cd frontend
npm install
npm run dev        # → http://localhost:5173
```

Open http://localhost:5173 in your browser.

## Examples

| Input | Optimized derivative |
|---|---|
| `x^3 + 2*x^2 + 5*x` | `3*x^2 + 4*x + 5` |
| `-x^2 - 3*x + 10` | `-2*x - 3` |
| `1 / x` | `-1/x^2` |
| `(2*x + 1)^3` | `6*(2*x + 1)^2` |

## Project Structure

```
autodiffx-web/
├── backend/          Node.js Express API
│   ├── server.js
│   └── bin/autodiffx.exe
├── frontend/         React (Vite) UI
│   └── src/
│       ├── App.jsx
│       └── components/
└── compiler/         Original C source (Flex + Bison)
```

## License

MIT
