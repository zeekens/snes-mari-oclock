"""Validate native pixels, actor geometry, minute policy, and lossless previews."""
import importlib.util,json,subprocess,hashlib
from PIL import Image,ImageChops
from pathlib import Path
spec=importlib.util.spec_from_file_location('bubble_builder',Path(__file__).with_name('build.py'))
B=importlib.util.module_from_spec(spec);spec.loader.exec_module(B)
test=B.WORK/'native-quality-test'
subprocess.run(['clang++','-std=c++17','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-O1','-fsanitize=address,undefined',str(B.ROOT/'build/github-pack-publication/device-firmware/tests/native_quality_test.cpp'),'-o',str(test)],check=True)
raw=B.WORK/'decoded.rgb';clocked=B.WORK/'clock.rgb'
subprocess.run([str(test),str(B.WORK/(B.ID+'-v1.sntl')),str(B.OUT/'packs'/(B.ID+'.sntl')),str(raw),str(clocked)],check=True)
assert raw.read_bytes()==(B.WORK/(B.ID+'.rgb')).read_bytes()
expected=[]
def occupied(art,x,y):
    return {(x+xx,y+yy) for yy in range(art.height) for xx in range(art.width) if art.getpixel((xx,yy))[3]}
for k in range(128):
    audit={};frame=B.render(k,audit)
    expected.append(B.base.preview.clock(frame,B.STYLE,B.base.DIGITS))
    assert frame.crop((0,0,64,21)).getbbox() is None,'Motion intrudes into the clock'
    actors={}
    for name,art,x,y in audit['actors']:
        pixels=occupied(art,x,y);actors[name]=pixels
        assert not pixels&audit['solid'],(k,name,'platform collision')
        assert all(0<=x<64 and 21<=y<64 for x,y in pixels),(k,name,'clipping')
    for name,pixels in actors.items():
        for other,opixels in actors.items():
            if name!=other:assert not pixels&opixels,(k,name,other,'premature collision')
    # Movement and facing match. Native sprites face left, mirrored sprites right.
    if 16<=k<32 or 68<=k<106:
        assert next(a[1] for a in audit['actors'] if a[0]=='Bub').tobytes() in [a.tobytes() for a in B.BUB]
    if 106<=k<124:
        assert next(a[1] for a in audit['actors'] if a[0]=='Bub').tobytes() in [B.ImageOps.mirror(a).tobytes() for a in B.BUB]
assert clocked.read_bytes()==b''.join(im.tobytes() for im in expected)
# The pop must coincide with first sprite contact, rather than a floating effect.
touch=[]
for t in range(57,81):
    q=(t-57)/23;x=round(20+9*q);feet=61-round(17*B.math.sin(B.math.pi*q))
    if occupied(B.BUB[3],x,feet-16)&occupied(B.CAUGHT[(t//4)%3],28,28):touch.append(t)
assert touch[0]==59
for scale in (1,8):
    gif=Image.open(B.OUT/'previews'/f'{B.ID}-{64*scale}.gif');assert gif.info['loop']==0
    k=0
    for f in range(gif.n_frames):
        gif.seek(f);duration=gif.info['duration'];assert duration>0 and duration%80==0
        actual=gif.convert('RGB')
        for _ in range(duration//80):
            reference=expected[k].resize(actual.size,Image.Resampling.NEAREST)
            assert ImageChops.difference(actual,reference).getbbox() is None,(scale,k)
            k+=1
    assert k==128
item=json.loads((B.OUT/'scenes.json').read_text())[0]
blob=(B.OUT/item['path']).read_bytes();assert hashlib.sha256(blob).hexdigest()==item['sha256']
report=dict(passed=True,id=B.ID,bytes=len(blob),sha256=item['sha256'],decoded_frames=128,independent_clock_frames=128,gif_frames_per_size=128,
            incremental_redraw_checks=2000,sanitizers='ASan + UBSan',minute_and_midnight_policy=True,malformed_input_tests=True,
            actor_collision_and_bounds_frames=128,pop_at_first_pixel_contact=True,physical_panel_tested=False,review_status='pending')
(B.OUT/'validation.json').write_text(json.dumps(report,indent=2)+'\n')
B.package()
print('All 128 frames passed. Native file, timing, geometry, clock, and both lossless GIFs verified.')
