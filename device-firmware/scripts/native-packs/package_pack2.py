"""Rebuild the complete Pack 2 2.2.0 archive from its retained collection files."""
import hashlib
import json
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile, ZipInfo

REPO = Path(__file__).resolve().parents[3]
COLLECTION = REPO / 'collections/native-clock-pack-2-2.2.0'
ARCHIVE = 'native-clock-pack-2-2.2.0.zip'


def build():
    manifest = json.loads((COLLECTION / 'pack-2/manifest.json').read_text())
    assert len(manifest['items']) == len({s['id'] for s in manifest['items']}) == 15
    for item in manifest['items']:
        assert item['library'] == 'pack-2'
        raw = (COLLECTION / 'pack-2' / item['path']).read_bytes()
        # Historical archives retain their own asset identity as the live catalog advances.
        assert raw == (REPO / item['catalog_path']).read_bytes()
        assert len(raw) == item['bytes']
        assert hashlib.sha256(raw).hexdigest() == item['sha256']
    paths = sorted(p for p in COLLECTION.rglob('*') if p.is_file()
                   and p.name not in (ARCHIVE, ARCHIVE + '.tmp', 'SHA256SUMS')
                   and '__pycache__' not in p.parts)
    temporary = COLLECTION / (ARCHIVE + '.tmp')
    with ZipFile(temporary, 'w') as archive:
        for path in paths:
            name = path.relative_to(COLLECTION).as_posix()
            raw = path.read_bytes()
            if name == 'README.md':
                raw = raw.replace(f'[Download all 15 scenes]({ARCHIVE}) · '.encode(), b'')
            info = ZipInfo(name, (2026, 11, 7, 0, 0, 0))
            info.external_attr = 0o100644 << 16
            archive.writestr(info, raw, compress_type=ZIP_DEFLATED)
    temporary.replace(COLLECTION / ARCHIVE)
    paths.append(COLLECTION / ARCHIVE)
    (COLLECTION / 'SHA256SUMS').write_text(''.join(
        hashlib.sha256(p.read_bytes()).hexdigest() + '  ' + p.relative_to(COLLECTION).as_posix() + '\n'
        for p in sorted(paths)))
    print(f'Pack 2: 15 scenes, {len(paths)-1} archive members, historical manifest hashes verified')


if __name__ == '__main__':
    build()
