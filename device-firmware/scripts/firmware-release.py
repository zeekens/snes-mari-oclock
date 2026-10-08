"""Create a public ESPHome update manifest; never copy the private binary to GitHub."""
import argparse
import hashlib
import json
import re
from pathlib import Path
from urllib.parse import urlsplit

MAX_OTA_BYTES = 0x1C0000


def manifest(binary: bytes, version: str, firmware_url: str, release_url: str, summary: str):
    if not re.fullmatch(r'sntl-\d+\.\d+\.\d+', version):
        raise ValueError('Use a versioned sntl-X.Y.Z release')
    if not 256 <= len(binary) <= MAX_OTA_BYTES or binary[0] != 0xE9:
        raise ValueError('Not an ESP32 OTA application or exceeds the OTA slot')
    # esp_app_desc_t begins immediately after the first segment header.
    if binary[32:36] != bytes.fromhex('3254cdab'):
        raise ValueError('Expected OTA app descriptor; do not use a factory image')
    # ESPHome stores its own version in esp_app_desc_t, and the project version
    # as a separate NUL-terminated constant reported by the native API.
    if version.encode('ascii') + b'\0' not in binary:
        raise ValueError('Requested project version is absent from the binary')
    for url in (firmware_url, release_url):
        parsed = urlsplit(url)
        if parsed.scheme != 'https' or not parsed.hostname or parsed.username or parsed.password or parsed.query or parsed.fragment:
            raise ValueError('URLs must use HTTPS and contain no credentials or query tokens')
    if not firmware_url.endswith('/' + version + '.bin'):
        raise ValueError('Use an immutable, versioned firmware filename')
    return {
        'name': 'SNES Clock firmware', 'version': version,
        'builds': [{'chipFamily': 'ESP32', 'ota': {
            'path': firmware_url, 'md5': hashlib.md5(binary).hexdigest(),
            'sha256': hashlib.sha256(binary).hexdigest(), 'size': len(binary),
            'release_url': release_url, 'summary': summary,
        }}],
    }


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--binary', type=Path, required=True)
    p.add_argument('--version', required=True)
    p.add_argument('--firmware-url', required=True)
    p.add_argument('--release-url', required=True)
    p.add_argument('--summary', required=True)
    p.add_argument('--output', type=Path, required=True)
    a = p.parse_args()
    result = manifest(a.binary.read_bytes(), a.version, a.firmware_url, a.release_url, a.summary)
    a.output.parent.mkdir(parents=True, exist_ok=True)
    a.output.write_text(json.dumps(result, indent=2) + '\n')
    print(f'Manifest written: {a.output}; private binary is not copied')
