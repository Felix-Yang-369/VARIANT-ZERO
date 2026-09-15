"""Original, deterministic stereo score + layered game foley. No remote samples.
Run with Python/numpy and ffmpeg. Masters are kept separate from game exports.
"""
from pathlib import Path
import numpy as np, wave, json, subprocess, math, argparse
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--stems-only',action='store_true',help='Export editable BGM stems without replacing game audio')
args=parser.parse_args()
ROOT=Path(__file__).resolve().parents[2]; OUT=ROOT/'public/audio/v1'; MASTER=ROOT/'output/audio'; OUT.mkdir(parents=True,exist_ok=True);MASTER.mkdir(parents=True,exist_ok=True)
SR=44100; rng=np.random.default_rng(90413)
def tone(m):return 440*2**((m-69)/12)
def env(n,attack=.008,release=.08):
 a=np.ones(n);aa=min(n,int(attack*SR));rr=min(n,int(release*SR));a[:aa]=np.sin(np.linspace(0,np.pi/2,aa))**2;a[-rr:]*=np.linspace(1,0,rr)**2 if rr else 1;return a

def noise(seconds,lo=80,hi=10000):
 n=int(seconds*SR);v=rng.normal(size=n);f=np.fft.rfftfreq(n,1/SR);z=np.fft.rfft(v);z*=np.minimum(1,(f/max(lo,1))**2)/(1+(f/hi)**4);a=np.fft.irfft(z,n);return a/(np.std(a)+1e-9)

def voice(kind,m,duration):
 t=np.arange(int(duration*SR))/SR;f=tone(m);p=2*np.pi*f*t
 if kind=='keys':x=(np.sin(p+1.3*np.exp(-t*5)*np.sin(2*p))*.66+np.sin(p*2+.5)*.18*np.exp(-t*3)+np.sin(p*3)*.04)*np.exp(-t*1.5)*env(len(t),.009,.22)
 elif kind=='harp':x=sum(np.sin(p*k+.2*k)*np.exp(-t*(1.8+k*.9))/k**1.6 for k in range(1,7))*env(len(t),.004,.1)*.6
 elif kind=='pad':x=sum(np.sin(p*k+np.sin(t*.6+k)*.022)*w for k,w in [(1,.5),(2,.15),(3,.045),(4,.02)])+np.sin(p*1.0012)*.18;x*=env(len(t),min(.75,duration*.2),min(1,duration*.28))
 elif kind=='bass':x=(np.sin(p)*.8+np.sin(p*2)*.18+np.sin(p*3)*.04)*np.exp(-t*1.3)*env(len(t),.018,.12)
 elif kind=='bell':x=(np.sin(p)*np.exp(-t*2)+np.sin(p*2.003)*.32*np.exp(-t*3.5)+np.sin(p*3.97)*.12*np.exp(-t*7))*env(len(t),.003,.3)*.55
 elif kind=='wood':x=(np.sin(p)*np.exp(-t*13)+np.sin(p*3.01)*.4*np.exp(-t*26))*env(len(t),.002,.1)
 else:x=np.sin(p)*env(len(t),.01,.08)
 return x.astype(np.float32)

def add(bus,x,start,amp=1,pan=0):
 i=int(start*SR);n=min(len(x),len(bus)-i)
 if n<=0:return
 pan=max(-.95,min(.95,pan));bus[i:i+n,0]+=x[:n]*amp*math.sqrt((1-pan)*.5);bus[i:i+n,1]+=x[:n]*amp*math.sqrt((1+pan)*.5)

def drum(kind):
 dur={'kick':.5,'snare':.3,'hat':.12,'shaker':.18}[kind];t=np.arange(int(dur*SR))/SR
 if kind=='kick':return (np.sin(2*np.pi*(42*t+2.3*(1-np.exp(-t*32))))*np.exp(-t*10)+noise(dur,1800,8000)*np.exp(-t*150)*.025)*env(len(t),.001,.04)
 if kind=='snare':return (noise(dur,1000,6500)*np.exp(-t*21)*.45+np.sin(2*np.pi*185*t)*np.exp(-t*30)*.3)*env(len(t),.002,.04)
 return noise(dur,4500,12500)*np.exp(-t*(48 if kind=='hat' else 26))*env(len(t),.002,.04)*.22
DRUMS={k:drum(k) for k in ['kick','snare','hat','shaker']}

