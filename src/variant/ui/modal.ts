import {icon} from './shell';
export function modalMarkup(title:string,body:string){return `<section class="modal-card" role="dialog" aria-modal="true" aria-label="${title}"><div class="modal-head"><span class="eyebrow">RESEARCH TERMINAL / 实验记录</span><button data-close aria-label="关闭弹窗">${icon('close')}关闭</button></div><h2>${title}</h2>${body}</section>`;}
