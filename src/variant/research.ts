import {BY_ID,SPECIES,FOUNDERS,REAGENTS,type Reagent,type SpeciesDefinition} from './content';
import {Random} from './random';
export interface Recipe {id:string;parents:string[];kind:'mutation'|'hybrid';cost:Record<string,number>;results:Array<{species:string|null;weight:number}>;grade:number;}
export interface ResearchState {unlocked:string[];reagents:Record<string,number>;experiments:number;rng:number;history:Array<{recipe:string;result:string|null;duplicate:boolean}>;}
export const reagentKey=(type:Reagent,grade:number)=>`${type}:${grade}`;
export function initialResearch(seed:number):ResearchState{return {unlocked:[...FOUNDERS],reagents:Object.fromEntries(Object.keys(REAGENTS).map(k=>[`${k}:1`,4])),experiments:0,rng:seed,history:[]};}
export const gradeFor=(s:SpeciesDefinition)=>s.category==='zero'?5:s.category==='aberrant'?4:s.cost>=5?3:s.cost>=3?2:1;
export function recipes():Recipe[]{
 const map=new Map<string,SpeciesDefinition[]>();
 for(const s of SPECIES.filter(s=>s.parents.length)){
  const key=s.parents.join('+')+'/'+s.reagent+'/'+gradeFor(s);map.set(key,[...(map.get(key)??[]),s]);
 }
 return [...map.entries()].flatMap(([id,group])=>{
  const s=group[0], min=gradeFor(s);
  return (min===5?[5]:Array.from({length:5-min},(_,i)=>i+min)).map(grade=>{
   const cost:Record<string,number>=grade===5?{zero:1} : {[reagentKey(s.reagent,grade)]:2};
   if(s.parents.length===2&&grade!==5)cost[reagentKey('unstable',grade)]=(cost[reagentKey('unstable',grade)]??0)+1;
   const cross=s.parents.length===2&&BY_ID[s.parents[0]].kingdom!==BY_ID[s.parents[1]].kingdom;
   const success=Math.min(95,(cross?55:75)+(grade-min)*7);
   // Exact weights displayed to the player. No implicit rarity roll.
   const weight=success/group.length;
   return {id:id+':g'+grade,parents:s.parents,kind:s.parents.length===1?'mutation':'hybrid',cost,grade,results:[...group.map(x=>({species:x.id as string|null,weight})),{species:null,weight:100-success}]} as Recipe;
  });
 });
}
export const RECIPES=recipes();
export function experiment(state:ResearchState,recipeId:string):{next:ResearchState;result:string|null;duplicate:boolean}{
 const recipe=RECIPES.find(r=>r.id===recipeId);if(!recipe)throw Error('实验方案不存在');
 if(!recipe.parents.every(p=>state.unlocked.includes(p)))throw Error('尚未解锁所需母体');
 for(const [k,n] of Object.entries(recipe.cost))if((state.reagents[k]??0)<n)throw Error('试剂不足，请前往出征获取');
 const next=structuredClone(state),rng=new Random(next.rng),result=rng.weighted(recipe.results.map(r=>({value:r.species,weight:r.weight})));next.rng=rng.state;
 for(const [k,n] of Object.entries(recipe.cost))next.reagents[k]-=n;
 const duplicate=!!result&&next.unlocked.includes(result);
 if(result&&!duplicate)next.unlocked.push(result);
 if(duplicate)for(const [k,n] of Object.entries(recipe.cost))next.reagents[k]+=Math.floor(n/2);
 next.experiments++;next.history.unshift({recipe:recipe.id,result,duplicate});next.history=next.history.slice(0,30);
 return {next,result,duplicate};
}
export function compatibility(a:string,b:string){if(a===b)return '同一模板请使用突变';const x=BY_ID[a],y=BY_ID[b];if(!x||!y)return '无效模板';if(!RECIPES.some(r=>r.parents.length===2&&r.parents.includes(a)&&r.parents.includes(b)))return '尚无可执行实验条件';return x.kingdom===y.kingdom?'高兼容':x.kingdom==='fauna'||y.kingdom==='fauna'?'跨界实验 · 较低兼容':'跨界实验 · 较高兼容';}
