# Legacy v1 baseline

The original Qt 5/qmake application was preserved at Git tag `legacy-v1`
before the LAN-only refactor began.

## Baseline verification

- The checked-in Windows executable starts successfully and its main window
  was captured in `docs/screenshots/legacy.png`.
- A source rebuild could not be performed in the migration environment because
  neither qmake/Qt nor CMake was installed or available on `PATH`.
- The baseline contains the original single-window discovery and control
  behavior without modifications.

