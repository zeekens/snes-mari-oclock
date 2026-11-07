"""Check the 2.3.0 collection and its offline archives against native metadata."""
from pathlib import Path
import json,hashlib,re,struct,zipfile,subprocess,tempfile
ROOT=Path(__file__).resolve().parents[2];COL=ROOT/'collections/native-clock-packs-2.3.0'
catalog=json.loads((COL/'catalog.json').read_text());assert len((COL/'catalog.json').read_bytes())<=24000
entries={s['id']:s for s in catalog['scenes']};assert len(entries)==45
frames=0
for n in (1,2,3):
 folder=COL/f'pack-{n}'
 with zipfile.ZipFile(COL/f'native-clock-pack-{n}-2.3.0.zip') as z:
  assert z.testzip() is None
  expected={p.relative_to(folder).as_posix() for p in folder.rglob('*') if p.is_file()}
  assert set(z.namelist())==expected
  for path in expected:assert z.read(path)==(folder/path).read_bytes()
  manifest=json.loads(z.read('manifest.json'));scenes=json.loads(z.read('previews/scenes.json'))
  assert len(manifest['items'])==len(scenes)==15
  assert {s['id'] for s in scenes}=={s['id'] for s in manifest['items']}=={s['id'] for s in entries.values() if s['library']==f'pack-{n}'}
  html=z.read('previews/index.html').decode()
  embedded=json.loads(re.search(r'<script type="application/json" id="scene-data">(.*?)</script>',html,re.S)[1]);assert embedded==scenes
  assert 'fetch(' not in html and not re.search(r'(?:src|href)=["\']https?://',html)
  for s,item in zip(scenes,manifest['items']):
   assert s['id']==item['id'];entry=entries[s['id']];b=z.read(item['path'])
   assert b==(ROOT/entry['path']).read_bytes() and item['catalog_path']==entry['path']
   assert len(b)==entry['bytes']==item['bytes']<=163840 and hashlib.sha256(b).hexdigest()==entry['sha256']==item['sha256']
   assert b[:5]==b'SNTL\x02';count=struct.unpack_from('<H',b,8)[0];frames+=count
   assert count==s['frames'] and struct.unpack_from('<H',b,12)[0]==s['frame_ms']==80
   mode,idle,idles,action,actions,idle_ms=struct.unpack('<6H',b[-12:])
   assert s['mode']==('minute' if mode else 'continuous')
   if mode:assert s['idle']==[idle,idles] and s['action']==[action,actions] and s['idle_ms']==idle_ms and actions*80<60000
   atlas=z.read('previews/'+s['id']+'.png');assert atlas[:8]==b'\x89PNG\r\n\x1a\n'
   assert struct.unpack('>II',atlas[16:24])==(640,64*((count+9)//10))
   offsets,stream,font=struct.unpack_from('<III',b,24);assert list(b[font:font+70])==s['font']
   for k in range(count):
    offset=stream+struct.unpack_from('<I',b,offsets+4*k)[0];assert list(b[offset:offset+8])==s['styles'][k]
  script=html.split('<script>')[1].split('</script>')[0]
  # Exercise real preview hour and midnight rollover without browser dependencies.
  logic=script[:script.index("document.querySelector('#minute').onclick")]
  check=logic+"\nminuteOfDay=12*60+59;next();if(displayedTime()!=='13:00')throw Error('hour');minuteOfDay=1439;next();if(displayedTime()!=='00:00')throw Error('midnight');"
  subprocess.run(['node','-e',"const document={querySelector:()=>({})};const performance={now:()=>0};"+check],check=True)
assert frames==12812
for line in (COL/'SHA256SUMS').read_text().splitlines():
 digest,path=line.split('  ',1);assert hashlib.sha256((COL/path).read_bytes()).hexdigest()==digest,path
print('2.3.0: 45 assets, 12812 frames, three offline ZIPs, native metadata, checksums and hour/midnight preview checks passed')
