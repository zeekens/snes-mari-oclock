from pathlib import Path
import base64,hashlib,io,json,re,struct,zipfile,subprocess,shlex
from PIL import Image
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'docs/scene-audit-2026-11-07'
html=(OUT/'index.html').read_text();pattern=r'<script id="gallery-data" type="application/json">(.*?)</script>';data=json.loads(re.search(pattern,html,re.S)[1]);prior=json.loads(re.search(pattern,(ROOT/'backups/scene-audit-2026-11-07/gallery.html').read_text(),re.S)[1]);assert data['artwork']==prior['artwork'];assert {s['id'] for s in data['scenes']}=={s['id'] for s in prior['scenes']};assert len(data['scenes'])==53
cat=json.loads((OUT/'catalog.json').read_text());assert cat['format']=='SNTL' and cat['version']==1;assert len((OUT/'catalog.json').read_bytes())<=24000
parser=ROOT/'build/scene-audit-2026-11-07/catalog-test'
cjson=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','libcjson'],text=True))
subprocess.run(['clang++','-std=c++17','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fsanitize=address,undefined','-I'+str(ROOT/'build/github-pack-publication/device-firmware/firmware/include'),str(Path(__file__).with_name('catalog.cpp')),*cjson,'-o',str(parser)],check=True)
subprocess.run([str(parser),str(OUT)],check=True)
for lib,count in [('pack-1',15),('pack-2',14),('pack-3',15)]:
 with zipfile.ZipFile(OUT/(lib+'-audit-2026-11-07.zip')) as z:
  assert z.testzip() is None;items=json.loads(z.read('manifest.json'))['items'];assert len(items)==count
  for s in items:
   b=z.read(s['path']);assert b==(OUT/s['path']).read_bytes();assert hashlib.sha256(b).hexdigest()==s['sha256'];assert len(b)==s['bytes']<=163840;assert s in cat['scenes']
for s in data['scenes']:
 atlas=Image.open(io.BytesIO(base64.b64decode(s['imageURL'].split(',')[1])));assert atlas.size==(640,64*((s['frames']+9)//10))
 assert s['auditNote']
 if s['mode']=='classic':assert s['frame_ms']==40 and s['action']==[300,60]
 else:
  blob=(OUT/s['path']).read_bytes();assert struct.unpack_from('<H',blob,12)[0]==s['frame_ms']==80
  assert hashlib.sha256(blob).hexdigest()==s['sha256'];assert struct.unpack_from('<H',blob,8)[0]==s['frames']
  if s['mode']=='minute':assert s['action'][1]*80<60000 and s['idle_ms']%40==0
script=html.split('</script><script>',1)[1].split('</script>',1)[0];js=ROOT/'build/scene-audit-2026-11-07/gallery-check.js';js.write_text(script);subprocess.run(['node','--check',str(js)],check=True)
subprocess.run(['node',str(Path(__file__).with_name('gallery_test.js')),str(js)],check=True)
report=dict(scenes=len(data['scenes']),pack_counts=[sum(s['library']==lib['id'] for s in cat['scenes']) for lib in cat['libraries']],native_frames=sum(s['frames'] for s in data['scenes'] if s['mode']!='classic'),firmware_catalog_parser=True,all_pack_hashes_and_zip_members_match=True,all_atlases_valid=True,all_native_cadences_ms=80,original_reference_art_preserved=True,gallery_javascript_syntax=True,physical_panel_tested=False)
(OUT/'delivery-validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
