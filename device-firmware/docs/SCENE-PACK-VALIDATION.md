# Scene pack overhaul validation — 2026-10-11

Firmware **sntl-2.2.0**, collection **native-clock-packs-2.0.0**: 41 scenes, three
libraries (15 / 11 / 15). Twelve minute-triggered action scenes; 29 slower
continuous scenes. The original classic Mario minute-jump behavior is retained.

## Completed checks

- All 12,300 decoded background frames equal both the frozen v1 decoder and
  original RGB renderer output. No palette or transparency conversion was used.
- Independent clock oracle matches all 12,300 corrected clock frames.
- Each scene passed 2,000 full/incremental comparisons including loop/seek,
  changing time, invalid time, overlay removal and forced repaint.
- All minute policies passed startup, consecutive minute, midnight, wraparound,
  nonconsecutive time correction, disabled animation and malformed-data tests.
- Firmware regression suite and ESPHome device build passed.
- All 41 public SNTL assets and three ZIPs downloaded and matched local bytes.
- All 41 scenes downloaded and activated on the ESP32. Every sampled composed
  framebuffer checksum matched the host renderer for that frame and clock time.
- All three native selectors were checked against their published scene lists.
  Home Assistant REST also showed the updated names and firmware up-to-date.
- Sabin was watched programmatically across a real minute change: idle and action
  frames both matched the host. No synthetic wall-clock change was used.
- Textbox, banner and fullscreen messages completed; subsequent scene proofs matched.
- Restart restored Sabin from the persistent cache; its framebuffer matched again.
- Original Mario Kart selection, display/animation switches and night schedule restored.

## Limits and remaining visual confirmation

The reported Sabin rectangle was not reproduced in source, decoded artwork,
preview, or the sampled device framebuffer. No unsupported claim of a transparency
repair is made. The physical LED panel cannot be certified from a framebuffer
checksum; user confirmation of that visible symptom remains pending.

Recorded render-duration samples ranged from 14.97 to 77.89 ms. The dense Rainbow Road,
Rooftop Escape and Chocobo scenes can exceed the 40 ms display tick. Playback now
uses 60–80 ms stored-frame intervals, but a fixed physical 25 FPS is not claimed.
The samples are diagnostics, not per-scene sustained frame-rate measurements.

## Deployment

Home Assistant offered the new firmware correctly, but its initial install failed
because the old firmware could not resolve the private download hostname. The
existing authenticated ESPHome push-OTA route via the device IP installed 2.2.0.
The running version was verified before activating the v2 catalog. The old
collection and private 2.1.3 firmware binary are preserved. No credential-bearing
firmware binary was published to GitHub.

OTA application: 1043456 bytes (slot 1,835,008); compile-time RAM: 73,360 bytes.
Largest scene: 157,394 bytes (limit 163,840). Timing extension: 16 bytes per scene.
Firmware SHA-256: `51ea095027f90376aa79f9a230143f2b4641b5b5cd98ea10c052e9a80bbd6176`.
Public assets verified at `5d5d38f6c2f14d49188ac33b46a60469ce174c1b`; catalog activated at `93c7a1f`.

## Live scene proofs

Each proof is `frame:hour:minute:valid:FNV1a`. All matched the independent host render.

