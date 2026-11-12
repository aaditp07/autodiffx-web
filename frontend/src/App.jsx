import { useState } from 'react'
import Header from './components/Header.jsx'
import ExpressionInput from './components/ExpressionInput.jsx'
import ResultPanel from './components/ResultPanel.jsx'
import ExamplesPanel from './components/ExamplesPanel.jsx'
import './App.css'

const API_URL = import.meta.env.VITE_API_URL || '/api'

export default function App() {
  const [result, setResult] = useState(null)
  const [loading, setLoading] = useState(false)
  const [error, setError] = useState(null)
  const [expression, setExpression] = useState('')

  async function handleCompile(expr) {
    setLoading(true)
    setError(null)
    setResult(null)
    setExpression(expr)

    try {
      const res = await fetch(`${API_URL}/compile`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ expression: expr }),
      })
      const data = await res.json()
      if (!res.ok) throw new Error(data.error || 'Server error')
      setResult(data)
    } catch (err) {
      setError(err.message)
    } finally {
      setLoading(false)
    }
  }

  return (
    <div className="app">
      <Header />
      <main className="main">
        <div className="left-col">
          <ExpressionInput onCompile={handleCompile} loading={loading} />
          <ExamplesPanel onSelect={handleCompile} />
        </div>
        <div className="right-col">
          {error && (
            <div className="api-error">
              <span className="error-icon">⚠</span> {error}
            </div>
          )}
          {loading && (
            <div className="loading-box">
              <div className="spinner" />
              <span>Compiling…</span>
            </div>
          )}
          {result && !loading && <ResultPanel result={result} />}
          {!result && !loading && !error && (
            <div className="empty-state">
              <div className="empty-icon">∂</div>
              <p>Enter an expression in <code>x</code> and press <strong>Compile</strong></p>
              <p className="hint">e.g. <code>x^3 + 2*x^2 + 5*x</code></p>
            </div>
          )}
        </div>
      </main>
      <footer className="footer">
        <p>AutodiffX · Flex + Bison + C compiler · PCD Innovative Assignment</p>
      </footer>
    </div>
  )
}
