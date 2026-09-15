import {chromium} from '@playwright/test';
import fs from 'node:fs/promises';
const dir='.verification/refinement';await fs.mkdir(dir,{recursive:true});
const browser=await chromium.launch({channel:'chrome',headless:true});
const report={chrome:browser.version(),views:[],alpha:null,errors:[]};
for(const [width,height] of [[1280,720],[1366,768],[1440,900],[390,844]]){
 const context=await browser.newContext({viewport:{width,height},reducedMotion:'reduce'}),p=await context.newPage();p.on('pageerror',e=>report.errors.push(e.message));await p.goto('http://127.0.0.1:5173/?qa=1');
 await p.evaluate(()=>{const q=window.__variantQA;q.repo.transaction(d=>d.lessons=['research']);q.render();});
 for(const view of ['hub','archive','lab']){await p.evaluate(view=>window.__variantQA.go(view),view);await p.evaluate(async()=>{await Promise.all([...document.images].map(i=>{i.loading='eager';return i.decode().catch(()=>{});}));});await p.screenshot({path:`${dir}/${view}-${width}.png`,fullPage:true,animations:'disabled'});report.views.push(await p.evaluate(({view,width,height})=>({view,width,height,scrollWidth:document.documentElement.scrollWidth,scrollHeight:document.documentElement.scrollHeight,images:[...document.images].filter(i=>!i.complete||i.naturalWidth===0).map(i=>i.src)}),{view,width,height}));}
 if(width===1440){report.alpha=await p.evaluate(async()=>{const i=new Image();i.src='/art/home-podium-v4.png';await i.decode();const c=document.createElement('canvas');c.width=i.width;c.height=i.height;const x=c.getContext('2d');x.drawImage(i,0,0);const d=x.getImageData(0,0,c.width,c.height).data;let transparent=0;for(let k=3;k<d.length;k+=4)if(d[k]===0)transparent++;return {width:i.width,height:i.height,transparent:transparent/(i.width*i.height)};});}
 await context.close();
}
await fs.writeFile(dir+'/visual.json',JSON.stringify(report,null,2));console.log(JSON.stringify(report));await browser.close();
