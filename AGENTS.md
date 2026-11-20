# Agent notes

Read this before helping someone install, change or debug this project. [README.md](README.md) covers the install steps, and [KEYMAP.md](KEYMAP.md) lists every key.

## Flashing

- Get the user's explicit go-ahead before every flash. The stock firmware is read-protected and can't be restored without Work Louder's official file.
- The user must put the board in bootloader mode. Before flashing, confirm it's there with `dfu-programmer atmega32u4 get bootloader-version`. If it reports "no device present", ask the user to try again.
- After a flash, check that the board comes back as "Micro Pad" (`ioreg -p IOUSB -w0 | grep -i "micro pad"`).
- Flash is about 98% full. Put new behaviour in the VIA layout or in Rex config, and treat firmware code as the last resort.

## Changing keys

The source of truth for keys is the VIA layout, `via/creator_micro.agents.layout.json`. Edit it with these keycode names: `TO(n)` for layers, `UG_*` for underglow, `KC_RGHT` for the right arrow, and VIA macros for shortcuts with more than one modifier. Older names load as empty keys (see the KEYMAP.md section "Keycode names for this firmware").

You can also write keys straight to the board over VIA raw HID, with no reload in VIA. Use `hidapi`: VID `0x574C`, PID `0xE6E3`, usage page `0xFF60`, usage `0x61`. Each packet is 32 bytes, preceded by a report ID of `0x00`.

| Command | Bytes | Reply |
|---|---|---|
| Get key | `04 layer row col` | keycode in bytes 4–5 |
| Set key | `05 layer row col hi lo` | echo |
| Get encoder | `14 layer encoder cw` | keycode in bytes 4–5 |
| Set encoder | `15 layer encoder cw hi lo` | echo |
| Status color | `42 hue sat val` | echo. Not saved. Sets the keys and the underglow. |

- **Read back** every write and compare it to what you sent. Then update the layout JSON and KEYMAP.md to match.
- **Grid:** rows and columns are the physical 4×4 grid, starting at 0. Encoder 0 is the wheel (top-left) and encoder 1 is the knob (top-right).
- **Close the HID handle** when you're done. A process that keeps it open makes every other open fail with "open failed".

## Debugging

- **Key presses:** the VIA matrix-state read always returns zeros, because the firmware is built without `VIA_INSECURE`. To check whether a key registers, temporarily map it to a letter, such as `KC_A`, and ask the user to press it in a text field. Then restore it.
- **Rex actions and other sessions:** an action in `init.lua` can't read another session until it attaches. Call `rex.call("session.attach", { session_id = ... })`. `rex.session.attach` exists only in `rex do` scripts.
- **Testing `goto_blocked` without moving the user's view:** `rex do goto_blocked dry=true` returns the match count.
- **Pane state:** `rex block call program_status -s SESSION -b BLOCK` returns `records[].state`, one of `idle`, `working`, `done`, `blocked` or `error`. Agents spend most of their waiting time in `done`, and `blocked` is usually brief.
