# Architecture

## Component diagram

```text
Application
├── SettingsRepository
├── AutomationEngine
├── DeviceManager
│   ├── DiscoveryService
│   └── DeviceController (one per device)
│       └── YeelightConnection
│           └── YeelightMessageParser
└── MainWindow
    ├── DeviceListModel
    ├── control pages
    └── AppLogger model
```

## Ownership model

`Application` owns settings, automation, the device manager, and the window.
`DeviceManager` is the only owner of device controllers. Each controller owns
one connection through Qt parent ownership. Each connection owns one TCP socket
and one stream parser. UI pages hold non-owning pointers to the selected
controller and disconnect their signals when selection changes.

## Signal flow

Discovery datagrams are parsed into `DeviceInfo` and `DeviceState`. The manager
deduplicates devices and updates the list model. UI actions call typed methods
on `DeviceController`; controllers validate capabilities and construct commands
through `YeelightCommand`. Responses and property notifications flow back
through the parser, connection, controller, and UI.

## Device lifecycle

Discovered devices use the advertised ID as stable identity, falling back to
address and port. A repeated ID updates the existing endpoint. Remembered
devices are restored offline and remain visible while connection attempts fail.
A discovered device is marked offline after 30 seconds without a response.

## Request lifecycle

Every command uses a monotonically increasing integer ID. A connection queues
commands while connecting, coalesces continuous color and brightness updates,
and tracks pending requests for three seconds. A response completes the matching
request. An error or timeout is propagated to the controller, which reverts an
optimistic state change when applicable.

## Reconnect behavior

Unexpected disconnects use delays of 1, 2, 4, 8, 15, and 30 seconds. The final
delay repeats until connection succeeds. A successful connection resets the
backoff and requests complete device state. An explicit user disconnect stops
reconnection.

## Settings storage

`QSettings` stores schema version 1, remembered devices, UI preferences,
custom effects, schedules, theme, developer mode, and window geometry.
Corrupt collections fall back to empty values instead of preventing startup.

## Automation limitations

The automation engine checks local time every 15 seconds and executes an action
at most once in its scheduled minute. It runs only while the desktop
application is open, does not replay missed actions, and does not endlessly
retry offline targets.

