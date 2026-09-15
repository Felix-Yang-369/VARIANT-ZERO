"""Original exploration cue. Standalone: does not alter game audio."""
from pathlib import Path
import json, math, struct, subprocess
import numpy as np
import imageio_ffmpeg

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / '根系之外'
OUT.mkdir(exist_ok=True)
STEMS = OUT / '分轨'
STEMS.mkdir(exist_ok=True)
SR, BPM, BARS = 44100, 72, 36
BEAT = 60 / BPM
LENGTH = BARS * 4 * BEAT + 8
N = round(LENGTH * SR)
FF = imageio_ffmpeg.get_ffmpeg_exe()
rng = np.random.default_rng(91426)
names = ['01-林冠持续音', '02-地下低音', '03-远处笛声', '04-种核拨弦', '05-木质脚步', '06-空气']
events = [[] for _ in names]

def note(track, beat, pitch, length, level, pan=0):
    events[track].append((beat, pitch, length, level, pan))

# D-centred modal harmony; open voicings leave room for the exploration motif.
chords = [[50,57,64,65], [46,53,60,64], [53,60,64,69], [48,55,62,64],
          [43,50,57,62], [46,53,60,65], [50,57,64,69], [45,52,59,62], [50,57,64,65]]
for block, chord in enumerate(chords):
    start = block * 16
    intensity = [.58,.72,.8,.85,.95,1,.9,.7,.45][block]
    for i, p in enumerate(chord):
        note(0,start,p,18,.038*intensity,(i-1.5)*.38)
    note(1,start,chord[0]-12,14,.115*intensity)
    if block in [2,3,4,5,6]:
        note(1,start+8,chord[0]-12,7,.07*intensity)
    for off, idx in [(1,1),(6.5,2),(10,3),(13.5,2)]:
        if block == 0 and off > 6.5: continue
        note(3,start+off,chord[idx]+12,3.8,.07*intensity,(-.4 if idx%2 else .35))
    if block < 8:
        for off in [3,7.5,11,14.5]:
            if block == 0 and off < 7: continue
            note(4,start+off+rng.uniform(-.025,.025),[48,55,50,60][int(off)%4],.5,.13*intensity,rng.uniform(-.55,.55))
    if block in [3,4,5]:
        for off in [5.5,12.75]:
            note(4,start+off,67,.32,.045,rng.uniform(-.65,.65))

# Call, silence, answer. The D-A-E-F identity becomes complete only in the middle.
phrases = [
 (7,[(0,74,2.2),(3,81,2.7)]),
 (24,[(0,76,2),(3.5,77,3),(7,74,2)]),
 (39,[(0,74,2),(2.5,81,2),(5.5,76,2),(8,77,3.5)]),
 (57,[(0,79,2),(3,76,2),(6,74,3)]),
 (70,[(0,74,2),(2.5,81,2.5),(6,76,2),(9,77,3),(13,79,1.5)]),
 (87,[(0,81,2),(3,84,2),(6,81,2),(9,77,3.5)]),
 (103,[(0,79,2),(3,77,2),(6,76,3),(10,74,3)]),
 (121,[(0,74,2.5),(4,81,3)]),
 (136,[(0,76,2),(3,74,4)])]
for start, phrase in phrases:
    for off,p,d in phrase:
        note(2,start+off+rng.uniform(-.035,.035),p,d,.13 if 38<start<110 else .105,-.12)

def sound(kind,pitch,dur):
    t=np.arange(round(dur*SR))/SR
    f=440*2**((pitch-69)/12)
    phase=2*np.pi*f*t
    attack,release=.02,min(.4,dur*.3)
    if kind==0:
        x=.55*np.sin(phase+.012*np.sin(t*2.1))+.22*np.sin(phase*1.0011)+.08*np.sin(phase*2)+.025*np.sin(phase*3)
        attack,release=2.4,3.0
    elif kind==1:
        x=.8*np.sin(phase)+.13*np.sin(phase*2)+.025*np.sin(phase*3)
        attack,release=.8,2.0
    elif kind==2:
        vib=np.sin(2*np.pi*4.7*t)*.065*(1-np.exp(-t*1.4))
        x=(.7*np.sin(phase+vib)+.13*np.sin(phase*2+vib)+.025*np.sin(phase*3))*(.94+.06*np.sin(t*3.7))
        # Filtered breath with no external recordings.
        breath=np.convolve(rng.normal(0,.05,len(t)),np.ones(9)/9,mode='same')
        x+=breath
        attack,release=.19,.6
    elif kind==3:
        x=sum(np.sin(phase*k+.1*k)*np.exp(-t*(.85+k*.72))/k**1.8 for k in range(1,7))*.7
        attack,release=.007,.3
    else:
        x=(np.sin(phase)*np.exp(-t*17)+.35*np.sin(phase*2.71)*np.exp(-t*30)+.15*np.sin(phase*4.13)*np.exp(-t*50))
        attack,release=.003,.12
    e=np.ones(len(t));a=min(len(t),round(attack*SR));r=min(len(t),round(release*SR))
    e[:a]*=np.sin(np.linspace(0,np.pi/2,a))**2
    e[-r:]*=np.cos(np.linspace(0,np.pi/2,r))**2
    return (x*e).astype(np.float32)

