// The 9 lifecycle stages. Each one is lit only when the backend response proves it happened
// (the flags come from parsing the real NanoKernel output).
export const STAGES = [
  { key: 'request', n: '01', label: 'REQUEST', lane: 'parent', tip: 'ShellForge hands the command to NanoKernel.' },
  { key: 'fork', n: '02', label: 'FORK', lane: 'parent', tip: 'The parent calls fork() to create a child.' },
  { key: 'child_created', n: '03', label: 'CHILD CREATED', lane: 'child', tip: 'The child exists and has its own PID.' },
  { key: 'exec', n: '04', label: 'EXEC', lane: 'child', tip: 'The child calls execvp() to load the program.' },
  { key: 'running', n: '05', label: 'RUNNING', lane: 'child', tip: 'The program replaced the child and is running.' },
  { key: 'waiting', n: '06', label: 'WAITING', lane: 'parent', tip: 'The parent blocks in waitpid().' },
  { key: 'terminated', n: '07', label: 'TERMINATED', lane: 'child', tip: 'The child ended and waitpid() returned.' },
  { key: 'status', n: '08', label: 'STATUS RETRIEVED', lane: 'parent', tip: 'The parent read the status with WIFEXITED / WEXITSTATUS.' },
  { key: 'complete', n: '09', label: 'COMPLETE', lane: 'parent', tip: 'NanoKernel printed the final result.' },
]
