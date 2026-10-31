import { useState } from 'react'
import { PageHeader } from '../components/ui'
import LifecycleDiagram from '../components/LifecycleDiagram'
import ProcessTree from '../components/ProcessTree'
import { CONCEPTS } from '../lib/concepts'

const MAP = { fork: 'fork', execvp: 'execvp', waitpid: 'waitpid', exit: 'exit' }

export default function Lifecycle({ last }) {
  const [sel, setSel] = useState('fork')
  const c = CONCEPTS.find((x) => x.id === MAP[sel])
  const live = c.live(last)
  return (
    <>
      <PageHeader title="Process Lifecycle" sub="Click fork(), execvp(), waitpid() or Exit Status to see what each step does." />
      <div className="grid life">
        <section className="panel pad"><LifecycleDiagram selected={sel} onSelect={setSel} /></section>
        <div className="stack">
          <section className="panel pad explain" aria-live="polite">
            <span className="tag">{c.tag}</span>
            <h2>{c.title}</h2>
            <p>{c.short}</p>
            <p className="dim">{c.detail}</p>
            <pre className="code">{c.code}</pre>
            {live && <p className="livefact">{live}</p>}
          </section>
          <section className="panel pad">
            <h2>Process tree</h2>
            <ProcessTree run={last} />
          </section>
        </div>
      </div>
    </>
  )
}
