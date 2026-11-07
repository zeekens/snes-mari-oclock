"""Build a local, reviewable Bubble Bobble scene using original arcade sprites."""
from pathlib import Path
import base64, hashlib, importlib.util, json, math, re, shutil, struct, zipfile
from PIL import Image, ImageDraw, ImageOps

ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'docs/bubble-bobble-2026-11-06'
WORK=ROOT/'build/bubble-bobble-2026-11-06'
SRC=OUT/'sources'
ID='bubble-bobble-01'
for directory in (WORK,OUT/'packs',OUT/'previews',OUT/'inspection'):
    directory.mkdir(parents=True,exist_ok=True)

def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path)
    obj=importlib.util.module_from_spec(spec);spec.loader.exec_module(obj);return obj

base=module('nes_builder',ROOT/'tools/nes-scenes/build.py')
encoder=base.encoder
encoder.RAW=WORK

def sprite(name,x,y,w=16,h=16,flip=False):
    im=Image.open(SRC/name).convert('RGBA').crop((x,y,x+w,y+h))
    return ImageOps.mirror(im) if flip else im

BUB=[sprite('bub.png',x,0,flip=True) for x in (0,16,32,48)]
BLOW=[sprite('bub.png',x,16,flip=True) for x in (0,16,32,48)]
BOB=[sprite('bob.png',x,0) for x in (0,16)]
ENEMY=[sprite('zenchan.png',x,0) for x in (0,16,32,48)]
CAUGHT=[sprite('zenchan.png',x,48) for x in (0,16,32)]
BUBBLE=[sprite('bubbles.png',x,0) for x in (0,16,32,48,64,80)]
FRUIT=sprite('items.png',0,0).resize((8,8),Image.Resampling.NEAREST)
STYLE=[4,1,240,255,224,0,0,0]

