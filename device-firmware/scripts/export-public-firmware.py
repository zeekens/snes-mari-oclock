"""Export only reviewable firmware source; never export build output or credentials."""
import argparse
import shutil
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FILES = ['requirements.txt', 'firmware/snes-clock.yaml', 'firmware/snes-clock-device.yaml',
         'scripts/firmware-release.py', 'scripts/export-public-firmware.py',
         'tests/firmware_release_test.py', 'tests/firmware_url_test.cpp', 'tests/native_stream_test.cpp',
         'tests/native_quality_test.cpp', 'docs/SCENE-PACK-QUALITY.md',
         'scene-packs/source-catalog.json', 'scene-packs/quality-policy.json',
         'scripts/native-packs/build.py', 'scripts/native-packs/verify.py',
         'scripts/native-packs/preview.py', 'scripts/native-packs/preview.html', 'scripts/native-packs/proof.cpp',
         'tests/fixtures/native_scene_v1.h', 'tests/fixtures/zelda-01.sntl', 'tests/fixtures/README.md',
         'docs/FIRMWARE-UPDATES.md', 'docs/FIRMWARE-UPDATE-VALIDATION.md', 'deploy/firmware-server/compose.yaml', 'deploy/firmware-server/nginx.conf']

def export(destination):
    paths = [ROOT / f for f in FILES]
    paths += sorted((ROOT / 'firmware/include').glob('*.h'))
    paths += sorted(p for p in (ROOT / 'firmware/components/native_library').iterdir() if p.suffix in ('.h','.cpp','.py'))
    paths += sorted(p for p in (ROOT / 'firmware/components/firmware_transport').iterdir() if p.suffix in ('.h','.cpp','.py'))
    # Also check against local secret values if provisioned. Report paths only.
    import yaml
    secret_file = ROOT / 'firmware/secrets.yaml'
    secrets = yaml.safe_load(secret_file.read_text()) if secret_file.exists() else {}
    values = [v for k, v in secrets.items() if k != 'firmware_download_user']
    needles = [str(v).encode() for v in values if len(str(v)) >= 8]
    for path in paths:
        data = path.read_bytes()
        if any(n in data for n in needles):
            raise ValueError(f'Credential detected in {path.relative_to(ROOT)}')
    for path in paths:
        output = destination / path.relative_to(ROOT)
        output.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(path, output)
    print(f'Exported {len(paths)} source files; no binaries or secrets')

if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('destination', type=Path)
    export(p.parse_args().destination)
