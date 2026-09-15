"""Check aligned 24-bit exports, additive reconstruction, and original master parity."""
from pathlib import Path
import json,wave
import numpy as np
ROOT=Path(__file__).resolve().parents[2]
def read(path):
 with wave.open(str(path),'rb') as w:
  rate,channels,width,frames=w.getframerate(),w.getnchannels(),w.getsampwidth(),w.getnframes();raw=w.readframes(frames)
 if width==3:
  a=np.frombuffer(raw,np.uint8).reshape(-1,3).astype(np.int32);a=a[:,0]|(a[:,1]<<8)|(a[:,2]<<16);a=np.where(a&0x800000,a-0x1000000,a)/8388608
 else:a=np.frombuffer(raw,'<i2').astype(np.float64)/32767
 return a.reshape(-1,channels),{'rate':rate,'channels':channels,'width':width,'frames':frames}
results=[]
for session in sorted((ROOT/'output/audio/stems-v1').glob('*/session.json')):
 d=json.loads(session.read_text());folder=session.parent;mix=np.zeros((d['frames'],2));peaks=[]
 for stem in d['stems']:
  x,m=read(folder/stem['file']);assert m=={'rate':44100,'channels':2,'width':3,'frames':d['frames']},m
  assert np.max(abs(x))<1 and np.max(abs(x))>0,stem
  mix+=x;peaks.append(float(np.max(abs(x))))
 reference,_=read(folder/'reference-premaster.wav');error=float(np.max(abs(reference-mix)));assert error<2e-6,error
 processed,_=read(folder/'reference-mastered.wav');original,_=read(ROOT/'output/audio'/f'{folder.name}-v1.wav');parity=float(np.max(abs(processed-original)));assert parity<3.1e-5,parity
 results.append({'track':folder.name,'stems':len(d['stems']),'frames':d['frames'],'sumMaxError':error,'masterMaxError':parity,'peak':max(peaks)})
assert len(results)==5
(ROOT/'.verification/audio/stems-results.json').write_text(json.dumps(results,indent=2))
print(json.dumps(results,indent=2))
