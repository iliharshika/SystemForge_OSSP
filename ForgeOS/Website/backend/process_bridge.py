"""
process_bridge.py - bridge between Flask and the REAL ForgeOS C program.

For every request we start the compiled ShellForge binary (Shellforge/forgeos),
feed it ONE validated command on stdin, then "exit". ShellForge calls
NanoKernel, which really does fork() -> execvp() -> waitpid(). We then parse
the lines NanoKernel printed. Nothing here is simulated and no shell is used.
"""
import re
import shutil
import subprocess
from pathlib import Path

ROOT = Path("/home/harshika/SystemForge_OSSP/ForgeOS")
SHELL_DIR = ROOT / "Shellforge"
BINARY = SHELL_DIR / "forgeos"
SOURCES = [SHELL_DIR / "shell.c", ROOT / "NanoKernel" / "process.c", ROOT / "NanoKernel" / "process.h"]
TIMEOUT_SECONDS = 10

# ---- Security: allowlist ---------------------------------------------------
FLAG = re.compile(r"^-[A-Za-z0-9]{1,8}$")          # e.g. -l  -la  -h  -a
WORD = re.compile(r"^[A-Za-z0-9_.,:@+=-]{1,40}$")  # echo words (no / $ ; | etc.)
PS_ARG = re.compile(r"^-?[A-Za-z]{1,6}$")          # ps  ps -e  ps aux

ALLOWED = {
    "pwd":      {"desc": "Print working directory", "args": None,   "examples": []},
    "ls":       {"desc": "List directory contents", "args": FLAG,   "examples": ["-l", "-la"]},
    "whoami":   {"desc": "Current user name",       "args": None,   "examples": []},
    "id":       {"desc": "User and group IDs",      "args": None,   "examples": []},
    "hostname": {"desc": "Machine host name",       "args": None,   "examples": []},
    "date":     {"desc": "Current date and time",   "args": None,   "examples": []},
    "uname":    {"desc": "Kernel information",      "args": FLAG,   "examples": ["-a"]},
    "uptime":   {"desc": "System uptime",           "args": None,   "examples": []},
    "df":       {"desc": "Disk usage",              "args": FLAG,   "examples": ["-h"]},
    "free":     {"desc": "Memory usage",            "args": FLAG,   "examples": ["-h"]},
    "ps":       {"desc": "Running processes",       "args": PS_ARG, "examples": ["aux", "-e"]},
    "echo":     {"desc": "Print text",              "args": WORD,   "examples": ["hello"]},
}

BLOCKED = {"rm", "sudo", "su", "shutdown", "reboot", "poweroff", "halt", "mkfs", "dd", "kill",
           "pkill", "killall", "chmod", "chown", "mv", "cp", "cd", "nano", "top", "less", "gedit"}

# Anything that could act as a shell operator, quote, expansion or control character.
META = set(";&|<>`$(){}[]\\\"'*?!#~\n\r\t\x00")


class CommandRejected(Exception):
    pass


def list_commands():
    return [{"command": c, "description": v["desc"], "examples": v["examples"]} for c, v in ALLOWED.items()]


def normalize_arguments(arguments):
    """Accept a list or a single string; return a clean list of words."""
    if arguments is None:
        return []
    if isinstance(arguments, str):
        arguments = arguments.split()
    if not isinstance(arguments, list) or len(arguments) > 8:
        raise CommandRejected("Arguments must be a short list of words.")
    return [str(a) for a in arguments]


def validate(command, arguments):
    if not isinstance(command, str) or not command.strip():
        raise CommandRejected("No command given.")
    command = command.strip()
    if any(ch in META for ch in command) or " " in command:
        raise CommandRejected("Command contains forbidden characters.")
    if command in BLOCKED:
        raise CommandRejected(f"'{command}' is blocked: it is dangerous or not allowed in the web demo.")
    if command not in ALLOWED:
        raise CommandRejected(f"'{command}' is not on the allowlist. Allowed: {', '.join(ALLOWED)}.")
    args = normalize_arguments(arguments)
    rule = ALLOWED[command]["args"]
    for a in args:
        if any(ch in META for ch in a):
            raise CommandRejected(f"Argument {a!r} contains a shell metacharacter, which is not allowed.")
        if rule is None:
            raise CommandRejected(f"'{command}' does not accept arguments here.")
        if not rule.match(a):
            raise CommandRejected(f"Argument {a!r} is not allowed for '{command}'.")
    return command, args


# ---- Binary management -----------------------------------------------------
def ensure_binary():
    """Compile with the project's own gcc command if the binary is missing or older than the sources."""
    newest = max(p.stat().st_mtime for p in SOURCES if p.exists())
    if BINARY.exists() and BINARY.stat().st_mtime >= newest:
        return True, "ready"
    r = subprocess.run(["gcc", "-Wall", "-Wextra", "shell.c", "../NanoKernel/process.c", "-o", "forgeos"],
                       cwd=SHELL_DIR, capture_output=True, text=True)
    if r.returncode != 0:
        return False, r.stderr.strip() or "gcc failed"
    return True, "compiled"