def wav(path,x):
    subprocess.run([FF,'-v','error','-y','-f','f32le','-ar',str(SR),'-ac','2','-i','pipe:0','-c:a','pcm_s24le',str(path)],input=x.astype('<f4').tobytes(),check=True)

mix=np.zeros((N,2),np.float32)
for k,name in enumerate(names):
    bus=np.zeros_like(mix)
    if k==5:
        # Slowly changing, low-level canopy air; band-limited by FFT.
        noise=rng.normal(size=N).astype(np.float32)
        spec=np.fft.rfft(noise);freq=np.fft.rfftfreq(N,1/SR)
        spec*=np.minimum(freq/350,1)**2/(1+(freq/2300)**4)
        air=np.fft.irfft(spec,n=N).astype(np.float32)
        air*=.0035*(.7+.3*np.sin(np.arange(N)/SR*.17))
        bus[:,0]=air;bus[:,1]=np.roll(air,811)
    for beat,pitch,duration,level,pan in events[k]:
        start=round(beat*BEAT*SR);x=sound(k,pitch,duration*BEAT)
        count=min(len(x),N-start)
        bus[start:start+count,0]+=x[:count]*level*math.sqrt((1-pan)/2)
        bus[start:start+count,1]+=x[:count]*level*math.sqrt((1+pan)/2)
    fade=np.ones(N,np.float32)
    fade[:SR*2]=np.linspace(0,1,SR*2);fade[-SR*8:]=np.linspace(1,0,SR*8)**1.5
    bus*=fade[:,None]
    wav(STEMS/(name+'.wav'),bus)
    mix+=bus
    print('Rendered',name,flush=True)

# Shared space, separately editable. Longer echoes belong only to tonal material.
reverb=np.zeros_like(mix)
for sec,gain in [(.083,.12),(.137,.1),(.229,.085),(.373,.075),(.571,.065),(.811,.055),(1.127,.043),(1.571,.03),(2.117,.018)]:
    shift=round(sec*SR)
    reverb[shift:]+=mix[:-shift,::-1]*gain
reverb[-SR*3:]*=np.linspace(1,0,SR*3)[:,None]
wav(STEMS/'07-共享空间.wav',reverb)
mix+=reverb
wav(OUT/'根系之外-72BPM-混音参考.wav',mix)
master=OUT/'根系之外-72BPM.wav'
subprocess.run([FF,'-v','error','-y','-i',str(OUT/'根系之外-72BPM-混音参考.wav'),'-af','loudnorm=I=-20:TP=-2:LRA=10','-ar',str(SR),'-c:a','pcm_s24le',str(master)],check=True)
subprocess.run([FF,'-v','error','-y','-i',str(master),'-c:a','libmp3lame','-b:a','256k',str(OUT/'根系之外-72BPM.mp3')],check=True)

# Standard MIDI retains the original notes, tempo and separate instrument tracks.
def vlq(v):
    b=[v&127];v>>=7
    while v:b.insert(0,(v&127)|128);v>>=7
    return bytes(b)
def chunk(data):return b'MTrk'+struct.pack('>I',len(data))+data
tempo=round(60000000/BPM)
midi=b'MThd'+struct.pack('>IHHH',6,1,6,480)
midi+=chunk(b'\x00\xff\x51\x03'+tempo.to_bytes(3,'big')+b'\x00\xff\x58\x04\x04\x02\x18\x08\x00\xff\x2f\x00')
for k,program in enumerate([89,43,73,46,115]):
    ev=[]
    for b,p,d,level,pan in events[k]:
        ev.append((round(b*480),bytes([0x90+k,p,min(110,max(30,round(level*500)))])))
        ev.append((round((b+d)*480),bytes([0x80+k,p,0])))
    data=bytes([0,0xc0+k,program]);last=0
    for tick,msg in sorted(ev,key=lambda v:v[0]):data+=vlq(tick-last)+msg;last=tick
    midi+=chunk(data+b'\x00\xff\x2f\x00')
(OUT/'根系之外-72BPM.mid').write_bytes(midi)
(OUT/'制作记录.json').write_text(json.dumps({'title':'根系之外','bpm':BPM,'meter':'4/4','bars':BARS,'duration':LENGTH,'sample_rate':SR,'synthesis':'Original deterministic synthesis; no sampled instruments','stems':'Unity-gain sum equals premaster, not loudness-normalized master','events':events},ensure_ascii=False,indent=2),encoding='utf-8')
print('DONE',OUT,flush=True)
