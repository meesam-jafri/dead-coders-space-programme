# DEAD CODERS: SPACE MISSION

A 2D sci-fi space exploration game built with **C++17 and raylib** for the NASA Space Apps Challenge.

The player selects a mission, spacecraft, ground team, scientific payloads, rocket, orbit, and launch date; watches the launch; operates the spacecraft; and receives a saved mission report. The game teaches spacecraft engineering through **simple choices and visible consequences**, not complicated minigames.

> **For Codex:** The hand-drawn storyboard is the main reference for screen composition, layout, navigation, and UX. Follow those drawings closely. The instructions below clarify implementation details and **override older drafts** where they conflict. Do not redesign this as a corporate dashboard.

## 1. Priorities and coding rules

- **Make a real, polished 2D game**, not a collection of plain text panels. Use clean shapes, smooth animations, consistent spacing, subtle shadows, and restrained sci-fi colors.
- **Implement the START SCREEN title now**: `DEAD CODERS` / `SPACE MISSION`. It is not a placeholder or future task.
- **Follow the paper storyboard** for every screen. Keep the three-column payload layout, mission cards, team portraits, calendar, status/telemetry panels, and paper-like report.
- The current playable content is the **easy Lunar Mission**. Show Mars and Jupiter as locked until their missions are implemented.
- Maintain the modular C++ structure in `src/` and shared declarations in `include/game.hpp`. Update the relevant module, not a new monolithic source. Preserve supplied assets. There is one game entry point; model tests have a separate test entry point.
- Use existing C++17, raylib, and standard-library features. Keep the virtual 16:9 canvas, correct scaled mouse input, resizing, and F11 fullscreen.
- Keep working systems and previous selections intact when navigating BACK/NEXT. Compile and fix errors rather than claiming untested success.

## 2. Art direction

