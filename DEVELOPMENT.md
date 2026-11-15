# Development map

The README and assets are design inputs. Do not edit them as part of code refactors.

- `include/game.hpp`: shared types, configurable catalog, declarations and resource owners.
- `src/main.cpp`: window lifecycle, frame loop, input mapping, shared navigation.
- `src/model.cpp`: budget, mass, power totals and readiness rules.
- `src/ui.cpp`: text, panels, buttons and consistent interaction.
- `src/rendering.cpp`: procedural spacecraft, planets and resource meters.
- `src/simulation.cpp`: mission transitions, time, energy, data and events.
- `src/screens.cpp`: screen layouts and player choices.
- `src/reports.cpp`: paper dossier, safe file saving and newest-first archive.
- `src/assets.cpp`: JPEG support through the installed raylib bundled decoder.
- `src/calendar.cpp`: Gregorian date helpers, deterministic fictional weather, currency and clocks.
- `tests/model_tests.cpp`: headless readiness, simulation, bounds and persistence checks.
- `tests/ui_tests.cpp`: hidden-window tests of actual controls, payload restrictions, tooltip opacity/pinning, locked previews and cursor loading.

Build with Ctrl+Shift+B, then run `build/game.exe` from the project root.
Run `build/game.exe --smoke-test` for an eight-frame startup/shutdown check.
Run `build/tests/model_tests.exe` and `build/tests/ui_tests.exe` after rebuilding their respective test entry points against the source modules (excluding `src/main.cpp`).
Use `build/game.exe --capture-stage 4 tooltip` for a deterministic visual capture; output goes to `build/review`. Other review scenes include `default`, `moon`, `mars`, `jupiter`, `empty` and `failure`.
Reports persist in `reports/` beside the running executable. Test reports are isolated under `build/tests/reports`.

When working on a feature, read its module plus the shared header instead of the entire project.
The flight model is educational and uses fictional values. Assets are loaded once and safely released on exit.
