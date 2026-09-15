"""Read-only snapshot of v0.4 rules. Existing snapshot is immutable and fails on drift."""
from pathlib import Path
import hashlib, json, shutil
root = Path(__file__).resolve().parents[1]
source = root.parent if (root.parent / 'package.json').exists() else root.parent / '零号变种'
dest = root / 'Reference' / 'v0.4'
files = list((source / 'src' / 'variant').glob('*.ts')) + [source / 'package.json', source / 'package-lock.json']
manifest = {}
for file in files:
    if not file.exists():
        continue
    relative = file.relative_to(source)
    data = file.read_bytes()
    target = dest / relative
    if target.exists() and target.read_bytes() != data:
        raise RuntimeError(f'Frozen source drift: {relative}; do not overwrite baseline')
    target.parent.mkdir(parents=True, exist_ok=True)
    if not target.exists():
        target.write_bytes(data)
    manifest[relative.as_posix()] = hashlib.sha256(data).hexdigest()
target = dest / 'sha256.json'
if target.exists() and json.loads(target.read_text('utf-8')) != manifest:
    raise RuntimeError('Manifest changed')
target.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
print(f'Frozen and verified {len(manifest)} files; original project untouched.')