def space(x,wet=.17,loop=False):
 # A stereo early-reflection field plus diffuse, decaying late reflections.
 y=x.copy()
 for delay,gain in [(.071,.18),(.113,.13),(.197,.10),(.293,.08),(.431,.06),(.619,.045),(.887,.025)]:
  n=int(delay*SR);r=np.roll(x[:,::-1],n,axis=0) if loop else np.concatenate([np.zeros((n,2),np.float32),x[:-n,::-1]]) if n<len(x) else np.zeros_like(x)
  y+=r*(wet/.17)*gain
 return y

def master(x,target=.115,peak=.79):
 x-=np.mean(x,axis=0);rms=np.sqrt(np.mean(x*x));x*=min(2.5,target/max(rms,1e-8));x=np.tanh(x*1.1)/1.1;x*=min(1,peak/max(np.max(np.abs(x)),1e-8));return x.astype(np.float32)

def savewav(path,x):
 with wave.open(str(path),'wb') as w:w.setnchannels(2);w.setsampwidth(2);w.setframerate(SR);w.writeframes((np.clip(x,-1,1)*32767).astype('<i2').tobytes())
manifest={'version':1,'sampleRate':SR,'music':{},'sfx':{}};metrics=[]
CHORDS=[[50,53,57,60,64],[46,50,53,57,60],[41,48,53,57,60],[48,52,55,62,67],[43,50,53,57,62],[46,53,57,60,65],[50,53,57,60,64],[45,52,57,62,64]]
MELODY=[[74,77,81,79,77,74],[72,74,77,81,77],[72,77,79,81,84,81],[79,76,74,72,74],[74,77,79,82,79],[77,81,84,81,77],[81,79,77,74,72],[76,74,73,76,69]]
tracks=[('sanctuary','原初之息',84,32,'温室与图鉴：柔和电钢琴、竖琴、空气弦层'),('laboratory','生命的分叉',76,24,'实验室：玻璃钟、细碎木音、克制脉冲'),('preparation','生态编队',96,24,'准备阶段：温暖低音、木质节奏与主题碎片'),('battle','共生前线',108,32,'自动战斗：有机鼓组、律动低音与层次推进'),('boss','零序列回响',120,32,'Boss：低频脉冲、切分节奏与紧张主题变奏')]
STEM_NAMES={'pad':'空气弦层','bass':'低音','harp':'竖琴琶音','bell':'玻璃钟','keys':'电钢琴旋律','wood':'木质打击','kick':'底鼓','snare':'军鼓','hat':'踩镲','shaker':'沙锤'}
stem_buses={}
def music_add(kind,bus,x,start,amp=1,pan=0):
 add(bus,x,start,amp,pan)
 if args.stems_only:
  if kind not in stem_buses:stem_buses[kind]=np.zeros_like(bus)
  add(stem_buses[kind],x,start,amp,pan)

def save24(path,x):
 subprocess.run(['ffmpeg','-hide_banner','-loglevel','error','-y','-f','f32le','-ar',str(SR),'-ac','2','-i','pipe:0','-c:a','pcm_s24le',str(path)],input=x.astype('<f4').tobytes(),check=True)

def export_stems(key,title,bpm,bars,n,premix,processed):
 folder=MASTER/'stems-v1'/key;folder.mkdir(parents=True,exist_ok=True)
 total=np.zeros_like(premix)
 for bus in stem_buses.values():
  bus[:len(bus)-n]+=bus[n:]
  total+=bus[:n]
 # Separate the existing shared stereo reflections, so dry instruments remain editable.
 reverb=premix-total
 gain=min(1,.78/max(float(np.max(abs(premix))),*(float(np.max(abs(b[:n]))) for b in stem_buses.values()),float(np.max(abs(reverb))),1e-8))
 fade=np.ones((n,1),np.float32);fade[:96,0]=np.linspace(0,1,96);fade[-96:,0]=np.linspace(1,0,96)
 entries=[]
 for i,(kind,bus) in enumerate(stem_buses.items(),1):
  name=f'{i:02d}-{kind}.wav';save24(folder/name,bus[:n]*gain*fade)
  entries.append({'file':name,'instrument':kind,'label':STEM_NAMES[kind],'role':'dry'})
 save24(folder/'90-reverb-return.wav',reverb*gain*fade)
 entries.append({'file':'90-reverb-return.wav','label':'共享空间混响','role':'effects-return'})
 save24(folder/'reference-premaster.wav',premix*gain*fade)
 save24(folder/'reference-mastered.wav',processed)
 data={'title':title,'bpm':bpm,'timeSignature':'4/4','bars':bars,'frames':n,'sampleRate':SR,'bitDepth':24,'channels':2,'linearGain':gain,'startSeconds':0,'stems':entries,'references':['reference-premaster.wav','reference-mastered.wav'],'note':'Import stems and reverb together at zero, unity gain. Do not also play references. Mastered reference includes nonlinear saturation and will not null against the stem sum.'}
 (folder/'session.json').write_text(json.dumps(data,ensure_ascii=False,indent=2))
 print('stems',key,len(entries),'tracks',flush=True)

