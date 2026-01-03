# Troubleshooting

## Device not discovered

Confirm that the device is powered, provisioned, and connected to the same
network. Try manual IP entry to distinguish a multicast problem from a TCP
connectivity problem.

## LAN Control disabled

Discovery or commands may be rejected until LAN Control is enabled in the
manufacturer's mobile application.

## Guest Wi-Fi isolation

Guest networks often prevent clients from communicating. Move both devices to a
trusted LAN or disable client isolation.

## Multicast blocked

Routers, managed switches, VPNs, and host firewalls can block UDP multicast at
`239.255.255.250:1982`. Manual IP entry bypasses discovery only.

## Firewall blocked

Allow the application to send and receive local UDP discovery traffic and make
outbound TCP connections to device port `55443`.

## Incorrect adapter

Disconnect unused VPN or virtual adapters temporarily. Discovery joins every
active non-loopback IPv4 interface, but network policy can still route replies
incorrectly.

## Stale IP address

Rediscover the device. A stable advertised device ID lets the manager update a
changed address without creating a duplicate.

## Device offline

The application keeps remembered entries visible and reconnects with bounded
backoff. Confirm power and address, then use **Reconnect** or **Refresh state**.

## Command unsupported

Controls remain unavailable when the device does not advertise the required
method. Firmware and product variants expose different LAN capabilities.

## Multiple network interfaces

If duplicate or missing replies occur, inspect VPN, Hyper-V, container, and
virtual machine adapters. Disable the irrelevant adapter and repeat discovery.

