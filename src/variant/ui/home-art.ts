import type {Family} from '../content';
/** Presentation only. Each specimen uses its own normalized foot anchor. */
export interface HomeSpecimen {src:string;scale:number;foot:[number,number];title:string}
export const HOME_SPECIMENS:Record<Family,HomeSpecimen>={
 legume:{src:'/art/home-legume-v3.png',scale:1,foot:[.5,.974],title:'万象，始于一粒种核。'},
 bloom:{src:'/art/home-bloom-v3.png',scale:.94,foot:[.5,.962],title:'未绽之花，蕴藏晨光。'},
 root:{src:'/art/home-root-v3.png',scale:.96,foot:[.5,.985],title:'古木无言，根系长存。'},
 fungi:{src:'/art/home-fungi-v3.png',scale:.96,foot:[.5,.983],title:'微光之下，万物相连。'},
 arthropod:{src:'/art/home-arthropod-v3.png',scale:.96,foot:[.5,.992],title:'玉甲之中，潜藏锋芒。'},
};
export function homeSpecimen(family:Family){const a=HOME_SPECIMENS[family];return `<img class="organism home-founder" src="${a.src}" width="1024" height="1024" alt="" decoding="async">`;}
