"""Opaque-pixel regression tests for the six corrected scenes."""
from pathlib import Path
import json,math
from unittest.mock import patch
from PIL import Image
import render_fixes as B
ROOT=B.ROOT;OUT=ROOT/'build/scene-audit-2026-11-07';OUT.mkdir(exist_ok=True)
report=[]
for id in ('metroid-04','zelda-03','mariokart-04'):
 frames=[]
 for k in range(300):
  im,actors=B.render(id,k);frames.append(im)
  for j,(name,p) in enumerate(actors):
   assert all(0<=x<64 and 21<=y<64 for x,y in p),(id,k,name,'bounds')
   for name2,q in actors[j+1:]:assert not p&q,(id,k,name,name2,'overlap')
   if id=='mariokart-04':
    bottom=max(y for x,y in p);depth=(bottom-28)/35;half=6+depth*25;center=32+math.sin(k/25*math.tau/12)*(1-depth)*7
    assert all(round(center-half)<=x<=round(center+half) for x,y in p if y==bottom),(id,k,name,'tyres off road')
 (OUT/(id+'-fixed.rgb')).write_bytes(b''.join(im.tobytes() for im in frames))
 report.append(dict(id=id,frames=300,opaque_actor_separation=True,actor_bounds=True,tyres_on_road=id=='mariokart-04'))
 print(id,'passed',flush=True)
# Capture the rendered opaque actor masks rather than only bounding boxes.
original=Image.Image.paste;captured=[];captured_art=[]
def capture(self,im,box=None,mask=None):
 if isinstance(im,Image.Image) and im.mode=='RGBA' and mask is not None:
  captured.append(B.occupied(im,*box[:2]));captured_art.append(im)
 return original(self,im,box,mask)
for id,count,render in [('ice-climber-01',128,B.ice),('ffvi-14',300,B.render_rooftops)]:
 frames=[]
 if id=='ffvi-14':B.ff_setup()
 else:B.ice(0)
 with patch.object(Image.Image,'paste',capture):
  for k in range(count):
   captured.clear();captured_art.clear();im=render(k);frames.append(im)
   solid=({(x,y) for x in range(64) for y in range(42,45) if x<=17 or x>=35}|{(x,y) for x in range(64) for y in range(61,64)}) if id=='ice-climber-01' else B.roof_geometry(k/25)
   assert len(captured)==2,(id,k,len(captured))
   for p in captured:
    assert not p&solid,(id,k,'solid surface collision',sorted(p&solid)[:5])
    assert all(0<=x<64 and 21<=y<64 for x,y in p),(id,k,'bounds')
   assert not captured[0]&captured[1],(id,k,'actors overlap')
   if id=='ice-climber-01':
    popo=captured[1]
    supported=any((x,y+1) in solid for x,y in popo)
    if not supported:
     # An unsupported actor must use the airborne hammer pose, not walk.
     from PIL import ImageOps
     pose=captured_art[1].tobytes()
     assert pose in (B.ice.module.POPO[3].tobytes(),ImageOps.mirror(B.ice.module.POPO[3]).tobytes()),(id,k,'walking without ground support')
    if k>=92 and k<=111:
     feet=max(y for x,y in popo)
     if k>92:assert feet>=previous_feet and feet-previous_feet<=2,(id,k,'discontinuous fall')
     previous_feet=feet
 (OUT/(id+'-fixed.rgb')).write_bytes(b''.join(im.tobytes() for im in frames))
 report.append(dict(id=id,frames=count,opaque_actor_separation=True,solid_surface_clearance=True,actor_bounds=True))
 if id=='ice-climber-01':report[-1]['grounded_walking_and_continuous_fall']=True
 print(id,'passed',flush=True)
(OUT/'geometry-validation.json').write_text(json.dumps(report,indent=2)+'\n')
frames=[];B.ff_setup()
with patch.object(Image.Image,'paste',capture):
 for k in range(300):
  captured.clear();frames.append(B.render_interceptor(k))
  assert 1<=len(captured)<=2
  if len(captured)==2:
   assert not captured[0]&captured[1],('ffvi-07',k,'dog overlaps Shadow')
   assert all(21<=y<62 for x,y in captured[1]),('ffvi-07',k,'foreground ledge')
(OUT/'ffvi-07-fixed.rgb').write_bytes(b''.join(im.tobytes() for im in frames))
report.append(dict(id='ffvi-07',frames=300,opaque_actor_separation=True,solid_surface_clearance=True,intentional_edge_entry_exit=True))
(OUT/'geometry-validation.json').write_text(json.dumps(report,indent=2)+'\n')
print('ffvi-07 passed',flush=True)
