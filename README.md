# Yeelight LAN

Yeelight LAN is a Qt 6 desktop application for discovering and controlling
compatible Yeelight devices directly over the local network. It does not
require a cloud account and does not include telemetry, remote Internet
control, a web server, or an automatic updater.

> Current version: **2.0.0**

## Features

- asynchronous multicast discovery across active IPv4 adapters
- manual IPv4 and IPv6 device entry
- independent persistent connections and reconnect state per device
- live power, brightness, color, temperature, effect, and reachability state
- capability-aware RGB, HSV, color temperature, brightness, and power controls
- built-in and custom local color-flow effects
- local recurring schedules while the application is running
- remembered devices, preferences, themes, effects, and schedules
- structured local logs, diagnostics, and an optional developer console

## Quick start

1. Enable LAN Control for the Yeelight device using the manufacturer's mobile
   application.
2. Connect the computer and device to the same local network.
3. Clone the repository.
4. Configure and build the project.
5. Start the application.

```sh
git clone https://github.com/CieplutkiGit/Yeelight-Control-Panel.git
cd Yeelight-Control-Panel
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Platform-specific executable paths and build details are listed below.

## Requirements

- CMake 3.21 or newer
- a C++17 compiler
- Qt 6.5 or newer with Core, Gui, Widgets, Network, and Test
- a compatible Yeelight device with LAN Control enabled

## Building

### Windows

Open a Qt-enabled developer terminal and run:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Linux

Make Qt 6.5 and a C++17 compiler available, then run:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### macOS

Qt 6.5 and CMake are required. If CMake cannot locate Qt, expose the Qt
installation through `CMAKE_PREFIX_PATH`, then run:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Running the application

On Windows:

```powershell
.\build\Release\YeelightControlPanel.exe
```

On Linux and macOS:

```sh
./build/YeelightControlPanel
```

The exact output path can vary depending on the selected generator or IDE.

1. Start Yeelight LAN.
2. Wait for compatible devices to appear in the sidebar.
3. Select a device.
4. Use the Dashboard, Color, Effects, Automations, Device, or Logs page.
5. Use **Add by IP** when multicast discovery is unavailable.

## Running tests

On Windows:

```powershell
ctest --test-dir build -C Release --output-on-failure
```

On Linux and macOS:

```sh
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure
```

The test suite covers command serialization, discovery parsing, message
parsing, device connections, device management, settings persistence, UI smoke
testing, effects, automation, and the LAN-only source policy.

## Device preparation

Initial Wi-Fi provisioning must be completed with the manufacturer's mobile
application. Use the same application to enable LAN Control; Yeelight LAN
cannot provision Wi-Fi or activate LAN Control itself.

The computer and device must be on the same local network. The router must
allow multicast and communication between local clients.

## Manual IP connection

Choose **Add by IP**, enter a literal IPv4 or IPv6 address and TCP port, and
choose whether to remember and immediately connect to the device. DNS names are
intentionally unsupported.

## Supported operations

Available controls depend on the methods advertised by each device. They can
include:

- power on and off
- toggle
- brightness
- RGB
- HSV
- color temperature
- color flow
- device naming
- timers
- music mode

## Limitations

The application does not provide cloud accounts, remote Internet control,
Wi-Fi provisioning, automatic LAN Control activation, cloud schedules,
background system services, or automatic updates.

RGBIC marketing names do not guarantee segment support in the public LAN
protocol. Segment controls remain hidden unless the device advertises a
recognized segment method. Yeelight LAN does not send undocumented segment
commands automatically.

## Troubleshooting

If discovery fails, verify LAN Control, firewall rules, multicast support,
client isolation, and network adapter selection. Manual IP entry can bypass
blocked multicast discovery, but it cannot bypass client isolation. See the
full [troubleshooting guide](docs/troubleshooting.md).

## How it works

Discovery uses UDP multicast at `239.255.255.250:1982`. Control uses a direct
TCP connection to each device, normally on port `55443`. Each device has an
independent connection and reconnect state. UI code does not own sockets or
construct protocol commands, and no cloud service is involved.

See the [architecture documentation](docs/architecture.md) and
[LAN protocol documentation](docs/lan-protocol.md) for implementation details.

## Documentation

- [Architecture documentation](docs/architecture.md)
- [LAN protocol documentation](docs/lan-protocol.md)
- [Troubleshooting guide](docs/troubleshooting.md)

## Privacy

Yeelight LAN communicates only with discovered or manually entered LAN
devices. It does not use cloud endpoints, analytics, telemetry, remote assets,
or an update service. Optional file logging is disabled by default.

## Contributing

Pull requests may be submitted. Before submitting changes, build the project,
run all tests, keep networking asynchronous, preserve capability checks, and do
not add Internet dependencies. Review the repository license before
contributing.

## License
No license or the basic one or idk just copy pasted it its build on top of yeelight api so idk I dont take responsibilty or some shi 

See [LICENSE](LICENSE) for the complete terms.
