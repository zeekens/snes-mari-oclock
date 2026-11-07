"""Local choreography revision; published collections and historical renderers stay intact."""
from pathlib import Path
import sys,math,importlib.util,json,inspect,runpy
from unittest.mock import patch
from PIL import Image,ImageDraw
ROOT=Path(__file__).resolve().parents[2]
V=ROOT/'docs/snes-expansion-75/variety'
sys.path[:0]=[str(V),str(V.parent)]
import render_round4 as F
R=F.R;N=F.N;B=F.B

def occupied(art,x,y):
 return {(x+xx,y+yy) for yy in range(art.height) for xx in range(art.width) if art.getpixel((xx,yy))[3]}

def actors():
 result=[]
 for name,i,x,y,w,h,right in R.ACTOR_LOG:
  art=F.SPR[name][i]
  if right!=F.META[name]['native_faces_right']:art=art.transpose(Image.Transpose.FLIP_LEFT_RIGHT)
  result.append((name,occupied(art,x,y)))
 return result

def render(id,k):
 t=k/25
 if id=='metroid-04':
  game=next(g for g in N.GAMES if g[0]=='metroid');recipe=game[6][3]
  R.ACTOR_LOG=[];R.background(recipe[1],t,False);R.atmosphere(recipe[3],t)
  # Three 18-pixel creatures have fixed, separate horizontal lanes.
  for j in range(3):N.actor('metroid',11+j*21,47+8*N.wave(t+j*2,6),t+j,True,1)
  for x in [4,30,58]:N.line([(x,63),(x+3*N.wave(t,3),55),(x-2,49)],'#55a398',2)
 elif id=='zelda-03':
  game=next(g for g in N.GAMES if g[0]=='zelda');recipe=game[6][2]
  R.ACTOR_LOG=[];B.BASE_BACKGROUND(recipe[1],t,False);R.atmosphere(recipe[3],t)
  N.ellipse(3,47,58,14,'#45689d');N.ellipse(8,49,48,8,'#91bfd0',True)
  N.actor('fairy',32,56+round(N.wave(t,3)),t,True,1)
  # Side lanes keep the small fairies out of the Great Fairy's silhouette.
  for j,(x,foot) in enumerate([(10,40),(54,38),(11,55)]):
   N.actor('fairy_small',x+N.wave(t+j,6),foot+2*N.wave(t+j*2,6),t+j,True,3)
  R.im.paste(N.FAIRY_TOP,(0,0))
 elif id=='mariokart-04':
  original=F.sprite
  def on_track(name,x,foot,stamp=0,right=True,pose=None):
   if name=='kart_mario':
    # A fixed centre lane stays inside every bend and bounds the tile dictionary.
    x=32
   original(name,x,foot,stamp,right,pose)
  F.sprite=on_track
  try:F.kart(4,t)
  finally:F.sprite=original
  return B.BEFORE_CLOCK.copy(),actors()
 else:raise KeyError(id)
 return R.im.copy(),actors()

FF=None

def ff_setup():
 global FF
 if FF is not None:return FF
 with patch.object(Path,'write_text',return_value=0),patch.object(Image.Image,'save',return_value=None):
  ns=runpy.run_path(str(ROOT/'docs/ff6-clock-concepts/revisions/2/render.py'))
 blend=ns['M']['blend'];g=blend.__globals__;s=inspect.getsource(blend)
 s=s[:s.index(' digits=Image.new')]+" return im\n";exec(s.replace('def blend(','def raw_blend('),g)
 FF=ns,g
 return FF

def roof_geometry(t):
 shift=round(t*16)%64
 solid=set()
 for x in range(64):
  u=(x+shift)%64
  if u<=43:
   top=round(63-u*7/12) if u<12 else 56 if u<=35 else round(56+(u-35)*7/8)
   solid.update((x,y) for y in range(top,64))
 return solid

def rooftops(t):
 ns,g=ff_setup();rg=ns['rg'];r,poly,line,actor=rg['r'],rg['poly'],rg['line'],rg['actor']
 rg['start']('#283454');r(0,27,64,24,'#5e6271')
 for j in range(6):
  x=(j*17-round(t*85/12))%85-16
  rg['stone'](x,30+j%3,12,26,'#535565','#797b80','#35394d');rg['window'](x+3,37,5,8,'#b6a17b')
 shift=round(t*16)%64
 # Lower the roofs so full-size actors can clear gaps without entering the clock.
 solid=roof_geometry(t)
 for x,y in solid:rg['B']['im'].putpixel((x,y),(77,67,72))
 for x in range(64):
  ys=[y for xx,y in solid if xx==x]
  if ys:rg['B']['im'].putpixel((x,min(ys)),(182,137,105))
 for name,x,delay in [('locke',35,0),('celes',11,.35)]:
  # Take off on the flat roof, cross the entire gap, and land on the next flat.
  phase=(x+8+shift-32)%64
  jump=round(6*math.sin(phase/44*math.pi)) if phase<44 else 0
  actor(name,x,56-jump,t-delay,face='right')
 ns['M']['omit_clock'](3,'#ffe0b5')

