import { PageHeader } from '../components/ui'
import { CONCEPTS } from '../lib/concepts'

export default function Concepts({ last }) {
  return (
    <>
      <PageHeader title="OS Concepts" sub="The ideas behind every run, in plain language." />
      <div className="cards">
        {CONCEPTS.map((c) => {
          const live = c.live(last)
          return (
            <article key={c.id} className="panel pad card">
              <span className="tag">{c.tag}</span>
              <h2>{c.title}</h2>
              <p>{c.short}</p>
              <p className="dim">{c.detail}</p>
              <pre className="code">{c.code}</pre>
              {live && <p className="livefact">{live}</p>}
            </article>
          )
        })}
      </div>
    </>
  )
}
