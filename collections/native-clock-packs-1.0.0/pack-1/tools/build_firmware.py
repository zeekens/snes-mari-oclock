"""Generate a compile-only ESPHome variant; never overwrite the device configuration."""
from pathlib import Path
import argparse,json
ROOT=Path(__file__).resolve().parents[1];P=ROOT
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output-dir',type=Path,help='Separate compile-only workspace; preserves previous build evidence')
parser.add_argument('ids',nargs='*')
args=parser.parse_args()
D=args.output_dir.resolve() if args.output_dir else P/'build';D.mkdir(parents=True,exist_ok=True)
manifest=json.loads((ROOT/'manifest.json').read_text())['items']
ids=args.ids or [i['id'] for i in manifest]
items=[i for i in manifest if i['id'] in ids];assert len(items)==len(ids)
h='#pragma once\n#include "native_scene.h"\nnamespace snes::native_scene { extern const Asset catalog['+str(len(items))+']; }\n'
(D/'native_scene_catalog.h').write_text(h)
lines=['#include "native_scene_catalog.h"','namespace snes::native_scene {']
for i,it in enumerate(items):
 data=(P/'packs'/(it['id']+'.sntl')).read_bytes();lines.append(f'alignas(4) static const uint8_t data_{i}[]={{')
 for k in range(0,len(data),32):lines.append(','.join(str(v) for v in data[k:k+32])+',')
 lines.append('};')
lines.append(f'const Asset catalog[{len(items)}]={{')
for i,it in enumerate(items):lines.append('{'+json.dumps(it['id'])+','+json.dumps(it['game']+' / '+it['title'])+f',data_{i},sizeof(data_{i})'+'},')
lines.append('}; }');(D/'native_scene_assets.cpp').write_text('\n'.join(lines))
y=(ROOT/'firmware/snes-clock.yaml').read_text()
y=y.replace('  name: ${device_name}','  name: ${device_name}\n  build_path: '+str(D/'build')+'\n  platformio_options:\n    build_flags:\n      - -DSNES_NATIVE_SCENES',1)
y=y.replace('    - include/','    - '+str(ROOT/'firmware/include')+'/').replace('  - source: components','  - source: '+str(ROOT/'firmware/components'))
y=y.replace('  includes:\n','  includes:\n    - '+str(ROOT/'firmware/include/native_scene.h')+'\n    - '+str(D/'native_scene_catalog.h')+'\n    - '+str(D/'native_scene_assets.cpp')+'\n',1)
y=y.replace('    on_value:\n',''.join('      - '+json.dumps(it['game']+' / '+it['title'])+'\n' for it in items)+'    on_value:\n',1)
y=y.replace('                id(scene).set_scene(choice);\n','')
(D/'sizecheck.yaml').write_text(y)
selection_path=D/'firmware-selection.json' if args.output_dir else P/'firmware-selection.json'
selection_path.write_text(json.dumps(dict(ids=ids,asset_bytes=sum((P/'packs'/(it['id']+'.sntl')).stat().st_size for it in items)),indent=2))
print('Compile-only variant:',D/'sizecheck.yaml')
