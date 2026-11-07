from pathlib import Path
import json,subprocess,hashlib
R=Path(__file__).resolve().parents[2];P=R/'build/github-pack-publication';O=R/'build/scene-audit-2026-11-07';F=P/'device-firmware';O.mkdir(exist_ok=True);test=O/'native-quality-test'
subprocess.run(['clang++','-std=c++17','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-O1','-fsanitize=address,undefined',str(F/'tests/native_quality_test.cpp'),'-o',str(test)],check=True)
cat=json.loads((P/'libraries/sntl-v1/catalog.json').read_text())['scenes'];old={s['id']:P/s['path'] for s in json.loads((F/'scene-packs/source-catalog.json').read_text())['scenes']}
report=[]
for s in cat+json.loads((R/'docs/bubble-bobble-2026-11-06/scenes.json').read_text()):
 id=s['id'];new=(P/s['path']) if id!='bubble-bobble-01' else R/'docs/bubble-bobble-2026-11-06'/s['path']
 original=old.get(id,R/('build/bubble-bobble-2026-11-06' if id=='bubble-bobble-01' else 'build/nes-scenes-2026-10-29')/(id+'-v1.sntl'))
 blob=new.read_bytes();assert len(blob)==s['bytes'] and hashlib.sha256(blob).hexdigest()==s['sha256'],id
 p=subprocess.run([str(test),str(original),str(new),str(O/(id+'.rgb')),str(O/(id+'-clock.rgb'))],capture_output=True,text=True)
 report.append(dict(id=id,path=str(new),passed=p.returncode==0,output=p.stdout+p.stderr))
 print(id,'PASS' if p.returncode==0 else 'FAIL',flush=True)
(O/'native-validation.json').write_text(json.dumps(report,indent=2));assert all(s['passed'] for s in report)
