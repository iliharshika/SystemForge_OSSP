import { useState } from 'react'
import { ChevronDown } from 'lucide-react'

function TNode({ kind, title, badges = [], meta = [], children }) {
  const [open, setOpen] = useState(true)
  return (
    <li className={`tn ${kind}`}>
      <button className="tn-card" onClick={() => setOpen((o) => !o)} aria-expanded={open}>
        <span className="tn-title">{title}</span>
        {badges.map((b) => <span key={b.l} className={`badge ${b.k}`}><em>{b.l}</em>{b.v}</span>)}
        {children && <ChevronDown size={14} className={open ? 'chev' : 'chev rot'} />}
      </button>
      {open && meta.length > 0 && <div className="tn-meta">{meta.map((m) => <span key={m}>{m}</span>)}</div>}
      {open && children && <ul>{children}</ul>}
    </li>
  )
}

/** Process tree built only from real PID/PPID values of one execution. */
export default function ProcessTree({ run }) {
  if (!run) {
    return (
      <ul className="tree">
        <TNode kind="root" title="ForgeOS" meta={['No process yet. Execute a command to grow the tree.']} />
      </ul>
    )
  }
  const exit = run.exit_status ?? (run.signal ? `signal ${run.signal}` : '—')
  return (
    <ul className="tree">
      <TNode kind="root" title="ForgeOS" meta={['ShellForge → NanoKernel']}>
        <TNode kind="parent" title="Parent process" badges={[{ l: 'PID', v: run.parent_pid, k: 'parent' }]} meta={['calls fork(), then waitpid()']}>
          <TNode kind="child" title="Child process"
                 badges={[{ l: 'PID', v: run.child_pid, k: 'child' }, { l: 'PPID', v: run.child_ppid ?? run.parent_pid, k: 'parent' }]}
                 meta={[`state ${run.state}`, `exit ${exit}`]}>
            <TNode kind="cmd" title={run.command} meta={['loaded by execvp()']} />
          </TNode>
        </TNode>
      </TNode>
    </ul>
  )
}
