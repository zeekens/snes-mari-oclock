# Artwork and source provenance

Game art remains the property of its respective owners. The scene composition,
stage geometry, animation tracks, and clock fitting are adaptations for 64×64.
Original sprite colors are retained; selected poses are cropped, fitted with
nearest-neighbour sampling, and composited with explicit background color keys.

## Ice Climber

Nintendo. Popo and Nana from **General Sprites**, uploaded by **Tox2564**:
[The Spriters Resource, asset 34779](https://www.spriters-resource.com/nes/iceclimbers/asset/34779/).
Retained as `sources/ice-climber.png` (424×194). The NES poses are used, not the
sheet's VS. System, Famicom Disk System or unused sections. Platforms and sparse
snow are code-drawn adaptations. Popo's selected hammer pose is fitted to 15×18;
his standing/walking poses to 11×15 and Nana to 13×16.

## Donkey Kong

Nintendo. **Characters, Enemies & Objects**, uploaded by **Deathbringer**,
contributor **MisterMike**; the retained sheet credits Mister Mike:
[The Spriters Resource, asset 46026](https://www.spriters-resource.com/nes/dk/asset/46026/).
Retained as `sources/donkey-kong.png` (484×421). Kong, Mario, Pauline and rotating
barrel poses are used. Girders, ladders, drum and flame are code-drawn adaptations.

## Cobra Triangle

Rare / Nintendo. Original player boat extracted from a lossless 256×224 game
screenshot, mirrored to point upriver at its original pixel size with transparent
edges trimmed. The initial 20×15 reduction was removed in the second art revision
to retain the hull, cockpit and engine details. A shoreline tree is
cropped from the second screenshot and fitted to 5×18.
[LaunchBox screenshot collection](https://gamesdb.launchbox-app.com/games/images/1614-cobra-triangle):

- `sources/cobra-gameplay.png`: [original screenshot](https://images.launchbox-app.com/856ec705-bd06-4d47-8015-ebc6c9289f3d.png).
- `sources/cobra-gameplay2.png`: [original screenshot](https://images.launchbox-app.com/35740536-798c-4c01-9345-42a65fbf30f6.png).
- `sources/cobra-stage1.png`: [VGMaps stage-one map](https://www.vgmaps.com/Atlas/NES/CobraTriangle-Stage1-RaceToTheFinish.png),
  mapped by **zagato blackfist**. Retained as a visual reference; its pixels are
  not embedded in the scene.

The riverbank layout, scrolling texture, water glints and wake are code-drawn
adaptations using the gameplay palette. The sky-blue water glints are chosen
for readable motion on the small panel. No ROM is included or required.

---

# Bubble Bobble artwork

Bubble Bobble characters and original arcade artwork are © Taito. Bub, Bob,
Zen-Chan, bubbles, and fruit are cropped from the original arcade sprite assets
retained by the [Bubble-Bobble-Replica project](https://github.com/MiquelVillarroya/Bubble-Bobble-Replica/tree/89c6a5dc442eabcbbd94a8de8e407bfbe344a1d6/resources/Art).
The original stage-one reference is retained as `sources/bubble-bobble/level1.png`.
The precise download URLs are recorded in `sources/bubble-bobble/provenance.json`.

The dragons, enemy, and bubbles retain their native 16×16 sprite pixels and
transparency. Fruit uses nearest-neighbour fitting to 8×8. The compact platform
layout, choreography, small ambient bubbles, pop effect, and live clock are
authored for the 64×64 clock. Platform colors and diagonal texture follow the
retained arcade stage reference.

This is fan artwork for the clock scene pack. No ownership of Taito artwork is claimed.

The NES sprite references were investigated, but no NES assets are used in this
scene. The actual retained sources listed above are arcade artwork.
