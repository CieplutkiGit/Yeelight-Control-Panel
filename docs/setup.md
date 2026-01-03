# Device setup

1. Provision the Yeelight device with the manufacturer's mobile application.
2. Connect the computer and device to the same trusted local network.
3. Enable LAN Control for the device in the mobile application.
4. Start Yeelight LAN and choose **Discover**.
5. If multicast is blocked, choose **Add by IP** and enter a literal address.

The desktop application intentionally does not provision Wi-Fi or activate LAN
Control. Those one-time operations may require the manufacturer's application.
Afterward, normal control is direct and local.

For manual devices, port `55443` is the normal default. Keep **Remember this
device** enabled to restore the entry on the next start. An offline remembered
device stays visible so its address and diagnostics can be corrected.

