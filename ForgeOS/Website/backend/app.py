"""ForgeOS Flask API.  Run:  python app.py   ->  http://127.0.0.1:5000"""
import json
import threading
from datetime import datetime
from pathlib import Path

from flask import Flask, jsonify, request
from flask_cors import CORS

import process_bridge as bridge

app = Flask(__name__)
CORS(app, origins=[r"http://localhost:\d+", r"http://127\.0\.0\.1:\d+"])

DATA_FILE = Path(__file__).resolve().parent / "data" / "history.json"
_lock = threading.Lock()


def load():
    try:
        return json.loads(DATA_FILE.read_text())
    except (OSError, ValueError):
        return []


def save(items):
    DATA_FILE.parent.mkdir(exist_ok=True)
    DATA_FILE.write_text(json.dumps(items, indent=1))


def stats(items):
    ok = sum(1 for h in items if h["result"] == "SUCCESS")
    last = items[-1] if items else None
    return {
        "total": len(items),
        "successful": ok,
        "failed": len(items) - ok,
        "last_child_pid": last["child_pid"] if last else None,
        "last_exit_status": last["exit_status"] if last else None,
        "last_result": last["result"] if last else None,
    }


@app.get("/api/health")
def health():
    ok, msg = bridge.ensure_binary()
    return jsonify(status="online" if ok else "error", binary=msg, binary_path=str(bridge.BINARY))


@app.get("/api/commands")
def commands():
    return jsonify(commands=bridge.list_commands())


@app.post("/api/execute")
def execute():
    body = request.get_json(silent=True) or {}
    try:
        res = bridge.run_command(body.get("command"), body.get("arguments"))
    except bridge.CommandRejected as e:
        return jsonify(success=False, rejected=True, error=str(e)), 400
    except RuntimeError as e:
        return jsonify(success=False, rejected=False, error=str(e)), 500

    now = datetime.now()
    entry = {"id": 0, "time": now.strftime("%H:%M:%S"), "date": now.strftime("%Y-%m-%d"),
             **{k: res[k] for k in ("command", "parent_pid", "child_pid", "child_ppid", "state",
                                     "exit_status", "result", "normal_termination", "signal",
                                     "stages", "output", "terminal_output")}}
    with _lock:
        items = load()
        entry["id"] = (items[-1]["id"] + 1) if items else 1
        items.append(entry)
        save(items[-200:])
    res["id"] = entry["id"]
    res["time"] = entry["time"]
    return jsonify(res)


@app.get("/api/history")
def history():
    with _lock:
        items = load()
    return jsonify(history=items, stats=stats(items))


@app.get("/api/processes")
def processes():
    with _lock:
        items = load()
    return jsonify(processes=list(reversed(items)))


@app.post("/api/history/clear")
def clear():
    with _lock:
        save([])
    return jsonify(history=[], stats=stats([]))


if __name__ == "__main__":
    app.run(host="127.0.0.1", port=5000, debug=False, threaded=True)
