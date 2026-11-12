import { useState } from 'react'
import './ExpressionInput.css'

export default function ExpressionInput({ onCompile, loading }) {
  const [value, setValue] = useState('')

  function handleSubmit(e) {
    e.preventDefault()
    const expr = value.trim()
    if (expr) onCompile(expr)
  }

  function handleKeyDown(e) {
    if (e.key === 'Enter' && !e.shiftKey) {
      e.preventDefault()
      handleSubmit(e)
    }
  }

  return (
    <div className="input-card">
      <h2 className="input-title">Expression</h2>
      <p className="input-hint">
        Enter a polynomial in <code>x</code>. Use <code>^</code> for power, <code>*</code> for multiply.
      </p>
      <form onSubmit={handleSubmit} className="input-form">
        <div className="input-wrap">
          <span className="input-prefix">f(x) =</span>
          <input
            className="input-field"
            type="text"
            value={value}
            onChange={(e) => setValue(e.target.value)}
            onKeyDown={handleKeyDown}
            placeholder="x^3 + 2*x^2 + 5*x"
            autoFocus
            spellCheck={false}
          />
        </div>
        <button className="compile-btn" type="submit" disabled={loading || !value.trim()}>
          {loading ? 'Compiling…' : 'Compile  →'}
        </button>
      </form>
      <div className="op-guide">
        <span>Supported operators:</span>
        {['+', '−', '*', '/', '^', '( )'].map(op => (
          <code key={op} onClick={() => setValue(v => v + (op === '−' ? '-' : op === '( )' ? '(' : op))}>
            {op}
          </code>
        ))}
      </div>
    </div>
  )
}
