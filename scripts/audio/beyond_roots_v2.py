"""Sample-based atmospheric rearrangement; original sources stay in place."""
from pathlib import Path
import subprocess, json
import numpy as np
import imageio_ffmpeg

ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'BGM-根系之外'
OUT.mkdir(parents=True,exist_ok=True)
FF=imageio_ffmpeg.get_ffmpeg_exe()
SR=44100; LENGTH=128; N=SR*LENGTH
PACK=Path('C:/Program Files/Image-Line/FL Studio 2026/Data/Patches/Packs')
rng=np.random.default_rng(91427)

def decode(p,filters=None):
    raw=Path(p).read_bytes()
    ogg=raw.find(b'OggS',0,256)
    embedded=raw[ogg:] if raw[:4]==b'RIFF' and ogg>0 else None
    cmd=[FF,'-v','error','-i','pipe:0' if embedded else str(p)]
    if filters:cmd+=['-af',filters]
    cmd+=['-f','f32le','-ar',str(SR),'-ac','2','pipe:1']
    return np.frombuffer(subprocess.run(cmd,input=embedded,capture_output=True,check=True).stdout,dtype='<f4').reshape(-1,2).copy()

def save(p,x):
    subprocess.run([FF,'-v','error','-y','-f','f32le','-ar',str(SR),'-ac','2','-i','pipe:0','-c:a','pcm_s24le',str(p)],input=x.astype('<f4').tobytes(),check=True)

