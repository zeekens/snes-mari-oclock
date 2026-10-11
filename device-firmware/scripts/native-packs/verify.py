"""Validate every scene with ASan/UBSan, independent clock oracle, and gallery output.
Usage: python scripts/native-packs/verify.py PUBLIC_CHECKOUT OUTPUT_DIRECTORY
Run build.py first. Requires clang++ and Pillow.
"""
from pathlib import Path
import json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
def verify(public,out):
 out=out.resolve();(out/'frames').mkdir(exist_ok=True)
 test=out/'native-quality-test'
 subprocess.run(['clang++','-std=c++17','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-O1','-fsanitize=address,undefined',str(ROOT/'tests/native_quality_test.cpp'),'-o',str(test)],check=True)
 source=json.loads((ROOT/'scene-packs/source-catalog.json').read_text());new=json.loads((out/'libraries/sntl-v1/catalog.json').read_text())
 with (out/'all-scenes-tests.log').open('w') as log:
  for s,n in zip(source['scenes'],new['scenes']):
   assert s['id']==n['id']
   subprocess.run([str(test),str(public/s['path']),str(out/n['path']),str(out/'frames'/(s['id']+'.rgb')),str(out/'frames'/(s['id']+'-clock.rgb'))],check=True,stdout=log)
 subprocess.run([sys.executable,str(ROOT/'scripts/native-packs/preview.py'),str(out)],check=True)
 print('All 41 scenes verified')
if __name__=='__main__':verify(Path(sys.argv[1]).resolve(),Path(sys.argv[2]))