def _launcher():
    # When ShellForge writes to a pipe, glibc fully buffers printf, so the child's lines are lost at
    # execvp() and the order is wrong. stdbuf -oL (line-buffered) keeps your C code untouched, the output
    # real, and every line atomic, so parent and child can never interleave in the middle of a line.
    if shutil.which("stdbuf"):
        return ["stdbuf", "-oL", "-eL", str(BINARY)]
    return [str(BINARY)]


# ---- Parsing the real NanoKernel output -------------------------------------
RX = {
    "parent": re.compile(r"\[NanoKernel\] Parent PID\s*:\s*(\d+)"),
    "child":  re.compile(r"\[NanoKernel\] Child PID\s*:\s*(\d+)"),
    "normal": re.compile(r"\[NanoKernel\] Normal termination\s*:\s*(YES|NO)"),
    "exit":   re.compile(r"\[NanoKernel\] Exit status\s*:\s*(-?\d+)"),
    "signal": re.compile(r"\[NanoKernel\] Signal\s*:\s*(\d+)"),
    "result": re.compile(r"\[NanoKernel\] Result\s*:\s*(.+?)\s*$"),
}


def parse(raw):
    # Defensive: if two processes ever wrote into the same line, split at the log tags.
    raw = re.sub(r"(?<=[^\n])(\[(?:NanoKernel|ShellForge)\])", r"\n\1", raw)
    start = raw.find("[ShellForge] Command received")
    if start < 0:
        return None
    lines = raw[start:].splitlines()
    # stop after the Result line (drops the next prompt / shutdown banner)
    cut = next((i for i, l in enumerate(lines) if l.startswith("[NanoKernel] Result")), len(lines) - 1)
    lines = lines[: cut + 1]

    info = {"parent_pids": [], "child_pids": [], "child_ppid": None}
    stages = dict.fromkeys(["request", "fork", "child_created", "exec", "running", "waiting",
                            "terminated", "status", "complete"], False)
    created_at = None
    out_lines, exec_failed = [], False
    for i, l in enumerate(lines):
        if "Request received" in l:
            stages["request"] = True
        elif "Creating child process" in l:
            stages["fork"] = True
        elif "Child process created" in l:
            stages["child_created"] = True
            created_at = i
        elif "Executing using execvp()" in l:
            stages["exec"] = True
        elif "Parent waiting for child" in l:
            stages["waiting"] = True
        elif "Child process terminated" in l:
            stages["terminated"] = True
        elif "execvp failed" in l:
            exec_failed = True
        m = RX["parent"].search(l)
        if m:
            info["parent_pids"].append(int(m.group(1)))
            # the "Parent PID" line printed by the CHILD (after "Child process created") is its getppid()
            if created_at is not None and i > created_at:
                info["child_ppid"] = int(m.group(1))
        m = RX["child"].search(l)
        if m:
            info["child_pids"].append(int(m.group(1)))
        for key in ("normal", "exit", "signal", "result"):
            m = RX[key].search(l)
            if m:
                info[key] = m.group(1)
        if not l.startswith("[NanoKernel]") and not l.startswith("[ShellForge]"):
            out_lines.append(l)
    stages["running"] = stages["exec"] and not exec_failed
    stages["status"] = "normal" in info
    stages["complete"] = "result" in info

    if not info["parent_pids"] or not info["child_pids"]:
        return None
    return {
        "parent_pid": info["parent_pids"][0],
        "child_pid": info["child_pids"][0],
        "child_ppid": info["child_ppid"],
        "stages": stages,
        "normal_termination": info.get("normal") == "YES",
        "exit_status": int(info["exit"]) if "exit" in info else None,
        "signal": int(info["signal"]) if "signal" in info else None,
        "result": info.get("result", "UNKNOWN"),
        "output": "\n".join(out_lines).strip("\n"),
        "terminal_output": "\n".join(lines).strip("\n"),
        "exec_failed": exec_failed,
    }


def run_command(command, arguments):
    command, args = validate(command, arguments)
    ok, msg = ensure_binary()
    if not ok:
        raise RuntimeError("Could not build the ForgeOS C program:\n" + msg)
    stdin = " ".join([command] + args) + "\nexit\n"
    try:
        p = subprocess.run(_launcher(), input=stdin, cwd=SHELL_DIR, text=True, errors="replace",
                           stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=TIMEOUT_SECONDS)
    except subprocess.TimeoutExpired:
        raise RuntimeError(f"ForgeOS did not finish within {TIMEOUT_SECONDS}s and was stopped.")
    parsed = parse(p.stdout)
    if parsed is None:
        raise RuntimeError("Could not read NanoKernel output from the C program.")
    parsed.update({
        "success": parsed["result"] == "SUCCESS",
        "command": " ".join([command] + args),
        "state": "TERMINATED",
    })
    return parsed