def render(k,audit=None):
    im=Image.new('RGB',(64,64));d=ImageDraw.Draw(im)
    # Stage-one pink diagonals, fitted to the panel; clear sky for the live clock.
    solid=set()
    def platform(left,top,right,bottom):
        for y in range(top,bottom+1):
            for x in range(left,right+1):
                color=(255,96,184) if (x-y)%6<3 else (160,0,80)
                if y==top:color=(255,184,224)
                im.putpixel((x,y),color);solid.add((x,y))
    platform(0,22,2,63);platform(61,22,63,63)
    platform(3,61,60,63);platform(3,40,18,42);platform(44,40,60,42)
    # Six spaced background bubbles, painted before actors so gameplay stays clear.
    # Separate lanes keep the outlines from joining into an eight shape.
    phase=k if k<16 else (k-16)%16
    bubbles=((0,5,35,8,(80,192,224)),(8,13,35,8,(80,192,224)),
             (4,23,25,3,(184,136,240)),(11,35,24,3,(104,216,208)),
             (6,55,56,8,(104,184,248)),(12,24,53,6,(240,144,200)))
    for offset,lane,bottom,travel,color in bubbles:
        q=(phase+offset)%16;x=lane+round(math.sin(q*math.pi/8));y=bottom-q*travel//16
        d.ellipse((x,y,x+3,y+3),outline=color);d.point((x+1,y),fill=(224,255,255))
    actors=[]
    def put(art,x,feet,label,check=True):
        x,feet=round(x),round(feet);im.paste(art,(x,feet-art.height),art)
        if check:actors.append((label,art,x,feet-art.height))
    put(BOB[(k//8)%2],44,40,'Bob')
    x,feet,pose=5,61,BUB[0]
    t=k-16
    if t<0:
        put(ENEMY[(k//8)%2],36,61,'Zen-Chan')
    else:
        if t<16:
            x=5+7*t/15;pose=BUB[(t//4)%4]
        elif t<28:
            x=12;pose=BLOW[min(3,(t-16)//3)]
        elif t<52:
            x=12;pose=BUB[0]
        elif t<57:
            x=12+8*(t-52)/4;pose=BUB[(t//4)%4]
        elif t<81:
            q=(t-57)/23;x=20+9*q;feet=61-round(17*math.sin(math.pi*q));pose=BUB[3]
        elif t<90:
            x=29+5*(t-81)/8;pose=BUB[(t//4)%4]
        elif t<108:
            x=34-29*(t-90)/17;pose=ImageOps.mirror(BUB[(t//4)%4])
        else:
            x=5;pose=BUB[0]
        if t<28:
            put(ENEMY[(t//5)%2],36,61,'Zen-Chan')
        if 16<=t<28:
            q=(t-16)/11
            put(BUBBLE[min(5,(t-16)//2)],26+10*q,60,'Blown bubble',False)
        if 28<=t<59:
            # Drift clear of Bob's ledge before rising through the central shaft.
            bx=36-8*min(1,(t-28)/7)
            rise=max(0,min(1,(t-36)/15))
            put(CAUGHT[(t//4)%3],bx,61-17*rise,'Trapped Zen-Chan')
        if 59<=t<65:
            # First opaque-pixel contact with Bub occurs at action frame 59.
            r=2+(t-59)
            for dx,dy in ((-1,0),(1,0),(0,-1),(0,1)):
                cx,cy=35+dx*r,36+dy*r
                d.line((cx,cy,cx+dx,cy+dy),fill=(224,255,192))
        if 65<=t<88:
            q=min(1,(t-65)/18)
            put(FRUIT,35,43+18*q*q,'Fruit',False)
        if 88<=t<94:
            for dx,dy in ((-3,0),(3,0),(0,-3),(0,3)):
                d.point((40+dx,51+dy),fill=(255,255,128))
        if t>=100:
            q=min(1,(t-100)/9)
            put(ENEMY[(t//5)%2],45-9*q,61,'Zen-Chan')
    put(pose,x,feet,'Bub')
    if audit is not None:
        audit.update(actors=actors,solid=solid,bub_x=round(x),bub_feet=round(feet),action_t=t)
    return im

def main():
    frames=[render(k) for k in range(128)]
    (WORK/(ID+'.rgb')).write_bytes(b''.join(im.tobytes() for im in frames))
    (WORK/(ID+'.json')).write_text(json.dumps(dict(item=dict(title='Bubble Bobble - Bubble Trouble'),styles=[dict(y=4,style=1,ink=STYLE[2:5],outline=STYLE[5:8])]*128,font=base.FONT,approved_frames_matched=0)))
    v1,meta=min([encoder.encode(ID,t) for t in (4,8)],key=lambda x:len(x[0]))
    (WORK/(ID+'-v1.sntl')).write_bytes(v1)
    blob=bytearray(v1);blob[4]=2;struct.pack_into('<H',blob,12,80)
    blob+=struct.pack('<4s6H',b'TIME',1,0,16,16,112,160)
    struct.pack_into('<I',blob,36,len(blob));assert len(blob)<=163840
    (OUT/'packs'/(ID+'.sntl')).write_bytes(blob)
    clocked=[base.preview.clock(im,STYLE,base.DIGITS) for im in frames]
    colors=sorted({c for im in clocked for c in im.getdata()});assert len(colors)<=256
    palette=[v for c in colors for v in c]+[0]*(768-3*len(colors));lookup={c:i for i,c in enumerate(colors)}
    indexed=[]
    for frame in clocked:
        im=Image.new('P',(64,64));im.putpalette(palette);im.putdata([lookup[c] for c in frame.getdata()]);indexed.append(im)
    for scale in (1,8):
        seq=[im.resize((64*scale,64*scale),Image.Resampling.NEAREST) for im in indexed]
        seq[0].save(OUT/'previews'/f'{ID}-{64*scale}.gif',save_all=True,append_images=seq[1:],duration=80,loop=0,optimize=False,disposal=1)
    clocked[0].resize((512,512),Image.Resampling.NEAREST).save(OUT/'previews'/(ID+'.png'))
    atlas=Image.new('RGB',(640,64*13))
    for k,im in enumerate(frames):atlas.paste(im,(k%10*64,k//10*64))
    atlas.save(OUT/'previews'/(ID+'-atlas.png'))
    strip=Image.new('RGB',(4*256,2*288),(16,24,32));d=ImageDraw.Draw(strip)
    for j,k in enumerate([0,32,43,60,76,82,99,127]):
        x,y=j%4*256,j//4*288
        strip.paste(clocked[k].resize((256,256),Image.Resampling.NEAREST),(x,y+24))
        d.text((x+8,y+5),f'Frame {k}',fill='white')
    strip.save(OUT/'inspection'/(ID+'-motion.png'))
    item=dict(id=ID,name='Bubble Bobble - Bubble Trouble',library='review',frames=128,mode='minute',idle=[0,16],action=[16,112],frame_ms=80,idle_ms=160,
              description='Bub blows a bubble, traps Zen-Chan, jumps to pop it, and collects the fruit. Bob waits on the opposite ledge. One action at each minute change.',
              bytes=len(blob),sha256=hashlib.sha256(blob).hexdigest(),styles=[STYLE]*128,font=base.DIGITS,tile_size=meta['tile_size'],palette_colors=meta['palette_colors'],path=f'packs/{ID}.sntl',preview=f'previews/{ID}-512.gif')
    (OUT/'scenes.json').write_text(json.dumps([item],separators=(',',':')))
    (OUT/'manifest.json').write_text(json.dumps(dict(version='0.1.0',format='SNTL',format_version=2,minimum_firmware='sntl-2.2.0',review_status='pending',installed=False,items=[item]),indent=2)+'\n')
    deliver(item)
    print(ID,len(blob),'bytes;',len(colors),'lossless preview colors')

def deliver(item):
    item=dict(item,imageURL='data:image/png;base64,'+base64.b64encode((OUT/'previews'/(ID+'-atlas.png')).read_bytes()).decode())
    template=(ROOT/'build/github-pack-publication/device-firmware/scripts/native-packs/preview.html').read_text()
    template=template.replace('SNES clock — Pack quality review','Bubble Bobble · Scene review').replace('SNES clock · All three packs','Bubble Bobble · Bubble Trouble')
    template=template.replace('Native 64 × 64 pixels. Centered clock. Slower travel and ambience; selected actions play once at the minute change.',item['description']+' Native 64 × 64 pixels with original arcade sprites. Local draft for review.')
    template=re.sub(r'<select id="library">.*?</select>','<select id="library"><option value="all">Bubble Bobble · draft</option></select>',template)
    template=template.replace("fetch('scenes.json').then(r=>r.json())",'Promise.resolve('+json.dumps([item],separators=(',',':'))+')')
    template=template.replace("s.image.src=s.id+'.png';","s.image.src=s.imageURL;")
    template=template.replace("s.library+' · '+","'Review draft · '+")
    template=template.replace('canvas{width:256px;height:256px;','canvas{max-width:100%;width:384px;aspect-ratio:1;height:auto;')
    template=template.replace('<main></main>','<main></main><p style="padding:0 4vw"><a href="previews/'+ID+'-512.gif">Animated action demo</a> · <a href="bubble-bobble-review-0.1.0.zip">Download review bundle</a> · <a href="CREDITS.md">Artwork credits</a> · <a href="validation.json">Validation</a></p>')
    template=template.replace('body{','a{color:#6cddb6}body{',1)
    template=template.replace('let scenes=[],paused=false,','let scenes=[],paused=matchMedia("(prefers-reduced-motion: reduce)").matches,')
    template=template.replace('requestAnimationFrame(tick);});',"document.querySelector('#pause').textContent=paused?'Play':'Pause';requestAnimationFrame(tick);});")
    (OUT/'index.html').write_text(template)
    # Append only the new draft. Keep all existing scene objects and art unchanged.
    gallery=ROOT/'docs/gallery.html';backup=ROOT/'backups/bubble-bobble-2026-11-06/gallery.html'
    backup.parent.mkdir(parents=True,exist_ok=True)
    if not backup.exists():shutil.copy2(gallery,backup)
    html=gallery.read_text();pattern=r'(<script id="gallery-data" type="application/json">)(.*?)(</script>)'
    match=re.search(pattern,html,re.S);data=json.loads(match[2]);prior=[s for s in data['scenes'] if s['id']!=ID]
    data['scenes']=prior+[item]
    html=html[:match.start(2)]+json.dumps(data,separators=(',',':'))+html[match.end(2):]
    html=html.replace('All scenes · 52','All scenes · 53')
    html=re.sub(r'<option value="review">.*?</option>','',html)
    html=re.sub(r'(<option value="all">.*?</option>)',r'\1<option value="review">New scene · Bubble Bobble</option>',html,count=1)
    html=html.replace("s.library==='nes-new'?'New NES · not installed':", "s.library==='review'?'Review draft':s.library==='nes-new'?'New NES · not installed':")
    html=html.replace('8 Classic originals + 44 scenes across 3 packs`','8 Classic originals + 44 pack scenes + 1 review draft`')
    if 'bubble-bobble-2026-11-06/index.html' not in html:html=html.replace('<a href="#artwork">','<a href="bubble-bobble-2026-11-06/index.html">Review Bubble Bobble</a> · <a href="#artwork">',1)
    original=json.loads(re.search(pattern,backup.read_text(),re.S)[2])
    assert all(s in data['scenes'] for s in original['scenes']) and data['artwork']==original['artwork']
    gallery.write_text(html)

def package():
    with zipfile.ZipFile(OUT/'bubble-bobble-review-0.1.0.zip','w',zipfile.ZIP_DEFLATED) as archive:
        for name in ('index.html','manifest.json','scenes.json','validation.json','CREDITS.md','README.md'):
            archive.write(OUT/name,name)
        for folder in ('packs','previews','inspection','sources'):
            for p in sorted((OUT/folder).glob('*')):archive.write(p,p.relative_to(OUT))

if __name__=='__main__':main()
