import {BY_ID,TRAITS,type Trait,type Ability} from './content';
import {Events,Random} from './random';
import type {Individual} from './expedition';
export interface Combatant {id:number;species:string;team:0|1;star:number;row:number;col:number;hp:number;maxHP:number;damage:number;interval:number;range:number;cooldown:number;moveCD:number;attacks:number;hits:number;shield:number;slowUntil:number;slowFactor:number;controlReady:number;baseMaxHP:number;stunUntil:number;rootUntil:number;poisonUntil:number;poisonDPS:number;ability:Ability;charged:boolean;leapt:boolean;revived:boolean;previousHP:number;historyClock:number;specialCD:number;}
export interface Projectile {id:number;source:number;target:number;life:number;}
export function traitCounts(units:Array<{species:string}>){const counts:Partial<Record<Trait,number>>={};for(const id of new Set(units.map(u=>u.species)))for(const t of BY_ID[id].traits)counts[t]=(counts[t]??0)+1;return counts;}
export class AutoBattle extends Events {
 units:Combatant[]=[];projectiles:Projectile[]=[];time=0;status:'running'|'paused'|'won'|'lost'='running';rng:Random;private projectileId=1;private acc=0;private revivals=[false,false];private traitSets: Array<Set<Trait>>=[];private economy=new Map<string,number>();
 constructor(player:Individual[],enemies:Array<{species:string;star:number;row:number;col:number;boss?:boolean}>,seed:number,round=1){super();this.rng=new Random(seed);let n=1000;
  this.units=[...player.map(u=>this.make(u,0,u.id,1)),...enemies.map(u=>this.make(u,1,n++,Math.min(.7,.35+round*.014)*(u.boss?1.4:1)))];
  for(const team of [0,1] as const){const counts=traitCounts(this.units.filter(u=>u.team===team));if(this.units.some(u=>u.team===team&&u.ability==='zero')){const winner=(Object.entries(counts) as Array<[Trait,number]>).sort((a,b)=>b[1]-a[1]||a[0].localeCompare(b[0]))[0];if(winner)counts[winner[0]]=(counts[winner[0]]??0)+1;}
   const traits=new Set((Object.keys(counts) as Trait[]).filter(t=>counts[t]!>=TRAITS[t].threshold));this.traitSets[team]=traits;
   for(const u of this.units.filter(u=>u.team===team)){if(traits.has('legume')&&BY_ID[u.species].family==='legume')u.damage*=1.15;if(traits.has('arthropod')&&BY_ID[u.species].family==='arthropod')u.interval/=1.15;if(traits.has('bloom'))u.shield+=u.maxHP*.08;if(u.ability==='vault')u.shield+=u.maxHP*.25;if(u.ability==='generator')for(const a of this.allies(u).filter(x=>this.distance(x,u)<=1))a.shield+=a.maxHP*.1;}
  }
 }
 private make(u:{species:string;star:number;row:number;col:number},team:0|1,id:number,scale:number):Combatant{const d=BY_ID[u.species],starScale=Math.pow(1.8,u.star-1);return {id,...u,team,hp:d.hp*starScale*scale,maxHP:d.hp*starScale*scale,damage:d.damage*starScale*scale,interval:d.interval,range:d.range,cooldown:.2+this.rng.next()*.4,moveCD:0,attacks:0,hits:0,shield:0,slowUntil:0,slowFactor:1,controlReady:0,baseMaxHP:d.hp*starScale*scale,stunUntil:0,rootUntil:0,poisonUntil:0,poisonDPS:0,ability:d.ability,charged:false,leapt:false,revived:false,previousHP:d.hp*starScale*scale,historyClock:0,specialCD:4};}
 distance(a:{row:number;col:number},b:{row:number;col:number}){return Math.abs(a.row-b.row)+Math.abs(a.col-b.col);}
 private allies(u:Combatant){return this.units.filter(x=>x.team===u.team&&x.hp>0);}
 private foes(u:Combatant){return this.units.filter(x=>x.team!==u.team&&x.hp>0);}
 private heal(u:Combatant,amount:number){if(u.hp<=0)return;const n=Math.min(u.maxHP-u.hp,amount*(this.traitSets[u.team]?.has('symbiotic')?1.2:1));u.hp+=n;if(n>0)this.emit({type:'heal',target:u.id,value:n});}
 private hurt(u:Combatant,n:number,source?:Combatant,reflect=false){if(u.hp<=0)return;const reduction=Math.min(.6,(u.ability==='armor'?.25:u.ability==='bunker'?.2:u.ability==='burrow'?.15:0)+(this.traitSets[u.team]?.has('root')?.1:0));n*=1-reduction;const shield=Math.min(u.shield,n);u.shield-=shield;n-=shield;u.hp=Math.max(0,u.hp-n);u.hits++;this.emit({type:'hit',source:source?.id,target:u.id,value:n});
  if(!reflect&&source&&source.hp>0&&this.distance(u,source)<=1&&u.ability==='thorns')this.hurt(source,n*.35,u,true);
  if(!reflect&&source&&source.hp>0&&u.ability==='sentinel'&&u.hits%4===0)this.hurt(source,u.damage,u,true);
  if(u.hp<=0){const carrier=this.units.find(x=>x.team===u.team&&x.hp>0&&x.ability==='zero');if(carrier&&!this.revivals[u.team]){this.revivals[u.team]=true;u.hp=u.maxHP*.25;u.revived=true;this.emit({type:'skill',target:u.id,detail:'零序列复生'});return;}this.emit({type:'death',source:u.id});if(u.ability==='deathburst')for(const enemy of this.foes(u).filter(x=>this.distance(x,u)<=2))this.hurt(enemy,u.damage*1.8,u,true);}
 }
 private poison(target:Combatant,u:Combatant){target.poisonUntil=this.time+4;target.poisonDPS=Math.max(target.poisonDPS,u.damage*.18*(this.traitSets[u.team].has('toxic')?1.3:1));}
 private attack(u:Combatant,target:Combatant){u.attacks++;let damage=u.damage;if(u.ability==='charge'&&!u.charged){damage*=2;u.charged=true;}if(u.ability==='capacitor'&&u.attacks%4===0)damage*=3;if((u.ability==='execute'||this.traitSets[u.team].has('predator'))&&target.hp/target.maxHP<.3)damage*=u.ability==='execute'?1.6:1.2;if(u.ability==='echo'&&u.attacks%3===0)damage+=target.damage;
  if(u.range>1){this.projectiles.push({id:this.projectileId++,source:u.id,target:target.id,life:.2});this.emit({type:'shot',source:u.id,target:target.id});}else this.emit({type:'skill',source:u.id,target:target.id,detail:'近战'});
  this.hurt(target,damage,u);
  const foes=this.foes(u).filter(x=>x!==target).sort((a,b)=>this.distance(a,target)-this.distance(b,target)||a.id-b.id);
  if(u.ability==='double'&&target.hp>0)this.hurt(target,damage*.45,u);
  if(['splash','sporepea'].includes(u.ability))for(const x of foes.filter(x=>this.distance(x,target)<=1))this.hurt(x,damage*(u.ability==='splash'?.4:.3),u);
  if(['split','volley'].includes(u.ability)&&u.attacks%3===0)for(const x of foes.slice(0,2))this.hurt(x,damage*.4,u);
  if(u.ability==='pierce'){const x=foes.find(x=>x.col===target.col||x.row===target.row);if(x)this.hurt(x,damage*.5,u);}
  if(['poison','spore','sporepea','mycomoth'].includes(u.ability)||(BY_ID[u.species].kingdom==='fungi'&&this.traitSets[u.team].has('fungi'))){this.poison(target,u);if(u.ability==='spore'&&foes[0]&&this.distance(foes[0],target)<=1)this.poison(foes[0],u);}
  if(u.ability==='frost'||this.traitSets[u.team].has('cryo')){target.slowFactor=Math.max(target.slowUntil>this.time?target.slowFactor:1,u.ability==='frost'?1/.7:1/.85);target.slowUntil=this.time+2;}
  if(u.ability==='root'&&u.attacks%4===0&&target.controlReady<=this.time){target.rootUntil=this.time+1.5;target.controlReady=this.time+3;}
  if(u.ability==='stun'&&u.attacks%4===0&&target.controlReady<=this.time){target.stunUntil=this.time+1;target.controlReady=this.time+3;}
  if(['drain','parasite'].includes(u.ability)){this.heal(u,damage*.3);if(u.ability==='parasite'){const a=this.allies(u).sort((a,b)=>a.hp/a.maxHP-b.hp/b.maxHP)[0];if(a)this.heal(a,damage*.2);}}
  if(u.ability==='battery'&&u.attacks%4===0){const a=this.allies(u).sort((a,b)=>a.hp/a.maxHP-b.hp/b.maxHP)[0];if(a)this.heal(a,u.maxHP*.06);}
  if(u.ability==='solar'&&u.attacks%5===0&&u.team===0)this.economy.set(u.species,Math.min(2,(this.economy.get(u.species)??0)+1));
  if(u.ability==='decay'){const floor=target.baseMaxHP*.85;target.maxHP=Math.max(Math.min(floor,target.maxHP),target.maxHP*.99);target.hp=Math.min(target.hp,target.maxHP);}
 }
 private step(){const dt=.05;this.time+=dt;for(const p of this.projectiles)p.life-=dt;this.projectiles=this.projectiles.filter(p=>p.life>0).slice(-200);
  for(const u of this.units){if(u.hp<=0)continue;if(u.poisonUntil>this.time)this.hurt(u,u.poisonDPS*dt,undefined,true);if(u.hp<=0)continue;
   u.historyClock+=dt;if(u.historyClock>=3){u.previousHP=u.hp;u.historyClock=0;}u.specialCD-=dt;
   if(u.specialCD<=0){u.specialCD=u.ability==='reverse'?6:['heal','nurse'].includes(u.ability)?4:5;const allies=this.allies(u).sort((a,b)=>a.hp/a.maxHP-b.hp/b.maxHP||a.id-b.id);if(this.traitSets[u.team].has('growth'))this.heal(u,u.maxHP*.03);
    if(['heal','nurse','mycomoth'].includes(u.ability))for(const a of allies.slice(0,u.ability==='nurse'?2:1))this.heal(a,a.maxHP*(u.ability==='nurse'?.06:.08));
    if(u.ability==='shield'&&allies[0])allies[0].shield=Math.min(allies[0].maxHP*.3,allies[0].shield+allies[0].maxHP*.12);
    if(u.ability==='reverse'&&allies[0])this.heal(allies[0],Math.min(allies[0].maxHP*.15,Math.max(0,allies[0].previousHP-allies[0].hp)));
    if(u.ability==='swarm')for(const foe of this.foes(u).sort((a,b)=>this.distance(u,a)-this.distance(u,b)).slice(0,2))this.hurt(foe,u.damage*.65,u);
   }
   if(u.stunUntil>this.time)continue;u.cooldown-=dt;u.moveCD-=dt;
   const enemies=this.foes(u);if(!enemies.length)break;
   const target=enemies.sort((a,b)=>((a.ability==='taunt'&&this.distance(u,a)<=3)?-1:0)-((b.ability==='taunt'&&this.distance(u,b)<=3)?-1:0)||this.distance(u,a)-this.distance(u,b)||a.id-b.id)[0];
   if(BY_ID[u.species].kingdom==='fauna'&&['leap','burrow'].includes(u.ability)&&!u.leapt){u.leapt=true;const weak=enemies.sort((a,b)=>a.maxHP-b.maxHP||a.id-b.id)[0];const near=this.neighbors(weak).find(pos=>!this.occupied(pos));if(near){u.row=near.row;u.col=near.col;this.emit({type:'move',source:u.id});}}
   if(this.distance(u,target)<=u.range&&u.cooldown<=0){const aura=this.allies(u).some(a=>a.ability==='aura'&&this.distance(a,u)<=1);u.cooldown=Math.max(.25,u.interval*(u.slowUntil>this.time?u.slowFactor:1)*(aura?.85:1));this.attack(u,target);}
   else if(BY_ID[u.species].kingdom==='fauna'&&this.distance(u,target)>u.range&&u.moveCD<=0&&u.rootUntil<=this.time){const next=this.path(u,target);if(next){u.row=next.row;u.col=next.col;this.emit({type:'move',source:u.id});}u.moveCD=(u.ability==='charge'?.25:.38)/(this.traitSets[u.team].has('arthropod')?1.15:1)*(u.slowUntil>this.time?u.slowFactor:1);}
  }
  if(!this.units.some(u=>u.team===1&&u.hp>0))this.status='won';else if(!this.units.some(u=>u.team===0&&u.hp>0)||this.time>=60)this.status='lost';
 }
 private occupied(pos:{row:number;col:number}){return this.units.some(u=>u.hp>0&&u.row===pos.row&&u.col===pos.col);}
 private neighbors(pos:{row:number;col:number}){return [{row:pos.row-1,col:pos.col},{row:pos.row,col:pos.col-1},{row:pos.row,col:pos.col+1},{row:pos.row+1,col:pos.col}].filter(x=>x.row>=0&&x.row<6&&x.col>=0&&x.col<8);}
 private path(u:Combatant,target:Combatant){const queue:Array<{row:number;col:number;first:{row:number;col:number}|null}>=[{row:u.row,col:u.col,first:null}],seen=new Set([u.row*8+u.col]);while(queue.length){const at=queue.shift()!;if(this.distance(at,target)<=u.range)return at.first;for(const n of this.neighbors(at)){const key=n.row*8+n.col;if(seen.has(key)||this.occupied(n))continue;seen.add(key);queue.push({...n,first:at.first??n});}}return null;}
 update(dt:number){if(this.status!=='running')return;this.acc+=Math.min(dt,.25);while(this.acc>=.05&&this.status==='running'){this.acc-=.05;this.step();}}
 income(){let total=[...this.economy.values()].reduce((a,b)=>a+b,0);for(const species of new Set(this.units.filter(u=>u.team===0&&u.hp>0).map(u=>u.species))){const units=this.units.filter(u=>u.species===species&&u.team===0&&u.hp>0),ability=BY_ID[species].ability;if(['income','generator'].includes(ability))total++;if(ability==='harvest')total+=2;if(ability==='vault'&&this.status==='won'&&units.some(u=>u.shield>0))total+=2;}return Math.min(5,total);}
 survivors(){return this.units.filter(u=>u.team===1&&u.hp>0).length;}
}
