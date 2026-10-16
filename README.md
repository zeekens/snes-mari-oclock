# SNES clock packs

## Current native packs — 2.0.0

**41 scenes across Pack 1 / Pack 2 / Pack 3 (15 / 11 / 15)**, using firmware
**sntl-2.2.0 or later**. Clock colons are centered, names use `Game - Scene`,
12 action scenes react to the minute change, and 29 travel/ambient scenes play
more slowly. Mario Kart remains continuously animated.

In Home Assistant, install the compatible firmware, press **Refresh Scene Library**,
then choose **Library** and **Scene**. The selected SNTL asset downloads from the
verified GitHub revision and stays in the device's persistent cache. Scene changes
do not require rebuilding or reflashing firmware.

- [Pack 1 ZIP](collections/native-clock-packs-2.0.0/native-clock-pack-1-2.0.0.zip)
- [Pack 2 ZIP](collections/native-clock-packs-2.0.0/native-clock-pack-2-2.0.0.zip)
- [Pack 3 ZIP](collections/native-clock-packs-2.0.0/native-clock-pack-3-2.0.0.zip)
- [Quality policy and format](device-firmware/docs/SCENE-PACK-QUALITY.md)
- [All-scene device validation](device-firmware/docs/SCENE-PACK-VALIDATION.md)
- [Interactive preview source](collections/native-clock-packs-2.0.0/preview/index.html)

All 41 downloads and sampled device framebuffers were verified against the host
renderer, including a real minute transition, message cleanup and restart recovery.
The reported Sabin rectangle was not reproduced; physical-panel confirmation of
that symptom remains pending. Credentials and device-specific firmware binaries
are not included in this public repository.

[Archived 1.0.0 native packs and original build-time workflow](collections/native-clock-packs-1.0.0/README.md)
remain available. The SNTL v2 collection requires the newer player; it is not
compatible with the older SNPK loader below.

## Current pack animations

All 41 scenes from the current **2.0.0** packs, rendered from the published scene
pixels with the corrected clock layout. These GIFs are previews, not installable packs.
The displayed time is fixed at **12:34**.

Continuous scenes retain their published playback speed. For minute-triggered
scenes, the GIF demonstrates a short idle → action → idle sequence; on the clock,
the action waits for a minute change rather than repeating at this preview rate.

### Pack 1 · 15 scenes

| | |
|---|---|
| **Zelda ALTTP - Light World Link's House Overworld**<br><img src="previews/native-2.0.0/zelda-01.gif" width="256" height="256" alt="Zelda ALTTP - Light World Link&#x27;s House Overworld animated preview"><br>Continuous · 24s loop | **Super Metroid - Escape Shaft Samus under Fire**<br><img src="previews/native-2.0.0/metroid-05.gif" width="256" height="256" alt="Super Metroid - Escape Shaft Samus under Fire animated preview"><br>Continuous · 24s loop |
| **Yoshi's Island - Egg Garden Yoshi Meets the Parade**<br><img src="previews/native-2.0.0/yoshi-04.gif" width="256" height="256" alt="Yoshi&#x27;s Island - Egg Garden Yoshi Meets the Parade animated preview"><br>Continuous · 24s loop | **Mega Man X - Central Highway Zero's Charge**<br><img src="previews/native-2.0.0/megaman-01.gif" width="256" height="256" alt="Mega Man X - Central Highway Zero&#x27;s Charge animated preview"><br>Continuous · 24s loop |
| **Mega Man X - Sting Chameleon Forest X's Buster**<br><img src="previews/native-2.0.0/megaman-03.gif" width="256" height="256" alt="Mega Man X - Sting Chameleon Forest X&#x27;s Buster animated preview"><br>Minute-triggered action | **Super Castlevania IV - Castle Approach Simon's Whip**<br><img src="previews/native-2.0.0/castlevania-03.gif" width="256" height="256" alt="Super Castlevania IV - Castle Approach Simon&#x27;s Whip animated preview"><br>Minute-triggered action |
| **Street Fighter II - Brazil Ryu versus Blanka**<br><img src="previews/native-2.0.0/streetfighter-02.gif" width="256" height="256" alt="Street Fighter II - Brazil Ryu versus Blanka animated preview"><br>Minute-triggered action | **Super Mario Kart - Mario Circuit Mario's Solo Lap**<br><img src="previews/native-2.0.0/mariokart-01.gif" width="256" height="256" alt="Super Mario Kart - Mario Circuit Mario&#x27;s Solo Lap animated preview"><br>Continuous · 18s loop |
| **Super Mario Kart - Mario Circuit 1 Whole-Course Minimap**<br><img src="previews/native-2.0.0/mariokart-05.gif" width="256" height="256" alt="Super Mario Kart - Mario Circuit 1 Whole-Course Minimap animated preview"><br>Continuous · 18s loop | **FF6 - Narshe Snowfall**<br><img src="previews/native-2.0.0/ffvi-01.gif" width="256" height="256" alt="FF6 - Narshe Snowfall animated preview"><br>Continuous · 24s loop |
| **FF6 - Phantom Train Last Departure**<br><img src="previews/native-2.0.0/ffvi-05.gif" width="256" height="256" alt="FF6 - Phantom Train Last Departure animated preview"><br>Continuous · 24s loop | **FF6 - Lete River Hold On!**<br><img src="previews/native-2.0.0/ffvi-09.gif" width="256" height="256" alt="FF6 - Lete River Hold On! animated preview"><br>Continuous · 24s loop |
| **FF6 - Opera House A Rose for You**<br><img src="previews/native-2.0.0/ffvi-11.gif" width="256" height="256" alt="FF6 - Opera House A Rose for You animated preview"><br>Minute-triggered action | **FF6 - Sealed Gate Esper Awakening**<br><img src="previews/native-2.0.0/ffvi-13.gif" width="256" height="256" alt="FF6 - Sealed Gate Esper Awakening animated preview"><br>Minute-triggered action |
| **FF6 - Whelk Shell Shock**<br><img src="previews/native-2.0.0/ffvi-18.gif" width="256" height="256" alt="FF6 - Whelk Shell Shock animated preview"><br>Minute-triggered action |  |

