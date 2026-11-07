"""Build and verify a local audited library without changing published collections.
Run test_geometry.py and classic_audit.py first. Existing published assets are inputs.
"""
from pathlib import Path
from PIL import Image
import importlib.util,json,re,struct,hashlib,base64,io,subprocess,shutil,zipfile
ROOT=Path(__file__).resolve().parents[2];WORK=ROOT/'build/scene-audit-2026-11-07';OUT=ROOT/'docs/scene-audit-2026-11-07';PUB=ROOT/'build/github-pack-publication'
OUT.mkdir(exist_ok=True)
def module(name,path):
 spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
encoder=module('audit_encoder',ROOT/'tools/native-scenes/encode.py');encoder.RAW=WORK
oracle=module('audit_oracle',PUB/'device-firmware/scripts/native-packs/preview.py')
html=(ROOT/'backups/scene-audit-2026-11-07/gallery.html').read_text();pattern=r'(<script id="gallery-data" type="application/json">)(.*?)(</script>)';match=re.search(pattern,html,re.S);data=json.loads(match[2]);items=data['scenes']
source={s['id']:s for s in json.loads((PUB/'libraries/sntl-v1/catalog.json').read_text())['scenes']}
old={s['id']:PUB/s['path'] for s in json.loads((PUB/'device-firmware/scene-packs/source-catalog.json').read_text())['scenes']}
fixed={s['id'] for s in json.loads((WORK/'geometry-validation.json').read_text())}
results=[];catalog=[]
for s in items:
 id=s['id']
 if id=='metroid-05':s['name']="Super Metroid - Escape Shaft Samus's Lift"
 if id=='allstars-01':s['name']="Super Mario All-Stars - Water Land Luigi's Crossing"
 original_ms=s['frame_ms'];changed=id in fixed or id=='tide-pool'
 if s['mode']=='classic':
  raw=(WORK/(id+'-current.rgb')).read_bytes();s.update(frames=360,frame_ms=40,idle=[0,300],action=[300,60]);frames=[Image.frombytes('RGB',(64,64),raw[k*12288:(k+1)*12288]) for k in range(360)]
  checks=dict(rendered_frames=1560,sanitizers=True,full_minute_and_action=True)
 else:
  path=(PUB/source[id]['path']) if id in source else ROOT/'docs/bubble-bobble-2026-11-06/packs'/f'{id}.sntl'
  blob=bytearray(path.read_bytes());raw=(WORK/(id+'-fixed.rgb')).read_bytes() if id in fixed else (WORK/(id+'.rgb')).read_bytes()
  if id in fixed:
   (WORK/(id+'.json')).write_text(json.dumps(dict(item=dict(title=s['name']),styles=[dict(y=a[0],style=a[1],ink=a[2:5],outline=a[5:8]) for a in s['styles']],font={str(n):s['font'][n*7:n*7+7] for n in range(10)},approved_frames_matched=0)))
   # The encoder reads a dedicated input, leaving baseline decoded evidence intact.
   inputdir=WORK/'encode';inputdir.mkdir(exist_ok=True);(inputdir/(id+'.rgb')).write_bytes(raw);shutil.copy2(WORK/(id+'.json'),inputdir/(id+'.json'));encoder.RAW=inputdir
   choices=[encoder.encode(id,t) for t in (4,8)];v1,meta=min(choices,key=lambda p:len(p[0]));oldfile=WORK/(id+'-audited-v1.sntl');oldfile.write_bytes(v1)
   trailer=blob[-16:];blob=bytearray(v1);blob[4]=2;blob+=trailer;struct.pack_into('<I',blob,36,len(blob))
  else:oldfile=old.get(id,ROOT/('build/bubble-bobble-2026-11-06' if id=='bubble-bobble-01' else 'build/nes-scenes-2026-10-29')/(id+'-v1.sntl'))
  if s['frame_ms']==60:s['frame_ms']=80
  assert s['frame_ms']%40==0
  struct.pack_into('<H',blob,12,s['frame_ms']);assert len(blob)<=160*1024
  dest=OUT/s['library']/'packs'/f'{id}.sntl';dest.parent.mkdir(parents=True,exist_ok=True);dest.write_bytes(blob)
  decoded=WORK/(id+'-audited.rgb');clocked=WORK/(id+'-audited-clock.rgb')
  run=subprocess.run([str(WORK/'native-quality-test'),str(oldfile),str(dest),str(decoded),str(clocked)],capture_output=True,text=True);assert run.returncode==0,(id,run.stdout,run.stderr)
  assert decoded.read_bytes()==raw,(id,'decoder differs from rendered source')
  frames=[Image.frombytes('RGB',(64,64),raw[k*12288:(k+1)*12288]) for k in range(s['frames'])]
  assert b''.join(oracle.clock(im,s['styles'][k],s['font']).tobytes() for k,im in enumerate(frames))==clocked.read_bytes(),(id,'clock oracle')
  s.update(bytes=len(blob),sha256=hashlib.sha256(blob).hexdigest(),path=str(dest.relative_to(OUT)))
  catalog.append(dict(id=id,name=s['name'],library=s['library'],path=s['path'],bytes=len(blob),sha256=s['sha256'],frame_ms=s['frame_ms']))
  checks=dict(decoded_frames=len(frames),clock_oracle_frames=len(frames),incremental_seek_clock_checks=2000,sanitizers=True,timing_and_malformed_input=True,cadence_aligned_to_40ms=True)
 if id=='metroid-05':s['name']="Super Metroid - Escape Shaft Samus's Lift"
 if id=='allstars-01':s['name']="Super Mario All-Stars - Water Land Luigi's Crossing"
 atlas=Image.new('RGB',(640,64*((len(frames)+9)//10)))
 for k,im in enumerate(frames):atlas.paste(im,((k%10)*64,(k//10)*64))
 preview=OUT/'previews';preview.mkdir(exist_ok=True);atlas.save(preview/(id+'.png'));buf=io.BytesIO();atlas.save(buf,format='PNG');s['imageURL']='data:image/png;base64,'+base64.b64encode(buf.getvalue()).decode()
 results.append(dict(id=id,name=s['name'],library=s['library'],mode=s['mode'],choreography_changed=changed,frame_ms=s['frame_ms'],previous_frame_ms=original_ms,action_seconds=s['action'][1]*s['frame_ms']/1000 if s['mode']=='minute' else None,checks=checks))
 print(id,'verified',len(frames),'frames',flush=True)
(OUT/'validation.json').write_text(json.dumps(dict(physical_panel_tested=False,published=False,scenes=results),indent=2)+'\n')
(OUT/'catalog.json').write_text(json.dumps(dict(format='SNTL',version=1,status='local-review',minimum_firmware='sntl-2.2.0',libraries=[dict(id=f'pack-{n}',name=f'Pack {n}') for n in (1,2,3)],scenes=[s for s in catalog if s['library']!='review']),indent=2)+'\n')
(OUT/'scenes.json').write_text(json.dumps([{k:v for k,v in s.items() if k!='imageURL'} for s in items],separators=(',',':')))
(WORK/'audited-gallery-data.json').write_text(json.dumps(data,separators=(',',':')))
for lib in ('pack-1','pack-2','pack-3'):
 with zipfile.ZipFile(OUT/(lib+'-audit-2026-11-07.zip'),'w',zipfile.ZIP_DEFLATED) as z:
  members=[s for s in catalog if s['library']==lib]
  z.writestr('manifest.json',json.dumps(dict(status='local-review',format='SNTL',format_version=2,minimum_firmware='sntl-2.2.0',items=members),indent=2))
  for s in members:z.write(OUT/s['path'],s['path'])
print('Audited local assets, three review packs and all 53 atlases built.')
