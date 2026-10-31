export function PageHeader({ title, sub }) {
  return (
    <header className="page-h">
      <h1>{title}</h1>
      {sub && <p>{sub}</p>}
    </header>
  )
}

export function ResultBadge({ result }) {
  if (!result) return <span className="rbadge idle">—</span>
  const kind = result === 'SUCCESS' ? 'ok' : result.includes('SIGNAL') ? 'bad' : 'warn'
  return <span className={`rbadge ${kind}`}><i />{result}</span>
}

// Colours each line of the real NanoKernel log by who printed it.
export function LogView({ text }) {
  if (!text) return null
  return (
    <pre className="log">
      {text.split('\n').map((l, i) => {
        let c = 'out'
        if (l.startsWith('[ShellForge]')) c = 'sf'
        else if (l.startsWith('[NanoKernel]')) {
          c = 'nk'
          if (/Result\s*:\s*SUCCESS/.test(l)) c = 'okline'
          else if (/Result\s*:/.test(l)) c = 'badline'
        }
        return <span key={i} className={c}>{l || ' '}{'\n'}</span>
      })}
    </pre>
  )
}

export function Empty({ children }) {
  return <div className="empty">{children}</div>
}
