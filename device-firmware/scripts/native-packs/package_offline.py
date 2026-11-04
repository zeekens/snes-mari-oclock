"""Build the Pack 1 and Pack 3 archive revisions from the published 2.0.0 assets."""
import hashlib
import json
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile, ZipInfo

REPO = Path(__file__).resolve().parents[3]
SOURCE = REPO / 'collections/native-clock-packs-2.0.0'
DEST = REPO / 'collections/native-clock-packs-2.0.1'
VERSION = '2.0.1'


def gallery(template, pack_name, scenes, version=VERSION):
    start = template.index('<select id="library">')
    end = template.index('</select>', start) + len('</select>')
    selector = f'<select id="library"><option value="all">{pack_name} · {len(scenes)} scenes</option></select>'
    page = template[:start] + selector + template[end:]
    page = page.replace('SNES clock · All three packs', f'SNES clock · {pack_name} · {version}')
    # Embedded metadata and local PNG atlases also work under file://.
    metadata = json.dumps(scenes, separators=(',', ':')).replace('<', r'\u003c')
    page = page.replace('</footer>', f'</footer><script type="application/json" id="scene-data">{metadata}</script>')
    loader = "fetch('scenes.json').then(r=>r.json())"
    if page.count(loader) != 1:
        raise ValueError('Preview template loader has changed')
    return page.replace(loader, "Promise.resolve(JSON.parse(document.getElementById('scene-data').textContent))")


def package(group, catalog, previews, template):
    manifest = json.loads((SOURCE / group / 'manifest.json').read_text())
    manifest['version'] = VERSION
    members = {}
    selected = {item['id']: item for item in catalog if item['library'] == group}
    scenes = []
    for item in manifest['items']:
        entry = selected.pop(item['id'])
        if any(item[key] != value for key, value in entry.items()):
            raise ValueError(f'Catalog disagrees with {item["id"]}')
        raw = (REPO / entry['path']).read_bytes()
        if len(raw) != entry['bytes'] or hashlib.sha256(raw).hexdigest() != entry['sha256']:
            raise ValueError(f'Bad scene asset: {item["id"]}')
        item['catalog_path'] = entry['path']
        item['path'] = 'packs/' + item['id'] + '.sntl'
        members[item['path']] = raw
        atlas = item['id'] + '.png'
        members['previews/' + atlas] = (SOURCE / 'preview' / atlas).read_bytes()
        scenes.append(previews[item['id']])
    if selected:
        raise ValueError(f'Missing scenes: {list(selected)}')
    members['manifest.json'] = (json.dumps(manifest, indent=2) + '\n').encode()
    members['previews/index.html'] = gallery(template, manifest['name'], scenes).encode()
    members['README.md'] = (
        f"# {manifest['name']} {VERSION}\n\n"
        "Extract the whole ZIP and open `previews/index.html` in a browser. "
        "Use **Next minute** to see the clock advance and trigger minute actions.\n\n"
        "The manifest lists each local scene relative to `manifest.json`. "
        "The separate `catalog_path` is the location used by the online clock catalog.\n"
    ).encode()
    target = DEST / f'native-clock-{group}-{VERSION}.zip'
    temp = target.with_suffix('.zip.tmp')
    with ZipFile(temp, 'w') as archive:
        for name, data in sorted(members.items()):
            entry = ZipInfo(name, (2026, 11, 4, 0, 0, 0))
            entry.external_attr = 0o100644 << 16
            archive.writestr(entry, data, compress_type=ZIP_DEFLATED)
    temp.replace(target)
    return target


if __name__ == '__main__':
    DEST.mkdir(parents=True, exist_ok=True)
    catalog = json.loads((REPO / 'libraries/sntl-v1/catalog.json').read_text())['scenes']
    previews = {s['id']: s for s in json.loads((SOURCE / 'preview/scenes.json').read_text())}
    template = Path(__file__).with_name('preview.html').read_text()
    archives = [package(group, catalog, previews, template) for group in ('pack-1', 'pack-3')]
    (DEST / 'SHA256SUMS').write_text(''.join(
        hashlib.sha256(path.read_bytes()).hexdigest() + '  ' + path.name + '\n' for path in archives
    ))
