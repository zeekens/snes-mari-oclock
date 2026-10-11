# Scene pack quality overhaul — 2.0.0

All 41 published scenes retain their IDs and library assignments. Names now use
`Game - Scene` (including FF6, DKC2 and Zelda ALTTP). Previous collections remain
immutable. The reviewed scene policy is `scene-packs/quality-policy.json`.

## Clock and pacing

The colon uses columns 31–32, centered between digit pairs. Both dot centers are
symmetrical within the 14-pixel numeral height. FF6 no longer moves it a pixel
lower. This is a player correction and also fixes existing v1 assets.

Twelve suitable scenes have short idle clips and play one action on a consecutive
minute transition. These include X shooting, Simon's whip, both Street Fighter
scenes and seven FF6 actions plus the Floating Continent's dog charge. Travel,
racing, scrolling and environmental scenes remain continuous at half speed
(two-thirds for Mario Kart). Original classic Mario minute-jump behavior is unchanged.

Quiet clips ping-pong; single-frame clips remain still. Idle excerpts retain the
ambient movement already present in those source frames. They are not independent
background layers. Source frames outside the selected excerpts remain in the pack
for reproducibility; no duplicate minute-long tracks or extra framebuffers exist.
Startup, invalid time, time corrections and disabled animation do not fire an action.
Consecutive midnight rollover does. Actions are cancelled when animation is disabled.

## Storage format and compatibility

SNTL v2 preserves the v1 header, palette, lossless tiles, frame commands and font.
Version byte is 2, period at byte 12 is 40–1000 ms (multiples of 20), total length
includes a 16-byte trailer after the 70-byte font:

`TIME`, mode u16 (0 continuous / 1 minute), idle-start u16, idle-count u16,
action-start u16, action-count u16, idle-period-ms u16. All words little endian.
Ranges and periods are validated before activation. Actions are limited to 30 s.

Firmware sntl-2.2.0 reads v1 and v2. Older firmware rejects v2 and retains the prior
scene; install the firmware before refreshing this catalog. Catalog JSON schema
and stable scene IDs do not change. Each scene grows by only 16 bytes; largest is
157,394 bytes against the 163,840-byte scene limit. Player state stays below 1 KiB.

## Verification and the reported boxes

Every one of the 12,300 scene frames is checked against the frozen v1 decoder,
then against the original pre-encoding RGB render in the local art workspace.
The scene pixel payload is unchanged: no transparency flattening, GIF disposal,
resizing or palette conversion is introduced in this release. An independent
Pillow clock oracle verifies the corrected C++ clock on all 12,300 frames.
Each scene also undergoes 2,000 incremental/full redraw comparisons, seeks, clock
changes, clock removal, forced repaints, malformed trailers and schedule tests
under address/undefined-behavior sanitizers.

Sabin's complete Fire Dance poses were inspected at enlarged native resolution.
No opaque sprite rectangle was reproduced in the source or decoded output.
This is **not a claim that the user's physical-panel symptom has been fixed**.
`Native Render Proof` exposes frame/time/FNV-1a of the device's actual composed
frame for comparison with the host. It reports unavailable for classic scenes,
display-off and messages. Matching proofs isolate any remaining visible issue
to the art interpretation or downstream panel/display path; they do not certify
physical panel quality.

## Reproduction

```
python scripts/native-packs/build.py PUBLIC_CHECKOUT build/scene-overhaul
python scripts/native-packs/verify.py PUBLIC_CHECKOUT build/scene-overhaul
```

The input catalog pins hashes and immutable collection paths. The gallery in
`build/scene-overhaul/collections/native-clock-packs-2.0.0/preview/` uses exact
native scene atlases, selectable packs, pause, and a next-minute action control.
Serve it using a local HTTP server. Previous submitted gallery reviews are untouched.
