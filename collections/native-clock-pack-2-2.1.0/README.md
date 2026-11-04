# Pack 2 — 2.1.0

Pack 2 now contains 14 scenes: its previous 11 plus **Ice Climber - Summit Partners**, **Donkey Kong - Barrel Girders**, and **Cobra Triangle - River Run**.

Requires **sntl-2.2.0 or later**. In Home Assistant, press **Refresh Scene Library**, choose **Pack 2**, then select a scene. No firmware rebuild or flash is required.

[Download complete Pack 2 ZIP](native-clock-pack-2-2.1.0.zip) · [Manifest](pack-2/manifest.json) · [New artwork credits](CREDITS.md) · [New scene validation](validation.json) · [Preview gallery source](previews/index.html)

Ice Climber and Donkey Kong run one action at a consecutive minute transition. Cobra Triangle runs continuously. The new sprites include the reviewed facing, jump-clearance, riverbank and boat-detail corrections. All 384 source frames and clock overlays match the production decoder; GIFs, incremental rendering, timing and malformed input checks passed under sanitizers. Mario also passed pixel collision checks against both girders and the barrel. The three new scenes have not been tested on the physical panel.

The original 11 assets retain their exact bytes and catalog paths. Previous collections remain immutable; Pack 1 and Pack 3 stay on 2.0.0. This archive includes all 14 Pack 2 native files. The online catalog points the three added entries into this collection.

The workspace renderer and build tools are included under `build-source/` for provenance. They use the existing SNES-clock workspace encoder and firmware test harness and are not standalone. Original source sheets and screenshot references are retained in `sources/`.
