"""Validate Pack 2's portable archive, catalog identity, and embedded preview data."""
import hashlib
import json
import re
import struct
from pathlib import Path
from zipfile import ZipFile

ROOT = Path(__file__).resolve().parents[2]
COLLECTION = ROOT / 'collections/native-clock-pack-2-2.2.0'
raw_catalog = (ROOT / 'libraries/sntl-v1/catalog.json').read_bytes()
catalog = json.loads(raw_catalog)
assert len(raw_catalog) <= 24000 and len(catalog['scenes']) == 45
entries = {s['id']: s for s in catalog['scenes']}
assert len(entries) == 45
for item in entries.values():
    raw = (ROOT / item['path']).read_bytes()
    assert len(raw) == item['bytes'] <= 160 * 1024
    assert hashlib.sha256(raw).hexdigest() == item['sha256']

with ZipFile(COLLECTION / 'native-clock-pack-2-2.2.0.zip') as archive:
    assert archive.testzip() is None
    manifest = json.loads(archive.read('pack-2/manifest.json'))
    assert manifest['version'] == '2.2.0' and len(manifest['items']) == 15
    scenes = json.loads(archive.read('previews/scenes.json'))
    html = archive.read('previews/index.html').decode()
    embedded = json.loads(re.search(r'<script type="application/json" id="scene-data">(.*?)</script>', html).group(1))
    assert scenes == embedded and len(scenes) == 15
    assert 'fetch(' not in html and not re.search(r'(?:src|href)=["\']https?://', html)
    assert 'native-clock-pack-2-2.2.0.zip' not in archive.read('README.md').decode()
    selected = {s['id'] for s in entries.values() if s['library'] == 'pack-2'}
    assert {s['id'] for s in scenes} == selected == {s['id'] for s in manifest['items']}
    previews = {s['id']: s for s in scenes}
    for item in manifest['items']:
        entry = entries[item['id']]
        raw = archive.read('pack-2/' + item['path'])
        assert item['catalog_path'] == entry['path']
        assert raw == (ROOT / entry['path']).read_bytes()
        assert len(raw) == item['bytes'] and hashlib.sha256(raw).hexdigest() == item['sha256']
        scene = previews[item['id']]
        frames = struct.unpack_from('<H', raw, 8)[0]
        period = struct.unpack_from('<H', raw, 12)[0]
        mode, idle, count, action, actions, idle_ms = struct.unpack('<6H', raw[-12:])
        assert scene['frames'] == frames and scene['frame_ms'] == period
        assert scene['mode'] == ('minute' if mode else 'continuous')
        if mode:
            assert scene['idle'] == [idle, count] and scene['action'] == [action, actions]
            assert scene['idle_ms'] == idle_ms
        atlas = archive.read('previews/' + item['id'] + '.png')
        assert atlas[:8] == b'\x89PNG\r\n\x1a\n'
        width, height = struct.unpack('>II', atlas[16:24])
        assert width == 640 and height == ((frames + 9) // 10) * 64
        frame_offsets, stream, font = struct.unpack_from('<III', raw, 24)
        assert list(raw[font:font+70]) == scene['font']
        for k in range(frames):
            offset = stream + struct.unpack_from('<I', raw, frame_offsets + k*4)[0]
            assert list(raw[offset:offset+8]) == scene['styles'][k]
    with ZipFile(ROOT / 'collections/native-clock-pack-2-2.1.1/native-clock-pack-2-2.1.1.zip') as previous:
        for name in previous.namelist():
            if name.endswith('.sntl') or name.startswith('previews/') and name.endswith(('.png', '.gif')):
                assert previous.read(name) == archive.read(name), name

for line in (COLLECTION / 'SHA256SUMS').read_text().splitlines():
    digest, name = line.split('  ', 1)
    assert hashlib.sha256((COLLECTION / name).read_bytes()).hexdigest() == digest, name
print('45 catalog assets; 15 portable Pack 2 scenes; unchanged previous assets; embedded preview metadata, timing, fonts, atlases and checksums passed')
