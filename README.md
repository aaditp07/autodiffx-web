# AutodiffX Web

A full-stack web application for **AutodiffX v2** — a research-grade mini-compiler for symbolic differentiation built with Flex + Bison + C.

**→ [Live Demo](#)** | **→ [Compiler Documentation](compiler/README.md)**

## What it does

Enter any mathematical expression involving variables (x, y, z), operators (+−\*/^), and transcendental functions (sin, cos, tan, ln, exp). The compiler:

1. **Lexes** the input into tokens
2. **Parses** it into an AST using an LALR(1) grammar
3. **Checks semantics** (variable validity, division-by-zero, ln domain)
4. **Differentiates** symbolically using the chain rule, product rule, quotient rule, power rule — plus sin, cos, tan, ln, exp
5. **Optimizes** via multi-pass constant folding, identity elimination, and like-term collection (3x² + 2x² → 5x²)
6. Supports **partial derivatives** (∂f/∂x, ∂f/∂y) via `-d <var>` CLI flag

## Example

```
Input      :  sin(x^2) + x * ln(x)
d/dx       :  cos(x^2)*2*x + ln(x) + 1
```

## Stack

| Layer | Tech |
|---|---|
| Compiler | Flex + Bison + C (pre-built binary) |
| Backend API | Node.js + Express |
| Frontend | React 18 + Vite |
| Deployment | Vercel (frontend) + Railway (backend) |

## Quick Start

**Prerequisites:** Node.js 18+, Windows (compiler binary is `.exe`)

```bash
# Backend
cd backend
npm install
npm start          # → http://localhost:3001

# Frontend (new terminal)
cd frontend
npm install
npm run dev        # → http://localhost:5173
```

## Compiler Source

All original Flex + Bison + C source files are in [`compiler/`](compiler/). See [compiler/README.md](compiler/README.md) for full documentation including:
- Grammar specification (BNF)
- Differentiation rules table
- Complexity analysis
- Architecture diagram

## License

MIT © 2024 — Aadit Pithava