def fade(x,a=.2,b=.5):
    x=x.copy();na=min(len(x)//2,int(a*SR));nb=min(len(x)//2,int(b*SR))
    if na:x[:na]*=np.sin(np.linspace(0,np.pi/2,na))[:,None]**2
    if nb:x[-nb:]*=np.cos(np.linspace(0,np.pi/2,nb))[:,None]**2
    return x

def add(bus,x,t,gain=1,pan=0):
    i=round(t*SR);count=min(len(x),N-i)
    if count>0:
        bus[i:i+count]+=x[:count]*gain*np.array([np.sqrt(1-pan),np.sqrt(1+pan)])

def space(x,amount=1):
    y=x.copy()
    for t,g in [(.11,.16),(.23,.13),(.39,.12),(.63,.1),(.97,.08),(1.41,.06),(1.99,.045),(2.71,.03),(3.53,.02)]:
        d=int(t*SR);y[d:]+=x[:-d,::-1]*g*amount
    return y

def rms(x):return float(np.sqrt(np.mean(x*x)))

# Continuous real bird recording, with crossfaded, offset repeats.
bird=decode(OUT/'素材/forest-birds-Magnesus-CC0.mp3','highpass=f=180,lowpass=f=8500')
bird=fade(bird,2,3)
forest=np.zeros((N,2),np.float32)
for i,t in enumerate(np.arange(0,LENGTH,22)):
    add(forest,bird[:,::(-1 if i%2 else 1)],t,.017/max(rms(bird),1e-8))
time=np.arange(N)/SR
# Life grows quiet at the encounter, then returns.
forest*=np.interp(time,[0,8,34,49,61,79,97,112,128],[.45,1,.8,.25,.22,.4,.8,1,0])[:,None]

# Rescore existing original harmony: remove the synthetic flute and steady percussion.
old=ROOT/'BGM-根系之外/制作工程/原创配乐分轨'
music=decode(old/'01-林冠持续音.wav','lowpass=f=2800')*.55
music+=decode(old/'02-地下低音.wav')*.38
music+=decode(old/'04-种核拨弦.wav')*.8
music*=np.interp(time,[0,12,24,40,64,85,105,118,128],[0,0,.7,.65,1,1,.65,.4,0])[:,None]
music=space(music,.7)

# Actual sung vowels, stretched without changing pitch. No synthesized fake choir.
vocal_files=[PACK/'Vocals/Laurie Webb Ahh D.wav',PACK/'Vocals/Laurie Webb Ooh C.wav',PACK/'Vocals/Laurie Webb Mmh.wav']
vocal=np.zeros_like(music)
def sample_pitch(x):
    mono=x.mean(axis=1); start=np.argmax(np.convolve(mono[::128]**2,np.ones(60),mode='same'))*128
    start=max(0,min(start-8192,len(mono)-16384));seg=mono[start:start+16384]
    if len(seg)<4096:return None
    spec=np.fft.rfft(seg*np.hanning(len(seg)));corr=np.fft.irfft(abs(spec)**2)
    lo,hi=int(SR/700),min(int(SR/110),len(corr)//2)
    lag=lo+np.argmax(corr[lo:hi]);return float(69+12*np.log2((SR/lag)/440))
pitch=sample_pitch(decode(vocal_files[0]))
print('Vocal source detected MIDI',pitch,flush=True)
target_base=74
semitones=target_base-round(pitch) if pitch is not None else 0
# One expressive voice with rests; no dense syllable grid.
for t,delta,gain in [(21,0,.075),(27,7,.06),(39,2,.075),(45,3,.065),(62,0,.085),(68,7,.07),(75,2,.075),(81,3,.065),(99,0,.065),(107,-5,.05)]:
    ratio=2**((semitones+delta)/12)
    filters=f'asetrate={SR*ratio},aresample={SR},atempo={1/ratio},atempo=0.65,highpass=f=200,lowpass=f=6500'
    x=decode(vocal_files[0],filters);x=fade(x,.28,.8)
    x*=gain/max(float(abs(x).max()),1e-8)
    add(vocal,x,t,1,-.2 if delta%2 else .18)
vocal=space(vocal,1.9)

# Processed vocal grains become an unidentified creature replying in the distance.
creature=np.zeros_like(music)
x=decode(vocal_files[2],'asetrate=30000,aresample=44100,highpass=f=300,lowpass=f=2200')
x=fade(x,.12,.5);x*=.026/max(float(abs(x).max()),1e-8)
for t in [12,52,92,116]:add(creature,x,t,1,-.6 if t<60 else .55)
creature=space(creature,2.3)

# Soft seed-shell percussion enters only after the forest has established itself.
percussion=np.zeros_like(music)
wood=decode(old/'05-木质脚步.wav')
wood*=np.interp(time,[0,35,48,62,89,103,128],[0,0,.4,.7,.6,0,0])[:,None]
percussion+=space(wood,.4)

mix=forest+music+vocal+creature+percussion
mix=fade(mix,3,9)
save(OUT/'根系之外-v2-林间回声-混音参考.wav',mix)
master=OUT/'根系之外-v2-林间回声.wav'
subprocess.run([FF,'-v','error','-y','-i',str(OUT/'根系之外-v2-林间回声-混音参考.wav'),'-af','loudnorm=I=-19:TP=-2:LRA=11','-ar',str(SR),'-c:a','pcm_s24le',str(master)],check=True)
mp3=OUT/'根系之外-v2-林间回声.mp3'
subprocess.run([FF,'-v','error','-y','-i',str(master),'-c:a','libmp3lame','-b:a','256k',str(mp3)],check=True)
subprocess.run([FF,'-v','error','-y','-ss','15','-i',str(master),'-t','45','-af','afade=t=in:d=1,afade=t=out:st=42:d=3','-c:a','libmp3lame','-b:a','256k',str(OUT/'先听这段-45秒.mp3')],check=True)
check=subprocess.run([FF,'-hide_banner','-i',str(mp3),'-af','loudnorm=I=-19:TP=-2:LRA=11:print_format=json','-f','null','-'],capture_output=True,check=True).stderr.decode('utf-8',errors='replace')
(OUT/'音频检查.txt').write_text(check,encoding='utf-8')
(OUT/'制作参数.json').write_text(json.dumps({'bpm':72,'length':128,'vocal_pitch_estimate_midi':pitch,'sources':[str(p) for p in vocal_files if p!=vocal_files[1]],'bird_source':'https://freesound.org/people/Magnesus/sounds/723913/','note':'Vocal source files remain in FL Studio installation. No isolated vocal samples distributed.'},ensure_ascii=False,indent=2),encoding='utf-8')
print(check[-650:],flush=True)
print('DONE',flush=True)
