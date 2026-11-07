"""Rebuild the local review; never publishes or contacts the clock."""
from pathlib import Path
import subprocess,sys
root=Path(__file__).resolve().parents[2]
for script in ('baseline.py','test_geometry.py','classic_audit.py','build.py','gallery.py','validate_delivery.py'):
 subprocess.run([sys.executable,str(Path(__file__).with_name(script))],cwd=root,check=True)
