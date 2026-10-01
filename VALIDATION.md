# Published pack validation — 2026-10-01

Pack commit: `03400729f378f8e02f927c812e9439bff8cc1c9a`. Device firmware: `packs-1.0.0`.

The test fetched each GitHub raw file over HTTPS and compared it byte-for-byte
with the reviewed local artifact. It then asked the physical ESP32 clock to
fetch each URL itself through its encrypted native API. Every scene reported
`ready`, the expected active scene ID, and the expected resident byte count.
The initial cache was cleared and every scene used a different hash, so all five
loads exercised the network rather than a resident-cache shortcut.

| Scene | Bytes | Activation observed | Render sample |
|---|---:|---:|---:|
| ghost-house | 1192 | 1.313 s | 17.92 ms |
| tide-pool | 344 | 1.567 s | 16.61 ms |
| lava-fortress | 2840 | 1.670 s | 16.90 ms |
| star-road | 324 | 1.784 s | 16.64 ms |
| bonus-room | 1484 | 1.842 s | 16.30 ms |

Activation observations include a 1.2-second diagnostic wait; they are not raw
network latency. Render time includes drawing and copying pixels to the HUB75
driver, not the panel scan period. All samples fit the 40 ms frame target.

The clock was restored to its prior Cape Luigi scene and brightness after the
test. Minimum free heap reported after the GitHub sequence was 68,748 bytes.
The deployed parser also previously passed native sanitizer, malformed-pack,
pixel-equivalence and LAN HTTP failure/cancellation/supersession tests.

Home Assistant control templates have been tested locally, including both
sources and the pinned-URL fallback. Installation and reconnect restoration in
a live HA instance remain pending. Physical panel appearance was not visually
confirmed by this test. These results cover the five files in this catalog,
not arbitrary maximum-size packs or arbitrary other hardware.