for key,title,bpm,bars,desc in tracks:
 stem_buses={}
 beat=60/bpm;duration=bars*4*beat;n=round(duration*SR);dry=np.zeros((n+SR*5,2),np.float32);pulse=np.zeros_like(dry);drums=np.zeros_like(dry)
 for bar in range(bars):
  chord=CHORDS[bar%8];st=bar*4*beat;section=bar//8;energy=[.65,1,.82,1.1][section];combat=key in ['battle','boss'];quiet=key in ['sanctuary','laboratory']
  for j,m in enumerate(chord):music_add('pad',dry,voice('pad',m+12,4*beat+1.4),st,.035 if combat else .054,(j-2)*.27)
  music_add('bass',pulse,voice('bass',chord[0]-12,1.7*beat),st,.25 if combat else .14,-.03)
  if not quiet:
   for off in [1.5,2.5,3.5]:music_add('bass',pulse,voice('bass',chord[0]-12+(12 if off==3.5 else 0),beat*.45),st+off*beat,.13*energy,.02)
  arp=[0,2,3,1,4,2,1,3] if key!='laboratory' else [0,3,1,4,2,3,4,1]
  for a,idx in enumerate(arp):
   if quiet and section==0 and a%2:continue
   m=chord[idx]+(24 if key=='laboratory' else 12)
   music_add('bell' if key=='laboratory' else 'harp',dry,voice('bell' if key=='laboratory' else 'harp',m,1.4),st+a*.5*beat,.052*energy if quiet else .062*energy,(-1 if a%2 else 1)*.35)
  motif=MELODY[bar%8]
  if key!='laboratory' or bar%2==0:
   for j,off in enumerate([.0,.75,1.5,2.5,3.25]):
    if section==0 and j in [1,4]:continue
    m=motif[(j+(1 if section==2 else 0))%len(motif)];vol=.14 if key=='sanctuary' else .095 if key=='laboratory' else .105
    music_add('keys' if key!='boss' else 'bell',dry,voice('keys' if key!='boss' else 'bell',m,1.7 if quiet else 1.1),st+off*beat,vol*energy,-.15)
  if key=='sanctuary' and section in [1,3]:
   for off,m in [(1.25,chord[2]+24),(3.5,chord[3]+24)]:music_add('bell',dry,voice('bell',m,2),st+off*beat,.025,.5)
  if key=='laboratory':
   for off in [1,2.75]:music_add('wood',pulse,voice('wood',chord[1]+12,.22),st+off*beat,.055,(-.4 if off==1 else .4))
  if not quiet:
   for off in ([0,1.5,2,3.25] if key=='boss' else [0,2]):music_add('kick',drums,DRUMS['kick'],st+off*beat,.32*energy)
   for off in [1,3]:music_add('snare',drums,DRUMS['snare'],st+off*beat,(.14 if key=='preparation' else .24)*energy,.1)
   for a in range(8):music_add('hat' if combat else 'shaker',drums,DRUMS['hat' if combat else 'shaker'],st+a*.5*beat,.16*energy*(.7 if a%2 else 1),.4 if a%2 else -.4)
   if bar%8==7:
    for off in [3.25,3.5,3.75]:music_add('snare',drums,DRUMS['snare'],st+off*beat,.12,.2)
  elif section>0:
   for off in [1.5,3.5]:music_add('shaker',drums,DRUMS['shaker'],st+off*beat,.065,.6)
 # Fold sustaining notes into the start, preserving exact beat-length loops.
 for bus in [dry,pulse,drums]:bus[:len(bus)-n]+=bus[n:]
 x=space(dry[:n],.24 if quiet else .14,True)+pulse[:n]+space(drums[:n],.04,True)
 premix=x.copy() if args.stems_only else None
 x=master(x,target=.104 if quiet else .126,peak=.78)
 # Sub-millisecond equal-value endpoint prevents sample discontinuity, no full-track fade.
 k=96;x[:k]*=np.linspace(0,1,k)[:,None];x[-k:]*=np.linspace(1,0,k)[:,None]
 if args.stems_only:
  export_stems(key,title,bpm,bars,n,premix,x)
  continue
 wav=MASTER/(key+'-v1.wav');savewav(wav,x)
 dest=OUT/(key+'-v1.mp3');subprocess.run(['ffmpeg','-hide_banner','-loglevel','error','-y','-i',str(wav),'-c:a','libmp3lame','-b:a','192k',str(dest)],check=True)
 manifest['music'][key]={'title':title,'bpm':bpm,'bars':bars,'duration':duration,'url':'/audio/v1/'+dest.name,'description':desc}
 metrics.append({'asset':key,'duration':duration,'peakDb':round(20*np.log10(np.max(abs(x))),2),'rmsDb':round(20*np.log10(np.sqrt(np.mean(x*x))),2),'dc':float(abs(x.mean())),'loopJump':float(np.max(abs(x[0]-x[-1])))})
 print('music',key,duration,flush=True)

