# Firmware updates through Home Assistant

The Firmware update entity checks the GitHub `firmware/releases/stable.json`
manifest at startup and every 24 hours. Check Firmware Update requests a check
immediately. Home Assistant shows the installed/available versions, release notes,
and Install control. Checks never install automatically. Scene Library refresh
is separate and does not update firmware.

This clock is provisioned with compiled Wi-Fi, API and OTA secrets. **Never attach
its binary, factory image, ELF, generated main.cpp, build logs, or secrets.yaml to
GitHub.** The public repository contains only source, release notes and metadata.
The OTA binary is served by `deploy/firmware-server` on the local Docker host,
behind the existing Traefik HTTPS endpoint, with separate read-only Basic Auth
credentials. TLS verification is enabled and redirects disabled. ESPHome checks
the manifest's MD5 during installation; SHA-256 is additionally published for
release audit. GitHub/TLS and private host access control provide trust, not MD5.
The manifest and release URL contain no credentials. Device-specific downloads
require access to the home network and the provisioned download credentials.

## Provisioning

Copy the four existing device secrets plus `firmware_download_user` and
`firmware_download_password` into ignored `firmware/secrets.yaml`. Use a randomly
generated URL-safe download password. The shared YAML's build-only defaults are
for compilation, not deployment. Build with the pinned ESPHome version in
requirements.txt using `esphome compile firmware/snes-clock-device.yaml`.
Credentials travel only in an Authorization header to the exact allowed HTTPS
host and release directory; URLs contain no credentials, including error logs.
The firmware transport rejects other origins, redirects, queries and traversal.

The server's `htpasswd` is generated locally and must not be committed. Keep its
parent directory private (0700); the mounted hash file must be readable by nginx.
It serves only authenticated GET/HEAD from the immutable releases directory.
No host port is published; Traefik routes only `/clock-firmware/` on the existing
Git hostname. Other routes/services are unchanged.

## Releasing

1. Increment `firmware_version` and test the source. Compile the device YAML.
2. Preserve `firmware.ota.bin` privately as `<version>.bin`; do not use the factory
   image. Keep the prior working release. Do not overwrite released filenames.
3. Upload the binary to the private server. Verify unauthenticated requests fail,
   authenticated HTTPS returns identical bytes, and the public source has no
   credentials or generated build output.
4. Run `scripts/firmware-release.py` with the binary/version, private HTTPS download
   URL and public release URL. It checks image type, OTA size, version presence,
   URL safety and emits the manifest without copying the binary.
5. Publish the source and release notes to GitHub, then replace the stable manifest.
   Publish only stable intended versions: ESPHome flags any different version,
   so an older manifest can offer a downgrade.
6. Check from Home Assistant, then install manually. Confirm the actual native API
   project version, stable uptime beyond 60 seconds, retained scene/settings, HA
   update entity up-to-date, and a subsequent restart. An OTA transfer alone is
   not proof the new firmware booted. Obtain physical panel confirmation too.

The initial installation needs the existing ESPHome push-OTA path to introduce
this feature. Subsequent releases use the Home Assistant Install control.

This installation uses its DHCP gateway as the primary DNS resolver on each
Wi-Fi connection, so the private download hostname resolves through the home
network DNS. DHCP addresses and the secondary DNS resolver are retained.
