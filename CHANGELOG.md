# Changelog

## 2.0.0

- Replaced the single-window Qt 5 implementation with a Qt 6/CMake application.
- Added asynchronous multi-adapter UDP discovery and typed response parsing.
- Added one non-blocking, reconnecting TCP connection per device.
- Added capability-aware power, brightness, RGB, HSV, and temperature controls.
- Added live state notifications, command tracking, timeouts, and queue coalescing.
- Added local effects, schedules, persistence, themes, logs, and diagnostics.
- Added manual IP entry and remembered-device restoration.
- Added platform CI, protocol tests, mock-device tests, and LAN-only policy checks.
- Removed legacy socket ownership, blocking network calls, qmake files, and binaries.

## 1.0.0

- Preserved as Git tag `legacy-v1`.

