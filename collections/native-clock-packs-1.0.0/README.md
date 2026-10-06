# Native clock packs · 1.0.0

41 selected clock faces in three independently compiled packs. Both discarded
Chrono Trigger faces and the standalone Zelda zoom tour are excluded.
Each ZIP includes native SNTL assets, animated GIF previews, an offline gallery,
the firmware source and build generator, artwork credits, hashes and validation.

**Requires a firmware rebuild using the included SNTL player.** These are not
legacy SNPK downloads or preconfigured flashable firmware. One pack is installed
at a time. The 4 MB board uses a 1,835,008-byte OTA application slot.

| Pack | Faces | Native assets | Compiled firmware | Free space | Download |
|---|---:|---:|---:|---:|---|
| [Pack 1 previews and build instructions](pack-1/README.md) | 15 | 757,427 B | 1,757,616 B | 75.6 KiB | [ZIP](snes-clock-pack-1-1.0.0.zip) |
| [Pack 2 previews and build instructions](pack-2/README.md) | 11 | 822,296 B | 1,822,160 B | 12.5 KiB | [ZIP](snes-clock-pack-2-1.0.0.zip) |
| [Pack 3 previews and build instructions](pack-3/README.md) | 15 | 819,556 B | 1,819,776 B | 14.9 KiB | [ZIP](snes-clock-pack-3-1.0.0.zip) |

All three selections passed full ESPHome 2026.8.2 compile checks with build-only
credentials. Packs 2 and 3 have limited headroom. Firmware settings and compiler
changes require another size check. Physical panel playback has not been tested
for these native packs. All 41 assets match their pixel-validated conversion
hashes (12,300 frames, five comparisons per frame). Exported catalogs and asset
arrays match the compiled selections. ZIP members and checksums were verified.

[Archive checksums](SHA256SUMS) · [Pack manifest](manifest.json).
