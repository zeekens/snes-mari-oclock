# SNES clock packs

Five downloadable SNPK v1 scene packs for the SNES clock's `packs-1.0.0` firmware.
This repository is the pack library; it does not contain a flashable firmware image.
The clock downloads one selected pack, checks its SHA-256, and renders it locally.
Mario, Luigi Forest and Cape Luigi Forest remain built-in fallback scenes.

| Scene | File bytes | Engine |
|---|---:|---:|
| Ghost House | 1,192 | 1 |
| Tide Pool | 344 | 2 |
| Lava Fortress | 2,840 | 3 |
| Star Road | 324 | 4 |
| Bonus Room | 1,484 | 5 |

Total: 6,184 bytes. Full hashes and filenames are in [the catalog](packs/manifest.json).
Packs contain indexed sprite artwork, not executable code. New scene behaviors
require compatible firmware. The device currently supports at most 16 KiB per
pack and keeps only the selected pack in RAM; rebooting returns to a built-in
scene until a controller reapplies the selection.

## Load from GitHub

Use an immutable, full 40-character commit SHA as the library base:

```text
https://raw.githubusercontent.com/zeekens/snes-mari-oclock/FULL_COMMIT_SHA/packs
```

Append a filename from the catalog and supply its complete SHA-256 to the device:

```yaml
action: esphome.snes_clock_load_scene_pack
data:
  pack_url: https://raw.githubusercontent.com/zeekens/snes-mari-oclock/FULL_COMMIT_SHA/packs/ghost-house-5153597b60d369d7.scn
  expected_hash: 5153597b60d369d7eeeecfd9cef06b46e9d6750c8428a07f4eda017eb4762f5b
```

Use the real commit SHA, not the placeholder above. Branch URLs, HTML `blob`
pages, redirects, authentication tokens, and private-repository downloads are
unsupported. HTTPS certificates and the expected file hash are verified.
Request acceptance is asynchronous: check Pack Status becomes `ready` and
Active Scene becomes the requested scene ID. A failed replacement preserves the
previous scene.

## Home Assistant controls

1. Copy [the HA package](home-assistant/snes_clock_packs.yaml) into your HA
   `packages/` directory. If needed, enable packages under the existing
   `homeassistant:` configuration with `packages: !include_dir_named packages`.
   Merge into existing settings; do not create duplicate keys.
2. Adjust ESPHome action and entity IDs to match your clock. Examples use
   `snes_clock`; renamed devices may have a different prefix for newly discovered
   diagnostics and the status binary sensor.
3. Validate HA configuration and restart to load the package.
4. Add [the dashboard card](home-assistant/dashboard-card.yaml).
5. Set the GitHub Library URL to the commit-pinned base above, choose GitHub as
   source, then select a face. Apply face retries the current choice.

Alternatively copy `packs/` contents into `/config/www/snes-clock/`, set the HA
Library URL to `http://homeassistant.local:8123/local/snes-clock` (adjust hostname
and port), and choose Home Assistant as source. HA's `/local/` files are served
without authentication; place only pack data there. Creating `www/` for the first
time requires a restart.

The helper records the desired face; Active Scene records the actual face. The
supplied automation reapplies selection on HA startup or when the clock becomes
available. Use this helper consistently; direct changes to the device's built-in
Scene select do not synchronize the helper. No persistent asset cache or timed
retry loop is implemented.

## Validation and artwork

All five packs passed local/native parser and pixel-equivalence tests and actual
ESP32 LAN HTTP tests, including invalid-download retention, cancellation, and
supersession. GitHub-hosted device validation is recorded separately after
publication. HA package installation and physical display verification are
installation-specific acceptance checks.

The artwork is game-derived. See [ARTWORK.md](ARTWORK.md) for provenance and
rights limitations; this repository does not grant rights to Nintendo artwork.
