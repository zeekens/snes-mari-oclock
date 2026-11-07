"""Reproduce the immutable 2.3.0 archives from retained, verified collection files."""
from pathlib import Path
import hashlib,json,zipfile
REPO=Path(__file__).resolve().parents[3]
COLLECTION=REPO/'collections/native-clock-packs-2.3.0'
def build():
 catalog=json.loads((COLLECTION/'catalog.json').read_text())
 entries={s['id']:s for s in catalog['scenes']}
 assert len(entries)==45
 archives=[]
 for n in (1,2,3):
  group=f'pack-{n}';folder=COLLECTION/group
  manifest=json.loads((folder/'manifest.json').read_text())
  assert manifest['version']=='2.3.0' and len(manifest['items'])==15
  assert {s['id'] for s in manifest['items']}=={s['id'] for s in entries.values() if s['library']==group}
  for item in manifest['items']:
   entry=entries[item['id']];raw=(folder/item['path']).read_bytes()
   assert item['catalog_path']==entry['path'] and raw==(REPO/entry['path']).read_bytes()
   assert len(raw)==item['bytes']==entry['bytes']<=160*1024
   assert hashlib.sha256(raw).hexdigest()==item['sha256']==entry['sha256']
  archive=COLLECTION/f'native-clock-{group}-2.3.0.zip'
  with zipfile.ZipFile(archive.with_suffix('.zip.tmp'),'w') as z:
   for path in sorted(folder.rglob('*')):
    if not path.is_file():continue
    info=zipfile.ZipInfo(path.relative_to(folder).as_posix(),(2026,11,7,0,0,0))
    info.external_attr=0o100644<<16
    z.writestr(info,path.read_bytes(),compress_type=zipfile.ZIP_DEFLATED)
  archive.with_suffix('.zip.tmp').replace(archive);archives.append(archive)
 paths=sorted(p for p in COLLECTION.rglob('*') if p.is_file() and p.name!='SHA256SUMS' and '__pycache__' not in p.parts)
 (COLLECTION/'SHA256SUMS').write_text(''.join(hashlib.sha256(p.read_bytes()).hexdigest()+'  '+p.relative_to(COLLECTION).as_posix()+'\n' for p in paths))
 print('Rebuilt three 2.3.0 archives with 15 verified scenes each')
 return archives
if __name__=='__main__':build()
