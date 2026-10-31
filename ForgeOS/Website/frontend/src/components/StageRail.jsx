import { STAGES } from '../lib/stages'

const X = (i) => 70 + i * 107
const Y = { parent: 64, child: 176 }

function nodeState(i, flags, revealed, animating) {
  if (!flags || i >= revealed) return 'idle'
  if (!flags[STAGES[i].key]) return 'missed'
  return animating && i === revealed - 1 ? 'active' : 'done'
}

function pathBetween(a, b) {
  const x1 = X(a), x2 = X(b), y1 = Y[STAGES[a].lane], y2 = Y[STAGES[b].lane]
  if (y1 === y2) return `M${x1 + 22} ${y1} H${x2 - 22}`
  const mx = (x1 + x2) / 2
  return `M${x1 + 22} ${y1} C${mx} ${y1} ${mx} ${y2} ${x2 - 22} ${y2}`
}

/** Two-lane swim diagram: parent lane (ice) and child lane (ember). */
export default function StageRail({ flags, revealed = 0, animating = false, pending = false }) {
  return (
    <div className={`rail-wrap ${pending ? 'pending' : ''}`}>
      <svg viewBox="0 0 1000 250" className="rail" role="img" aria-label="Process lifecycle stages across parent and child lanes">
        <defs>
          <linearGradient id="gp" x1="0" x2="1"><stop offset="0" stopColor="#5cc8ff" stopOpacity=".10" /><stop offset="1" stopColor="#5cc8ff" stopOpacity=".02" /></linearGradient>
          <linearGradient id="gc" x1="0" x2="1"><stop offset="0" stopColor="#ff7a3d" stopOpacity=".10" /><stop offset="1" stopColor="#ff7a3d" stopOpacity=".02" /></linearGradient>
        </defs>
        <rect x="4" y="12" width="992" height="104" rx="12" fill="url(#gp)" className="band" />
        <rect x="4" y="128" width="992" height="112" rx="12" fill="url(#gc)" className="band" />
        <text x="18" y="32" className="lane-t p">PARENT PROCESS</text>
        <text x="18" y="148" className="lane-t c">CHILD PROCESS</text>

        {STAGES.slice(0, -1).map((s, i) => {
          const on = flags && i + 1 < revealed && flags[s.key] && flags[STAGES[i + 1].key]
          const live = on && animating && i + 1 === revealed - 1
          return <path key={s.key} d={pathBetween(i, i + 1)} className={`seg ${on ? 'on' : ''} ${live ? 'live' : ''} ${STAGES[i + 1].lane}`} />
        })}

        {STAGES.map((s, i) => {
          const st = nodeState(i, flags, revealed, animating)
          const words = s.label.split(' ')
          const half = words.length > 1 ? [words[0], words.slice(1).join(' ')] : [s.label]
          const y = Y[s.lane]
          return (
            <g key={s.key} className={`node ${s.lane} ${st}`}>
              <title>{`${s.n} ${s.label}: ${s.tip}${st === 'missed' ? ' (not observed in the log)' : ''}`}</title>
              {st === 'active' && <circle cx={X(i)} cy={y} r="22" className="ring" />}
              <circle cx={X(i)} cy={y} r="22" className="disc" />
              <text x={X(i)} y={y + 4} textAnchor="middle" className="num">{s.n}</text>
              {half.map((w, k) => (
                <text key={k} x={X(i)} y={y + 42 + k * 13} textAnchor="middle" className="lbl">{w}</text>
              ))}
            </g>
          )
        })}
      </svg>
    </div>
  )
}
