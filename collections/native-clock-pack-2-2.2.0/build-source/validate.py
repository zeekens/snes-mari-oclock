"""Use the production C++ decoder, sanitizer suite, and independent pixel oracle."""
from pathlib import Path
import importlib.util, json, subprocess, hashlib
from PIL import Image, ImageChops

HERE=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('build_nes',HERE/'build.py')
B=importlib.util.module_from_spec(spec);spec.loader.exec_module(B)
test=B.WORK/'native-quality-test'
subprocess.run(['clang++','-std=c++17','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-O1','-fsanitize=address,undefined',str(B.ROOT/'build/github-pack-publication/device-firmware/tests/native_quality_test.cpp'),'-o',str(test)],check=True)
report=[]
for item,scene in zip(json.loads((B.OUT/'scenes.json').read_text()),B.SCENES):
    id=item['id'];raw=B.WORK/(id+'-decoded.rgb');clocked=B.WORK/(id+'-clock.rgb')
    result=subprocess.run([str(test),str(B.WORK/(id+'-v1.sntl')),str(B.OUT/item['path']),str(raw),str(clocked)],check=True,capture_output=True,text=True)
    assert raw.read_bytes()==(B.WORK/(id+'.rgb')).read_bytes(),id
    expected=[]
    for k in range(item['frames']):
        frame=scene['render'](k)
        # The riverbanks intentionally continue and scroll behind Cobra's clock.
        if id!='cobra-triangle-01':
            assert frame.crop((0,0,64,21)).tobytes()==scene['render'](0).crop((0,0,64,21)).tobytes()
        if id=='donkey-kong-01':
            audit={};B.donkey(k,audit)
            def occupied(actor):
                art,x,feet=actor
                return {(x+xx,feet-art.height+yy) for yy in range(art.height) for xx in range(art.width) if art.getpixel((xx,yy))[3]}
            mario=occupied(audit['mario'])
            for x,y in mario:
                assert y>35+x//16+2 and y<61-x//16,(k,'Mario intersects a girder',x,y)
            if 'barrel' in audit:
                assert not mario.intersection(occupied(audit['barrel'])),(k,'Mario intersects barrel')
        expected.append(B.preview.clock(frame,item['styles'][k],item['font']))
    assert clocked.read_bytes()==b''.join(im.tobytes() for im in expected),id
    for scale in (1,8):
        gif=Image.open(B.OUT/'previews'/f'{id}-{64*scale}.gif');assert gif.info['loop']==0
        k=0
        for f in range(gif.n_frames):
            gif.seek(f);duration=gif.info['duration'];assert duration>0 and duration%80==0
            actual=gif.convert('RGB');assert actual.size==(64*scale,64*scale)
            for _ in range(duration//80):
                reference=expected[k].resize(actual.size,Image.Resampling.NEAREST)
                assert ImageChops.difference(actual,reference).getbbox() is None,(id,scale,k)
                k+=1
        assert k==item['frames']
    blob=(B.OUT/item['path']).read_bytes();assert hashlib.sha256(blob).hexdigest()==item['sha256']
    # Continuous seam must be no worse than a regular frame step.
    frames=[scene['render'](k) for k in range(item['frames'])]
    def changed(a,b):return sum(x!=y for x,y in zip(a.getdata(),b.getdata()))
    seam=changed(frames[-1],frames[0]);largest=max(changed(a,b) for a,b in zip(frames,frames[1:]))
    assert seam<=largest,(id,seam,largest)
    report.append(dict(id=id,passed=True,decoded_frames=item['frames'],clock_oracle_frames=item['frames'],gif_frames_per_size=item['frames'],incremental_seek_clock_checks=2000,scenery_behind_clock=id=='cobra-triangle-01',mario_pixel_collision_checks=item['frames'] if id=='donkey-kong-01' else 0,loop_seam_changed_pixels=seam,largest_step_changed_pixels=largest,sanitizers='ASan + UBSan',timing_and_malformed_input_tests=True,bytes=len(blob),sha256=item['sha256']))
    print(result.stdout.strip())
(B.OUT/'validation.json').write_text(json.dumps(dict(passed=True,physical_panel_tested=False,scenes=report),indent=2)+'\n')
print('All three scenes passed pixel, GIF, timing, bounds and sanitizer checks.')
