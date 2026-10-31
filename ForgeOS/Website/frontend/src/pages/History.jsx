import { Trash2 } from 'lucide-react'
import { PageHeader, ResultBadge, Empty } from '../components/ui'

export default function History({ history, clear }) {
  const rows = [...history].reverse()
  return (
    <>
      <PageHeader title="Execution History" sub="Only real executions are recorded here." />
      <section className="panel">
        <div className="bar">
          <span>{rows.length} {rows.length === 1 ? 'execution' : 'executions'}</span>
          <button className="btn ghost" disabled={!rows.length} onClick={() => window.confirm('Clear all execution history?') && clear()}>
            <Trash2 size={15} /> Clear History
          </button>
        </div>
        {rows.length === 0 ? <Empty>No executions yet.</Empty> : (
          <div className="scroll">
            <table>
              <thead><tr><th>TIME</th><th>COMMAND</th><th>PID</th><th>EXIT STATUS</th><th>RESULT</th></tr></thead>
              <tbody>
                {rows.map((r) => (
                  <tr key={r.id}>
                    <td className="mono dim">{r.time}</td>
                    <td className="mono">{r.command}</td>
                    <td className="mono c-child">{r.child_pid}</td>
                    <td className="mono">{r.exit_status ?? (r.signal ? `sig ${r.signal}` : '—')}</td>
                    <td><ResultBadge result={r.result} /></td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        )}
      </section>
    </>
  )
}
