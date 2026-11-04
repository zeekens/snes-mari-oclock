"""Build three NES clock scenes from retained source pixels; no hardware writes."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageOps
import importlib.util, json, math, struct, hashlib, shutil, subprocess, zipfile

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'docs/nes-scenes-2026-10-29'
WORK = ROOT / 'build/nes-scenes-2026-10-29'
SRC = OUT / 'sources'
for p in (WORK, OUT/'packs', OUT/'previews', OUT/'inspection'):
    p.mkdir(parents=True, exist_ok=True)

def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    obj = importlib.util.module_from_spec(spec); spec.loader.exec_module(obj)
    return obj

encoder = module('encoder', ROOT/'tools/native-scenes/encode.py')
encoder.RAW = WORK
preview = module('clock_oracle', ROOT/'build/github-pack-publication/device-firmware/scripts/native-packs/preview.py')
FONT = {'0':[14,17,19,21,25,17,14], '1':[4,12,4,4,4,4,14],
        '2':[14,17,1,2,4,8,31], '3':[31,2,4,2,1,17,14],
        '4':[2,6,10,18,31,2,2], '5':[31,16,30,1,1,17,14],
        '6':[6,8,16,30,17,17,14], '7':[31,1,2,4,8,8,8],
        '8':[14,17,17,14,17,17,14], '9':[14,17,17,15,1,2,12]}
DIGITS = [b for n in '0123456789' for b in FONT[n]]

def sprite(file, box, keys, size=None):
    im = Image.open(SRC/file).convert('RGBA').crop(box)
    im.putdata([(*p[:3], 0 if p[:3] in keys else 255) for p in im.getdata()])
    if size: im = im.resize(size, Image.Resampling.NEAREST)
    return im

BLACK = {(0,0,0)}
DKKEY = {(0,128,0),(128,0,128),(0,0,0)}
POPO = [sprite('ice-climber.png',box,BLACK) for box in
        [(2,20,17,40),(18,20,33,40),(35,20,50,40),(87,12,110,40)]]
POPO = [im.resize((11,15) if n<3 else (15,18),Image.Resampling.NEAREST) for n,im in enumerate(POPO)]
NANA = sprite('ice-climber.png',(304,20,321,40),BLACK,(13,16))
KONG = [sprite('donkey-kong.png',box,DKKEY,(24,14)) for box in
        [(1,137,49,170),(53,137,101,170),(103,137,151,170),(155,137,203,170)]]
MARIO = [sprite('donkey-kong.png',box,DKKEY,(11,11)) for box in
         [(1,21,17,37),(18,21,34,37),(35,21,51,37),(62,21,78,37)]]
BARREL = [sprite('donkey-kong.png',(345+i*17,134,361+i*17,150),DKKEY,(7,7)) for i in range(4)]
PAULINE = sprite('donkey-kong.png',(255,21,271,45),DKKEY,(7,11))
BOAT = ImageOps.mirror(sprite('cobra-gameplay.png',(112,108,143,132),{(36,24,140)}))
BOAT = BOAT.crop(BOAT.getbbox())
TREE = sprite('cobra-gameplay2.png',(213,98,223,145),{(0,168,0)},(5,18))

def paste(im, art, x, feet):
    im.paste(art,(round(x),round(feet)-art.height),art)

def ice(k):
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
            x=35-(t-60)*.8;feet=42;pose=1+(t//4)%2
        elif t<96:
            q=(t-76)/19;x=23-13*q;feet=42+19*q*q;pose=3
        else:
            x=10;feet=61;pose=0
    # NES poses face left. Face right on the outward trip, left on return.
    returning=76<=k<112
    art=POPO[pose] if returning else ImageOps.mirror(POPO[pose])
    # Align the wider hammer pose about the same body anchor.
    paste(im,art,x-(2 if pose==3 else 0),feet)
    return im

def donkey(k,audit=None):
    im=Image.new('RGB',(64,64));d=ImageDraw.Draw(im)
    def girder(y,sign):
        for x in range(64):
            yy=y+sign*(x//16)
            d.point((x,yy),fill=(248,68,120));d.point((x,yy+2),fill=(248,68,120))
            if x%2==0:d.point((x,yy+1),fill=(160,24,68))
    girder(35,1);girder(61,-1)
    for x,top,bottom in [(48,38,58),(7,40,60)]:
        d.line((x,top,x,bottom),fill=(92,204,232));d.line((x+4,top,x+4,bottom),fill=(92,204,232))
        for y in range(top+2,bottom,4):d.line((x,y,x+4,y),fill=(92,204,232))
    t=k-16
    pose=(k//8)%2 if t<0 else (3 if t<15 else 0)
    paste(im,KONG[pose],2,35);paste(im,PAULINE,43,33)
    d.line((39,33,57,33),fill=(248,68,120))
    feet=60;mp=0
    if t>=0:
        # One barrel travels along the upper girder, drops, then rolls left.
        if t<42:bx=26+31*t/41;by=35+round(bx)//16
        elif t<54:bx=57;by=38+(t-42)*20/11
        else:bx=57-(t-54)*1.15;by=61-(round(bx)+6)//16
        if bx>=-7:paste(im,BARREL[(t//3)%4],bx,by)
        if 67<=t<=103:
            feet-=round(10*math.sin(math.pi*(t-67)/36));mp=3
        if audit is not None:audit['barrel']=(BARREL[(t//3)%4],round(bx),round(by))
    paste(im,MARIO[mp],20,feet)
    if audit is not None:audit['mario']=(MARIO[mp],20,round(feet))
    # Oil drum, with a two-pose small flame.
    d.rectangle((0,53,5,59),fill=(44,106,215));d.line((0,54,5,54),fill=(170,224,255))
    d.line((1,51,2,48+(k//4)%2,4,52),fill=(248,126,32),width=1)
    return im

def cobra(k):
    k=(k+2)%128
    water=(36,24,140);im=Image.new('RGB',(64,64),water);d=ImageDraw.Draw(im)
    # Pixel-stepped isometric banks, derived from the stage-one palette and shape.
    # Translation (-2,+1) along the river; 32-step periodic texture.
    scroll=k//4
    for y in range(64):
        for x in range(64):
            cross=x+2*y
            if cross<71 or cross>147:
                cliff=(65<=cross<71) or (147<cross<=153)
                sx=(x+2*scroll)%32;sy=(y-scroll)%16
                color=(0,168,0)
                if cliff:color=(200,76,12) if (sx+sy*3)%7<4 else (64,32,0)
                elif (sx*7+sy*11)%61<3:color=(0,100,0)
                im.putpixel((x,y),color)
    for offset in (0,16):
        phase=(scroll+offset)%32
        paste(im,TREE,84-2*phase,40+phase)
    # Sparse moving water glints never pass over the clock.
    for n in range(9):
        x=(n*19-2*scroll)%64;y=24+(n*13+scroll)%32
        if 77<x+2*y<141:d.line((x,y,x+2,y-1),fill=(80,100,208))
    bob=round(math.sin(k*2*math.pi/128))
    # The mirrored original boat points up/right; its wake extends down/left.
    x,feet=23,51+bob
    for n in range(3):
        q=(k//4+n*3)%8
        d.line((x+2-q,feet+q//2,x+5-q,feet+q//2),fill=(170,192,240) if n==0 else (80,100,208))
    paste(im,BOAT,x,feet)
    return im

SCENES=[
 dict(id='ice-climber-01',name='Ice Climber - Summit Partners',render=ice,frames=128,mode='minute',idle=[0,16],action=[16,112],ink=[224,240,255],outline=[0,0,0],description='Popo climbs through the ice shelf, pauses beside Nana, then returns. One climb on each minute change.'),
 dict(id='donkey-kong-01',name='Donkey Kong - Barrel Girders',render=donkey,frames=128,mode='minute',idle=[0,16],action=[16,112],ink=[255,238,220],outline=[0,0,0],description='Kong releases a barrel across the girders. Mario jumps as it rolls past; the scene settles back to idle.'),
 dict(id='cobra-triangle-01',name='Cobra Triangle - River Run',render=cobra,frames=128,mode='continuous',idle=[0,1],action=[0,128],ink=[228,255,255],outline=[16,16,64],description='The Cobra boat runs upriver through diagonal banks, with a moving wake and water glints.')]

def main():
    scenes=[]
    for s in SCENES:
        id=s['id'];frames=[s['render'](k) for k in range(s['frames'])]
        styles=[dict(y=4,style=1,ink=s['ink'],outline=s['outline']) for _ in frames]
        style=[4,1,*s['ink'],*s['outline']]
        (WORK/(id+'.rgb')).write_bytes(b''.join(im.tobytes() for im in frames))
        (WORK/(id+'.json')).write_text(json.dumps(dict(item=dict(title=s['name']),styles=styles,font=FONT,approved_frames_matched=0)))
        choices=[encoder.encode(id,t) for t in (4,8)]
        v1,meta=min(choices,key=lambda c:len(c[0]));(WORK/(id+'-v1.sntl')).write_bytes(v1)
        blob=bytearray(v1);blob[4]=2;struct.pack_into('<H',blob,12,80)
        blob+=struct.pack('<4s6H',b'TIME',int(s['mode']=='minute'),*s['idle'],*s['action'],160)
        struct.pack_into('<I',blob,36,len(blob));assert len(blob)<=163840
        (OUT/'packs'/(id+'.sntl')).write_bytes(blob)
        clocked=[preview.clock(im,style,DIGITS) for im in frames]
        # Fixed palette, lossless GIF exports. No quantization or dither.
        colors=sorted({rgb for im in clocked for rgb in im.getdata()});assert len(colors)<=256
        cmap={c:i for i,c in enumerate(colors)};pal=[v for c in colors for v in c]+[0]*(768-3*len(colors))
        indexed=[]
        # A minute-scene GIF is explicitly an action demo, with a short idle lead-in.
        sequence=list(range(16))+list(range(16,len(frames))) if s['mode']=='minute' else list(range(len(frames)))
        for k in sequence:
            im=Image.new('P',(64,64));im.putpalette(pal);im.putdata([cmap[c] for c in clocked[k].getdata()]);indexed.append(im)
        for scale in (1,8):
            seq=[im.resize((64*scale,64*scale),Image.Resampling.NEAREST) for im in indexed]
            seq[0].save(OUT/'previews'/f'{id}-{64*scale}.gif',save_all=True,append_images=seq[1:],duration=80,loop=0,optimize=False,disposal=1)
        clocked[0].resize((512,512),Image.Resampling.NEAREST).save(OUT/'previews'/(id+'.png'))
        atlas=Image.new('RGB',(640,64*math.ceil(len(frames)/10)))
        for k,im in enumerate(frames):atlas.paste(im,((k%10)*64,(k//10)*64))
        atlas.save(OUT/'previews'/(id+'-atlas.png'))
        strip=Image.new('RGB',(8*192,224),(16,24,32));dd=ImageDraw.Draw(strip)
        for j,k in enumerate([0,16,32,48,64,80,96,120]):
            strip.paste(clocked[k].resize((192,192),Image.Resampling.NEAREST),(j*192,25));dd.text((j*192+5,5),f'frame {k}',fill='white')
        strip.save(OUT/'inspection'/(id+'-motion.png'))
        item={k:v for k,v in s.items() if k not in ('render','ink','outline')}
        item.update(library='pack-2',frame_ms=80,idle_ms=160,bytes=len(blob),sha256=hashlib.sha256(blob).hexdigest(),styles=[style]*len(frames),font=DIGITS,tile_size=meta['tile_size'],palette_colors=meta['palette_colors'],path=f'packs/{id}.sntl',preview=f'previews/{id}-512.gif')
        scenes.append(item);print(id,len(blob),'bytes',flush=True)
    (OUT/'scenes.json').write_text(json.dumps(scenes,separators=(',',':')))
    (OUT/'manifest.json').write_text(json.dumps(dict(version='1.0.0',format='SNTL',format_version=2,minimum_firmware='sntl-2.2.0',review_status='accepted-for-pack-2',installed=False,items=scenes),indent=2)+'\n')
    overview=Image.new('RGB',(3*320,362),(16,24,32));d=ImageDraw.Draw(overview)
    for j,s in enumerate(scenes):
        d.text((j*320+16,12),s['name'],fill='white')
        overview.paste(Image.open(OUT/'previews'/(s['id']+'.png')).resize((288,288),Image.Resampling.NEAREST),(j*320+16,40))
        d.text((j*320+16,340),s['mode']+' / '+str(s['bytes'])+' bytes',fill=(140,216,192))
    overview.save(OUT/'overview.png')

if __name__=='__main__':main()
