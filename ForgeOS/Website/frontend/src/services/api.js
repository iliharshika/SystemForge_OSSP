// All calls go to Flask through the Vite proxy (/api -> http://127.0.0.1:5000/api).
const BASE = import.meta.env.VITE_API_URL || '/api'

async function request(path, options) {
  let res
  try {
    res = await fetch(BASE + path, options)
  } catch {
    throw new Error('Cannot reach the ForgeOS backend. Start it with: cd backend && python app.py')
  }
  const data = await res.json().catch(() => ({}))
  if (!res.ok) {
    const err = new Error(data.error || `Request failed (${res.status})`)
    err.rejected = !!data.rejected
    throw err
  }
  return data
}

export const health = () => request('/health')
export const commands = () => request('/commands')
export const history = () => request('/history')
export const processes = () => request('/processes')
export const clearHistory = () => request('/history/clear', { method: 'POST' })
export const execute = (command, args) =>
  request('/execute', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ command, arguments: args }),
  })
