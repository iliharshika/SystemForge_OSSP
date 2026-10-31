import { useEffect, useState } from 'react'
import { PageHeader, ResultBadge, Empty } from '../components/ui'
import ProcessTree from '../components/ProcessTree'
import * as api from '../services/api'

export default function Monitor({ history }) {
  const [rows, setRows] = useState([])
  const [pick, setPick] = useState(null)
  useEffect(() => { api.processes().then((r) => setRows(r.processes)).catch(() => {}) }, [history])
  const chosen = rows.find((r) => r.id === pick) || rows[0] || null

  return (
    <>
      <PageHeader title="Process Monitor" sub="Every child process ForgeOS has created. Select a row to see its tree." />
      <div className="grid mon">
        <section className="panel">
          {rows.length === 0 ? <Empty>No processes yet. Run a command from Execute Process.</Empty> : (
            <div className="scroll">
              <table>
                <thead><tr><th>PID</th><th>PPID</th><th>COMMAND</th><th>STATE</th><th>EXIT STATUS</th><th>RESULT</th></tr></thead>
                <tbody>
                  {rows.map((r) => (
                    <tr key={r.id} className={chosen?.id === r.id ? 'sel' : ''} onClick={() => setPick(r.id)} tabIndex={0}
                        onKeyDown={(e) => e.key === 'Enter' && setPick(r.id)}>
                      <td className="mono c-child">{r.child_pid}</td>
                      <td className="mono c-parent">{r.child_ppid ?? r.parent_pid}</td>
                      <td className="mono">{r.command}</td>
                      <td><span className="state">{r.state}</span></td>
                      <td className="mono">{r.exit_status ?? (r.signal ? `sig ${r.signal}` : '—')}</td>
                      <td><ResultBadge result={r.result} /></td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          )}
          <p className="note pad-s">Each child is waited for before the request returns, so its state is always TERMINATED once it appears here.</p>
        </section>
        <section className="panel pad">
          <h2>Process tree</h2>
          <ProcessTree run={chosen} />
        </section>
      </div>
    </>
  )
}