The look is **simple, clean, polished, and game-like**: **dark navy (#0F1B26) backgrounds, warm cream (#F5E8D1) text, teal (#20A5A6) selections and success, and coral (#DD5639) primary actions and warnings.** Use lighter/darker shades for readable contrast. Preserve the current spacecraft, planet and scientific artwork colors. Do not use red for successful installations. Spacecraft need distinctive silhouettes and mechanical detail: layered hulls, segmented arrays, antennas, telescope housings, vents, docking structures and engine clusters. Keep the selected craft static when payload choices change. Use the attached sketches to decide where objects and buttons belong.

- Use rounded or carefully shaped panels, good contrast, gentle highlights, hover scaling, and click feedback.
- Keep the important artwork large and the explanatory text short. Explain engineering choices in a short, opaque tooltip drawn above the screen. Hover an info icon for a preview; click to pin, then click elsewhere to close.
- Prefer actual supplied assets over generic placeholder boxes. If an optional asset is absent, use a clean shape-based fallback; never crash.
- **COST remains visible throughout planning** (spacecraft, team, payload, rocket, orbit, and date). The total must keep updating rather than restarting on each screen. Clicking it may show a simple breakdown.
- Every planning screen needs **BACK** and **NEXT/CONFIRM** controls, with a clear reason whenever NEXT is disabled.

## 3. Fonts — use the fonts already in `assets/font/`

**Use the actual supplied fonts, not the default raylib font for everything.** Different fonts have different jobs:

| Purpose | Font file | Usage |
| --- | --- | --- |
| **Main START SCREEN title** | `LIQUIDNIGHT-BF64bf9967d3b8f.ttf` | Large `DEAD CODERS` / `SPACE MISSION` title; distinctive futuristic display font |
| Screen headings / section titles | `PolarisBold.ttf` | `SELECT MISSION`, `SELECT SHIP`, `SELECT TEAM`, `SELECT PAYLOAD`, etc. |
| Buttons / navigation / important labels | `PolarisSemibold.ttf` | PLAY, SETTINGS, INFO, BACK, NEXT, COST, tabs |
| Normal UI / tooltips / descriptions / telemetry | `PolarisRegular.ttf` | Readable body text, measurements, explanations, warnings |
| **Mission report body text** | **`Tigreal Rough.ttf`** | **Mandatory:** report paragraphs, event descriptions, discoveries, lessons, and recommendations |
| Mission report title and section headings | `PolarisBold.ttf` | Clear heading hierarchy against the paper background |

Other variants are available in the same folder: `Polaris Bold.otf`, `Polaris Semibold.otf`, `Polaris.otf`, `Tigreal Rough.otf`. Prefer the `.ttf` versions above; use an available equivalent if one fails to load.

**Font implementation:**
- Load fonts **once** using raylib `LoadFontEx()`. Use `DrawTextEx()` / `MeasureTextEx()` so text is positioned, centered, and wrapped correctly; unload on exit.
- Suggested virtual-canvas sizes: main title **58–76 px**, screen titles **32–42 px**, buttons **22–26 px**, body/tooltip **18–22 px**, report body **20–23 px** (adjust to the font's actual readability).
- Give **Tigreal Rough** enough size and line spacing so the report remains legible. Do not use it for small telemetry numbers or compact buttons.
- Keep font filenames and spaces exactly as shown. Fall back gracefully if a font is unavailable.
- For icons, use image assets or simple graphics rather than assuming a font contains special icon glyphs.

## 4. Existing asset folders

```
assets/
  background/
  buttons/
  cursor/
  font/
  jupiter/
  kenney-ui-pack-space-expansion/
  mars/
  moon/
    1.jpg
    2.jpg
    3.jpg
  sound/
    brown noise.mp3
    button click.mp3
    error sound.mp3
    mission complete sound.mp3
    rocket launch sound.mp3
    payloadadded.wav
```

These are the **known** folders/files from the current screenshots. Other existing files may also be present; inspect the directories before choosing specific asset names. Do not invent file paths for an icon or picture that has not been confirmed.

### Kenney UI Pack — Space Expansion

Use `assets/kenney_ui-pack-space-expansion/` as a **visual resource for game UI**:

- Inspect its actual images/atlases first, then choose suitable buttons, icons, tabs, arrows, bars, frames, and panel decorations.
- Reuse matching elements across screens to make the interface cohesive. Use neutral Kenney frames tinted to the navy/cream palette, with teal selected states and coral primary actions.
- Use `DrawTexturePro()` for image sprites and source rectangles where appropriate. If a panel can be resized cleanly with nine-slice rendering, use `DrawTextureNPatch()` where compatible with the installed raylib version.
- Preserve transparency, aspect ratios, and readable text. **Do not stretch decorative corners or use mismatched assets just to use the pack.**
- `assets/buttons/` and `assets/cursor/` may also contain useful custom elements; inspect them before drawing replacements.

### Sound mapping

| Asset | Use |
| --- | --- |
| `button click.mp3` | Button clicks and selections |
| `error sound.mp3` | Invalid selection, failed validation, mission warning |
| `payloadadded.wav` | Add payload; optionally a softer variation for removing it |
| `rocket launch sound.mp3` | Launch cinematic |
| `mission complete sound.mp3` | Mission completion / results reveal |
| `brown noise.mp3` | Optional very quiet space/control-room ambience |

Load sounds once; do not restart a sound every frame. Respect the SETTINGS sound toggle. Keep audio levels modest. Do not fail to launch if an optional sound is missing.

### Moon report images

Use **`assets/moon/1.jpg`**, **`assets/moon/2.jpg`**, and **`assets/moon/3.jpg`** in the **Mission Report** for now.

- Show 1–3 images in clean frames with short captions or source labels if known.
- They are **reference/illustration images**, not automatically pictures captured by the player's fictional spacecraft.
- **Do not invent NASA credits or scientific claims**; only attribute them to NASA if their actual source is confirmed.
- Crop/fit neatly without stretching; if an image is missing, show a subtle placeholder and continue.

## 5. Screens — follow the paper storyboard

1. **START SCREEN:** Starfield, Earth rising partly from bottom of screen, small rocket/ship detail, and the large `DEAD CODERS` / `SPACE MISSION` title at the top-center. Tagline `Build. Launch. Explore.`; **PLAY, SETTINGS, INFO** arranged vertically. Apply **Liquidnight to the main heading now**, with polished title hierarchy and subtle motion.
2. **SELECT MISSION:** Three illustrated cards **Moon / Mars / Jupiter**; only Moon unlocked. Hover smoothly expands one card over 0.3 seconds to reveal its objective, difficulty, challenge and reward; leaving restores all three cards. Mars/Jupiter expansion is preview only. BACK and NEXT.
3. **SELECT SHIP:** Three visually different ships (**Scout / Explorer / Carrier**) with cost, capacity, and capability. Selecting highlights a ship. Keep persistent COST and BACK.
4. **SELECT TEAM:** Four portraits (**Doctor / Engineer / Scientist / Pilot**) with plus/minus selection, cost, and team count. The Lunar Mission is robotic: selected specialists work from **mission control**. Doctor is mainly relevant to future crewed missions.
5. **SELECT PAYLOAD:** **Left:** mouse-wheel list with +/− and a visible draggable scrollbar (track clicks and drag supported). **Center:** large preview of chosen ship. **Right:** checklist and resource totals. **IMPORTANT UPDATED RULE BELOW:** ship artwork is static when payloads change.
6. **SELECT ROCKET:** Select one of three simple rockets; cost and payload mass determine compatibility. No rocket-building minigame.
7. **SELECT ORBIT:** Choose a basic lunar orbit profile from highlighted paths/options. Keep it visual and straightforward; do not make an orbital-physics puzzle.
8. **SELECT LAUNCH DATE:** Real Gregorian month grid with Monday-Sunday headings, correct month lengths/leap years, previous/next month controls, ISO selected date, weather symbols, wind speeds, transfer suitability and clear coral blocked-day labels. Weather is fictional deterministic data, never a live forecast. Perform a short automatic readiness check.
9. **LAUNCH SEQUENCE:** Countdown, ignition, liftoff, simple stage separation, spacecraft deployment, and transition to lunar orbit. Include launch audio and simplified telemetry. Allow pause/skip without breaking mission state.
10. **MISSION OPERATIONS:** Cinematic camera-follow view: **spacecraft fixed large in foreground**, huge **curved Moon horizon below** with lunar terrain visibly passing beneath it; subtle drifting stars. **No full Moon globe or visible orbit diagram.** Left STATUS panel; upper-right TELEMETRY; prominent MISSION TIME and TIME LEFT; pause and speed ×1/×3/×5. The easy Lunar Mission has at most **1–2 simple random issues**, with quick clear choices (such as low battery or camera reset).
11. **MISSION REPORT + ARCHIVE:** Scrollable cream-paper dossier. **Tigreal Rough for report text**, readable heading font, SUCCESS/FAILURE stamp, photos `1.jpg`–`3.jpg`, mission summary, choices, timeline, problems/responses, science returned, performance, brief NASA lessons, and improvement tips. Save reports locally and let the player reopen earlier attempts.

## 6. IMPORTANT CHANGE — payload selection does NOT change ship artwork

**This explicitly replaces the older instruction to visually attach equipment to the spacecraft.**

- The **selected ship picture remains exactly the same** as payloads are added/removed. Do **not** draw additional solar panels, cameras, antennas, modules, or other parts onto the spaceship.
- Still show the ship prominently in the **center panel** to keep the layout from the paper storyboard.
- When the player clicks **+** or **−**, update **only the text/UI/data**: selected-item highlight, checklist, number of items, **COST**, mass, electrical power, battery/storage capacity, available slots, and brief descriptions.
- Play `payloadadded.wav` for a successful addition and give a small UI animation/checkmark. A simple count/tick animation is enough.
- Each component still has actual **gameplay effects**: cameras/sensors collect data, batteries store energy, solar panels generate power, antennas transmit data, etc. The **visual ship sprite does not change**, but mission calculations **do**.
- Validate required equipment, ship mass limit, budget, power, and slots. Show an understandable error if an invalid configuration is attempted.
- Optional slots may be empty. BACK and NEXT preserve the selection.

**Example:** Adding a camera updates the checklist to `Camera ✓`, adds its mass/cost/power to the totals, and enables camera observations during the mission. **Do not attach a camera graphic to the ship preview.**

## 7. Mission mechanics — keep them simple

- Ship + team + payloads + rocket contribute to **total cost**. Payloads contribute to total mass, slots, power requirements, and science capabilities.
- During mission operations: battery energy is stored in **Wh**, electrical load/generation measured in **W**; sunlight affects charging; science fills storage; data is returned when communication is possible.
- Use the selected payloads to decide which research appears in the report. Do not show camera discoveries if the player did not install a camera.
- Limit easy Lunar Mission operational events to **1–2 total**, chosen randomly from appropriate simple scenarios. Avoid repetitive or unavoidable failures.
- Make the timer and **TIME LEFT** clearly visible. Time scaling changes simulation and visual movement together.
- Distinguish real engineering principles from simplified fictional prices, failure chances, rocket statistics, and scores.

## 8. Detailed mission report and saved archive

Design this to look like the **paper report drawn in the storyboard**, not a dark software dashboard.

- Cream/off-white document, subtle paper texture if available, dark text, tidy separators, wide margins; scroll within the report.
- **Body text: `Tigreal Rough.ttf`** throughout the report. Heading font may be `PolarisBold.ttf`.
- Include: mission/date/result, ship/team/payload/rocket/orbit/date, cost/mass/power, mission elapsed time, science collected and returned, chronological launch/operations event log, problems and player responses, health, score, and 2–3 specific lessons/recommendations.
- Use `assets/moon/1.jpg`, `2.jpg`, and `3.jpg` as illustrative images, with short captions and no fabricated attribution.
- Make score secondary to clear success/failure reasons. **No hard-coded fake results** in the final report; derive text and numbers from actual GameState.
- Save an individual report snapshot per completed attempt, including events and selections, using a **simple local file with C++ file I/O**; no database required. Show **newest first** in an archive. Reports should remain after restarting the game.
- Provide **MAIN MENU**, **RETRY/REDESIGN**, **ARCHIVE**, and **NEXT MISSION** only if a next mission is genuinely implemented.

## 9. NASA CubeSat research — simple integration

Use *NASA CubeSat 101: Basic Concepts and Processes for First-Time CubeSat Developers (2017)* to support the game's existing rules, **not** to add heavy minigames.

- **Mass/slots:** More hardware uses spacecraft capacity.
- **Power/battery:** Instruments consume electricity; solar helps recharge; eclipse removes solar generation.
- **Communication/storage:** Data must be stored and transmitted; a communication blackout is not automatically a hardware failure.
- **Readiness:** Quick automatic prelaunch validation instead of manual testing minigames.
- **Learning:** One-sentence tooltips and 2–3 short report lessons tied to player decisions.

Do not describe fictional numerical parameters as official NASA data. The 2017 guide is an educational reference, not a claim about today's regulations.

## 10. What Codex should implement first

1. **Start page now:** actual **Liquidnight** title, good title positioning, tagline, star/Earth atmosphere, and PLAY/SETTINGS/INFO buttons matching the sketch.
2. **Fonts across all screens:** Polaris hierarchy and **Tigreal Rough report body**; correct readable sizes and text wrapping.
3. **Kenney UI assets and audio:** inspect real contents, reuse consistently where useful, and hook up the known sound files.
4. **Payload UI change:** keep ship sprite static; update checklist, cost, mass, power, requirements, and successful-add sound only.
5. **Mission report:** use the three Moon JPEGs, actual recorded gameplay data, Tigreal Rough body text, and persistent archive.
6. Preserve the complete playable Lunar Mission, cinematic near-Moon camera framing, and all existing controls/navigation.

### Definition of done

The game compiles, remains playable from START to REPORT, matches the hand-drawn UI layouts, has a working large start-page title, uses the specified fonts and available art/sound assets, keeps the ship unchanged during payload selection, saves detailed readable reports, and does not break BACK/NEXT, COST, mission time, or fullscreen behavior.

## Current polish — navy / cream / teal / coral

- **Currency:** all current planning screens and new report snapshots use `$` / USD formatting (for example `$1,200`). These are fictional educational dollar amounts, not real spacecraft prices. Existing archived reports keep their original wording.
- **Mission selection:** illustrated destination cards with distinct Moon/Mars/Jupiter treatments, availability badges, selection states and a concise lunar objective. Mars and Jupiter remain locked.
- **Ships:** Scout is a compact hexagonal survey bus; Explorer has a long instrument body and four arrays; Carrier has twin equipment pods, a central spine and multiple engines. Art is procedural and independent of payload installation.
- **Calendar:** launch dates use the real Gregorian calendar. Weather is reproducible per date. Wind at or above 25 km/h, lightning or a closed transfer window blocks launch. Caution dates may consume additional arrival energy. Browsing months does not clear the chosen date.
- **Time:** display simulation elapsed/remaining time separately from the accelerated mission clock. The educational conversion is 1 simulation second = 0.08 mission hours (4.8 minutes). x1/x3/x5 changes simulation progress; pausing freezes it. Do not label this as a validated physical trajectory model; a future accuracy algorithm can replace the centralized model.
- **Report:** cream paper, dark masthead, teal rules, success/failure stamp, numbered section links, performance and configuration sections, timeline, framed/captioned reference images, and a draggable document scrollbar. Values come from the saved attempt, including when reopened from archive.
- **Warnings:** use coral borders/text for failed readiness, unsuitable launch conditions, critical battery/storage/health and anomalies; include the specific reason. Successful payload additions use teal confirmation.
- **Audio:** provided button click for UI actions, payload WAV only for installation, error sound for invalid actions/anomalies, rocket sound during the cinematic, mission-complete sound at results, and low-volume looping brown-noise ambience. Sound OFF silences current playback as well as future triggers.
- **Information:** use the supplied `assets/icons/info.png` for contextual information controls. No asset file is changed; masking for display happens in memory.

### Build and checks

Use Ctrl+Shift+B, then run `build/game.exe` from the repository root. `--smoke-test` checks startup/shutdown. `--capture-stage N` renders a deterministic test fixture to `build/review/stage-N.png` without saving it into the mission archive. See `DEVELOPMENT.md` for the module map. Automated model tests include date arithmetic, readiness, resource bounds and complete mission runs; screenshots complement those checks rather than proving every interaction.
### Final UI polish

- Keep the storyboard layout and all gameplay values. Ship artwork stays unchanged when payloads are installed.
- Large shadowed Polaris Bold page headings; Liquidnight start title, Polaris Semibold controls, Polaris Regular stats, and Tigreal Rough report prose.
- COST has a bordered container, animated dollar total, brief pulse and signed price delta. Failed installation flashes/shakes only the affected equipment row and explains the restriction.
- Payload readiness uses animated checkboxes, followed by a separate aligned statistics section. Installed counts appear as badges beside the plus button; zero has no badge.
- Teal selected controls are distinct from coral primary actions, neutral controls and disabled controls. Operations has separate x1/x3/x5 buttons.
- Contextual engineering tooltips use one opaque, screen-bounded overlay with a teal border. Relevant notes say `Based on NASA CubeSat 101 (2017)`; game prices and numerical models remain fictional.
- Educational reference: [NASA CubeSat 101 (October 2017)](https://www.nasa.gov/wp-content/uploads/2017/03/nasa_csli_cubesat_101_508.pdf), including design considerations and preflight/environmental testing. This is not a source of the game's fictional mission values.
- `assets/cursor/cursor.png` is drawn at the physical pointer position after canvas scaling. It stays crisp at 32 pixels; the system cursor returns outside the canvas, on focus loss, on exit, or if loading fails.
- Optional visual review: `build/game.exe --capture-stage 4 tooltip` or `--capture-stage 1 moon` (also `mars`, `jupiter`, `default`). Captures are written under `build/review/`; they do not create mission reports.