import {SPECIES,BY_ID,FAMILY,ABILITIES,growthStage,type SpeciesDefinition} from '../content';
import type {SaveV4} from '../storage';
import {pageHeading,icon} from './shell';
import {researchArt} from './research-art';
import {speciesCard} from './specimen';
type Context={repo:{data:SaveV4};filter:string;archiveMode:string;archiveSelection:string;zoom:number};
export function highlightArchive(selection:string){
 const related=new Set([selection,...BY_ID[selection].parents,...SPECIES.filter(s=>s.parents.includes(selection)).map(s=>s.id)]);
 document.querySelectorAll<HTMLElement>('[data-node]').forEach(n=>{n.classList.toggle('selected',n.dataset.node===selection);n.classList.toggle('related',related.has(n.dataset.node!));n.setAttribute('aria-pressed',String(n.dataset.node===selection));});
 document.querySelectorAll<SVGElement>('[data-parent]').forEach(p=>p.classList.toggle('related',p.dataset.parent===selection||p.dataset.child===selection));
}
export function locateArchiveNode(selection:string,focus=false){
 const viewport=document.querySelector<HTMLElement>('#network-viewport'),node=document.querySelector<HTMLElement>(`[data-node="${selection}"]`);
 if(!viewport||!node)return;
 const a=viewport.getBoundingClientRect(),b=node.getBoundingClientRect();
 viewport.scrollLeft+=b.left-a.left-(a.width-b.width)/2;
 viewport.scrollTop+=b.top-a.top-(a.height-b.height)/2;
 if(focus)node.focus({preventScroll:true});
}
export function createArchiveView(ctx:Context){
 const {repo,filter,archiveMode,archiveSelection,zoom}=ctx;
 const known=(s:SpeciesDefinition)=>repo.data.research.unlocked.includes(s.id);
 const relation=(id:string)=>{const s=BY_ID[id];return `<button class="relation-card" data-relation="${id}">${researchArt(s,!known(s),'node')}<span><small>${id}</small><b>${known(s)?s.name:'未解析'}</b></span>${icon('arrow')}</button>`;};
 function details(s:SpeciesDefinition){
  const k=known(s),children=SPECIES.filter(x=>x.parents.includes(s.id));
  return `<div class="detail-specimen">${researchArt(s,!k,'detail')}<span class="detail-orbit" aria-hidden="true"></span></div><span class="eyebrow">${s.id} · ${FAMILY[s.family]}</span><h2>${k?s.name:'未解析的遗传分叉'}</h2><span class="stage-badge">${k?growthStage(s):'未知形态 · 等待解析'}</span><p class="detail-ability">${k?ABILITIES[s.ability]:'沿父代关系探索实验条件，解析新的生命形态。'}</p>${k?`<dl class="detail-stats">${[['耐久',s.hp],['伤害',s.damage],['攻击间隔',s.interval+'s'],['射程',s.range+' 格'],['培养费用',s.cost+' BIO']].map(([n,v])=>`<div><dt>${n}</dt><dd>${v}</dd></div>`).join('')}</dl>`:''}<section class="relations"><h3>直接父代 <small>${s.parents.length}</small></h3>${s.parents.length?s.parents.map(relation).join(''):'<p>始祖母体 · 初始开放</p>'}<h3>后代分支 <small>${children.length}</small></h3>${children.slice(0,3).map(x=>relation(x.id)).join('')||'<p>尚无后续记录</p>'}${children.length>3?`<details><summary>展开其余 ${children.length-3} 个分支</summary>${children.slice(3).map(x=>relation(x.id)).join('')}</details>`:''}</section><p class="archive-footnote">出征升星不会改变永久物种谱系。</p>`;
 }
 function network(items:SpeciesDefinition[]){
  const familyIds=filter==='all'?Object.keys(FAMILY):[filter],visible=new Set(items.map(s=>s.id)),positions=new Map<string,{x:number;y:number}>(),used=new Map<string,number>();
  const depth=(s:SpeciesDefinition):number=>s.category==='founder'?0:s.category==='zero'?5:s.category==='aberrant'?4:s.parents.length>1?3:1+Number(s.parents.some(p=>BY_ID[p].category!=='founder'));
  for(const s of items){const row=depth(s),col=familyIds.indexOf(s.family),key=col+':'+row,n=used.get(key)??0;used.set(key,n+1);positions.set(s.id,{x:col*360+28+(n%2)*168,y:row*260+68+Math.floor(n/2)*124});}
  const width=familyIds.length*360+28,height=1570;
  return `<div class="network-caption"><span>当前节点与直接亲缘高亮</span><span>拖动浏览 · Tab 选择</span></div><div id="network-viewport" class="network-viewport" tabindex="0" aria-label="谱系网络，可拖动或滚动；节点支持 Tab 与 Enter"><div class="network-space" style="width:${width*zoom}px;height:${height*zoom}px"><div class="network-plane" style="width:${width}px;height:${height}px;transform:scale(${zoom})">${familyIds.map((f,i)=>`<span class="network-family" style="left:${i*360+28}px">${FAMILY[f as keyof typeof FAMILY]}</span>`).join('')}<svg width="${width}" height="${height}" class="network-lines" aria-hidden="true">${items.flatMap(s=>s.parents.filter(p=>visible.has(p)).map(p=>{const a=positions.get(p)!,b=positions.get(s.id)!;return `<path data-parent="${p}" data-child="${s.id}" d="M${a.x+74},${a.y+100} C${a.x+74},${a.y+160} ${b.x+74},${b.y-55} ${b.x+74},${b.y}" class="${s.parents.length>1?'hybrid-line':''}"/>`;})).join('')}</svg>${items.map(s=>{const p=positions.get(s.id)!;return `<button class="network-node ${known(s)?'discovered':'unseen'}" data-node="${s.id}" style="left:${p.x}px;top:${p.y}px" aria-label="${s.id} ${known(s)?s.name:'未知物种'}">${researchArt(s,!known(s),'node')}<span>${s.id}</span><b>${known(s)?s.name:'未解析'}</b></button>`;}).join('')}</div></div></div>`;
 }
 function renderArchive(){
  const items=SPECIES.filter(s=>filter==='all'||s.family===filter);
  document.querySelector('#screen')!.innerHTML=pageHeading('变种档案','LIVING ARCHIVE',`${repo.data.research.unlocked.length} / 45 物种已发现 · 沿着亲缘关系，寻找生命的下一种可能。`)+`<div class="archive-tools"><label>生命体系 <select id="family-filter"><option value="all">全部体系</option>${Object.entries(FAMILY).map(([id,n])=>`<option value="${id}" ${filter===id?'selected':''}>${n}</option>`).join('')}</select></label><div class="view-switch"><button data-archive-mode="network" aria-pressed="${archiveMode==='network'}">谱系网络</button><button data-archive-mode="list" aria-pressed="${archiveMode==='list'}">列表视图</button></div>${archiveMode==='network'?`<div class="zoom-controls"><button data-zoom="-" aria-label="缩小谱系">−</button><button data-zoom="reset">${Math.round(zoom*100)}%</button><button data-zoom="+" aria-label="放大谱系">＋</button><button data-locate>定位当前物种</button></div>`:''}</div><div class="archive-layout"><section class="archive-main">${archiveMode==='list'?`<div class="species-grid">${items.map(s=>`<div>${speciesCard({unlocked:repo.data.research.unlocked,selectedUnit:null},s,{hide:true,presentation:'research'})}<button data-species="${s.id}">查看 ${s.id} 遗传关系 ${icon('arrow')}</button></div>`).join('')}</div>`:network(items)}</section><section id="archive-detail" class="archive-detail" tabindex="-1" aria-label="物种详情">${details(BY_ID[archiveSelection])}</section></div>`;
  const e=document.querySelector<HTMLElement>('#network-viewport');if(e){let dragging=false,x=0,y=0,left=0,top=0;e.onpointerdown=ev=>{if((ev.target as HTMLElement).closest('button'))return;dragging=true;x=ev.clientX;y=ev.clientY;left=e.scrollLeft;top=e.scrollTop;e.setPointerCapture(ev.pointerId);};e.onpointermove=ev=>{if(dragging){e.scrollLeft=left-(ev.clientX-x);e.scrollTop=top-(ev.clientY-y);}};e.onpointerup=e.onpointercancel=()=>{dragging=false;};}
  highlightArchive(archiveSelection);
 }
 return {renderArchive,details};
}
