# ForgeOS: Linux Process Lifecycle Control Center

A web interface for the **real** ForgeShell C project. Every click on the website starts your compiled
`Shellforge/forgeos` program, which really performs `fork()` → `execvp()` → `waitpid()`. The website shows
exactly what NanoKernel printed. Nothing is simulated.

```
React (Vite)  →  Flask REST API  →  Shellforge/forgeos  →  NanoKernel  →  fork / execvp / waitpid  →  Linux kernel
```

## Folder layout

```
ForgeOS/
├── Shellforge/      shell.c, forgeos        (YOUR files, untouched)
├── NanoKernel/      process.c, process.h    (YOUR files, untouched)
├── backend/         app.py, process_bridge.py, requirements.txt
├── frontend/        React + Vite site
├── README.md
└── run.sh
```

## 1. One-time setup (Ubuntu)

```bash
sudo apt update
sudo apt install -y gcc python3 python3-venv python3-pip nodejs npm
node -v        # must be v18 or newer (see Troubleshooting if not)

cd ~/SystemForge_OSSP/ForgeOS
chmod +x run.sh
./run.sh setup
```

## 2. Run it

**Terminal 1: backend**
```bash
cd ~/SystemForge_OSSP/ForgeOS
./run.sh backend
```
**Terminal 2: frontend**
```bash
cd ~/SystemForge_OSSP/ForgeOS
./run.sh frontend
```
Open the URL Vite prints (normally **http://localhost:5173**).

(Manual equivalents: `cd backend && source .venv/bin/activate && python app.py` and `cd frontend && npm run dev`.)

## What changed in your C code?

**Nothing.** `shell.c`, `process.c` and `process.h` are untouched. The Flask backend only *runs* the binary.

One detail worth knowing: when a C program prints to a pipe instead of a terminal, glibc buffers `printf`.
Because `fork()` copies that buffer and `execvp()` throws the child's copy away, the child's `[NanoKernel]` lines
disappear and the command output arrives out of order. The bridge avoids this by launching the binary as
`stdbuf -o0 -e0 ./forgeos`, which makes it print unbuffered. If the binary is missing or older than the sources,
the backend recompiles it with your own gcc command.

## API

| Method | Path | Purpose |
|---|---|---|
| GET  | `/api/health` | Backend + binary status |
| GET  | `/api/commands` | Allowed commands |
| POST | `/api/execute` | Run a command through ForgeOS `{ "command": "ls", "arguments": ["-l"] }` |
| GET  | `/api/history` | Past executions + dashboard stats |
| GET  | `/api/processes` | Same executions for the Process Monitor |
| POST | `/api/history/clear` | Clear history |

`/api/execute` returns the real `parent_pid`, `child_pid`, `child_ppid`, `exit_status`, `result`, `output`,
`terminal_output` and a `stages` object (which lifecycle steps appear in the real log). History is saved in
`backend/data/history.json`.

## Security

* Allowlist only: `pwd ls whoami id hostname date uname uptime df free ps echo`.
* Per-command argument rules (e.g. `ls` accepts only flags such as `-l`, `-la`), so no paths.
* Rejects `; & | < > ` $ ( ) { } [ ] \ " ' * ? ! # ~`, newlines and control characters.
* Blocks `rm sudo shutdown reboot mkfs dd kill pkill chmod chown` and more.
* No `shell=True`; arguments are validated and passed to a fixed binary. 10-second timeout.
* The server binds to `127.0.0.1` only.

## Demo script for your lecturer

1. **Execute Process** → `pwd` → **CREATE & EXECUTE PROCESS**. Watch the two-lane diagram light up: the parent lane (blue) forks, the child lane (orange) runs `execvp()`.
2. Show PID / PPID: the child PPID equals the parent PID.
3. Run `ls -z` to show a **real failure**: exit status 2, `COMMAND FAILED`.
4. Open **Terminal** and type `ps`, then compare the PIDs with **Process Monitor**.
5. Open **Process Lifecycle** and click `fork()`, `execvp()`, `waitpid()`, Exit Status.
6. Try `rm -rf /` in the Terminal to show it is blocked by the allowlist.

## Troubleshooting

* **"Backend offline" banner**: start the backend (Terminal 1) and keep it running.
* **`node -v` is older than 18**: `curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash - && sudo apt install -y nodejs`
* **`ensurepip` / venv error**: `sudo apt install -y python3-venv`
* **Port 5000 or 5173 busy**: `ss -ltnp | grep -E '5000|5173'` then stop the old process.
* **gcc error from the backend**: run the gcc command in `Shellforge/` by hand to see the message.
* **Using WSL**: open the Vite URL in your Windows browser; `localhost` is forwarded automatically.
