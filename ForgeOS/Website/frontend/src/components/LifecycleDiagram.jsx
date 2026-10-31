// Interactive fork/exec/wait diagram. fork(), execvp(), waitpid() and Exit Status are clickable.
const NODES = [
  { id: 'user', x: 380, y: 40, t: 'USER COMMAND' },
  { id: 'sf', x: 380, y: 100, t: 'ShellForge' },
  { id: 'nk', x: 380, y: 160, t: 'NanoKernel' },
  { id: 'fork', x: 380, y: 235, t: 'fork()', click: true },
  { id: 'parent', x: 190, y: 320, t: 'PARENT', lane: 'parent' },
  { id: 'child', x: 570, y: 320, t: 'CHILD', lane: 'child' },
  { id: 'waitpid', x: 190, y: 400, t: 'waitpid()', click: true, lane: 'parent' },
  { id: 'wait', x: 190, y: 470, t: 'WAIT', lane: 'parent' },
  { id: 'execvp', x: 570, y: 400, t: 'execvp()', click: true, lane: 'child' },
  { id: 'prog', x: 570, y: 470, t: 'EXECUTE PROGRAM', lane: 'child' },
  { id: 'term', x: 570, y: 540, t: 'TERMINATE', lane: 'child' },
  { id: 'exit', x: 380, y: 610, t: 'EXIT STATUS', click: true },
  { id: 'result', x: 380, y: 670, t: 'RESULT' },
]
const EDGES = [
  'M380 60 V80', 'M380 120 V140', 'M380 180 V215',
  'M380 255 V285 H190 V300', 'M380 255 V285 H570 V300',
  'M190 340 V380', 'M190 420 V450', 'M190 490 V575 H380 V590',
  'M570 340 V380', 'M570 420 V450', 'M570 490 V520', 'M570 560 V575 H380',
  'M380 630 V650',
]

export default function LifecycleDiagram({ selected, onSelect }) {
  return (
    <svg viewBox="0 0 760 700" className="flow" role="group" aria-label="Process lifecycle diagram">
      <defs>
        <marker id="arr" viewBox="0 0 10 10" refX="8" refY="5" markerWidth="6" markerHeight="6" orient="auto-start-reverse">
          <path d="M0 0 L10 5 L0 10 z" fill="#6b7a93" />
        </marker>
      </defs>
      {EDGES.map((d, i) => <path key={i} d={d} className="fedge" markerEnd={i === 11 ? undefined : 'url(#arr)'} />)}
      <text x="285" y="277" textAnchor="middle" className="fnote">returns child PID</text>
      <text x="475" y="277" textAnchor="middle" className="fnote">returns 0</text>
      {NODES.map((n) => {
        const act = (e) => { if (n.click && (e.type === 'click' || e.key === 'Enter' || e.key === ' ')) { e.preventDefault(); onSelect(n.id) } }
        return (
          <g key={n.id} className={`fnode ${n.lane || ''} ${n.click ? 'click' : ''} ${selected === n.id ? 'sel' : ''}`}
             onClick={act} onKeyDown={act} tabIndex={n.click ? 0 : undefined} role={n.click ? 'button' : undefined}
             aria-label={n.click ? `Explain ${n.t}` : undefined}>
            <rect x={n.x - 85} y={n.y - 20} width="170" height="40" rx="9" />
            <text x={n.x} y={n.y + 5} textAnchor="middle">{n.t}</text>
          </g>
        )
      })}
    </svg>
  )
}