def render_rooftops(k):
 ns,g=ff_setup();rg=ns['rg']
 rg['SCENES']=[tuple(rooftops if j==5 and s[0]==14 else v for j,v in enumerate(s)) for s in rg['SCENES']]
 item=next(dict(a) for a in ns['M']['items'] if a['n']==14);item['animated']='native'
 return g['raw_blend'](item,k)


def ice(k):
    from PIL import ImageOps
    spec=importlib.util.spec_from_file_location("nes_scenes",ROOT/"tools/nes-scenes/build.py")
    if not hasattr(ice,"module"):
        ice.module=importlib.util.module_from_spec(spec);spec.loader.exec_module(ice.module)
    POPO,NANA,paste=ice.module.POPO,ice.module.NANA,ice.module.paste

    im=Image.new('RGB',(64,64),(0,0,0)); d=ImageDraw.Draw(im)
    # Open mountain sky is also the clock's natural negative space.
    for x,y in [(2,24),(59,31),(7,45),(53,52),(30,25)]:
        d.point((x,y+(k//6)%3),fill=(88,127,170))
    for y,segments in [(42,[(0,17),(35,63)]),(61,[(0,63)])]:
        for left,right in segments:
            d.rectangle((left,y,right,min(y+2,63)),fill=(133,171,255))
            d.line((left,y,right,y),fill=(255,255,255))
            for x in range(left,right,7):
                d.line((x,y+1,x,min(y+3,63)),fill=(60,89,171))
    # Nana stands on the upper shelf, Popo climbs through the existing gap.
    paste(im,NANA,46,42)
    x,feet,pose=10,61,0
    if k>=16:
        t=k-16
        if t<12:
            x=10+8*t/11;pose=1+(t//4)%2
        elif t<48:
            q=(t-12)/35; x=18+17*q; feet=round(61-19*q-8*math.sin(math.pi*q)); pose=3
        elif t<60:
            x=35;feet=42;pose=0 if t<54 else 3
        elif t<76:
            # Jump off the shelf into the opening; never walk unsupported.
            q=(t-60)/16;x=35-12*q;feet=42-4*math.sin(math.pi*q);pose=3
        elif t<96:
            # Stay inside the opening until the entire body clears the shelf.
            q=(t-76)/19;x=23;feet=42+19*(.6*q+.4*q*q);pose=3
        elif t<108:
            x=23-13*(t-96)/11;feet=61;pose=1+(t//4)%2
        else:
            x=10;feet=61;pose=0
    # NES poses face left. Face right on the outward trip, left on return.
    returning=76<=k<124
    art=POPO[pose] if returning else ImageOps.mirror(POPO[pose])
    # Align the wider hammer pose about the same body anchor.
    paste(im,art,x-(2 if pose==3 else 0),feet)
    return im

def interceptor(k):
 ns,g=ff_setup();legacy=ns['oldfirst'].__globals__
 im=legacy['basefirst'](7,k)
 # A lower foreground ledge separates the charge from Shadow's upper patrol.
 d=ImageDraw.Draw(im);d.rectangle((0,62,63,63),fill='#4c424a');d.line((0,62,63,62),fill='#a78b74')
 t=k/25;a=t%6
 if a<.8 or a>=4:return im
 if a<2:x=68-67*(a-.8)/1.2;foot=62;right=False
 elif a<2.4:x=1+9*(a-2)/.4;foot=62-round(math.sin((a-2)/.4*math.pi)*4);right=True
 else:x=10+58*(a-2.4)/1.6;foot=62;right=True
 dog=legacy['dogs'][[0,1,2,1][int(t*10)%4]]
 dog=dog.resize((round(dog.width*17/dog.height),17),Image.Resampling.NEAREST)
 if right:dog=dog.transpose(Image.Transpose.FLIP_LEFT_RIGHT)
 im.paste(dog,(round(x),foot-dog.height),dog)
 if 1.96<=a<2.16:ImageDraw.Draw(im).line([(0,56),(4,54),(2,60)],fill='#ffe1bc')
 return im

def render_interceptor(k):
 ns,g=ff_setup();A=ns['M']['A']
 if not hasattr(render_interceptor,'original'):render_interceptor.original=A['render']
 A['render']=lambda n,k:interceptor(k) if n==7 else render_interceptor.original(n,k)
 item=next(dict(a) for a in ns['M']['items'] if a['n']==7);item['animated']='native'
 return g['raw_blend'](item,k)