### Pack 2 · 11 scenes

| | |
|---|---|
| **Super Metroid - Maridia Drifting Colony**<br><img src="previews/native-2.0.0/metroid-04.gif" width="256" height="256" alt="Super Metroid - Maridia Drifting Colony animated preview"><br>Continuous · 24s loop | **Super Mario All-Stars - Water Land Luigi's Ferry**<br><img src="previews/native-2.0.0/allstars-01.gif" width="256" height="256" alt="Super Mario All-Stars - Water Land Luigi&#x27;s Ferry animated preview"><br>Continuous · 24s loop |
| **DKC2 - Rambi Rumble Rhino Charge**<br><img src="previews/native-2.0.0/dkc2-01.gif" width="256" height="256" alt="DKC2 - Rambi Rumble Rhino Charge animated preview"><br>Continuous · 24s loop | **DKC2 - Squawks's Shaft Lantern Flight**<br><img src="previews/native-2.0.0/dkc2-02.gif" width="256" height="256" alt="DKC2 - Squawks&#x27;s Shaft Lantern Flight animated preview"><br>Continuous · 24s loop |
| **Street Fighter II - Suzaku Castle Ryu versus Chun-Li**<br><img src="previews/native-2.0.0/streetfighter-01.gif" width="256" height="256" alt="Street Fighter II - Suzaku Castle Ryu versus Chun-Li animated preview"><br>Minute-triggered action | **Super Mario Kart - Rainbow Road Yoshi under the Stars**<br><img src="previews/native-2.0.0/mariokart-04.gif" width="256" height="256" alt="Super Mario Kart - Rainbow Road Yoshi under the Stars animated preview"><br>Continuous · 18s loop |
| **FF6 - Zozo Rain & Rooftops**<br><img src="previews/native-2.0.0/ffvi-03.gif" width="256" height="256" alt="FF6 - Zozo Rain &amp; Rooftops animated preview"><br>Continuous · 24s loop | **FF6 - Floating Continent Edge of Ruin**<br><img src="previews/native-2.0.0/ffvi-07.gif" width="256" height="256" alt="FF6 - Floating Continent Edge of Ruin animated preview"><br>Minute-triggered action |
| **FF6 - Narshe Cavern Dance for Treasure**<br><img src="previews/native-2.0.0/ffvi-10.gif" width="256" height="256" alt="FF6 - Narshe Cavern Dance for Treasure animated preview"><br>Continuous · 24s loop | **FF6 - Ultros Fire Dance**<br><img src="previews/native-2.0.0/ffvi-15.gif" width="256" height="256" alt="FF6 - Ultros Fire Dance animated preview"><br>Minute-triggered action |
| **FF6 - Forest Swarm Dance & Cure**<br><img src="previews/native-2.0.0/ffvi-22.gif" width="256" height="256" alt="FF6 - Forest Swarm Dance &amp; Cure animated preview"><br>Minute-triggered action |  |

### Pack 3 · 15 scenes

