/**
 * AutodiffX Backend API
 *
 * POST /api/compile
 *   Body: { expression: "x^3 + 2*x^2 + 5*x" }
 *   Response: { tokens, ast, expression, simplified, derivative, optimized, errors }
 */

const express = require("express");
const cors = require("cors");
const { spawn } = require("child_process");
const path = require("path");

const app = express();
const PORT = process.env.PORT || 3001;
const BIN_PATH = path.join(__dirname, "bin", "autodiffx.exe");

app.use(cors());
app.use(express.json());

// Health check
app.get("/api/health", (_req, res) => {
  res.json({ status: "ok", bin: BIN_PATH });
});

// Main compile endpoint
app.post("/api/compile", (req, res) => {
  const { expression } = req.body;

  if (!expression || typeof expression !== "string") {
    return res.status(400).json({ error: "Missing or invalid 'expression' field." });
  }

  // Strip anything that could break shell injection — whitelist characters
  const safe = /^[x0-9\s\+\-\*\/\^\(\)\.\#]+$/;
  if (!safe.test(expression)) {
    return res.status(400).json({
      error: "Expression contains invalid characters. Only x, digits, +−*/^() are allowed.",
    });
  }

  const proc = spawn(BIN_PATH, [], { shell: false });

  let stdout = "";
  let stderr = "";

  proc.stdin.write(expression + "\n");
  proc.stdin.end();

  proc.stdout.on("data", (d) => (stdout += d.toString()));
  proc.stderr.on("data", (d) => (stderr += d.toString()));

  proc.on("close", (code) => {
    if (code !== 0 && stdout.length === 0) {
      return res.status(500).json({ error: "Compiler process failed.", details: stderr });
    }

    // Parse the structured output from autodiffx
    const result = parseOutput(stdout, expression);
    res.json(result);
  });

  proc.on("error", (err) => {
    res.status(500).json({ error: "Could not launch compiler.", details: err.message });
  });
});

/**
 * Parse the text output of autodiffx into a structured object.
 * The output format for a valid expression looks like:
 *   Tokens     : ...
 *   AST        :
 *       + ...
 *   Semantic   : OK
 *   Expression : ...
 *   Simplified : ...
 *   Derivative : ...
 *   Optimized  : ...
 */
function parseOutput(raw, input) {
  const lines = raw.split("\n");
  const result = {
    input,
    tokens: null,
    ast: [],
    semantic: null,
    expression: null,
    simplified: null,
    derivative: null,
    optimized: null,
    errors: [],
    rawOutput: raw,
  };

  let inAst = false;

  for (const line of lines) {
    const trimmed = line.trim();

    if (trimmed.startsWith("Tokens     :")) {
      result.tokens = trimmed.replace("Tokens     :", "").trim();
      inAst = false;
    } else if (trimmed.startsWith("AST        :")) {
      inAst = true;
    } else if (trimmed.startsWith("Semantic   :")) {
      result.semantic = trimmed.replace("Semantic   :", "").trim();
      inAst = false;
    } else if (trimmed.startsWith("Expression :")) {
      result.expression = trimmed.replace("Expression :", "").trim();
      inAst = false;
    } else if (trimmed.startsWith("Simplified :")) {
      result.simplified = trimmed.replace("Simplified :", "").trim();
    } else if (trimmed.startsWith("Derivative :")) {
      result.derivative = trimmed.replace("Derivative :", "").trim();
    } else if (trimmed.startsWith("Optimized  :")) {
      result.optimized = trimmed.replace("Optimized  :", "").trim();
    } else if (
      trimmed.startsWith("Lexical error") ||
      trimmed.startsWith("Syntax error") ||
      trimmed.startsWith("Semantic error") ||
      trimmed.startsWith("Compilation stopped")
    ) {
      result.errors.push(trimmed);
      inAst = false;
    } else if (inAst && trimmed.length > 0 && !trimmed.startsWith("===") && !trimmed.startsWith("---")) {
      result.ast.push(line); // preserve indentation for tree display
    }
  }

  return result;
}

app.listen(PORT, () => {
  console.log(`AutodiffX API running on http://localhost:${PORT}`);
});
