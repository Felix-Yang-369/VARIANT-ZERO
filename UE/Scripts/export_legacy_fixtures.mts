import { writeFileSync, mkdirSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { SPECIES, FOUNDERS, enemyRoster } from '../Reference/v0.4/src/variant/content.ts';
import { Random } from '../Reference/v0.4/src/variant/random.ts';
import { Expedition } from '../Reference/v0.4/src/variant/expedition.ts';
import { AutoBattle } from '../Reference/v0.4/src/variant/battle.ts';

const directory = fileURLToPath(new URL('../Reference/v0.4/fixtures/', import.meta.url));
mkdirSync(directory, { recursive:true });
const seeds = [1, 42, 20260914, 4294967295];
const pool = SPECIES.slice(0,12).map(s=>s.id);
const shops = seeds.map(seed => {
  const run = Expedition.create(pool, seed);
  run.state.id = `fixture-${seed}`; // Normalize wall-clock identifier only.
  const steps: unknown[] = [{ action:'create', state:structuredClone(run.state) }];
  run.buy(0); steps.push({ action:'buy(0)', state:structuredClone(run.state) });
  run.deploy(run.state.units[0].id,4,0); steps.push({ action:'deploy(first,4,0)', state:structuredClone(run.state) });
  run.refresh(); steps.push({ action:'refresh', state:structuredClone(run.state) });
  run.lock(); steps.push({ action:'lock', state:structuredClone(run.state) });
  return { seed, pool, steps };
});
const random = seeds.map(seed=>{const r=new Random(seed);return {seed,values:Array.from({length:32},()=>r.next()),finalState:r.state};});
const battles = SPECIES.map(s=>{
  const player = [{id:1,species:s.id,star:1,row:4,col:0,paid:s.cost}];
  const enemies = enemyRoster(1);
  const battle = new AutoBattle(player,enemies,42,1);
  for(let i=0;i<1200 && battle.status==='running';i++) battle.update(.05);
  return {species:s.id,seed:42,round:1,step:.05,maxSteps:1200,player,enemies,status:battle.status,time:battle.time,units:battle.units,events:battle.drainEvents()};
});
for(const [name,data] of Object.entries({species:SPECIES,random,shops,battles})) writeFileSync(directory+name+'.json',JSON.stringify(data,null,2)+'\n');
console.log(`Exported ${SPECIES.length} species, ${random.length} RNG traces, ${shops.length} shop traces, ${battles.length} battle traces. These are baselines, not proof of native parity.`);
