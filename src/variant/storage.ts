import {loadSave,type Preferences,type StoragePort} from '../settings';
import {BY_ID,FOUNDERS,OLD_MAP} from './content';
import {initialResearch,type ResearchState} from './research';
import type {ExpeditionState} from './expedition';
export const KEY='variant-zero-v4',BACKUP=KEY+'-backup';
export interface SaveV4 {version:4;research:ResearchState;settings:Preferences;lessons:string[];records:{runs:number;wins:number;bestRound:number;bestTime:number;unlockedRegion:number};legacy:unknown;active:ExpeditionState|null;}
export function fresh(seed:number,settings?:Preferences):SaveV4{return {version:4,research:initialResearch(seed),settings:settings??{musicVolume:.25,sfxVolume:.6,muted:false,reducedMotion:false,tutorials:[]},lessons:[],records:{runs:0,wins:0,bestRound:0,bestTime:0,unlockedRegion:0},legacy:null,active:null};}
const nonneg=(v:unknown)=>typeof v==='number'&&Number.isFinite(v)&&v>=0;
export function validate(raw:unknown):raw is SaveV4 {
 try{const d=raw as SaveV4;if(d.version!==4||!d.research||!Array.isArray(d.research.unlocked)||!FOUNDERS.every(x=>d.research.unlocked.includes(x))||d.research.unlocked.some(x=>!BY_ID[x])||new Set(d.research.unlocked).size!==d.research.unlocked.length)return false;
  if(!d.research.reagents||Object.entries(d.research.reagents).some(([k,n])=>!(/^(cryo|attack|regen|toxic|unstable):[1-4]$/.test(k)||k==='zero')||!nonneg(n)||!Number.isInteger(n)))return false;
  if(!nonneg(d.research.rng)||!nonneg(d.research.experiments)||!Array.isArray(d.research.history)||!Array.isArray(d.lessons)||d.lessons.some(x=>typeof x!=='string'))return false;
  if(!d.settings||!['musicVolume','sfxVolume'].every(k=>nonneg((d.settings as any)[k])&&(d.settings as any)[k]<=1)||typeof d.settings.muted!=='boolean'||typeof d.settings.reducedMotion!=='boolean')return false;
  if(!d.records||Object.values(d.records).some(v=>!nonneg(v))||d.records.unlockedRegion>2)return false;
  if(d.active){const s=d.active;if(!['prepare','battle','result','ended'].includes(s.phase)||typeof s.id!=='string'||!Array.isArray(s.units)||!Array.isArray(s.pool)||s.pool.some(x=>!d.research.unlocked.includes(x))||new Set(s.pool).size!==s.pool.length||s.pool.length<5||s.pool.length>12)return false;
   if(![s.round,s.bio,s.integrity,s.capacity,s.xp,s.shopRng,s.rewardRng,s.seed,s.nextId,s.elapsed].every(nonneg)||s.capacity<3||s.capacity>8||s.round<1||s.round>20||s.integrity>100)return false;
   if(!Array.isArray(s.shop)||s.shop.length!==5||s.shop.some(x=>x!==null&&!s.pool.includes(x))||!Array.isArray(s.claimed))return false;
   if(new Set(s.units.map(x=>x.id)).size!==s.units.length||s.units.some(x=>!s.pool.includes(x.species)||![1,2,3].includes(x.star)||!nonneg(x.paid)||!Number.isInteger(x.id)||!Number.isInteger(x.row)||!Number.isInteger(x.col)||x.row< -1||x.row>5||(x.row>=0&&x.row<3)||x.col<0||x.col>7))return false;
   const board=s.units.filter(x=>x.row>=0);if(board.length>s.capacity||s.units.length-board.length>8||new Set(board.map(x=>x.row*8+x.col)).size!==board.length)return false;
   if(s.claimed.some(x=>!Number.isInteger(x)||x<1||x>s.round)||!s.income||!Number.isFinite(s.streak))return false;
  }return true;
 }catch{return false;}
}
export class Repository {
 data:SaveV4;blocked=false;notice='';recovery:SaveV4|null=null;
 constructor(private storage:StoragePort,seed:number,reduced=false){this.data=fresh(seed);try{
  const raw=storage.getItem(KEY);if(raw){let parsed;try{parsed=JSON.parse(raw);}catch{}if(validate(parsed)){this.data=parsed;if(this.data.active?.phase==='battle')this.data.active.phase='prepare';return;}
   this.blocked=true;this.notice='本地存档损坏，原始数据未覆盖。可以导出原始数据，或恢复上一份有效备份。';const backup=storage.getItem(BACKUP);if(backup){try{const v=JSON.parse(backup);if(validate(v))this.recovery=v;}catch{}}return;
  }
  const legacy=loadSave(storage,reduced);this.data=fresh(seed,legacy.data.settings);this.data.legacy={records:legacy.data.records,unlocked:legacy.data.unlocked,discoveries:legacy.data.discoveries};for(const old of legacy.data.discoveries){const id=OLD_MAP[old.replace('plant:','')];if(id&&!this.data.research.unlocked.includes(id))this.data.research.unlocked.push(id);}this.storage.setItem(KEY,JSON.stringify(this.data));
 }catch{this.blocked=true;this.notice='本机存储不可写，暂停提交资源与出征操作。请检查浏览器存储权限后重试。';}}
 transaction(change:(draft:SaveV4)=>void){if(this.blocked)throw Error(this.notice);const next=structuredClone(this.data);change(next);if(!validate(next))throw Error('状态校验失败，操作未提交');try{const previous=this.storage.getItem(KEY);if(previous)this.storage.setItem(BACKUP,previous);this.storage.setItem(KEY,JSON.stringify(next));}catch{throw Error('保存失败，本次操作未提交；请重试。');}this.data=next;}
 recover(){if(!this.recovery)throw Error('没有可恢复的备份');const raw=this.storage.getItem(KEY);try{if(raw)this.storage.setItem(KEY+'-damaged-'+Date.now(),raw);this.storage.setItem(KEY,JSON.stringify(this.recovery));}catch{throw Error('备份恢复失败，原始数据仍保留');}this.data=structuredClone(this.recovery);if(this.data.active?.phase==='battle')this.data.active.phase='prepare';this.blocked=false;this.notice='';}
 exportRaw(){return this.storage.getItem(KEY)??JSON.stringify(this.data);}
}
