import { PageHeader } from '../components/ui'

const TEAM = [['2520030607', 'Ili Harshika'], ['2520030512', 'Gurrapu Nikshitha'], ['2520030365', 'Angela Jose']]

export default function About() {
  return (
    <>
      <PageHeader title="About Project" sub="Create. Execute. Synchronize. Terminate." />
      <div className="grid two">
        <section className="panel pad">
          <h2>Linux Process Creation, Execution and Termination System</h2>
          <p className="dim">ForgeShell / ForgeOS demonstrates the full Linux process lifecycle with real system calls: fork(), execvp(), waitpid(), getpid(), getppid(), WIFEXITED(), WEXITSTATUS(), WIFSIGNALED() and WTERMSIG().</p>
          <dl className="meta">
            <div><dt>Course</dt><dd>Operating Systems and Systems Programming</dd></div>
            <div><dt>Course code</dt><dd className="mono">25CS2104E</dd></div>
            <div><dt>Section</dt><dd>1</dd></div>
            <div><dt>Team</dt><dd>7</dd></div>
          </dl>
          <h3>Team members</h3>
          <ul className="team">
            {TEAM.map(([id, n]) => <li key={id}><span className="mono">{id}</span><b>{n}</b></li>)}
          </ul>
        </section>
        <section className="panel pad">
          <h2>Architecture</h2>
          <div className="arch">
            <div className="ice"><b>ShellForge</b><span>User-facing command interface.</span></div>
            <div className="ember"><b>NanoKernel</b><span>Project-level process-management module. It is not the Linux kernel.</span></div>
            <div className="ember"><b>Linux Kernel</b><span>The actual operating-system kernel that performs the system calls.</span></div>
          </div>
          <h3>This website</h3>
          <p className="dim">React → Flask REST API → the compiled ShellForge/NanoKernel C program → Linux. The backend runs the real binary for every request and reports exactly what NanoKernel printed. Only an allowlist of safe commands is accepted.</p>
        </section>
      </div>
    </>
  )
}