if args.stems_only:
 print("Editable stems saved to",MASTER/"stems-v1",flush=True)
 raise SystemExit(0)

# Each event has a distinct transient, tonal body and spatial tail. Variants use separate seeds.
SFX={
 'select':('选择标本',.24,1,'ui',.045), 'navigate':('终端翻页',.38,1,'ui',.09),'cancel':('撤销返回',.28,1,'ui',.07),'invalid':('操作受限',.4,2,'ui',.12),
 'buy':('样本培养',.7,2,'economy',.09),'sell':('样本回收',.65,2,'economy',.09),'deploy':('扎根部署',.55,2,'economy',.1),'refresh':('商店重组',.7,2,'economy',.15),'lock':('保存锁定',.3,1,'ui',.07),'capacity':('容量研究',1.4,4,'experiment',.25),
 'merge':('升星共鸣',1.8,5,'experiment',.25),'experiment':('遗传实验启动',1.6,5,'experiment',.25),'discovery':('首次发现',2.8,6,'result',.3),'duplicate':('稳定复现',1.8,5,'result',.3),'failure':('分化未成',1.6,5,'result',.3),
 'shot_seed':('种荚射击',.22,0,'attack',.11),'shot_spore':('孢子喷发',.35,0,'attack',.15),'hit_soft':('生物受击',.22,0,'impact',.09),'hit_shell':('甲壳受击',.29,0,'impact',.11),'melee':('节肢近战',.34,1,'attack',.14),'move':('足肢移动',.16,0,'movement',.18),
 'heal':('再生流光',.8,2,'heal',.3),'death':('生命消散',.75,1,'impact',.16),'explosion':('爆裂孢囊',1.3,5,'impact',.3),'revive':('零序列复生',2.0,6,'experiment',.5),
 'start':('出征启动',1.5,5,'result',.4),'warning':('最终敌袭',2.0,7,'result',.6),'won':('回合突破',2.7,7,'result',.5),'lost':('未能突破',2.6,7,'result',.5),'reward':('试剂入库',1.1,3,'economy',.2),'pause':('终端暂停',.25,2,'ui',.1),'resume':('终端继续',.35,2,'ui',.1)
}
for index,(key,(title,dur,priority,group,interval)) in enumerate(SFX.items()):
 urls=[]
 for variant in range(3 if priority<3 else 1):
  rng=np.random.default_rng(73000+index*91+variant);n=int(dur*SR);b=np.zeros((n,2),np.float32);shift=(variant-1)*.35
  def layer(kind,m,at=0,d=.3,amp=.3,pan=0):add(b,voice(kind,m+shift,min(d,dur-at)),at,amp,pan)
  def swish(at=0,d=.2,amp=.2,lo=1200,hi=7500):
   d=min(d,dur-at);t=np.arange(int(d*SR))/SR;x=noise(d,lo,hi)*np.sin(np.pi*t/d)**2*np.exp(-t*5);add(b,x,at,amp,variant*.1-.1)
  if key in ['select','navigate','cancel','lock','pause','resume']:
   notes={'select':[81,88],'navigate':[74,81,86],'cancel':[79,74],'lock':[69,81],'pause':[74,69],'resume':[69,74,81]}[key]
   for i,m in enumerate(notes):layer('wood' if key=='lock' else 'bell',m,i*.045,.2,.26/(i+1)**.3,(-.2 if i%2 else .2))
   if key=='navigate':swish(.02,.23,.08,2500,11000)
  elif key=='invalid':layer('keys',46,0,.25,.28);layer('wood',45,.1,.2,.18)
  elif key in ['shot_seed','shot_spore','hit_soft','hit_shell','melee','move','death','explosion']:
   if key=='shot_seed':layer('wood',55,0,.16,.55);swish(.0,.13,.24,400,4300);layer('harp',79,.02,.15,.09)
   elif key=='shot_spore':swish(0,.28,.31,900,6500);layer('keys',63,.015,.22,.19)
   elif key=='hit_soft':swish(0,.16,.35,90,2100);layer('bass',40,0,.17,.35)
   elif key=='hit_shell':swish(0,.12,.22,2100,9000);layer('bell',73,0,.28,.28);layer('wood',49,.0,.12,.2)
   elif key=='melee':swish(0,.2,.33,450,5200);layer('wood',44,.07,.23,.55);swish(.11,.13,.2,100,2300)
   elif key=='move':swish(0,.12,.11,800,4200);layer('wood',45,0,.12,.24)
   elif key=='death':swish(0,.56,.14,1300,6600);layer('keys',59,0,.7,.22);layer('keys',47,.1,.55,.2)
   else:
    t=np.arange(n)/SR;add(b,np.sin(2*np.pi*(32*t+3*(1-np.exp(-t*13))))*np.exp(-t*5)*env(n,.002,.4),0,.7);swish(0,.8,.5,90,3100);swish(.06,.95,.19,2800,10000)
  elif key in ['buy','sell','deploy','refresh','reward']:
   if key=='deploy':swish(0,.33,.24,120,3000);layer('wood',43,0,.3,.45);layer('harp',74,.1,.4,.23)
   else:
    notes={'buy':[62,69,74],'sell':[81,77,74],'refresh':[62,65,69,74],'reward':[74,81,86]}[key]
    for i,m in enumerate(notes):layer('harp',m,.06+i*.09,.5,.32,(i-1)*.25)
    swish(0,.3,.1,1200,8000)
  elif key in ['heal','capacity','merge','experiment','revive']:
   notes={'heal':[74,81,86],'capacity':[62,69,74,81],'merge':[62,65,69,74,81],'experiment':[50,57,63,69,74],'revive':[45,57,62,69,74,81]}[key]
   for i,m in enumerate(notes):layer('bell' if key!='experiment' else 'keys',m,i*.13,dur-i*.13,.26,(i/(len(notes)-1)-.5)*.9)
   swish(0,min(1,dur),.13,1500,8500)
  else:
   notes={'discovery':[62,69,74,77,81,86],'duplicate':[62,69,74,81],'failure':[65,62,57,50],'start':[50,57,62,69,74],'warning':[50,51,50,51,57],'won':[62,65,69,74,77,81],'lost':[62,60,57,53,50]}[key]
   for i,m in enumerate(notes):layer('keys',m,i*.17,dur-i*.17,.29,(i%3-1)*.25)
   if key in ['warning','start']:add(b,DRUMS['kick'],0,.4);swish(.05,.7,.1,600,7000)
   elif key in ['discovery','won']:layer('bell',93,.7,dur-.7,.12,.45)
  b=space(b,.12 if priority>=3 else .04);b=master(b,target=.105 if priority<3 else .13,peak=.69);b*=env(len(b),.003,.07)[:,None]
  name=f'{key}-{variant+1}.wav';savewav(OUT/name,b);urls.append('/audio/v1/'+name)
  metrics.append({'asset':name,'duration':dur,'peakDb':round(20*np.log10(max(1e-9,np.max(abs(b)))),2),'rmsDb':round(20*np.log10(max(1e-9,np.sqrt(np.mean(b*b)))),2),'dc':float(abs(b.mean()))})
 manifest['sfx'][key]={'title':title,'urls':urls,'priority':priority,'group':group,'interval':interval,'duration':dur}
(OUT/'manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2));(ROOT/'.verification/audio/asset-metrics.json').write_text(json.dumps(metrics,indent=2,default=float))
# A shareable 36s listening reel, with crossfades between scenes.
preview=np.zeros((SR*38,2),np.float32)
for i,(key,*_) in enumerate(tracks[:3]):
 with wave.open(str(MASTER/(key+'-v1.wav'))) as f:x=np.frombuffer(f.readframes(f.getnframes()),dtype='<i2').reshape(-1,2)/32767
 seg=x[SR*12:SR*26].copy();seg*=env(len(seg),1,1.5)[:,None];preview[SR*(i*12):SR*(i*12)+len(seg)]+=seg*.9
savewav(MASTER/'research-suite-preview.wav',preview);subprocess.run(['ffmpeg','-hide_banner','-loglevel','error','-y','-i',str(MASTER/'research-suite-preview.wav'),'-b:a','192k',str(MASTER/'research-suite-preview.mp3')],check=True)
print('Finished',len(manifest['music']),'scores',len(manifest['sfx']),'events',sum(len(x['urls']) for x in manifest['sfx'].values()),'samples',flush=True)
