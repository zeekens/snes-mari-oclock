# SNTL v1 format

SNTL is a flash-resident native tile/placement-track format. It is separate from the existing SNPK v1 downloaded sprite packs and has no network loader in this implementation. All integers are little-endian. All offsets and lengths are validated before use. Hashes in the manifest cover each complete asset.

The 40-byte header contains: magic `SNTL` (4 bytes), version 1 (u8), tile width/height 4 or 8 (u8), palette count (u16), timeline frame count (u16), tile count (u16), frame duration 40 ms (u16), keyframe interval 50 (u16), tile-offset-table position (u32), dictionary position (u32), frame-offset-table position (u32), command-stream position (u32), font position (u32), total asset length (u32).

The palette follows the header, with three RGB bytes per entry. The tile-offset table has tile-count + 1 u32 offsets relative to the dictionary. A tile entry starts with mode 0 for literal palette indices, or mode 1 for RLE. RLE control bytes encode length minus one in bits 0–6; bit 7 selects one repeated following index, otherwise the indicated number of literal indices follows. Every tile decodes to exactly tile-width squared indices.

The frame-offset table contains one u32 stream-relative offset per timeline frame. Multiple entries can share the same command block. Each block starts with clock y, outline style, RGB ink, RGB outline (8 bytes). It then contains unsigned base-128 varints: skip-plus-one, run length, and that many tile IDs. A zero skip terminates the block. Tile positions are row-major over a 64×64 canvas. Keyframes replace every tile; other frames replace changed runs. A seek reconstructs at most 50 frames of tile-map state, without drawing intermediate canvases.

The final 70 bytes are ten 5×7 decimal glyphs, seven row bytes per digit. The clock is drawn at runtime from actual hour/minute arguments. Outline styles reproduce the three reviewed clock treatments; no fixed 12:34 glyph pixels are in the scenery assets.

The C++ player stores at most 256 u16 tile IDs, a 64-byte tile scratch area and small metadata. A compile-time assertion bounds the entire player below 1 KiB. The existing display framebuffer is reused. No scene-sized heap allocation or GIF decoder is used.
