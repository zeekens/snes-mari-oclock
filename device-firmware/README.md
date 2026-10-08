# SNES Clock firmware source

ESPHome firmware for the measured HUBClock ESP32-WROOM-32, 4 MB flash and 64×64
HUB75 panel. Includes Classic scenes and downloadable SNTL scene libraries.

The Firmware entity in Home Assistant checks for releases daily. Use Check
Firmware Update to check immediately, and Install to install manually.
See [firmware updates](docs/FIRMWARE-UPDATES.md) for provisioning, release
publication, private download hosting and verification.

Install requirements.txt into a Python 3.12 virtual environment, then run
`esphome compile firmware/snes-clock.yaml` for a build-only configuration.
For the actual clock, provision ignored secrets.yaml and use
`esphome compile firmware/snes-clock-device.yaml`.

Firmware images are device-specific and contain credentials. They are served
privately on the owner's network, never attached to these public releases.
Other owners must build/provision their own firmware and update download source.

Artwork attribution and usage context: [ARTWORK.md](../ARTWORK.md).
