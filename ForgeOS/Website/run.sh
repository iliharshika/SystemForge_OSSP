#!/usr/bin/env bash
# ForgeOS helper.  Usage:  ./run.sh setup | backend | frontend | all
set -e
ROOT="$(cd "$(dirname "$0")" && pwd)"

case "${1:-help}" in
  setup)
    echo "==> Compiling the C project (your original gcc command)"
    (cd "$ROOT/Shellforge" && gcc -Wall -Wextra shell.c ../NanoKernel/process.c -o forgeos)
    echo "==> Python virtual environment + Flask"
    python3 -m venv "$ROOT/backend/.venv"
    "$ROOT/backend/.venv/bin/pip" install -q -r "$ROOT/backend/requirements.txt"
    echo "==> Frontend packages"
    (cd "$ROOT/frontend" && npm install --no-audit --no-fund)
    echo "Setup done. Run ./run.sh backend in one terminal and ./run.sh frontend in another."
    ;;
  backend)
    cd "$ROOT/backend" && exec "$ROOT/backend/.venv/bin/python" app.py
    ;;
  frontend)
    cd "$ROOT/frontend" && exec npm run dev
    ;;
  all)
    "$0" backend & BACK=$!
    trap 'kill $BACK 2>/dev/null' EXIT INT TERM
    sleep 1
    "$0" frontend
    ;;
  *)
    echo "Usage: ./run.sh setup | backend | frontend | all"
    ;;
esac