| Scene | Playback | Bytes | Device proof |
|---|---|---:|---|
| Zelda ALTTP - Light World Link's House Overworld | continuous | 17,958 | `91:2:47:1:6a536e15` |
| Super Metroid - Escape Shaft Samus under Fire | continuous | 36,498 | `32:2:47:1:a1e453d1` |
| Yoshi's Island - Egg Garden Yoshi Meets the Parade | continuous | 45,379 | `25:2:47:1:6e6128a1` |
| Mega Man X - Central Highway Zero's Charge | continuous | 31,628 | `30:2:47:1:7f54a927` |
| Mega Man X - Sting Chameleon Forest X's Buster | minute | 67,806 | `57:2:47:1:41cff3e5` |
| Super Castlevania IV - Castle Approach Simon's Whip | minute | 27,921 | `59:2:47:1:a35507fd` |
| Street Fighter II - Brazil Ryu versus Blanka | minute | 47,548 | `135:2:47:1:bee164a3` |
| Super Mario Kart - Mario Circuit Mario's Solo Lap | continuous | 51,079 | `30:2:47:1:04f74643` |
| Super Mario Kart - Mario Circuit 1 Whole-Course Minimap | continuous | 46,372 | `29:2:47:1:ade4e26d` |
| FF6 - Narshe Snowfall | continuous | 74,949 | `17:2:47:1:6e491aa8` |
| FF6 - Phantom Train Last Departure | continuous | 62,625 | `16:2:47:1:97055b1f` |
| FF6 - Lete River Hold On! | continuous | 83,335 | `10:2:48:1:3fc8f4ec` |
| FF6 - Opera House A Rose for You | minute | 16,733 | `23:2:48:1:c38a61d4` |
| FF6 - Sealed Gate Esper Awakening | minute | 129,705 | `38:2:48:1:6d95afc3` |
| FF6 - Whelk Shell Shock | minute | 18,131 | `22:2:48:1:199987d5` |
| Super Metroid - Maridia Drifting Colony | continuous | 102,716 | `18:2:48:1:7dabd9c0` |
| Super Mario All-Stars - Water Land Luigi's Ferry | continuous | 34,292 | `29:2:48:1:f35262d1` |
| DKC2 - Rambi Rumble Rhino Charge | continuous | 86,204 | `8:2:48:1:a3de2fd8` |
| DKC2 - Squawks's Shaft Lantern Flight | continuous | 83,120 | `73:2:48:1:9cd0c958` |
| Street Fighter II - Suzaku Castle Ryu versus Chun-Li | minute | 15,115 | `93:2:48:1:d4acad3e` |
| Super Mario Kart - Rainbow Road Yoshi under the Stars | continuous | 157,394 | `64:2:49:1:75f87fa2` |
| FF6 - Zozo Rain & Rooftops | continuous | 117,496 | `49:2:49:1:2dfa0bae` |
| FF6 - Floating Continent Edge of Ruin | minute | 46,617 | `10:2:49:1:29ee405b` |
| FF6 - Narshe Cavern Dance for Treasure | continuous | 45,541 | `24:2:49:1:d3d9d51e` |
| FF6 - Ultros Fire Dance | minute | 76,457 | `10:2:49:1:dfa6e5e1` |
| FF6 - Forest Swarm Dance & Cure | minute | 57,520 | `15:2:49:1:127491d9` |
| Zelda ALTTP - Fairy Fountain Wings over Water | continuous | 71,357 | `16:2:49:1:ddc63dce` |
| Zelda ALTTP - Sanctuary Zelda's Escort | continuous | 37,756 | `26:2:49:1:4eb753b0` |
| Zelda ALTTP - Pyramid Chamber Ganon's Fire Ring | continuous | 46,966 | `26:2:49:1:4ffc8b61` |
| Super Metroid - Crateria Gunship Arrival | continuous | 35,660 | `34:2:49:1:769d50ee` |
| Yoshi's Island - Kamek's Flight Midnight Magic | continuous | 56,340 | `23:2:50:1:2f5f6423` |
| Super Mario All-Stars - Bowser's Keep Fire and Falling Bricks | continuous | 26,081 | `34:2:50:1:6916ae7f` |
| DKC2 - Gloomy Gulch Dixie's Lantern | continuous | 77,466 | `17:2:50:1:295801b6` |
| FF6 - Figaro Desert Citadel | continuous | 4,431 | `44:2:50:1:1e1fddfc` |
| FF6 - Blackjack Above the Clouds | continuous | 63,227 | `18:2:50:1:d5a9cb53` |
| FF6 - Magitek Esper Chamber | continuous | 25,183 | `33:2:50:1:3decdc89` |
| FF6 - Phantom Forest Interceptor! | continuous | 99,648 | `67:2:50:1:bbc25620` |
| FF6 - South Figaro Rooftop Escape | continuous | 111,798 | `54:2:50:1:38c49f77` |
| FF6 - Figaro Desert Chocobo Dash | continuous | 82,906 | `53:2:50:1:63773244` |
| FF6 - Ultros Fire on the River | minute | 53,953 | `10:2:51:1:023324de` |
| FF6 - Magitek Armor Runic & Ice | minute | 27,024 | `4:2:51:1:26f930a0` |

Restart proof: `13:2:53:1:ef343913`.

Restored settings:
```json
{
  "Library": "Pack 1",
  "Scene": "Super Mario Kart - Mario Circuit Mario's Solo Lap",
  "Night Schedule": true,
  "Display": true,
  "Animation": true
}
```
