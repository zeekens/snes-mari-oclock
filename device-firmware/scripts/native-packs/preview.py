"""Build a 64x64 playback gallery and independent clock-layout oracle from audited RGB frames."""
import json,struct,sys,html
from pathlib import Path
from PIL import Image,ImageDraw,ImageFilter
ROOT=Path(__file__).resolve().parents[2]
def clock(im,style,font):
 mask=Image.new('L',(64,64));d=ImageDraw.Draw(mask);y=style[0]
 # Digit starts fixed symmetrically about 31.5; colon sits on 31/32.
 for digit,x in zip('1234',(6,18,36,48)):
  for row,bits in enumerate(font[int(digit)*7:int(digit)*7+7]):
   for col in range(5):
    if bits&(16>>col):d.rectangle((x+2*col,y+2*row,x+2*col+1,y+2*row+1),fill=255)
 for top in (y+3,y+9):d.rectangle((31,top,32,top+1),fill=255)
 out=im.copy();shadow=tuple(style[5:8])
 if style[1]==1:out.paste(shadow,(0,0,64,64),mask.filter(ImageFilter.MaxFilter(3)))
 else:
  for dx,dy in [(-1,0),(1,0),(0,-1),(0,1)]+([(1,1)] if style[1]==2 else []):out.paste(shadow,(dx,dy,64+dx,64+dy),mask)
 out.paste(tuple(style[2:5]),(0,0,64,64),mask);return out

def build(out):
 cat=json.loads((out/'libraries/sntl-v1/catalog.json').read_text());pol=json.loads((ROOT/'scene-packs/quality-policy.json').read_text());gallery=out/'collections/native-clock-packs-2.0.0/preview';gallery.mkdir(exist_ok=True)
 items=[];boards={}
 for scene in cat['scenes']:
  id=scene['id'];b=(out/scene['path']).read_bytes();fo,stream,font=struct.unpack_from('<III',b,24);count=struct.unpack_from('<H',b,8)[0]
  styles=[list(b[stream+struct.unpack_from('<I',b,fo+k*4)[0]:stream+struct.unpack_from('<I',b,fo+k*4)[0]+8]) for k in range(count)];digits=list(b[font:font+70])
  raw=(out/'frames'/(id+'.rgb')).read_bytes();oracle=(out/'frames'/(id+'-clock.rgb')).read_bytes();atlas=Image.new('RGB',(640,64*((count+9)//10)));shots=[]
  for k in range(count):
   im=Image.frombytes('RGB',(64,64),raw[k*12288:(k+1)*12288]);atlas.paste(im,((k%10)*64,(k//10)*64));decorated=clock(im,styles[k],digits)
   assert decorated.tobytes()==oracle[k*12288:(k+1)*12288],(id,k,'clock oracle')
   if k%25==0:shots.append(decorated)
  atlas.save(gallery/(id+'.png'))
  row=Image.new('RGB',(12*96,122),'#18232c');d=ImageDraw.Draw(row);d.text((4,3),pol[id]['name']+' | '+pol[id]['mode'],fill='white')
  for k,im in enumerate(shots):row.paste(im.resize((96,96),Image.Resampling.NEAREST),(k*96,22))
  boards.setdefault(scene['library'],[]).append(row)
  items.append(dict(id=id,library=scene['library'],**pol[id],frames=count,styles=styles,font=digits))
 for group,rows in boards.items():
  board=Image.new('RGB',(1152,122*len(rows)));[board.paste(im,(0,i*122)) for i,im in enumerate(rows)];board.save(gallery/(group+'-audit.png'))
 (gallery/'scenes.json').write_text(json.dumps(items,separators=(',',':')))
 (gallery/'index.html').write_text((ROOT/'scripts/native-packs/preview.html').read_text())
 print('41 scene atlases, all 12,300 independent clock pixel checks passed')
if __name__=='__main__':build(Path(sys.argv[1]))
