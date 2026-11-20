# Creator Micro for AI agents

Custom QMK firmware and a VIA layout that turn a **Work Louder Creator Micro (v1)** into a control pad for AI coding agents running in [Rex](https://www.superlogical.com/rex).

- **Agent layer:** dictation, approve and deny, accept a suggestion, switch session and pane, split and zoom, and jump to the agent that needs you.
- **Status lights:** the keys and underglow show the most urgent agent state across all Rex sessions.

| State | Color |
|---|---|
| Blocked (asking for permission or an answer) | Amber |
| Error | Red |
| Done (in the last 10 minutes) | Green |
| Working | Blue |
| Idle | Dim white |

The board also has a basics layer, a design layer (InDesign and Illustrator) and a utility layer. Every key is listed in [KEYMAP.md](KEYMAP.md).

> **Read before flashing.** This replaces the stock Work Louder firmware. The stock firmware is read-protected, so it **can't be backed up**. To go back, you need the official firmware file from Work Louder. Flash at your own risk.

## What's in here

| Path | What it is |
|---|---|
| `firmware/keymaps/agents/` | The QMK keymap: VIA, encoder map, status-color HID command, indicator brightness, layer LEDs |
| `firmware/micro.c.patch` | A one-line patch to mainline `work_louder/micro` so the encoders follow the keymap's encoder map |
| `firmware/work_louder_micro_agents.hex` | Prebuilt firmware |
| `via/creator_micro.agents.layout.json` | The VIA layout: all four layers, encoders and macros |
| `rex/init.lua` | Rex shortcuts, plus the `goto_blocked` action used by the wheel click |
| `status/` | `status.py`, which watches Rex and colors the board. Also a launchd plist (macOS) or systemd unit (Linux) to start it at login, and a udev rule (Linux). |
| `KEYMAP.md` | The full key reference |
| `AGENTS.md` | Notes for AI agents helping you install or change this |

## Requirements

- Work Louder Creator Micro **v1** (USB ID `574C:E6E3`)
- macOS or Linux. Linux support is untested. See [Linux](#linux).
- [Rex](https://www.superlogical.com/rex), with agents that report status, such as Claude Code
- `dfu-programmer` (`brew install dfu-programmer`)
- Python 3, for the status lights
- Docker or a QMK toolchain, only if you build the firmware yourself

## Install

### Flash

1. Put the board in bootloader mode. With the stock firmware, use its reset key. With this firmware, go to layer 3 (press the layer pad three times) and press the key in the top row, column 2. Or hold the wheel click (top-left) while plugging in. That second method also clears the saved keymap.
2. Check that it's there. `dfu-programmer atmega32u4 get bootloader-version` should print a version.
3. Flash it:

   ```sh
   dfu-programmer atmega32u4 erase --force
   dfu-programmer atmega32u4 flash firmware/work_louder_micro_agents.hex
   dfu-programmer atmega32u4 reset
   ```

The board comes back as "Micro Pad".

### Load the layout

Open [usevia.app](https://usevia.app) in Chrome, connect the board, and load `via/creator_micro.agents.layout.json`.

**Change these keys to suit you:**

| Key | Default | What it's for |
|---|---|---|
| Layer 0, row 2, col 1 | Alt+2 | Switch to Rex through a window manager |
| Layer 0, row 4, left pad | Alt+1 | Switch to the browser |
| Layer 0, row 3, col 1 | Types `yes to all` + Enter | Macro example |
| Layer 0, row 3, col 4 | Types `git switch main && git pull` | Macro example |

### Rex shortcuts

Copy `rex/init.lua` to `~/.config/rex/init.lua`, or merge it into yours. Then run:

```sh
rex config check && rex config reload
```

The agent keys send **Hyper** (Ctrl+Opt+Shift+Cmd) plus a key, and `init.lua` maps each one to a Rex action. The wheel click runs `goto_blocked`:

1. It jumps to the next pane whose agent is asking for permission or an answer.
2. If there are none, it jumps to agents that finished their turn in the last hour.

Each click moves to the next one.

### Status lights

```sh
cd status
python3 -m venv .venv && .venv/bin/pip install -r requirements.txt
.venv/bin/python status.py        # try it in the foreground first
```

To start it at login, copy `status/creator-micro-status.plist` to `~/Library/LaunchAgents/`. Fill in `INSTALL_DIR` and `REX_BIN_DIR`, then run:

```sh
launchctl bootstrap gui/$(id -u) ~/Library/LaunchAgents/creator-micro-status.plist
```

### Linux

Linux support is **untested**. The firmware, the layout and the Rex config don't depend on the operating system. Three things change:

1. **Permissions:** install the udev rule so your user can reach the board and its bootloader without root:

   ```sh
   sudo cp status/99-creator-micro.rules /etc/udev/rules.d/
   sudo udevadm control --reload && sudo udevadm trigger
   ```

   Then unplug the board and plug it back in.
2. **Finding the board:** nothing to do. `status.py` looks for the VIA raw-HID interface by usage page. If the HID backend reports no usage page, which libusb often does on Linux, it falls back to USB interface 1.
3. **Start at login:** use the systemd user service instead of launchd. Fill in `INSTALL_DIR` and `REX_BIN_DIR` in `status/creator-micro-status.service`, then run:

   ```sh
   cp status/creator-micro-status.service ~/.config/systemd/user/
   systemctl --user daemon-reload && systemctl --user enable --now creator-micro-status
   journalctl --user -u creator-micro-status -f   # watch its output
   ```

Install `dfu-programmer` for flashing from your package manager.

## Build the firmware yourself

```sh
git clone --depth 1 https://github.com/qmk/qmk_firmware.git && cd qmk_firmware
git submodule update --init --depth 1 lib/lufa lib/printf lib/vusb
git apply /path/to/firmware/micro.c.patch
cp -R /path/to/firmware/keymaps/agents keyboards/work_louder/micro/keymaps/
docker run --rm -v "$PWD":/qmk_firmware -w /qmk_firmware ghcr.io/qmk/qmk_cli \
  qmk compile -kb work_louder/micro -km agents
```

The firmware uses about 98% of the flash (around 380 bytes free). Put new key ideas in the VIA layout, not in firmware code.

## Pitfalls

- **Old stock layouts break on this firmware.** Stock VIA files use older keycode numbers. `TO(17)` means "layer 1" on stock firmware but "layer 17" here, so the layer pad does nothing. `RGB_*` and `KC_RIGHT` load as empty keys. Use `TO(1)`, `UG_*` and `KC_RGHT` instead. Details are in [KEYMAP.md](KEYMAP.md#keycode-names-for-this-firmware).
- **Wheel and knob:** the wheel (top-left) is encoder 0, and its click is the bootloader key. The knob (top-right) is encoder 1.
- **Lighting keys vs status lights:** the status script overrides whatever color the lighting keys set, within a couple of seconds.

## Using it with another multiplexer

Only the Rex-specific parts need replacing. The firmware and the HID color command don't care which multiplexer you use.

| Part | What changes |
|---|---|
| Firmware and `0x42` color command | Nothing |
| VIA layout | Change the Hyper keys to whatever your multiplexer can bind. Terminal-based multiplexers may not receive Cmd chords, so you may need Ctrl+Alt chords or prefix-key macros. |
| `rex/init.lua` | Replace with your multiplexer's keybinding config |
| `status.py` | Rewrite `worst()`, the function that asks Rex for pane states. Everything else stays. |
| Wheel-click jump | Needs a "focus the blocked agent" command, or a macro that opens the multiplexer's picker filtered to blocked agents |

## License

Everything outside `firmware/` is under the MIT license (see [LICENSE](LICENSE)). The firmware keymap and patch build on [QMK](https://github.com/qmk/qmk_firmware), so they are GPL-2.0-or-later. If you share the `.hex`, share this source with it.
