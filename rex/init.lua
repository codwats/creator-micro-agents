-- Creator Micro agent layer: Hyper (ctrl+alt+shift+cmd) + key
local h = "ctrl+alt+shift+cmd+"
for key, action in pairs({
  p = "client.palette.toggle", s = "session.switch",
  n = "session.new", b = "client.tab.new", z = "pane.zoom",
  f = "pane.focus_next", l = "pane.split.auto",
  left = "session.previous", right = "session.next",
  ["[Comma]"] = "client.tab.previous", ["[Period]"] = "client.tab.next",
}) do rex.bind(h .. key, action) end

-- Wheel click (Hyper+R): jump to a pane whose agent needs you.
-- Panes asking for permission or an answer ('blocked') come first; if there are none,
-- panes whose agent finished its turn ('done'). Each press moves to the next one.
local last_blocked
local DONE_MAX_AGE = 3600 -- skip finished panes idle longer than this (seconds)

-- seconds since an RFC 3339 timestamp such as 2026-11-20T08:16:02.78-07:00
local function age(ts)
  local y, mo, d, h, mi, se, sign, oh, om = (ts or ""):match("(%d+)-(%d+)-(%d+)T(%d+):(%d+):(%d+)[%.%d]*([Z+-])(%d*):?(%d*)")
  if not y then return math.huge end
  local tz = os.time() - os.time(os.date("!*t")) -- local offset; ponytail: off by an hour across DST changes
  local t = os.time{ year = y, month = mo, day = d, hour = h, min = mi, sec = se } + tz
  if sign ~= "Z" then t = t - (sign == "-" and -1 or 1) * (tonumber(oh) * 3600 + tonumber(om) * 60) end
  return os.time() - t
end
rex.action{
  name = "goto_blocked",
  title = "Go to Pane Needing Input",
  run = function(ctx, args)
    local by_state = { blocked = {}, done = {} }
    for _, s in ipairs((rex.call("session.list") or {}).sessions or {}) do
      -- other sessions can't be read until this stream attaches to them
      rex.call("session.attach", { session_id = s.session_id })
      for _, b in ipairs((rex.call("session.list_blocks", { session_id = s.session_id }) or {}).blocks or {}) do
        local st = rex.call("block.method", { session_id = s.session_id, block_id = b.block_id, method = "program_status" })
        for _, r in ipairs((st or {}).records or {}) do
          local list = by_state[r.state]
          if r.state == "done" and age(r.updated_at) > DONE_MAX_AGE then list = nil end
          if list then list[#list + 1] = { s = s.session_id, b = b.block_id }; break end
        end
      end
    end
    local found = #by_state.blocked > 0 and by_state.blocked or by_state.done
    if #found == 0 or (args and args.dry) then return { found = #found } end
    local pick = found[1]
    for i, f in ipairs(found) do
      if f.b == last_blocked then pick = found[i % #found + 1] end
    end
    last_blocked = pick.b
    rex.call("session.focus_block", { session_id = pick.s, block_id = pick.b })
    rex.client.queue("session.select", { session_id = pick.s })
    return { found = #found, session_id = pick.s, block_id = pick.b }
  end,
}
rex.bind(h .. "r", "goto_blocked")
