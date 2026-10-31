import { useEffect, useRef, useState } from 'react'
import { LogView } from '../components/ui'

const WELCOME = "ForgeOS terminal. Every command runs through the real ShellForge → NanoKernel C program.\nType 'help' for the allowed commands, 'clear' to wipe the screen."

export default function Terminal({ commands, run, online }) {
  const [items, setItems] = useState([{ k: 'sys', text: WELCOME }])
  const [input, setInput] = useState('')
  const [busy, setBusy] = useState(false)
  const [hist, setHist] = useState([])
  const [idx, setIdx] = useState(-1)
  const end = useRef(null), box = useRef(null)

  useEffect(() => { end.current?.scrollIntoView({ block: 'end' }) }, [items, busy])

  async function submit(e) {
    e.preventDefault()
    const text = input.trim()
    if (!text || busy) return
    setInput(''); setIdx(-1); setHist((h) => [text, ...h])
    const echo = { k: 'cmd', text }
    if (text === 'clear') return setItems([])
    if (text === 'help') {
      const lines = commands.map((c) => `  ${c.command.padEnd(9)} ${c.description}${c.examples.length ? `  (${c.examples.map((x) => `${c.command} ${x}`).join(', ')})` : ''}`)
      return setItems((p) => [...p, echo, { k: 'sys', text: 'Allowed commands:\n' + lines.join('\n') }])
    }
    const [c, ...a] = text.split(/\s+/)
    setItems((p) => [...p, echo]); setBusy(true)
    try {
      const r = await run(c, a)
      setItems((p) => [...p, { k: 'log', text: r.terminal_output }])
    } catch (er) {
      setItems((p) => [...p, { k: 'err', text: er.message }])
    } finally { setBusy(false) }
  }

  function keys(e) {
    if (e.key === 'ArrowUp') { e.preventDefault(); const n = Math.min(idx + 1, hist.length - 1); if (hist[n] !== undefined) { setIdx(n); setInput(hist[n]) } }
    if (e.key === 'ArrowDown') { e.preventDefault(); const n = idx - 1; setIdx(n); setInput(n >= 0 ? hist[n] : '') }
  }

  return (
    <section className="term" onClick={() => box.current?.focus()}>
      <div className="win-bar big">
        <i /><i /><i />
        <span>FORGEOS TERMINAL</span>
        <b className={online ? 'on' : 'off'}>{online ? 'ONLINE' : 'OFFLINE'}</b>
      </div>
      <div className="term-body">
        {items.map((it, i) => it.k === 'log' ? <LogView key={i} text={it.text} /> : (
          <pre key={i} className={`line ${it.k}`}>{it.k === 'cmd' ? <><span className="ps">forgeos&gt;</span> {it.text}</> : it.text}</pre>
        ))}
        {busy && <pre className="line sys">forking child process…</pre>}
        <form onSubmit={submit} className="prompt">
          <span className="ps">forgeos&gt;</span>
          <input ref={box} value={input} onChange={(e) => setInput(e.target.value)} onKeyDown={keys} autoFocus
                 spellCheck="false" autoComplete="off" aria-label="Terminal input" disabled={busy} />
        </form>
        <div ref={end} />
      </div>
    </section>
  )
}
