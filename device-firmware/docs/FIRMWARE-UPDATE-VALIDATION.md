# Firmware updater validation — 2026-10-08

Release: sntl-2.1.3. Source published to GitHub and Forgejo; device binaries are
private. The release remains a prerelease because restart recovery is unresolved.

## Passed

- Full host regression suite and native scene ASan/UBSan fixture tests.
- Firmware image/manifest validation, mismatched version/image rejection, URL
  allowlist tests (origin, plaintext, userinfo, query, traversal and extension).
- ESPHome compilation; application uses approximately 57% of its OTA slot.
- Public export excludes credentials and generated firmware/build artifacts.
- Private HTTPS download: no credentials and wrong credentials receive HTTP 401;
  correct authentication returns the exact built image.
- HA discovered the Firmware update entity and Check Firmware Update button.
- HA reported 2.1.3 available while 2.1.2 was installed. Its update.install service
  downloaded/flashed 2.1.3 in approximately 9 seconds. Native API then confirmed
  project version 2.1.3 and HA reported installed/latest 2.1.3, up to date.
- The prior Zelda scene restored after installation. Largest selected assets
  from each of the three libraries loaded; all three message modes completed.

## Fixed during validation

- The clock could not resolve the private download hostname. It now uses its
  DHCP gateway as primary DNS, without changing its IP addressing. The corrected
  private HTTP update path passed on the device.
- The upstream URL-auth path can expose credentials in download error logs. The
  new transport uses an Authorization header scoped to the exact private HTTPS
  release directory. It never puts credentials in URLs and rejects redirects.

## Open hardware gate

After the scene/message tests, a manual restart with ffvi-14 selected left the
native API unavailable. The recovery OTA port initially remained reachable.
This resembles the earlier SNTL recovery issue; its cause is not established.
No partition changes or further recovery uploads were attempted. The user was
asked to disconnect power for 10 seconds and reconnect. Restoration of the original
Zelda selection failed while the API was unavailable. Physical-panel confirmation
and restart recovery are therefore still required before calling this stable.

GitHub's raw manifest can remain cached for up to five minutes after publication;
the test waited for the normal update check to see the new version.

Detailed device/API logs, state snapshots and private binary checks are retained
locally under ignored build/firmware-update/. Never publish that directory.
