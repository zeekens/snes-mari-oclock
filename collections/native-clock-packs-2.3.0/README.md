# Native clock packs 2.3.0

45 scenes, 15 in each pack, including Bubble Bobble. Requires sntl-2.2.0 or newer.

- [Pack 1](native-clock-pack-1-2.3.0.zip)
- [Pack 2](native-clock-pack-2-2.3.0.zip)
- [Pack 3](native-clock-pack-3-2.3.0.zip)

Fixes scene geometry in Ice Climber, Maridia, Fairy Fountain, Rainbow Road, Floating Continent and South Figaro rooftops. Ice Climber's return jump remains supported or airborne through the gap. Interceptor uses a separate foreground ledge. Roof jumps clear the roof surfaces; rivals stay on the road; ambient actors stay separated. Samus's Lift and Luigi's Crossing were renamed to match their artwork.

Fifteen scenes changed from 60 ms to 80 ms frames for an even cadence on the 40 ms refresh interval. Those scenes run 25% slower; existing 80 ms scenes are unchanged. Six native scene fixes and all 45 native assets passed pixel, clock and playback validation (12,812 frames), sanitizers and 2,000 redraw/seek comparisons each. All eight built-ins passed 1,560 samples each. Independent review found and resolved an unsupported Ice Climber walk and a gallery seek boundary error.

Tide Pool's built-in fish separation fix is in companion firmware sntl-2.2.1. The native pack assets work with sntl-2.2.0. Refresh Scene Library in Home Assistant, then reselect the current scene to load its new asset. Hardware visual playback remains unverified.

Each ZIP contains 15 native assets, a portable manifest, offline playback previews, credits and validation. Earlier collections remain unchanged. Retained audit authoring source is in `audit-source/`; archive reproduction uses the repository's package_reviewed.py.
