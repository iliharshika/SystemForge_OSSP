import { useEffect, useMemo, useState } from 'react'
import { Play, ShieldAlert, Loader2 } from 'lucide-react'
import { PageHeader, LogView } from '../components/ui'
import StageRail from '../components/StageRail'
import ProcessInfo from '../components/ProcessInfo'
import { STAGES } from '../lib/stages'

const reduced = typeof window !== 'undefined' && window.matchMedia('(prefers-reduced-motion: reduce)').matches

export default function Execute({ commands, run }) {
  const [cmd, setCmd] = useState('pwd')
  const [args, setArgs] = useState('')
  const [phase, setPhase] = useState('idle') // idle | pending | replay | done
  const [res, setRes] = useState(null)
  const [err, setErr] = useState(null)
  const [shown, setShown] = useState(0)

  const sel = useMemo(() => commands.find((c) => c.command === cmd), [commands, cmd])

  useEffect(() => {
    if (phase !== 'replay') return
    if (shown >= STAGES.length) { setPhase('done'); return }
    const t = setTimeout(() => setShown((s) => s + 1), shown === 0 ? 120 : 380)
    return () => clearTimeout(t)
  }, [phase, shown])

  async function go() {
    setErr(null); setRes(null); setShown(0); setPhase('pending')
    try {
      const r = await run(cmd, args)
      setRes(r)
      if (reduced) { setShown(STAGES.length); setPhase('done') } else setPhase('replay')
    } catch (e) {
      setErr(e); setPhase('idle')
    }
  }

  const busy = phase === 'pending' || phase === 'replay'
  const active = phase === 'replay' && shown > 0 ? STAGES[shown - 1] : null

  return (
    <>
      <PageHeader title="Execute Linux Process" sub="Pick a command. ForgeOS will really fork a child, exec the program and wait for it." />

      <section className="panel pad exec">
        <div className="field">
          <label htmlFor="cmd">Command</label>
          <select id="cmd" value={cmd} onChange={(e) => { setCmd(e.target.value); setArgs('') }} disabled={busy}>
            {commands.map((c) => <option key={c.command} value={c.command}>{c.command}</option>)}
          </select>
          <small>{sel?.description}</small>
        </div>
        <div className="field grow">
          <label htmlFor="args">Arguments</label>
          <input id="args" value={args} onChange={(e) => setArgs(e.target.value)} disabled={busy || (sel && sel.examples.length === 0 && cmd !== 'echo')}
                 placeholder={sel?.examples.length ? `e.g. ${sel.examples[0]}` : 'this command takes no arguments'}
                 onKeyDown={(e) => e.key === 'Enter' && !busy && go()} spellCheck="false" autoComplete="off" />
          <div className="chips">
            {sel?.examples.map((x) => <button key={x} className="chip" onClick={() => setArgs(x)} disabled={busy}>{cmd} {x}</button>)}
          </div>
        </div>
        <button className="btn go" onClick={go} disabled={busy}>
          {busy ? <Loader2 size={17} className="spin" /> : <Play size={17} />} CREATE &amp; EXECUTE PROCESS
        </button>
      </section>

      {err && (
        <div className="alert" role="alert">
          <ShieldAlert size={18} />
          <div><b>{err.rejected ? 'Blocked by the allowlist' : 'Execution failed'}</b><p>{err.message}</p></div>
        </div>
      )}

      <section className="panel pad">
        <div className="sec-h">
          <h2>Live process visualization</h2>
          <span className="cap">
            {phase === 'idle' && 'Waiting for a command'}
            {phase === 'pending' && 'Running the C program: fork → execvp → waitpid…'}
            {phase === 'replay' && 'Replaying stages found in the real NanoKernel log'}
            {phase === 'done' && 'Stages shown are the ones proven by the log'}
          </span>
        </div>
        <StageRail flags={res?.stages} revealed={shown} animating={phase === 'replay'} pending={phase === 'pending'} />
        <p className="tip">{active ? `${active.n} ${active.label}: ${active.tip}` : phase === 'done' ? 'All observed stages completed.' : '\u00a0'}</p>
      </section>

      <section className="panel pad">
        <h2>Process information</h2>
        <ProcessInfo run={phase === 'replay' || phase === 'done' ? res : null} />
      </section>

      {(phase === 'replay' || phase === 'done') && res && (
        <div className="grid two">
          <section className="panel pad">
            <h2>Program output</h2>
            <pre className="out-box">{res.output || '(the program printed nothing)'}</pre>
          </section>
          <section className="win">
            <div className="win-bar"><i /><i /><i /><span>nanokernel.log</span></div>
            <LogView text={res.terminal_output} />
          </section>
        </div>
      )}
    </>
  )
}
