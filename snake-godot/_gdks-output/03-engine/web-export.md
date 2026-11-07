# Snake - M3 Web Export Checklist

> **Owner:** Érico (Engine Coordinator) · **Date:** 2026-11-07
> Implements Technical Design §9.4. The project files are ready: `export_presets.cfg` (Web preset) and `build/` in `.gitignore`.

## Already set up in the project

| Item | Value | Why |
|------|-------|-----|
| Preset | **Web** → `build/web/index.html` | Output stays out of git |
| Thread support | **Off** | Runs on any static host (itch.io, GitHub Pages) with no special server headers |
| Canvas resize policy | Adaptive | The game fills the browser window and follows rotation |
| Focus canvas on start | On | Keys work without clicking first |
| Excluded from the build | `tests/*` | The headless test script isn't part of the game |
| Renderer | GL Compatibility | Required for web (WebGL 2) |

## Steps in the Godot editor (once)

1. **Install export templates:** Editor → **Manage Export Templates…** → **Download and Install** (for 4.7). This is a one-time download of a few hundred MB.
2. Open **Project → Export…** and check that the **Web** preset appears with no red errors at the bottom. If Godot rewrites some options, that's normal.

## Test on the PC

3. Click the **Remote Debug / Run in Browser** button (the browser icon at the top right of the editor, next to the Play buttons). Godot builds and opens the game at `http://localhost:8060`.
   Opening `index.html` directly from disk **does not work**. It must be served over http.
4. Check:
   - [ ] The title screen appears, with music after the first key press or click
   - [ ] A full run works with keyboard and with mouse-drag swipes
   - [ ] Switching to another tab pauses the game
   - [ ] **Reload the page: the best score is still there**

## Test on a phone (same Wi-Fi)

5. Editor → **Editor Settings → Export → Web → HTTP Host** → `0.0.0.0` (so other devices can connect). Run in Browser again.
6. On the phone, open `http://<PC IP address>:8060/index.html`. If it doesn't load, allow Godot through the Windows Firewall for private networks.
7. Check:
   - [ ] Swipes turn reliably, including quick L-shaped double swipes
   - [ ] Tap starts, tap retries, the **II** button pauses
   - [ ] Portrait gives the 15×20 grid, landscape gives 20×15; rotating mid-run pauses
   - [ ] Sound starts after the first tap
   - [ ] **Speed cap:** play past about 30 fruits. Is 15 moves/sec still fair with swipes? If not, lower `speed_cap` in `config/game_config.tres` (try 12)
   - [ ] Best score is kept after a reload
8. Set **HTTP Host** back to `localhost` when you're done.

## Export for sharing (optional)

9. Project → Export… → **Web** → **Export Project…** → keep `build/web/index.html` → Export (uncheck *Export With Debug* for the final build).
10. To publish, zip the contents of `build/web/` and upload it, for example to itch.io as an HTML game. *(Publishing is your call; nothing is uploaded automatically.)*

## Results

| Check | PC browser | Phone |
|-------|-----------|-------|
| Runs | | |
| Input | | |
| Audio | | |
| Best score after reload | | |
| Speed cap OK | n/a | |
