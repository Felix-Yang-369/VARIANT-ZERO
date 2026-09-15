import {chromium} from '@playwright/test';
import fs from 'node:fs';
const browser=await chromium.launch({channel:'chrome',headless:true});const page=await browser.newPage({viewport:{width:1440,height:900}});const errors=[];page.on('pageerror',e=>errors.push(e.message));await page.goto('http://127.0.0.1:5173/?qa=1');await page.waitForTimeout(1000);
for(const size of [32,180]){const data=await page.evaluate(async size=>{const image=new Image();image.src='/favicon.svg';await image.decode();const c=document.createElement('canvas');c.width=c.height=size;c.getContext('2d').drawImage(image,0,0,size,size);return c.toDataURL().split(',')[1];},size);fs.writeFileSync(size===32?'public/favicon-32.png':'public/apple-touch-icon.png',Buffer.from(data,'base64'));}
await page.screenshot({path:'.verification/variant/hub-1440.png',fullPage:true});
for(const name of ['archive','lab','records']){await page.locator(`[data-nav="${name}"]`).first().click();if(await page.locator('[data-skip-lessons]').isVisible())await page.locator('[data-skip-lessons]').click();await page.screenshot({path:`.verification/variant/${name}-1440.png`,fullPage:true});}
await page.evaluate(()=>{const q=window.__variantQA;q.repo.transaction(d=>{d.research.unlocked=Array.from({length:45},(_,i)=>`V-${String(i+1).padStart(3,'0')}`)});q.go('archive');});await page.locator('[data-node="V-006"]').click();await page.locator('#archive-detail').screenshot({path:'.verification/variant/growth-comparison.png'});
await page.locator('[data-archive-mode="list"]').click();await page.screenshot({path:'.verification/variant/all-species.png',fullPage:true});
fs.writeFileSync('.verification/variant/visual-check.json',JSON.stringify({version:browser.version(),errors,date:new Date().toISOString()},null,2));await browser.close();