| | |
|---|---|
| **Zelda ALTTP - Fairy Fountain Wings over Water**<br><img src="previews/native-2.0.0/zelda-03.gif" width="256" height="256" alt="Zelda ALTTP - Fairy Fountain Wings over Water animated preview"><br>Continuous · 24s loop | **Zelda ALTTP - Sanctuary Zelda's Escort**<br><img src="previews/native-2.0.0/zelda-04.gif" width="256" height="256" alt="Zelda ALTTP - Sanctuary Zelda&#x27;s Escort animated preview"><br>Continuous · 24s loop |
| **Zelda ALTTP - Pyramid Chamber Ganon's Fire Ring**<br><img src="previews/native-2.0.0/zelda-05.gif" width="256" height="256" alt="Zelda ALTTP - Pyramid Chamber Ganon&#x27;s Fire Ring animated preview"><br>Continuous · 24s loop | **Super Metroid - Crateria Gunship Arrival**<br><img src="previews/native-2.0.0/metroid-01.gif" width="256" height="256" alt="Super Metroid - Crateria Gunship Arrival animated preview"><br>Continuous · 24s loop |
| **Yoshi's Island - Kamek's Flight Midnight Magic**<br><img src="previews/native-2.0.0/yoshi-03.gif" width="256" height="256" alt="Yoshi&#x27;s Island - Kamek&#x27;s Flight Midnight Magic animated preview"><br>Continuous · 24s loop | **Super Mario All-Stars - Bowser's Keep Fire and Falling Bricks**<br><img src="previews/native-2.0.0/allstars-03.gif" width="256" height="256" alt="Super Mario All-Stars - Bowser&#x27;s Keep Fire and Falling Bricks animated preview"><br>Continuous · 24s loop |
| **DKC2 - Gloomy Gulch Dixie's Lantern**<br><img src="previews/native-2.0.0/dkc2-03.gif" width="256" height="256" alt="DKC2 - Gloomy Gulch Dixie&#x27;s Lantern animated preview"><br>Continuous · 24s loop | **FF6 - Figaro Desert Citadel**<br><img src="previews/native-2.0.0/ffvi-02.gif" width="256" height="256" alt="FF6 - Figaro Desert Citadel animated preview"><br>Continuous · 24s loop |
| **FF6 - Blackjack Above the Clouds**<br><img src="previews/native-2.0.0/ffvi-06.gif" width="256" height="256" alt="FF6 - Blackjack Above the Clouds animated preview"><br>Continuous · 24s loop | **FF6 - Magitek Esper Chamber**<br><img src="previews/native-2.0.0/ffvi-08.gif" width="256" height="256" alt="FF6 - Magitek Esper Chamber animated preview"><br>Continuous · 24s loop |
| **FF6 - Phantom Forest Interceptor!**<br><img src="previews/native-2.0.0/ffvi-12.gif" width="256" height="256" alt="FF6 - Phantom Forest Interceptor! animated preview"><br>Continuous · 24s loop | **FF6 - South Figaro Rooftop Escape**<br><img src="previews/native-2.0.0/ffvi-14.gif" width="256" height="256" alt="FF6 - South Figaro Rooftop Escape animated preview"><br>Continuous · 24s loop |
| **FF6 - Figaro Desert Chocobo Dash**<br><img src="previews/native-2.0.0/ffvi-16.gif" width="256" height="256" alt="FF6 - Figaro Desert Chocobo Dash animated preview"><br>Continuous · 24s loop | **FF6 - Ultros Fire on the River**<br><img src="previews/native-2.0.0/ffvi-17.gif" width="256" height="256" alt="FF6 - Ultros Fire on the River animated preview"><br>Minute-triggered action |
| **FF6 - Magitek Armor Runic & Ice**<br><img src="previews/native-2.0.0/ffvi-19.gif" width="256" height="256" alt="FF6 - Magitek Armor Runic &amp; Ice animated preview"><br>Minute-triggered action |  |

## Original downloadable SNPK packs

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

## Animated clock faces

All eight faces, rendered at 25 fps. Each 12-second loop includes a minute change.

