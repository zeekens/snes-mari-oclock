"""Reproducible SNTL v2 timing upgrade from immutable, lossless v1 assets.
Usage: python scripts/native-packs/build.py PUBLIC_CHECKOUT OUTPUT_DIRECTORY
No pixel re-quantization or conversion through GIF. Existing scene IDs are retained.
"""
from pathlib import Path
import hashlib,json,struct,sys
ROOT=Path(__file__).resolve().parents[2]
def build(public,out):
 catalog=json.loads((ROOT/'scene-packs/source-catalog.json').read_text())
 policies=json.loads((ROOT/'scene-packs/quality-policy.json').read_text())
 assert set(policies)=={s['id'] for s in catalog['scenes']}
 reports=[]
 for scene in catalog['scenes']:
  source=(public/scene['path']).read_bytes()
  assert hashlib.sha256(source).hexdigest()==scene['sha256'],scene['id']
  assert source[:5]==b'SNTL\x01' and struct.unpack_from('<I',source,36)[0]==len(source)
  policy=policies[scene['id']]; frames=struct.unpack_from('<H',source,8)[0]
  idle=policy.get('idle',[0,1]);action=policy.get('action',[0,frames]);mode=int(policy['mode']=='minute')
  assert all(0<=start<frames and 0<count<=frames-start for start,count in (idle,action))
  period=policy['frame_ms'];assert period in (60,80,100,120)
  blob=bytearray(source);blob[4]=2;struct.pack_into('<H',blob,12,period)
  blob+=struct.pack('<4s6H',b'TIME',mode,*idle,*action,policy.get('idle_ms',120))
  struct.pack_into('<I',blob,36,len(blob));assert len(blob)<=160*1024
  # The only changed bytes are the version, frame interval, length and appended schedule.
  assert blob[40:len(source)]==source[40:]
  relative=f"collections/native-clock-packs-2.0.0/{scene['library']}/packs/{scene['id']}.sntl"
  path=out/relative;path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(blob)
  scene.update(name=policy['name'],path=relative,bytes=len(blob),sha256=hashlib.sha256(blob).hexdigest())
  assert len(scene['name'].encode())<64
  reports.append(dict(id=scene['id'],library=scene['library'],**policy,bytes=len(blob),stored_frames=frames,pixel_payload_unchanged=True))
 data=json.dumps(catalog,separators=(',',':'))+'\n';assert len(data.encode())<=24000
 (out/'libraries/sntl-v1').mkdir(parents=True,exist_ok=True);(out/'libraries/sntl-v1/catalog.json').write_text(data)
 collection=out/'collections/native-clock-packs-2.0.0'
 (collection/'quality-report.json').write_text(json.dumps(reports,indent=2)+'\n')
 for group in catalog['libraries']:
  group_items=[dict(s,playback=policies[s['id']]) for s in catalog['scenes'] if s['library']==group['id']]
  (collection/group['id']/'manifest.json').write_text(json.dumps(dict(name=group['name'],version='2.0.0',format='SNTL',format_version=2,minimum_firmware='sntl-2.2.0',items=group_items),indent=2)+'\n')
 print(f'Built {len(reports)} scenes, {sum(p["mode"]=="minute" for p in policies.values())} minute-triggered; largest {max(s["bytes"] for s in catalog["scenes"])} bytes')
if __name__=='__main__':build(Path(sys.argv[1]),Path(sys.argv[2]))
