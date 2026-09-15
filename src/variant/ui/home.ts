import './home.css';
import {SPECIES,BY_ID,FAMILY,ABILITIES,type Family} from '../content';
import type {SaveV4} from '../storage';
import {art} from './specimen';
import {icon} from './shell';
import {HOME_SPECIMENS,homeSpecimen} from './home-art';
export const SCENES={home:'/art/immersive-home-v3.png',lab:'/art/immersive-lab-v1.png',fields:[0,1,2].map(i=>`/art/immersive-field${i}-v1.png`)};
export function homeView(d:SaveV4,family:string){
 const selected=(family in HOME_SPECIMENS?family:'legume') as Family;
 const run=d.active&&d.active.phase!=='ended'?d.active:null;
 const branch=SPECIES.filter(s=>s.family===selected&&['founder','variant'].includes(s.category));
 const chain=[branch[0],branch[1],branch.find(s=>s.parents.includes(branch[1]?.id))??branch[branch.length-1]];
 const h=d.research.history[0],primal=branch[0],display=HOME_SPECIMENS[selected];
 const label=run?'继续出征':'开始出征';
 return `<section class="base-home" aria-label="原初生态研究所">
  <header class="base-overview">
   <div class="home-heading"><span class="eyebrow">${icon('leaf')} 自适应生态署 · 研究中枢</span><h1>${display.title}</h1><p>在实验室创造生命，在战场构筑生态。</p></div>
   <div class="base-status"><span class="discovery-count"><small>已发现物种</small><b>${String(d.research.unlocked.length).padStart(2,'0')}<em> / 45</em></b></span><span class="run-status"><small>${run?'活动出征':'部署状态'}</small><b>${run?'回合 '+run.round:'等待出征'}</b><small>${run?'指挥完整度 '+run.integrity:'选择母体，构筑你的第一组生态。'}</small></span></div>
  </header>
  <div class="living-stage sacred-stage" aria-label="原初母体展示区">
   <div class="chamber-scene" style="--specimen-scale:${display.scale};--foot-x:${display.foot[0]*100}%;--foot-y:${display.foot[1]*100}%">
    <div class="chamber-halo" aria-hidden="true"></div>
    <div class="podium-shadow" aria-hidden="true"></div><img class="cultivation-podium" src="/art/home-podium-v4.png" alt="" decoding="async">
    <div class="contact-shadow" aria-hidden="true"></div>
    <div class="resident primal-resident">${homeSpecimen(selected)}</div>
    <svg class="chamber-foreground" viewBox="0 0 800 600" fill="none" aria-hidden="true"><ellipse cx="400" cy="458" rx="270" ry="56" stroke="currentColor" stroke-opacity=".2"/><path d="M142 476a270 56 0 0 0 516 0M120 437v12m560-12v12M385 516h30M400 510v12" stroke="currentColor" stroke-width="1.5"/><path d="M548 387h50l20-20h120" stroke="currentColor" stroke-opacity=".7"/><circle cx="548" cy="387" r="3" stroke="currentColor"/></svg>
    <div class="resident-label" aria-live="polite"><span>${primal.id} / 原初母体</span><strong>${primal.name}</strong><small>${FAMILY[selected]} · 原初生命</small><p>${ABILITIES[primal.ability]}</p></div>
   </div>
   <div class="home-stage-note"><span class="status-dot"></span> 原初生态研究所 <small>PRIMORDIAL CONSERVATORY</small></div>
  </div>
  <button class="home-mobile-start primary" data-nav="deploy">${icon('deploy')}${label}${icon('arrow')}</button>
  <div class="home-dock">
   <section class="growth-preview"><div class="dock-heading"><h2>${icon('leaf')} 成长谱系</h2><button data-nav="archive" aria-label="查看完整图鉴">${icon('arrow')}</button></div><div class="family-tabs" aria-label="切换成长谱系">${Object.entries(FAMILY).map(([id,n])=>`<button data-home-family="${id}" aria-pressed="${id===selected}">${n}</button>`).join('')}</div><div class="home-growth">${chain.map((s,i)=>{const known=d.research.unlocked.includes(s.id);return `${i?`<span class="growth-arrow">${icon('arrow')}</span>`:''}<button class="growth-node" data-home-species="${s.id}" aria-label="查看 ${known?s.name:s.id+' 未解析'} 的遗传关系"><div class="growth-visual">${s.category==='founder'?homeSpecimen(s.family):art(s,!known)}${known?'':`<span class="growth-lock">${icon('lock')}</span>`}</div><small>${known?s.name:`<span>${s.id}</span> <span>未解析</span>`}</small><span class="growth-step">${['01 · 母体','02 · 分化','03 · 后代'][i]}</span></button>`;}).join('')}</div></section>
   <section class="latest-research"><span class="eyebrow">${icon('lab')} 最近研究</span><h2>${h?(h.result?BY_ID[h.result].name:'等待下一次分化'):'第一份发现，始于好奇。'}</h2><p>${h?(h.duplicate?'重复发现 · 试剂返还已存入档案。':h.result?'新物种已收录，可带入下次出征。':'未形成稳定物种，母体模板保留。'):'五种母体已就绪，探索生命的下一种可能。'}</p><button data-nav="lab">进入实验室 ${icon('arrow')}</button></section>
   <button class="field-entry" data-nav="records"><span class="eyebrow">${icon('records')} 区域调查</span><h2>绽变之后</h2><p>翠境计划的下一份线索，<br>等待你的调查。</p><span class="field-link">阅读调查档案 ${icon('arrow')}</span></button>
  </div>
 </section>`;
}
