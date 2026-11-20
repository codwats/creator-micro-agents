# Creator Micro keymap

Full key reference for the Work Louder Creator Micro (v1) running the `agents` QMK keymap. Layer 0 is for working with Rex and AI agents. Layer 1 is for basic use, layer 2 for design apps and layer 3 for utilities.


## Physical layout

```
 [wheel]  [key]   [key]   [knob]
 [key]    [key]   [key]   [key]
 [key]    [key]   [key]   [key]
 [touch]  [key]   [key]   [touch]
```

The wheel (top-left) scrolls and clicks. The knob (top-right) turns. Its press is mapped, but it may have no switch.
Touch pads are the bottom-left and bottom-right.

## Layer 0: Agents

Every Rex shortcut is **Hyper** (Ctrl+Opt+Shift+Cmd) plus a key.

| | Col 1 | Col 2 | Col 3 | Col 4 |
|---|---|---|---|---|
| **Top** | Wheel click: go to pane needing input (Hyper+R) | Command palette (Hyper+P) | Session picker (Hyper+S) | Knob press: play/pause |
| **Row 2** | Switch to Rex (Alt+2, window manager) | Split pane (Hyper+L) | Next pane (Hyper+F) | Zoom pane (Hyper+Z) |
| **Row 3** | Types `yes to all` + Enter | Types `n` + Enter (deny) | Right arrow (accept Claude suggestion) | Types `git switch main && git pull` (no Enter) |
| **Row 4** | Touch pad: switch to browser (Alt+1, window manager) | Dictate (Alt+Space) | Enter | Touch pad: next layer |

Encoders on layer 0:

| Control | Turn left | Turn right |
|---|---|---|
| Wheel | Previous session (Hyper+Left) | Next session (Hyper+Right) |
| Knob | Volume down | Volume up |

### Rex actions behind the Hyper keys

| Key | Rex action |
|---|---|
| Hyper+R | `goto_blocked`, a custom action in `init.lua` (wheel click). It jumps to the next pane whose agent needs you: first panes asking for permission or an answer (blocked), then panes whose agent finished its turn in the last hour (done). Each click moves to the next one. |
| Hyper+P | `client.palette.toggle` |
| Hyper+S | `session.switch` |
| Hyper+N | `session.new` (bound in Rex, no key assigned) |
| Hyper+B | `client.tab.new` (bound in Rex, no key assigned) |
| Hyper+Z | `pane.zoom` |
| Hyper+F | `pane.focus_next` |
| Hyper+L | `pane.split.auto` |
| Hyper+Left / Right | `session.previous` / `session.next` (wheel) |
| Hyper+, / . | `client.tab.previous` / `client.tab.next` (bound in Rex, no key assigned) |

Approve and Deny type into whichever pane has focus. Click the wheel first to jump to the blocked pane.

## Layer 1: Basics

| | Col 1 | Col 2 | Col 3 | Col 4 |
|---|---|---|---|---|
| **Top** | Wheel click: play/pause | Emoji picker (Ctrl+Cmd+Space) | Spotlight (Cmd+Space) | Knob press: none |
| **Row 2** | Copy | Paste | Cut | Undo |
| **Row 3** | Select all | Find | Save | Screenshot (Cmd+Shift+4) |
| **Row 4** | Paste without formatting | Dictate (Alt+Space) | Enter | Touch pad: next layer |

Encoders: wheel = volume, knob = scroll.

## Layer 2: Design (InDesign and Illustrator)

| | Col 1 | Col 2 | Col 3 | Col 4 |
|---|---|---|---|---|
| **Top** | Wheel click: fit page (Cmd+0) | Selection (V) | Direct selection (A) | Knob press: none |
| **Row 2** | Type (T) | Pen (P) | Rectangle (M) | Eyedropper (I) |
| **Row 3** | Group (Cmd+G) | Ungroup (Cmd+Shift+G) | Outline text (Cmd+Shift+O) | Hide guides (Cmd+;) |
| **Row 4** | Hide panels (Tab) | Bring to front (Cmd+Shift+]) | Send to back (Cmd+Shift+[) | Touch pad: next layer |

Encoders: wheel = zoom out / in (Cmd+- / Cmd+=), knob = undo / redo.

## Layer 3: Utility

| | Col 1 | Col 2 | Col 3 | Col 4 |
|---|---|---|---|---|
| **Top** | Wheel click: mute | **RESET** | empty | Knob press: none |
| **Row 2** | Lights on/off | Lighting mode | Hue down | Hue up |
| **Row 3** | Brightness down | Brightness up | Previous track | Next track |
| **Row 4** | Lock screen (Ctrl+Cmd+Q) | Mission Control | Play/pause | Touch pad: back to layer 0 |

Encoders: wheel = volume, knob = display brightness.
The lighting keys change the underglow. The status watcher resets the color within a couple of seconds.

## Layer switching

The bottom-right touch pad moves to the next layer on every layer: 0, 1, 2, 3, then back to 0.

| On layer | Pad sends | Indicator LED |
|---|---|---|
| 0 | `TO(1)` | none |
| 1 | `TO(2)` | LED 1 |
| 2 | `TO(3)` | LED 2 |
| 3 | `TO(0)` | LED 3 |

Set indicator brightness (0 to 5) in VIA under **Lighting > Indicators**. The board saves it. The indicators come back on after the Mac wakes from sleep.

## Keycode names for this firmware

The original layout file came from the stock firmware, which uses older keycode numbers. On this firmware, use these names in the layout file:

| Use | Not | Why |
|---|---|---|
| `TO(1)`, `TO(2)`, `TO(3)`, `TO(0)` | `TO(17)`, `TO(18)`, `TO(19)`, `TO(16)` | The old file wrote layer 1 as `TO(17)`. This firmware reads it as layer 17, which doesn't exist, so the pad did nothing. |
| `UG_TOGG`, `UG_NEXT`, `UG_HUED`, `UG_HUEU`, `UG_VALD`, `UG_VALU` | `RGB_TOG`, `RGB_MOD`, `RGB_HUD`, `RGB_HUI`, `RGB_VAD`, `RGB_VAI` | VIA loaded the old names as empty keys. |
| `KC_RGHT` | `KC_RIGHT` | VIA loaded `KC_RIGHT` as an empty key. |

Shortcuts with more than one modifier, such as Cmd+Shift+G, are VIA macros, not nested keycodes.

## Status lights

`status.py` checks every Rex pane every 2 seconds. It sends the most urgent state as a color to the keys and the underglow.

| State | Color |
|---|---|
| blocked (needs input) | Amber |
| error | Red |
| done (for 10 minutes) | Green |
| working | Blue |
| idle | Dim white |

## Entering the bootloader

Go to layer 3 (press the layer pad three times from layer 0), then press RESET (top row, col 2).

The bootloader key is the wheel click (top-left). Holding the wheel click while plugging in should work, but it also clears the saved keymap, so prefer RESET.

Flashing steps are in [README.md](README.md#flash).
