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
