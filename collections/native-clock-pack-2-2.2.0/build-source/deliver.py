"""Create a standalone review gallery and append the new faces to the local library."""
from pathlib import Path
import json, base64, re, shutil, zipfile, hashlib
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'docs/nes-scenes-2026-10-29'
items=json.loads((OUT/'scenes.json').read_text())
for s in items:
    s['imageURL']='data:image/png;base64,'+base64.b64encode((OUT/'previews'/(s['id']+'-atlas.png')).read_bytes()).decode()
template=(ROOT/'build/github-pack-publication/device-firmware/scripts/native-packs/preview.html').read_text()
template=template.replace('SNES clock — Pack quality review','NES clock scenes · Review')
template=template.replace('SNES clock · All three packs','NES clock scenes')
template=template.replace('Native 64 × 64 pixels. Centered clock. Slower travel and ambience; selected actions play once at the minute change.','Ice Climber · Donkey Kong · Cobra Triangle. Native 64 × 64 pixels, live clock, original game sprites. Assigned to Pack 2. Physical-panel playback remains untested.')
template=re.sub(r'<select id="library">.*?</select>','<select id="library"><option value="all">All three new scenes</option></select>',template)
template=template.replace('<span id="time"></span>','<span id="time"></span><p><a href="nes-scenes-1.0.0.zip">Download all three scenes + previews</a> · <a href="CREDITS.md">Artwork credits</a> · <a href="validation.json">Validation</a> · <a href="../gallery.html">Complete library</a></p><button id="export">Export review notes</button>')
template=template.replace('canvas{width:256px;height:256px;','a{color:#6cddb6}textarea{width:100%;box-sizing:border-box;min-height:66px;background:#101820;color:#e5edf4;border:1px solid #647d8e;border-radius:6px;padding:8px}canvas{max-width:100%;width:256px;aspect-ratio:1;height:auto;')
template=template.replace('let scenes=[],paused=false,','let scenes=[],paused=matchMedia("(prefers-reduced-motion: reduce)").matches,')
template=template.replace("fetch('scenes.json').then(r=>r.json()).then(async items=>{",'Promise.resolve('+json.dumps(items,separators=(',',':'))+').then(async items=>{')
template=template.replace("s.image.src=s.id+'.png';","s.image.src=s.imageURL;")
template=template.replace("tag.textContent=s.library+' · '+", "tag.textContent='Pack 2 · '+")
template=template.replace("main.append(a);s.image=new Image;",'''let description=document.createElement('p');description.textContent=s.description;a.append(description);
let links=document.createElement('p');for(let [label,url] of [['Native scene',s.path],['Animated GIF',s.preview]]){let l=document.createElement('a');l.textContent=label;l.href=url;l.style.margin='8px';links.append(l);}a.append(links);
let review=document.createElement('select');review.setAttribute('aria-label',s.name+' review');for(let value of ['Unreviewed','Keep','Rework','Discard']){let o=new Option(value,value);review.add(o);}
let note=document.createElement('textarea');note.placeholder='Review notes';note.setAttribute('aria-label',s.name+' notes');let saved=readReviews()[s.id]||{};review.value=saved.decision||'Unreviewed';note.value=saved.notes||'';
function save(){let all=readReviews();all[s.id]={decision:review.value,notes:note.value};try{localStorage.setItem(reviewKey,JSON.stringify(all));}catch{note.title='Browser storage unavailable; export your review notes.';}}
review.onchange=save;note.oninput=save;a.append(review,note);main.append(a);s.image=new Image;''')
template=template.replace('let scenes=[]', '''const reviewKey='snes-clock:nes-scenes:reviews:v1';function readReviews(){try{return JSON.parse(localStorage.getItem(reviewKey)||'{}');}catch{return {};}}
document.querySelector('#export').onclick=()=>{let blob=new Blob([JSON.stringify(readReviews(),null,2)],{type:'application/json'}),url=URL.createObjectURL(blob),a=document.createElement('a');a.href=url;a.download='nes-scene-reviews.json';a.click();setTimeout(()=>URL.revokeObjectURL(url),1000);};
let scenes=[]''')
template=template.replace("requestAnimationFrame(tick);});", "document.querySelector('#pause').textContent=paused?'Play':'Pause';requestAnimationFrame(tick);});")
template=template.replace('These previews use the exact decoded SNTL scene pixels and the firmware clock geometry. Minute actions do not run on startup. Artwork is retained without GIF conversion, palette reduction or smoothing. The physical panel still needs visual confirmation.','All 384 stored frames match the production SNTL decoder and independent clock oracle. Ice Climber and Donkey Kong wait for a minute change; use the button to preview their actions. GIF downloads demonstrate the actions in a repeating loop. Cobra Triangle runs continuously. Artwork uses nearest-neighbour fitting and lossless palettes. Physical-panel playback has not been tested.')
(OUT/'index.html').write_text(template)

# Preserve the existing embedded scene objects and review storage exactly.
gallery=ROOT/'docs/gallery.html';backup=ROOT/'backups/nes-scenes-2026-10-29/gallery.html'
backup.parent.mkdir(parents=True,exist_ok=True)
if not backup.exists():shutil.copy2(gallery,backup)
html=gallery.read_text();pattern=r'(<script id="gallery-data" type="application/json">)(.*?)(</script>)'
match=re.search(pattern,html,re.S);data=json.loads(match[2]);old_ids={s['id'] for s in items}
retained=[s for s in data['scenes'] if s['id'] not in old_ids]
data['scenes']=retained+items
html=html[:match.start(2)]+json.dumps(data,separators=(',',':'))+html[match.end(2):]
html=html.replace('All scenes · 49','All scenes · 52')
html=re.sub(r'<option value="nes-new">.*?</option>','',html)
html=html.replace('Pack 2 · 11 scenes','Pack 2 · 14 scenes')
html=html.replace('8 Classic originals + 41 scenes across 3 packs','8 Classic originals + 44 scenes across 3 packs').replace('8 Classic originals + 41 pack scenes + 3 new NES scenes','8 Classic originals + 44 scenes across 3 packs')
html=html.replace("s.library.replace('pack-','Pack ')+' · '+", "(s.library==='nes-new'?'New NES · not installed':s.library.replace('pack-','Pack '))+' · '+")
if 'nes-scenes-2026-10-29/index.html' not in html:html=html.replace('<a href="#artwork">','<a href="nes-scenes-2026-10-29/index.html">Review the 3 new NES scenes</a> · <a href="#artwork">')
gallery.write_text(html)
original=json.loads(re.search(pattern,backup.read_text(),re.S)[2])
assert all(s in data['scenes'] for s in original['scenes'])
assert data['artwork']==original['artwork']
assert len({s['id'] for s in data['scenes']})==len(data['scenes'])

with zipfile.ZipFile(OUT/'nes-scenes-1.0.0.zip','w',zipfile.ZIP_DEFLATED) as z:
    for name in ['index.html','manifest.json','scenes.json','validation.json','CREDITS.md','README.md','overview.png']:
        z.write(OUT/name,name)
    for folder in ('packs','previews','inspection','sources'):
        for p in sorted((OUT/folder).glob('*')):z.write(p,p.relative_to(OUT))
print('Review gallery, 52-face complete gallery, and downloadable ZIP created. All 49 prior faces preserved.')
