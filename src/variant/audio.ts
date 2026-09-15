import catalogue from '../../public/audio/v1/manifest.json';
import type {Preferences} from '../settings';
export type MusicScene=keyof typeof catalogue.music;
export type SoundName=keyof typeof catalogue.sfx;
export const MUSIC=catalogue.music;
export const EFFECTS=catalogue.sfx;
type Voice={source:AudioBufferSourceNode;gain:GainNode;pan:StereoPannerNode;priority:number};
type MusicVoice={source:AudioBufferSourceNode;gain:GainNode;key:MusicScene;started:number;offset:number};
/** Asset playback is presentation-only; audio variants never use simulation RNG. */
export class AudioManager {
 private ctx?:AudioContext;private musicBus?:GainNode;private sfxBus?:GainNode;private duck?:GainNode;
 private master?:GainNode;private limiter?:DynamicsCompressorNode;
 private prefs:Preferences;private scene:MusicScene='sanctuary';private preview?:MusicScene;
 private voices=new Set<Voice>();private musicVoices=new Set<MusicVoice>();
 private buffers=new Map<string,AudioBuffer>();private loading=new Map<string,Promise<AudioBuffer>>();
 private offsets=new Map<MusicScene,number>();private last=new Map<string,number>();private variants=new Map<string,number>();
 private epoch=0;private musicEpoch=0;private duckUntil=0;private ducked=false;private running=true;private focusPaused=false;private unlocked=false;private disposed=false;
 private peakVoices=0;private dropped=0;private played=0;private failures=new Set<string>();private lastPlayed:SoundName|null=null;
 failed=false;
 constructor(prefs:Preferences){this.prefs={...prefs};}
 async unlock(){
  if(this.disposed)return;
  try {
   if(!this.ctx){
    this.ctx=new AudioContext();this.master=this.ctx.createGain();this.master.gain.value=.78;
    this.limiter=this.ctx.createDynamicsCompressor();this.limiter.threshold.value=-7;this.limiter.knee.value=8;this.limiter.ratio.value=8;this.limiter.attack.value=.004;this.limiter.release.value=.15;
    this.musicBus=this.ctx.createGain();this.sfxBus=this.ctx.createGain();this.duck=this.ctx.createGain();
    this.musicBus.connect(this.duck);this.duck.connect(this.master);this.sfxBus.connect(this.master);this.master.connect(this.limiter);this.limiter.connect(this.ctx.destination);
    this.apply(this.prefs);
   }
   await this.ctx.resume();this.unlocked=true;this.focusPaused=false;
   for(const name of ['select','navigate','cancel','invalid','shot_seed','hit_soft'] as SoundName[])void this.load(EFFECTS[name].urls[0]).catch(()=>{});
   void this.ensureMusic();
  }catch{this.failed=true;}
 }
 private async load(url:string):Promise<AudioBuffer>{
  const ready=this.buffers.get(url);if(ready)return ready;
  const pending=this.loading.get(url);if(pending)return pending;
  const request=(async()=>{const response=await fetch(url);if(!response.ok)throw Error(`Audio ${response.status}`);const buffer=await this.ctx!.decodeAudioData(await response.arrayBuffer());if(this.disposed)throw Error('Audio disposed');this.buffers.set(url,buffer);this.failures.delete(url);this.failed=this.failures.size>0;
   const musicUrls=Object.values(MUSIC).map(m=>m.url);const cached=[...this.buffers.keys()].filter(k=>musicUrls.includes(k));for(const old of cached.slice(0,-2))if(old!==url)this.buffers.delete(old);
   return buffer;
  })().catch(error=>{if(!this.disposed){this.failed=true;this.failures.add(url);}throw error;}).finally(()=>this.loading.delete(url));
  this.loading.set(url,request);return request;
 }
 apply(prefs:Preferences){
  this.prefs={...prefs};const t=this.ctx?.currentTime??0;
  this.sfxBus?.gain.setTargetAtTime(prefs.muted?0:prefs.sfxVolume,t,.025);this.musicBus?.gain.setTargetAtTime(prefs.muted?0:prefs.musicVolume,t,.04);
  if(prefs.muted){this.epoch++;this.stopEffects();this.stopMusic(true);}
  else{if(prefs.sfxVolume===0){this.epoch++;this.stopEffects();}if(prefs.musicVolume===0)this.stopMusic(true);else void this.ensureMusic();}
 }
 setScene(scene:MusicScene){if(this.scene===scene)return;this.scene=scene;if(!this.preview){this.musicEpoch++;void this.ensureMusic();}}
 setRunning(running:boolean){if(this.running===running)return;this.running=running;if(!running){this.epoch++;this.musicEpoch++;this.stopEffects();if(!this.preview)this.stopMusic(true);}else void this.ensureMusic();}
 suspendForBlur(){this.focusPaused=true;this.preview=undefined;this.epoch++;this.musicEpoch++;this.stopEffects();this.stopMusic(true);}
 previewMusic(scene:MusicScene){this.preview=scene;this.offsets.delete(scene);this.musicEpoch++;void this.ensureMusic();}
 stopPreview(){if(!this.preview)return;this.preview=undefined;this.musicEpoch++;this.stopMusic(false);void this.ensureMusic();}
 private async ensureMusic(){
  const key=this.preview??this.scene,token=this.musicEpoch;
  if(!this.ctx||!this.unlocked||this.focusPaused||this.disposed||(!this.running&&!this.preview)||this.prefs.muted||!this.prefs.musicVolume)return;
  if([...this.musicVoices].some(v=>v.key===key&&v.source.loop))return;
  try{
   const buffer=await this.load(MUSIC[key].url);
   if(token!==this.musicEpoch||key!==(this.preview??this.scene)||this.focusPaused||this.disposed||(!this.running&&!this.preview)||this.prefs.muted||!this.prefs.musicVolume)return;
   if([...this.musicVoices].some(v=>v.key===key&&v.source.loop))return;
   const ctx=this.ctx,t=ctx.currentTime;
   // Retain only one outgoing track during rapid navigation.
   while(this.musicVoices.size>1)this.removeMusic(this.musicVoices.values().next().value!,true);
   for(const old of this.musicVoices){this.offsets.set(old.key,this.position(old));old.source.loop=false;old.gain.gain.cancelScheduledValues(t);old.gain.gain.setTargetAtTime(0,t,.18);old.source.stop(t+.75);}
   const source=ctx.createBufferSource(),gain=ctx.createGain();source.buffer=buffer;source.loop=true;source.loopStart=0;source.loopEnd=Math.min(buffer.duration,MUSIC[key].duration);
   const offset=(this.offsets.get(key)??0)%source.loopEnd;source.connect(gain);gain.connect(this.musicBus!);gain.gain.setValueAtTime(0,t);gain.gain.setTargetAtTime(1,t,.28);
   const voice={source,gain,key,started:t,offset};this.musicVoices.add(voice);source.onended=()=>{this.musicVoices.delete(voice);source.disconnect();gain.disconnect();};source.start(0,offset);
  }catch{/* Local audio failure never blocks a game action. A later user gesture can retry. */}
 }
 private position(v:MusicVoice){return (v.offset+(this.ctx!.currentTime-v.started))%MUSIC[v.key].duration;}
 private removeMusic(v:MusicVoice,preserve:boolean){if(preserve&&v.source.loop)this.offsets.set(v.key,this.position(v));this.musicVoices.delete(v);try{v.source.stop();v.source.disconnect();v.gain.disconnect();}catch{}}
 private stopMusic(preserve:boolean){this.musicEpoch++;for(const v of [...this.musicVoices])this.removeMusic(v,preserve);}
 play(name:SoundName,options:{pan?:number;volume?:number}={}){
  const def=EFFECTS[name];if(!def||!this.ctx||!this.unlocked||this.focusPaused||this.disposed||this.prefs.muted||!this.prefs.sfxVolume)return false;
  const now=this.ctx.currentTime,group=`group:${def.group}`,groupInterval=def.priority<3?({attack:.085,impact:.07,movement:.16,ui:.035,heal:.22} as Record<string,number>)[def.group]??.045:0;
  if(now-(this.last.get(name)??-Infinity)<def.interval||now-(this.last.get(group)??-Infinity)<groupInterval){this.dropped++;return false;}
  this.last.set(name,now);this.last.set(group,now);const variant=this.variants.get(name)??0;this.variants.set(name,variant+1);const url=def.urls[variant%def.urls.length],epoch=this.epoch;
  const start=(buffer:AudioBuffer)=>{
   if(epoch!==this.epoch||this.focusPaused||this.disposed||this.prefs.muted||!this.prefs.sfxVolume||this.ctx!.currentTime-now>(def.priority<3?.3:1.5))return;
   if(this.voices.size>=12){const victim=[...this.voices].sort((a,b)=>a.priority-b.priority)[0];if(victim.priority>=def.priority){this.dropped++;return;}this.removeEffect(victim);}
   const source=this.ctx!.createBufferSource(),gain=this.ctx!.createGain(),pan=this.ctx!.createStereoPanner();source.buffer=buffer;
   gain.gain.value=options.volume??1;pan.pan.value=Math.max(-.65,Math.min(.65,options.pan??0));source.connect(gain);gain.connect(pan);pan.connect(this.sfxBus!);
   const voice={source,gain,pan,priority:def.priority};this.voices.add(voice);this.peakVoices=Math.max(this.peakVoices,this.voices.size);this.played++;this.lastPlayed=name;
   source.onended=()=>this.removeEffect(voice,false);source.start();if(def.priority>=5){this.duckUntil=this.ctx!.currentTime+Math.min(def.duration,2);if(!this.ducked){this.ducked=true;this.duck?.gain.setTargetAtTime(.48,this.ctx!.currentTime,.035);}}
  };
  const cached=this.buffers.get(url);if(cached)start(cached);else void this.load(url).then(start).catch(()=>{});
  return true;
 }
 private removeEffect(v:Voice,stop=true){this.voices.delete(v);try{if(stop)v.source.stop();v.source.disconnect();v.gain.disconnect();v.pan.disconnect();}catch{}}
 private stopEffects(){for(const v of [...this.voices])this.removeEffect(v);this.duckUntil=0;this.ducked=false;this.duck?.gain.setTargetAtTime(1,this.ctx?.currentTime??0,.03);}
 update(){if(this.ctx&&this.ducked&&this.ctx.currentTime>this.duckUntil){this.ducked=false;this.duck?.gain.setTargetAtTime(1,this.ctx.currentTime,.25);}}
 reset(){this.epoch++;this.musicEpoch++;this.preview=undefined;this.stopEffects();this.stopMusic(false);this.offsets.clear();this.last.clear();this.running=false;}
 debug(){const current=[...this.musicVoices].find(v=>v.source.loop);return {activeSfx:this.voices.size,activeMusic:this.musicVoices.size,peakSfx:this.peakVoices,ducked:this.ducked,played:this.played,dropped:this.dropped,lastPlayed:this.lastPlayed,scene:this.scene,preview:this.preview??null,position:current?this.position(current):this.offsets.get(this.scene)??0,running:this.running,unlocked:this.unlocked,focusPaused:this.focusPaused,failed:this.failed,failedAssets:[...this.failures],loading:this.loading.size,cached:this.buffers.size,contextState:this.ctx?.state??'uninitialized'};}
 dispose(){this.disposed=true;this.reset();this.buffers.clear();void this.ctx?.close().catch(()=>{});}
}