| | |
|---|---|
| **Mario**<br><img src="previews/mario.gif" width="256" alt="Mario animated clock face"><br>[Native 64×64 GIF](previews/mario-64.gif) | **Luigi Forest**<br><img src="previews/luigi.gif" width="256" alt="Luigi Forest animated clock face"><br>[Native 64×64 GIF](previews/luigi-64.gif) |
| **Cape Luigi Forest**<br><img src="previews/cape-luigi.gif" width="256" alt="Cape Luigi Forest animated clock face"><br>[Native 64×64 GIF](previews/cape-luigi-64.gif) | **Ghost House**<br><img src="previews/ghost-house.gif" width="256" alt="Ghost House animated clock face"><br>[Native 64×64 GIF](previews/ghost-house-64.gif) |
| **Tide Pool**<br><img src="previews/tide-pool.gif" width="256" alt="Tide Pool animated clock face"><br>[Native 64×64 GIF](previews/tide-pool-64.gif) | **Lava Fortress**<br><img src="previews/lava-fortress.gif" width="256" alt="Lava Fortress animated clock face"><br>[Native 64×64 GIF](previews/lava-fortress-64.gif) |
| **Star Road**<br><img src="previews/star-road.gif" width="256" alt="Star Road animated clock face"><br>[Native 64×64 GIF](previews/star-road-64.gif) | **Bonus Room**<br><img src="previews/bonus-room.gif" width="256" alt="Bonus Room animated clock face"><br>[Native 64×64 GIF](previews/bonus-room-64.gif) |

[Download all eight GIFs in native and enlarged sizes](previews/all-eight-clock-faces.zip).

Crisp nearest-neighbour enlargement; legacy background smoothing is disabled. These GIFs are previews, not installable packs. See [artwork provenance](ARTWORK.md).

## Final Fantasy VI animation collection

[Download the 19-face animation package](collections/ffvi-1.0.0/ffvi-clock-animations-1.0.0.zip) · [Browse all FFVI previews](collections/ffvi-1.0.0/README.md)

Native 64×64 and enlarged 512×512 GIFs, PNG stills, an offline HTML gallery, and a SHA-256 manifest. Includes 14 Keep faces and the latest revisions of five faces still marked Change; discarded faces are excluded.

These are preview assets with a fixed 12:34 display, **not installable SNPK packs**. See the [FFVI artwork credits](collections/ffvi-1.0.0/ARTWORK.md).

## Load from GitHub

Use an immutable, full 40-character commit SHA as the library base:

```text
https://raw.githubusercontent.com/zeekens/snes-mari-oclock/03400729f378f8e02f927c812e9439bff8cc1c9a/packs
```

Append a filename from the catalog and supply its complete SHA-256 to the device:

```yaml
action: esphome.snes_clock_load_scene_pack
data:
  pack_url: https://raw.githubusercontent.com/zeekens/snes-mari-oclock/03400729f378f8e02f927c812e9439bff8cc1c9a/packs/ghost-house-5153597b60d369d7.scn
  expected_hash: 5153597b60d369d7eeeecfd9cef06b46e9d6750c8428a07f4eda017eb4762f5b
```

The URL above is the device-tested immutable version. Branch URLs, HTML `blob`
pages, redirects, authentication tokens, and private-repository downloads are
unsupported. HTTPS certificates and the expected file hash are verified.
Request acceptance is asynchronous: check Pack Status becomes `ready` and
Active Scene becomes the requested scene ID. A failed replacement preserves the
previous scene.

## Home Assistant controls

With `packs-1.1.0` firmware, use the existing ESPHome **Scene** selector: it offers
all eight scenes and automatically downloads the five remote packs from the
pinned GitHub library above. No extra HA package is required for that flow.
Remote choices persist and are requested again after boot/Wi-Fi reconnection,
once network and clock time are ready for TLS.

The optional package below provides a separate source selector for HA hosting
or a custom GitHub library. Use one controller consistently: its desired face
and the native Scene selector do not synchronize with each other.

1. Copy [the HA package](home-assistant/snes_clock_packs.yaml) into your HA
   `packages/` directory. If needed, enable packages under the existing
   `homeassistant:` configuration with `packages: !include_dir_named packages`.
   Merge into existing settings; do not create duplicate keys.
2. Adjust ESPHome action and entity IDs to match your clock. Examples use
   `snes_clock`; renamed devices may have a different prefix for newly discovered
   diagnostics and the status binary sensor.
3. Validate HA configuration and restart to load the package.
4. Add [the dashboard card](home-assistant/dashboard-card.yaml).
5. Choose GitHub as source, then select a face. The script defaults to the
   tested commit above when the GitHub Library URL helper is empty. Set a
   different commit-pinned URL to override it. Apply face retries the choice.

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
supersession. All five files also downloaded directly from GitHub and activated
on the ESP32; see [device validation](VALIDATION.md). HA package installation and physical display verification are
installation-specific acceptance checks.

The artwork is game-derived. See [ARTWORK.md](ARTWORK.md) for provenance and
rights limitations; this repository does not grant rights to Nintendo artwork.
