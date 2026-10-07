# Downloadable SNTL libraries

**Experimental:** this index targets development firmware. Host pixel validation
passes, but device performance and restart recovery are still under test. Do not
treat the index as a released firmware upgrade. Existing SNPK firmware ignores it.

This catalog exposes the existing 41 validated native faces as three libraries
for clock firmware `sntl-2.0.0` or later. The clock downloads and verifies one
selected scene at a time into an inactive flash cache, then activates it. The
previous cached scene survives interrupted downloads. No Wi-Fi credentials,
API encryption keys, or preconfigured firmware are included.

The original native pack archives and assets are unchanged. Older `packs-1.x`
firmware cannot consume this catalog. Maximum individual asset size supported
by this loader is 160 KiB. Pack selection no longer requires compiling all its
scenes into one firmware image. Rendering remains 64×64 with a 40 ms animation
interval and a separately drawn live clock.

Scenes and artwork credits are in `collections/native-clock-packs-1.0.0`.
