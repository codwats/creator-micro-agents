"""Poll Rex pane status and show the most urgent one as a global color on the Creator Micro."""
import json, subprocess, time, hid
from datetime import datetime, timezone

# state -> (priority, hue, sat, val); QMK hue is 0-255
COLORS = {'blocked': (5, 21, 255, 200), 'error': (4, 0, 255, 200), 'done': (3, 85, 255, 200),
          'working': (2, 170, 255, 200), 'idle': (1, 0, 0, 40)}
DONE_FOR = 600  # seconds a 'done' stays green

def rex(*a):
    return json.loads(subprocess.run(['rex', *a, '--json'] if a[0] != 'block' or a[1] != 'call' else ['rex', *a],
                                     capture_output=True, text=True, timeout=10).stdout or '{}')

def worst():
    best = 'idle'
    for s in rex('ls').get('sessions', []):
        for b in rex('block', 'ls', '-s', s['session_id']).get('blocks', []):
            r = rex('block', 'call', 'program_status', '-s', s['session_id'], '-b', b['block_id'])
            for rec in r.get('records', []):
                st = rec.get('state', 'idle')
                if st not in COLORS: st = 'idle'
                age = (datetime.now(timezone.utc) - datetime.fromisoformat(rec['updated_at'])).total_seconds()
                if st == 'done' and age > DONE_FOR: st = 'idle'
                if COLORS[st][0] > COLORS[best][0]: best = st
    return best

def send(state):
    _, h, s, v = COLORS[state]
    for d in hid.enumerate(0x574c, 0xe6e3):
        if d['usage_page'] == 0xff60 and d['usage'] == 0x61:
            dev = hid.device(); dev.open_path(d['path'])
            dev.write(bytes([0, 0x42, h, s, v]).ljust(33, b'\0')); dev.close(); return

if __name__ == '__main__':
    last = None
    while True:
        try:
            st = worst()
            if st != last: send(st); last = st; print(st, flush=True)
        except Exception as e:
            print('error:', e, flush=True); last = None
        time.sleep(2)
