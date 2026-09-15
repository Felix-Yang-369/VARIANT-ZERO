"""Derive audition loops from existing original score; generate original tool cues."""
from pathlib import Path
import wave, json, hashlib
import numpy as np
ROOT=Path(r'D:\Projects\VariantZeroUE'); OUT=ROOT/'ArtSource/Audio/P0'; OUT.mkdir(parents=True,exist_ok=True)
SRC=Path(r'D:\Projects\零号变种\玻璃下的春天\02-WAV无损')
report=[]
def write(name,x,sr=44100,source='Original synthesized cue'):
    x=np.clip(x,-.95,.95)
    if x.ndim==1:x=np.column_stack([x,x])
    p=OUT/(name+'.wav')
    with wave.open(str(p),'wb') as w:w.setnchannels(2);w.setsampwidth(2);w.setframerate(sr);w.writeframes((x*32767).astype('<i2').tobytes())
    report.append(dict(name=name,seconds=len(x)/sr,peak=float(np.max(np.abs(x))),seam_jump=float(np.max(np.abs(x[0]-x[-1]))),source=source,sha256=hashlib.sha256(p.read_bytes()).hexdigest()))
for name,file in [('SW_Conservatory','01-原初之息-84BPM.wav'),('SW_Alert','04-共生前线-108BPM.wav')]:
    p=SRC/file
    with wave.open(str(p)) as w:
        assert w.getsampwidth()==2 and w.getnchannels()==2
        sr=w.getframerate();x=np.frombuffer(w.readframes(w.getnframes()),dtype='<i2').reshape(-1,2).astype(np.float32)/32768
    n=int(sr*2);y=x[n:].copy();mix=np.linspace(0,1,n)[:,None];y[-n:]=x[-n:]*(1-mix)+x[:n]*mix
    write(name,y*.7,sr,str(p))
rng=np.random.default_rng(20260915)
for name,dur,freq in [('SW_Pulse',.17,740),('SW_Shield',.65,260),('SW_Repair',1.4,523),('SW_Hit',.19,115)]:
    t=np.arange(int(44100*dur))/44100;env=np.minimum(1,t/.015)*np.exp(-t/(dur*.3));x=(np.sin(2*np.pi*(freq*t+80*t*t))+.3*np.sin(2*np.pi*freq*1.5*t))*env*.13
    if name=='SW_Hit':x+=rng.normal(0,.025,len(t))*env
    x[-500:]*=np.linspace(1,0,500);write(name,x)
(OUT/'manifest.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps(report,ensure_ascii=False))
