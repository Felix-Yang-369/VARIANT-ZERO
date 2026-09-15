"""Loop the approved v2 master; retain the untouched original and attribution."""
from pathlib import Path
import wave, json, hashlib
import numpy as np
root=Path(r'D:\Projects\VariantZeroUE')
source=root.parent/'零号变种'/'BGM-根系之外'/'根系之外.wav'
with wave.open(str(source),'rb') as w:
    sr=w.getframerate(); channels=w.getnchannels(); width=w.getsampwidth()
    raw=w.readframes(w.getnframes())
assert channels==2 and width==3
b=np.frombuffer(raw,dtype=np.uint8).reshape(-1,3).astype(np.int32)
v=b[:,0]|(b[:,1]<<8)|(b[:,2]<<16)
v=np.where(v&0x800000,v-0x1000000,v)
x=v.reshape(-1,channels).astype(np.float32)/8388608
n=sr*3
y=x[n:].copy(); a=np.linspace(0,1,n)[:,None]
y[-n:]=x[-n:]*(1-a)+x[:n]*a
k=int(sr*.01);y[:k]*=np.linspace(0,1,k)[:,None];y[-k:]*=np.linspace(1,0,k)[:,None]
assert np.isfinite(y).all()
out=root/'ArtSource/Audio/Exploration';out.mkdir(parents=True,exist_ok=True)
target=out/'SW_BeyondRoots.wav'
with wave.open(str(target),'wb') as w:
    w.setnchannels(channels);w.setsampwidth(2);w.setframerate(sr)
    w.writeframes((np.clip(y,-.98,.98)*32767).astype('<i2').tobytes())
report=dict(source=str(source),source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),seconds=len(y)/sr,peak=float(np.abs(y).max()),seam_jump=float(np.abs(y[0]-y[-1]).max()),sha256=hashlib.sha256(target.read_bytes()).hexdigest(),note='Approved v2 mixed master. Bird/vocal layers attenuate with entire exploration track during combat; stems not independently mixed.')
(out/'manifest.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps(report,ensure_ascii=False))
