import { LayoutDashboard, Play, GitFork, Activity, Terminal, History, BookOpen, Info, PanelLeftClose, PanelLeftOpen } from 'lucide-react'

export const PAGES = [
  { id: 'dashboard', label: 'Dashboard', icon: LayoutDashboard },
  { id: 'execute', label: 'Execute Process', icon: Play },
  { id: 'lifecycle', label: 'Process Lifecycle', icon: GitFork },
  { id: 'monitor', label: 'Process Monitor', icon: Activity },
  { id: 'terminal', label: 'Terminal', icon: Terminal },
  { id: 'history', label: 'Execution History', icon: History },
  { id: 'concepts', label: 'OS Concepts', icon: BookOpen },
  { id: 'about', label: 'About Project', icon: Info },
]

export default function Sidebar({ page, setPage, collapsed, setCollapsed, online }) {
  return (
    <aside className={`side ${collapsed ? 'min' : ''}`}>
      <div className="brand">
        <div className="mark"><GitFork size={20} strokeWidth={2.4} /></div>
        {!collapsed && (
          <div>
            <b>FORGEOS</b>
            <small>Linux Process Lifecycle Control Center</small>
          </div>
        )}
      </div>

      <nav aria-label="Main">
        {PAGES.map(({ id, label, icon: Icon }) => (
          <button key={id} className={page === id ? 'nav on' : 'nav'} onClick={() => setPage(id)}
                  title={collapsed ? label : undefined} aria-current={page === id ? 'page' : undefined}>
            <Icon size={18} />
            {!collapsed && <span>{label}</span>}
          </button>
        ))}
      </nav>

      <div className="side-foot">
        {!collapsed && <p className="tagline">Create. Execute. Synchronize. Terminate.</p>}
        <div className="status" title="Flask bridge to the C program">
          <span className={`dot ${online === null ? 'wait' : online ? 'on' : 'off'}`} />
          {!collapsed && <span>{online === null ? 'Connecting…' : online ? 'NanoKernel bridge online' : 'Backend offline'}</span>}
        </div>
        <button className="collapse" onClick={() => setCollapsed(!collapsed)} aria-label="Toggle sidebar">
          {collapsed ? <PanelLeftOpen size={18} /> : <PanelLeftClose size={18} />}
          {!collapsed && <span>Collapse</span>}
        </button>
      </div>
    </aside>
  )
}
