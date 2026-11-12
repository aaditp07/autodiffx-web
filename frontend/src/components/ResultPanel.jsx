import { useState } from 'react'
import './ResultPanel.css'

const PHASES = [
  { key: 'tokens',    label: '1 · Tokens',      color: '#58a6ff', icon: '⬡' },
  { key: 'ast',       label: '2 · AST',          color: '#a371f7', icon: '🌳' },
  { key: 'semantic',  label: '3 · Semantic',     color: '#3fb950', icon: '✓' },
  { key: 'expr',      label: '4 · Expression',   color: '#d29922', icon: 'f' },
  { key: 'derivative',label: '5 · Derivative',   color: '#58a6ff', icon: "f'" },
  { key: 'optimized', label: '6 · Optimized',    color: '#3fb950', icon: '★' },
]

export default function ResultPanel({ result }) {
  const [activeTab, setActiveTab] = useState('all')
  const hasErrors = result.errors && result.errors.length > 0

  return (
    <div className="result-panel">
      {/* Summary bar */}
      <div className={`result-summary ${hasErrors ? 'has-errors' : 'ok'}`}>
        <div className="summary-status">
          {hasErrors ? '✕ Compilation Error' : '✓ Compiled Successfully'}
        </div>
        <div className="summary-input">
          <code>{result.input}</code>
        </div>
      </div>

      {/* Errors */}
      {hasErrors && (
        <div className="error-block">
          {result.errors.map((e, i) => (
            <div key={i} className="error-line">{e}</div>
          ))}
        </div>
      )}

      {/* Tabs */}
      <div className="tabs">
        {['all', 'pipeline', 'raw'].map(t => (
          <button key={t} className={`tab ${activeTab === t ? 'active' : ''}`} onClick={() => setActiveTab(t)}>
            {t === 'all' ? 'Results' : t === 'pipeline' ? 'Pipeline' : 'Raw Output'}
          </button>
        ))}
      </div>

      {activeTab === 'all' && (
        <div className="results-grid">
          <ResultCard label="Tokens" color="#58a6ff" icon="⬡">
            <code className="mono-wrap">{result.tokens || '—'}</code>
          </ResultCard>

          <ResultCard label="Semantic Check" color={hasErrors && result.errors.some(e => e.startsWith('Semantic')) ? '#f85149' : '#3fb950'} icon="✓">
            {result.semantic
              ? <span className="semantic-ok">OK</span>
              : result.errors.filter(e => e.startsWith('Semantic')).map((e, i) => (
                  <div key={i} className="semantic-err">{e}</div>
                ))
            }
          </ResultCard>

          <ResultCard label="Expression" color="#d29922" icon="f(x)">
            <code className="expr-big">{result.expression || '—'}</code>
          </ResultCard>

          <ResultCard label="Derivative  f′(x)" color="#a371f7" icon="∂">
            <code className="expr-big">{result.derivative || '—'}</code>
          </ResultCard>

          <ResultCard label="Optimized  f′(x)" color="#3fb950" icon="★" highlight>
            <code className="expr-big highlight">{result.optimized || '—'}</code>
          </ResultCard>

          <ResultCard label="AST" color="#58a6ff" icon="🌳" full>
            <pre className="ast-tree">{result.ast ? result.ast.join('\n') : '—'}</pre>
          </ResultCard>
        </div>
      )}

      {activeTab === 'pipeline' && (
        <div className="pipeline">
          {PHASES.map(({ key, label, color, icon }) => {
            let content = null
            if (key === 'tokens') content = result.tokens
            else if (key === 'ast') content = result.ast ? result.ast.join('\n') : null
            else if (key === 'semantic') content = result.semantic || result.errors.filter(e => e.startsWith('Semantic')).join('\n')
            else if (key === 'expr') content = result.expression
            else if (key === 'derivative') content = result.derivative
            else if (key === 'optimized') content = result.optimized

            return (
              <div key={key} className="pipeline-step">
                <div className="pipeline-label" style={{ color }}>
                  <span className="pipeline-icon">{icon}</span>
                  <span>{label}</span>
                </div>
                <div className="pipeline-arrow">↓</div>
                <pre className="pipeline-content">{content || '—'}</pre>
              </div>
            )
          })}
        </div>
      )}

      {activeTab === 'raw' && (
        <pre className="raw-output">{result.rawOutput}</pre>
      )}
    </div>
  )
}

function ResultCard({ label, color, icon, children, highlight, full }) {
  return (
    <div className={`result-card ${highlight ? 'highlighted' : ''} ${full ? 'full-width' : ''}`}>
      <div className="card-header" style={{ borderColor: color }}>
        <span className="card-icon" style={{ color }}>{icon}</span>
        <span className="card-label">{label}</span>
      </div>
      <div className="card-body">{children}</div>
    </div>
  )
}
