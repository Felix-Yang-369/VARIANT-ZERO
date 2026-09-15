import {atlasLayout} from '../art';
import type {SpeciesDefinition} from '../content';
/** Original atlas renderer shared by gameplay and page-specific presentation. */
export function art(s:SpeciesDefinition,hidden=false){const frame=s.frame,{columns,rows}=atlasLayout(s.art);return frame===undefined?`<img class="organism ${hidden?'unknown':''}" src="/art/${s.art}.png" alt="" loading="lazy">`:`<span class="organism atlas ${hidden?'unknown':''}" aria-hidden="true" style="background-image:url('/art/${s.art}.png');background-size:${columns*100}% ${rows*100}%;background-position:${(frame%columns)/(columns-1)*100}% ${Math.floor(frame/columns)/(rows-1)*100}%"></span>`;}
