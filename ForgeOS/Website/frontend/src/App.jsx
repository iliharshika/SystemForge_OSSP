import { useCallback, useEffect, useState } from 'react'
import Sidebar from './components/Sidebar'
import * as api from './services/api'
import Dashboard from './pages/Dashboard'
import Execute from './pages/Execute'
import Lifecycle from './pages/Lifecycle'
import Monitor from './pages/Monitor'
import Terminal from './pages/Terminal'
import History from './pages/History'
import Concepts from './pages/Concepts'
import About from './pages/About'

const IDS = ['dashboard', 'execute', 'lifecycle', 'monitor', 'terminal', 'history', 'concepts', 'about']
const fromHash = () => (IDS.includes(location.hash.slice(1)) ? location.hash.slice(1) : 'dashboard')

export default function App() {
  const [page, setPageState] = useState(fromHash)
  const [collapsed, setCollapsed] = useState(() => window.innerWidth < 900)
  const [online, setOnline] = useState(null)
  const [commands, setCommands] = useState([])
  const [history, setHistory] = useState([])
  const [stats, setStats] = useState(null)

  const setPage = (p) => { location.hash = p; setPageState(p) }
  useEffect(() => {
    const h = () => setPageState(fromHash())
    window.addEventListener('hashchange', h)
    return () => window.removeEventListener('hashchange', h)
  }, [])

  const refresh = useCallback(async () => {
    const h = await api.history()
    setHistory(h.history); setStats(h.stats)
  }, [])

  useEffect(() => {
    const ping = () => api.health().then((h) => setOnline(h.status === 'online')).catch(() => setOnline(false))
    ping()
    api.commands().then((r) => setCommands(r.commands)).catch(() => {})
    refresh().catch(() => {})
    const t = setInterval(ping, 10000)
    return () => clearInterval(t)
  }, [refresh])

  const run = useCallback(async (cmd, args) => {
    const res = await api.execute(cmd, args)
    setOnline(true)
    await refresh()
    return res
  }, [refresh])

  const clear = async () => {
    const h = await api.clearHistory()
    setHistory(h.history); setStats(h.stats)
  }

  const last = history.length ? history[history.length - 1] : null

  return (
    <div className="app">
      <Sidebar page={page} setPage={setPage} collapsed={collapsed} setCollapsed={setCollapsed} online={online} />
      <main className="main">
        {online === false && (
          <div className="banner" role="alert">
            Backend offline. In another terminal run: <code>cd ~/SystemForge_OSSP/ForgeOS/backend &amp;&amp; source .venv/bin/activate &amp;&amp; python app.py</code>
          </div>
        )}
        {page === 'dashboard' && <Dashboard stats={stats} last={last} go={setPage} />}
        {page === 'execute' && <Execute commands={commands} run={run} />}
        {page === 'lifecycle' && <Lifecycle last={last} />}
        {page === 'monitor' && <Monitor history={history} />}
        {page === 'terminal' && <Terminal commands={commands} run={run} online={online} />}
        {page === 'history' && <History history={history} clear={clear} />}
        {page === 'concepts' && <Concepts last={last} />}
        {page === 'about' && <About />}
      </main>
    </div>
  )
}
