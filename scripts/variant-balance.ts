import {Expedition} from '../src/variant/expedition';import {AutoBattle} from '../src/variant/battle';import {FOUNDERS,BY_ID} from '../src/variant/content';
export function simulate(seed:number,strategy:'guard'|'attack'|'greed',pool=FOUNDERS){const e=Expedition.create(pool,seed);const reports=[];while(e.state.phase!=='ended'){
 const s=e.state;let loops=0;const priority=strategy==='guard'?['V-003','V-001','V-004','V-005','V-002']:strategy==='attack'?['V-001','V-005','V-004','V-003','V-002']:['V-002','V-001','V-005','V-004','V-003'];
 if(strategy==='greed'&&s.round>=11){const flowers=s.units.filter(u=>u.species==='V-002');for(const u of flowers.slice(1))e.sell(u.id);priority.splice(0,priority.length,'V-001','V-005','V-004','V-003','V-002');}
 const deploy=()=>{const candidates=s.units.filter(u=>u.row<0).sort((a,b)=>b.star-a.star||priority.indexOf(a.species)-priority.indexOf(b.species));for(const u of candidates){if(s.units.filter(x=>x.row>=0).length>=s.capacity)break;const cols=[3,4,2,5,1,6,0,7];const pos=cols.map(col=>({row:3,col})).find(p=>!s.units.some(x=>x.row===p.row&&x.col===p.col));if(pos)e.deploy(u.id,pos.row,pos.col);}};
 while(loops++<12){deploy();if(s.capacity<8&&s.bio>=(strategy==='greed'&&s.round<11?16:8)&&s.units.length>=s.capacity)e.upgrade();
 let bought=false;const offers=s.shop.map((id,i)=>({id,i})).filter(x=>x.id).sort((a,b)=>{const matching=(id:string)=>s.units.filter(u=>u.species===id&&u.star<3).length;return matching(b.id!)-matching(a.id!)||priority.indexOf(a.id!)-priority.indexOf(b.id!);});
 for(const {id,i}of offers){if(!id||s.bio<BY_ID[id].cost)continue;try{e.buy(i);bought=true;}catch{}}
 deploy();if(s.freeRefresh||s.bio>(strategy==='greed'&&s.round<11?24:8))e.refresh();else if(!bought)break;
 }
 deploy();if(!s.units.some(u=>u.row>=0))throw Error('bot empty');e.start();const b=new AutoBattle(s.units.filter(u=>u.row>=0),e.enemies(),s.seed^s.round,s.round);while(b.status==='running'){b.update(.25);b.drainEvents();}e.finish(b.status==='won',b.survivors(),b.income(),b.time);reports.push({round:s.round,won:b.status==='won',integrity:s.integrity,time:+b.time.toFixed(1),bio:s.bio,capacity:s.capacity,stars:s.units.filter(u=>u.row>=0).map(u=>u.star)});if(s.phase==='result')e.next();
 }return {seed,strategy,won:e.state.won,round:e.state.round,integrity:e.state.integrity,time:e.state.elapsed,reports};}
if(process.argv[1]?.includes('variant-balance')){const results=[];for(const strategy of ['guard','attack','greed'] as const)for(const seed of [42,2026,9017])results.push(simulate(seed,strategy));console.log(JSON.stringify(results,null,2));}
