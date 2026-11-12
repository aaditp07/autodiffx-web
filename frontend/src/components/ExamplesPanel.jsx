import './ExamplesPanel.css'

const EXAMPLES = [
  { label: 'Cubic polynomial', expr: 'x^3 + 2*x^2 + 5*x' },
  { label: 'Quadratic', expr: '-x^2 - 3*x + 10' },
  { label: 'Product rule', expr: 'x^2 * (x + 1)' },
  { label: 'Quotient rule', expr: '1 / x' },
  { label: 'Chain rule (power)', expr: '(2*x + 1)^3' },
  { label: 'Negative exponent', expr: 'x^-2' },
  { label: 'Semantic error', expr: 'y^2 + x' },
  { label: 'Lexical error', expr: 'x @ 2' },
]

export default function ExamplesPanel({ onSelect }) {
  return (
    <div className="examples-card">
      <h2 className="examples-title">Examples</h2>
      <div className="examples-list">
        {EXAMPLES.map(({ label, expr }) => (
          <button key={expr} className="example-btn" onClick={() => onSelect(expr)}>
            <span className="example-label">{label}</span>
            <code className="example-expr">{expr}</code>
          </button>
        ))}
      </div>
    </div>
  )
}
