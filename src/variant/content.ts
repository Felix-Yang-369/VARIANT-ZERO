export type Family = 'legume'|'bloom'|'root'|'fungi'|'arthropod';
export type Reagent = 'cryo'|'attack'|'regen'|'toxic'|'unstable';
export type Trait = Family|'cryo'|'toxic'|'growth'|'symbiotic'|'predator';
export type Category = 'founder'|'variant'|'hybrid'|'chimera'|'aberrant'|'zero';
export type Ability = 'shot'|'income'|'armor'|'poison'|'charge'|'double'|'frost'|'splash'|'split'|'drain'|'heal'|'aura'|'battery'|'shield'|'thorns'|'root'|'taunt'|'spore'|'deathburst'|'parasite'|'stun'|'swarm'|'execute'|'leap'|'pierce'|'solar'|'harvest'|'capacitor'|'bunker'|'volley'|'sentinel'|'generator'|'nurse'|'vault'|'sporepea'|'burrow'|'mycomoth'|'echo'|'reverse'|'decay'|'zero';
export interface SpeciesDefinition { id:string; name:string; family:Family; kingdom:'flora'|'fungi'|'fauna'; category:Category; rarity:'common'|'elite'|'epic'|'legendary'|'zero'; cost:number; hp:number; damage:number; interval:number; range:number; ability:Ability; traits:Trait[]; parents:string[]; reagent:Reagent; art:string; frame?:number; }
export const FAMILY:Record<Family,string>={legume:'豆科',bloom:'花系',root:'根系',fungi:'菌界',arthropod:'节肢类'};
export const REAGENTS:Record<Reagent,string>={cryo:'低温',attack:'攻击',regen:'再生',toxic:'毒素',unstable:'不稳定'};
export const ABILITIES:Record<Ability,string>={shot:'远程种荚射击，射程为曼哈顿距离 8 格。',income:'每回合存活时额外获得 1 BIO，每个物种最多计一次。',armor:'木质甲壳减少受到伤害的 25%。',poison:'命中附加持续 4 秒的毒素，每秒造成攻击力 18% 伤害。',charge:'移动时加速；首次近战攻击造成双倍伤害。',double:'每次攻击追加一枚 45% 伤害的种荚。',frost:'命中降低目标移动和攻击速度 30%，持续 2 秒。',splash:'攻击对目标邻格敌人造成 40% 溅射伤害。',split:'每第三次攻击分裂到另外两个目标，各造成 40% 伤害。',drain:'造成伤害的 30% 转为自身治疗。',heal:'每 4 秒治疗最虚弱友军最大耐久的 8%。',aura:'邻格友军攻击间隔缩短 15%，同源不叠加。',battery:'每 4 次攻击为最虚弱友军回复自身最大耐久的 6%。',shield:'每 5 秒为最虚弱友军提供其最大耐久 12% 的护盾。',thorns:'受近战伤害时反弹本次伤害的 35%。',root:'每 4 次攻击禁锢目标 1.5 秒。',taunt:'吸引距离 3 格内敌人优先攻击自己。',spore:'毒素命中同时感染一个邻格敌人。',deathburst:'死亡时对两格内敌人造成攻击力 180% 伤害。',parasite:'攻击伤害的 30% 治疗自身、20% 治疗最虚弱友军。',stun:'每 4 次攻击使目标眩晕 1 秒。',swarm:'每 5 秒对两名近处敌人各造成 65% 伤害。',execute:'目标耐久低于 30% 时攻击伤害提高 60%。',leap:'首次行动跃迁到敌方最脆弱单位附近的空格。',pierce:'攻击对直线上的第二名敌人造成 50% 伤害。',solar:'每累计 5 次攻击，本回合收益 +1 BIO，上限 2。',harvest:'存活时 +2 BIO，但射击间隔较长；同物种只计一次。',capacitor:'每第四次攻击释放三倍伤害的蓄能射击。',bunker:'减伤 20%，具备中距离炮台火力。',volley:'每第三次攻击额外攻击另外两名敌人，各造成 40% 伤害。',sentinel:'每承受 4 次伤害，反击本次伤害来源 100% 攻击伤害。',generator:'每回合存活 +1 BIO，开战为邻格友军提供 10% 护盾。',nurse:'每 4 秒治疗两名最虚弱友军各 6% 最大耐久。',vault:'开战提供自身 25% 护盾；护盾未破时胜利额外 +2 BIO。',sporepea:'种荚命中产生毒素并对邻格造成 30% 传播伤害。',burrow:'移动近战根兽，首次行动潜入后排，受伤减免 15%。',mycomoth:'移动菌蛾，攻击施毒，每 5 秒治疗最虚弱友军最大耐久的 8%。',echo:'每第三次攻击复制当前目标一次普通攻击的伤害。',reverse:'每 6 秒将最低耐久友军恢复到 3 秒前的耐久，上限 15%。',decay:'攻击永久削弱本场目标最大耐久的 1%，最多 15%。',zero:'开战复制人数最多的一个己方羁绊标签；每场首次友军死亡时以 25% 耐久复生一次。'};
const rows: Array<[string,Family,number,Ability,string[],Reagent,Category?,string?]> = [
 ['原豆母体','legume',1,'shot',[],'attack','founder','pea'],['原花母体','bloom',2,'income',[],'regen','founder','sun'],['原根母体','root',3,'armor',[],'regen','founder','wall'],['原孢体','fungi',4,'poison',[],'toxic','founder'],['原节肢体','arthropod',5,'charge',[],'attack','founder'],
 ['双荚体','legume',2,'double',['V-001'],'attack'],['冰豆','legume',2,'frost',['V-001'],'cryo'],['爆裂豆','legume',3,'splash',['V-001'],'attack'],['裂殖豆','legume',4,'split',['V-006'],'unstable'],['寄生豆','legume',3,'drain',['V-007'],'toxic'],
 ['晨露花','bloom',2,'heal',['V-002'],'regen'],['共鸣花','bloom',3,'aura',['V-002'],'unstable'],['蓄电花','bloom',3,'battery',['V-011'],'attack'],['寒幕花','bloom',4,'shield',['V-012'],'cryo'],['丰盈花','bloom',5,'harvest',['V-013'],'regen'],
 ['荆棘根','root',2,'thorns',['V-003'],'attack'],['地缚根','root',3,'root',['V-003'],'toxic'],['诱饵树','root',4,'taunt',['V-016'],'unstable'],['永生根','root',5,'heal',['V-017'],'regen'],
 ['毒雾菌','fungi',1,'spore',['V-004'],'toxic'],['爆孢菌','fungi',2,'deathburst',['V-004'],'attack'],['寄生菌','fungi',3,'parasite',['V-020'],'toxic'],['共生菌','fungi',3,'aura',['V-020'],'regen'],['神经菌','fungi',4,'stun',['V-022'],'cryo'],
 ['蜂群体','arthropod',2,'swarm',['V-005'],'unstable'],['猎杀螳','arthropod',3,'execute',['V-005'],'attack'],['跃迁蛛','arthropod',4,'leap',['V-026'],'cryo'],['甲壳蝎','arthropod',3,'thorns',['V-025'],'toxic'],['钻刺蜈蚣','arthropod',5,'pierce',['V-027'],'attack'],
 ['光能射手','legume',2,'solar',['V-001','V-002'],'attack','hybrid','solar'],['丰收射手','bloom',3,'harvest',['V-001','V-002'],'regen','hybrid','harvest'],['辉光蓄能者','legume',5,'capacitor',['V-001','V-002'],'unstable','hybrid','capacitor'],
 ['种荚堡垒','root',2,'bunker',['V-001','V-003'],'attack','hybrid','bunker'],['连射堡垒','legume',4,'volley',['V-001','V-003'],'attack','hybrid','volley'],['反击守卫','root',4,'sentinel',['V-001','V-003'],'toxic','hybrid','sentinel'],
 ['供能壁垒','root',2,'generator',['V-002','V-003'],'regen','hybrid','generator'],['修复花盾','bloom',3,'nurse',['V-002','V-003'],'regen','hybrid','nurse'],['储能圣果','root',5,'vault',['V-002','V-003'],'unstable','hybrid','vault'],
 ['孢子豆','fungi',3,'sporepea',['V-001','V-004'],'toxic','hybrid'],['掘地根兽','arthropod',4,'burrow',['V-003','V-005'],'attack','chimera'],['菌翼蛾','arthropod',4,'mycomoth',['V-004','V-005'],'regen','chimera'],
 ['回声种','legume',4,'echo',['V-009'],'unstable','aberrant'],['逆时孢','fungi',5,'reverse',['V-024'],'unstable','aberrant'],['凋序蝎','arthropod',5,'decay',['V-029'],'unstable','aberrant'],['零序列载体','fungi',5,'zero',['V-042','V-043'],'unstable','zero'],
];
const JUVENILES=[5,6,7,10,11,15,16,19,20,24,25];
let frame = 0;
export const SPECIES:SpeciesDefinition[]=rows.map(([name,family,cost,ability,parents,reagent,category='variant',art],i)=>{
 const f=art===undefined?frame++:undefined;
 const isTank=family==='root';
 return {id:`V-${String(i+1).padStart(3,'0')}`,name,family,kingdom:family==='fungi'?'fungi':family==='arthropod'?'fauna':'flora',cost,ability,parents,reagent,category,
 rarity:category==='zero'?'zero':category==='aberrant'?'legendary':cost>=4?'epic':cost>=2?'elite':'common',
 hp:Math.round((130+cost*36)*(isTank?2.1:family==='arthropod'?1.35:1)), damage:Math.round((19+cost*7)*(isTank?.72:1)),interval:family==='bloom'?1.8:family==='arthropod'?1.1:1.4,range:family==='arthropod'?1:family==='root'?4:8,
 traits:[family,...(parents.length===2?[rows[Number(parents[0].slice(2))-1][1],rows[Number(parents[1].slice(2))-1][1]].filter(x=>x!==family):[]),...([reagent==='cryo'?'cryo':reagent==='toxic'?'toxic':reagent==='regen'?'symbiotic':reagent==='unstable'?'growth':'predator'] as Trait[])].filter((x,j,a)=>a.indexOf(x)===j),
 art:category==='founder'?'vz-founders-divine-v2':JUVENILES.includes(i)?'vz-juveniles':art??`vz-sheet-${Math.floor(f!/12)+1}`,frame:category==='founder'?i:JUVENILES.includes(i)?JUVENILES.indexOf(i):f===undefined?undefined:f%12};
});
export const BY_ID=Object.fromEntries(SPECIES.map(s=>[s.id,s])) as Record<string,SpeciesDefinition>;
export const FOUNDERS=SPECIES.filter(s=>s.category==='founder').map(s=>s.id);
export const OLD_MAP=Object.fromEntries(['solar','harvest','capacitor','bunker','volley','sentinel','generator','nurse','vault'].map((k,i)=>[k,SPECIES[29+i].id]));
export const CAPACITY_XP:Record<number,number>={3:6,4:12,5:20,6:32,7:44,8:Infinity};
export const SHOP_ODDS:Record<number,number[]>={3:[65,25,8,1,1],4:[50,30,15,4,1],5:[35,35,22,6,2],6:[25,30,30,12,3],7:[18,25,30,20,7],8:[12,18,30,25,15]};
export const TRAITS:Record<Trait,{name:string;threshold:number;desc:string}>={legume:{name:'豆科',threshold:2,desc:'豆科攻击伤害 +15%。'},bloom:{name:'花系',threshold:2,desc:'全部友军获得 8% 开场护盾。'},root:{name:'根系',threshold:2,desc:'全部友军减伤 +10%。'},fungi:{name:'菌界',threshold:2,desc:'菌类攻击附加 4 秒毒素。'},arthropod:{name:'节肢',threshold:2,desc:'节肢单位移动、攻速 +15%。'},cryo:{name:'低温',threshold:2,desc:'所有友军攻击附加 15% 减速。'},toxic:{name:'毒素',threshold:2,desc:'毒素伤害 +30%。'},growth:{name:'高速生长',threshold:2,desc:'每 5 秒友军回复 3% 耐久。'},symbiotic:{name:'共生',threshold:2,desc:'治疗量 +20%。'},predator:{name:'捕食者',threshold:2,desc:'攻击低于 30% 耐久目标时伤害 +20%。'}};
export const REGIONS=[{name:'翠境外庭',reagent:'regen' as Reagent,story:'翠境计划的试验田仍与城市相连。记录异常根系，找回 V-000 最初的环境适应报告。'}, {name:'孢雾隔离区',reagent:'toxic' as Reagent,story:'零序列出现在毫无亲缘关系的生命中。研究员开始怀疑，传播的并不只是遗传物质。'}, {name:'零序列核心',reagent:'unstable' as Reagent,story:'V-000 并非一个物种。它是一套能在生命间传播的自主进化算法——算法正在成为生命的规则。'}];
export const regionOf=(round:number)=>round<=6?0:round<=13?1:2;
export const isBoss=(round:number)=>[6,13,20].includes(round);
export function enemyRoster(round:number):Array<{species:string;star:number;row:number;col:number;boss:boolean}> {
 const region=regionOf(round), count=Math.min(8,2+Math.floor(round/3));
 const sets=[['V-005','V-001','V-020','V-003'],['V-025','V-026','V-022','V-007','V-017'],['V-027','V-029','V-040','V-043','V-034','V-024']];
 return Array.from({length:count},(_,i)=>({species:sets[region][(i+round)%sets[region].length],star:round>=16?2:round>=9&&i<2?2:1,row:i<4?2:1,col:(i%4)*2,boss:isBoss(round)&&i===0}));
}
export const PROTOCOLS=[{title:'01 / 翠境计划',text:'为了应对粮食危机、污染与极端气候，人类开始培育能够主动适应环境的人工生命。城市仍在运转，变种区却以异常的速度生长。'}, {title:'02 / 绽变事件',text:'自适应基因引擎的首个完整实验 V-000 产生了未经设计的零序列。植物、菌类和节肢生物开始跨越原有的进化边界。'}, {title:'03 / 培育员职责',text:'你属于自适应生态署。发现、稳定并应用新的生命体系，在变种区取得试剂，回到实验室扩张物种谱系。'}];

export function growthStage(s:SpeciesDefinition){return s.category==='founder'?'01 · 原初母体':s.category==='zero'?'05 · 零序列':s.category==='aberrant'?'04 · 异常觉醒':s.parents.length>1?'03 · 融合成体':s.parents.every(p=>BY_ID[p].category==='founder')?'02 · 专长生长':'03 · 特化成体';}
