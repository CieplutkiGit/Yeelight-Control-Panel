# Yeelight LAN protocol

## Multicast discovery

Discovery sends three SSDP-like UDP datagrams to `239.255.255.250:1982`:
immediately, after 500 ms, and after 1500 ms. A session ends after four seconds.
The socket binds `AnyIPv4` with address sharing and reuse enabled and joins the
multicast group on each active non-loopback IPv4 interface.

## Discovery headers

Header names are case-insensitive and may arrive in any order. The parser reads
`Location`, `id`, `model`, `fw_ver`, `support`, `power`, `bright`,
`color_mode`, `ct`, `rgb`, `hue`, `sat`, and `name`. Unknown headers are
ignored. An invalid `yeelight://address:port` location rejects the response.

The space-separated `support` header is the sole source of standard command
capabilities. Segment support is never inferred from a product name.

## TCP framing

Each device has its own TCP socket. Commands and responses are compact UTF-8
JSON followed by exactly one carriage-return/line-feed pair. The stream parser
retains incomplete data and splits only at complete delimiters.

## Request IDs

Request IDs are positive, monotonically increasing integers scoped to a device
controller. A Yeelight hardware ID is never reused as a command request ID.

## Responses and errors

A successful response contains an ID and result array. A protocol error contains
the matching ID plus an error code and message. Requests time out after three
seconds if neither form arrives.

## Property notifications

The device can send a `props` notification without a request. Recognized values
update live power, brightness, color temperature, RGB, HSV, flow, music,
delayed-off, reachability, and last-seen state.

## Supported command mapping

- `set_power`: explicit on and off
- `toggle`: toggle power
- `set_bright`: brightness
- `set_rgb`: RGB integer color
- `set_hsv`: hue and saturation
- `set_ct_abx`: white color temperature
- `start_cf` and `stop_cf`: local color flows
- `set_name`: device LAN name
- `cron_get`, `cron_add`, and `cron_del`: device timer operations
- `set_music`: optional local music callback mode

## Capability detection

Standard UI controls stay disabled when their method is absent from `support`.
The opt-in developer console can send a model-specific method only to the
currently selected LAN device and clearly warns that unsupported commands can
be rejected.

