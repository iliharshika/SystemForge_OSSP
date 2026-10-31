import { ArrowRight } from 'lucide-react'
import { PageHeader, ResultBadge } from '../components/ui'
import ProcessInfo from '../components/ProcessInfo'

const CHAIN = [
  ['USER', 'types a command', 'ice'],
  ['ShellForge', 'command interface', 'ice'],
  ['NanoKernel', 'fork · execvp · waitpid', 'ember'],
  ['Linux Kernel', 'real OS services', 'ember'],
]

export default function Dashboard({ stats, last, go }) {
  const none = !stats || stats.total === 0
  const cells = [
    ['TOTAL EXECUTIONS', stats?.total ?? 0, ''],
    ['SUCCESSFUL', stats?.successful ?? 0, 'ok'],
    ['FAILED', stats?.failed ?? 0, 'bad'],
    ['LAST CHILD PID', none ? 'No executions yet' : stats.last_child_pid, 'child'],
    ['LAST EXIT STATUS', none ? 'No executions yet' : stats.last_exit_status ?? 'signal', ''],
  ]
  return (
    <>
      <PageHeader title="ForgeOS Control Center" sub="Monitor and visualize the Linux process lifecycle." />

      <section className="readout" aria-label="Execution statistics">
        {cells.map(([k, v, c]) => (
          <div key={k} className={`cell ${c}`}>
            <span className="k">{k}</span>
            <strong className={typeof v === 'string' ? 'small' : ''}>{v}</strong>
          </div>
        ))}
      </section>

      <div className="grid two">
        <section className="panel pad">
          <h2>Last execution</h2>
          {last ? (
            <>
              <ProcessInfo run={last} />
              <p className="note">Recorded at {last.time} from the real NanoKernel output.</p>
            </>
          ) : (
            <div className="empty">
              <p>Nothing has run yet. Every number on this page comes from a real fork() / execvp() / waitpid() run.</p>
              <button className="btn" onClick={() => go('execute')}>Execute your first process <ArrowRight size={16} /></button>
            </div>
          )}
        </section>

        <section className="panel pad">
          <h2>How a command travels</h2>
          <ol className="chain">
            {CHAIN.map(([t, s, c], i) => (
              <li key={t} className={c}><b>{t}</b><span>{s}</span></li>
            ))}
          </ol>
          <p className="note">NanoKernel is this project's own process-management module. The Linux Kernel underneath is the real one that performs the system calls.</p>
        </section>
      </div>
    </>
  )
}
