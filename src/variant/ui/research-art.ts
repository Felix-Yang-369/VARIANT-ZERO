import {type SpeciesDefinition} from '../content';
import {art} from './organism-art';
import {HOME_SPECIMENS} from './home-art';
/** Page-only presentation. The shop and battlefield keep their original atlas. */
export type ResearchArtSize = 'node' | 'card' | 'detail';
export function researchArt(s:SpeciesDefinition,hidden=false,size:ResearchArtSize='card') {
 if(s.category!=='founder'||hidden)return `<span class="research-art art-${size}"><span class="sprite-window">${art(s,hidden)}</span></span>`;
 const a=HOME_SPECIMENS[s.family];
 return `<span class="research-art art-${size}" style="--art-scale:${a.scale};--art-foot-x:${a.foot[0]*100}%;--art-foot-y:${a.foot[1]*100}%"><img class="organism" src="${a.src}" alt="" decoding="async" loading="lazy"></span>`;
}
