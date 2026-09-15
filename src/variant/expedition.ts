import {BY_ID,SHOP_ODDS,CAPACITY_XP,REGIONS,regionOf,isBoss,enemyRoster} from './content';
import {Events,Random} from './random';
export interface Individual {id:number;species:string;star:number;row:number;col:number;paid:number;}
export interface ExpeditionState {id:string;seed:number;regionStart:number;round:number;phase:'prepare'|'battle'|'result'|'ended';bio:number;integrity:number;capacity:number;xp:number;pool:string[];units:Individual[];shop:Array<string|null>;locked:boolean;shopRng:number;rewardRng:number;freeRefresh:boolean;nextId:number;streak:number;won:boolean|null;income:{base:number;interest:number;streak:number;organisms:number};lastResult:null|{won:boolean;survivors:number;damage:number;rewards:Record<string,number>};claimed:number[];elapsed:number;}
export class Expedition extends Events {
 state:ExpeditionState;
 constructor(state:ExpeditionState){super();this.state=state;}
 static create(pool:string[],seed:number,region=0){
  if(new Set(pool).size!==pool.length||pool.length<5||pool.length>12||pool.some(id=>!BY_ID[id]))throw Error('出战池需要 5–12 个有效且不重复的物种');
  for(let c=1;c<=5;c++)if(!pool.some(id=>BY_ID[id].cost===c))throw Error(`缺少 ${c} BIO 费用档物种`);
  const s:ExpeditionState={id:`${Date.now().toString(36)}-${seed.toString(16)}`,seed,regionStart:region,round:1,phase:'prepare',bio:10,integrity:100,capacity:3,xp:0,pool:[...pool],units:[],shop:[],locked:false,shopRng:(seed^0x9e3779b9)>>>0,rewardRng:(seed^0x85ebca6b)>>>0,freeRefresh:true,nextId:1,streak:0,won:null,income:{base:0,interest:0,streak:0,organisms:0},lastResult:null,claimed:[],elapsed:0};
  const x=new Expedition(s);x.roll();return x;
 }
 private prepare(){if(this.state.phase!=='prepare')throw Error('仅准备阶段可以操作');}
 roll(){const s=this.state,rng=new Random(s.shopRng);s.shop=Array.from({length:5},()=>{const cost=rng.weighted(SHOP_ODDS[s.capacity].map((weight,i)=>({value:i+1,weight})));return rng.pick(s.pool.filter(id=>BY_ID[id].cost===cost));});s.shopRng=rng.state;}
 refresh(){this.prepare();const s=this.state;if(!s.freeRefresh&&s.bio<2)throw Error('刷新需要 2 BIO');if(s.freeRefresh)s.freeRefresh=false;else s.bio-=2;this.roll();}
 lock(){this.prepare();this.state.locked=!this.state.locked;}
 buy(index:number){this.prepare();const s=this.state,id=s.shop[index];if(!id)throw Error('该样本已售出');const d=BY_ID[id];if(s.bio<d.cost)throw Error(`缺少 ${d.cost-s.bio} BIO`);
  const merge=s.units.filter(u=>u.species===id&&u.star===1).length>=2;
  if(s.units.filter(u=>u.row<0).length>=8&&!merge)throw Error('备战席已满，请出售或部署生物');
  s.bio-=d.cost;s.shop[index]=null;s.units.push({id:s.nextId++,species:id,star:1,row:-1,col:0,paid:d.cost});this.merge();this.emit({type:'buy',detail:id});
 }
 private merge(){const s=this.state;for(let star=1;star<3;star++)for(const species of s.pool){let group=s.units.filter(u=>u.species===species&&u.star===star);while(group.length>=3){group.sort((a,b)=>(b.row>=0?1:0)-(a.row>=0?1:0)||a.id-b.id);const [keep,...consume]=group.slice(0,3);keep.star++;keep.paid+=consume.reduce((n,u)=>n+u.paid,0);s.units=s.units.filter(u=>!consume.includes(u));this.emit({type:'merge',source:keep.id,value:keep.star});group=s.units.filter(u=>u.species===species&&u.star===star);}}}
 sell(id:number){this.prepare();const s=this.state,u=s.units.find(u=>u.id===id);if(!u)throw Error('生物已不在阵容中');s.bio+=u.paid;s.units=s.units.filter(x=>x!==u);this.emit({type:'sell',source:id,value:u.paid});}
 deploy(id:number,row:number,col:number){this.prepare();const s=this.state,u=s.units.find(u=>u.id===id);if(!u)throw Error('请选择生物');if(row===-1){if(u.row<0)return;if(s.units.filter(u=>u.row<0).length>=8)throw Error('备战席已满');u.row=-1;return;}
  if(!Number.isInteger(row)||!Number.isInteger(col)||row<3||row>5||col<0||col>7)throw Error('只能部署在己方三行');const other=s.units.find(x=>x.row===row&&x.col===col);
  if(u.row<0&&!other&&s.units.filter(x=>x.row>=0).length>=s.capacity)throw Error('生态容量已满');
  if(other){other.row=u.row;other.col=u.col;}u.row=row;u.col=col;this.emit({type:'move',source:id});
 }
 gainXP(amount:number){const s=this.state;s.xp+=amount;while(s.capacity<8&&s.xp>=CAPACITY_XP[s.capacity]){s.xp-=CAPACITY_XP[s.capacity];s.capacity++;}if(s.capacity===8)s.xp=0;}
 upgrade(){this.prepare();if(this.state.capacity===8)throw Error('容量已达上限');if(this.state.bio<4)throw Error('研究容量需要 4 BIO');this.state.bio-=4;this.gainXP(4);}
 start(){this.prepare();if(!this.state.units.some(u=>u.row>=0))throw Error('请先部署至少一个生物');this.state.phase='battle';return this.state.units.filter(u=>u.row>=0);}
 finish(won:boolean,survivors:number,organismIncome:number,time:number){const s=this.state;if(s.phase!=='battle'||s.claimed.includes(s.round))return false;s.elapsed+=time;s.claimed.push(s.round);const region=regionOf(s.round),damage=won?0:5+(region+1)*2+survivors;s.integrity=Math.max(0,s.integrity-damage);s.streak=won?Math.max(0,s.streak)+1:Math.min(0,s.streak)-1;
  const streak=Math.min(3,Math.floor(Math.abs(s.streak)/2));s.income={base:5,interest:Math.min(5,Math.floor(s.bio/10)),streak,organisms:Math.min(5,Math.max(0,Math.floor(organismIncome)))};s.bio+=Object.values(s.income).reduce((a,b)=>a+b,0);this.gainXP(2);
  const rng=new Random(s.rewardRng),type=REGIONS[(region+s.regionStart)%3].reagent;const reward:Record<string,number>={[`${type}:1`]:2};const randomType=rng.pick(['cryo','attack','regen','toxic','unstable']);reward[`${randomType}:${Math.min(4,region+1)}`]=(reward[`${randomType}:${Math.min(4,region+1)}`]??0)+2;
  if(s.round%3===0)reward[`${type}:${Math.min(4,region+2)}`]=2;if(isBoss(s.round)){reward[`${type}:4`]=2;reward.zero=1;}
  s.rewardRng=rng.state;s.lastResult={won,survivors,damage,rewards:reward};s.phase='result';if(s.integrity===0||s.round===20){s.phase='ended';s.won=won&&s.integrity>0&&s.round===20;}this.emit({type:'round',value:s.round});return true;
 }
 next(){const s=this.state;if(s.phase!=='result')throw Error('尚未完成本回合');s.round++;s.phase='prepare';s.lastResult=null;s.freeRefresh=true;if(!s.locked)this.roll();}
 enemies(){return enemyRoster(this.state.round);}
}
