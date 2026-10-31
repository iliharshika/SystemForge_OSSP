import { ResultBadge } from './ui'

const dash = '—'
export default function ProcessInfo({ run }) {
  const r = run
  const f = (v) => (v === null || v === undefined ? dash : v)
  const exit = r ? (r.exit_status !== null && r.exit_status !== undefined ? r.exit_status : r.signal ? `signal ${r.signal}` : dash) : dash
  const cells = [
    ['COMMAND', r ? <code>{r.command}</code> : dash],
    ['PARENT PID', r ? <span className="badge parent">{r.parent_pid}</span> : dash],
    ['CHILD PID', r ? <span className="badge child">{r.child_pid}</span> : dash],
    ['CHILD PPID', r ? <span className="badge parent">{f(r.child_ppid ?? r.parent_pid)}</span> : dash],
    ['PROCESS STATE', r ? <span className="state">{r.state}</span> : dash],
    ['NORMAL TERMINATION', r ? (r.normal_termination ? 'YES' : 'NO') : dash],
    ['EXIT STATUS', exit],
    ['RESULT', <ResultBadge result={r?.result} />],
  ]
  return (
    <dl className="info">
      {cells.map(([k, v]) => (
        <div key={k}><dt>{k}</dt><dd>{v}</dd></div>
      ))}
    </dl>
  )
}
