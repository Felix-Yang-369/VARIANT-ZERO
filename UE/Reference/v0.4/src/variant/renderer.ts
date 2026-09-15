import {atlasLayout} from './art';
import Phaser from 'phaser';
import {SPECIES,BY_ID,regionOf} from './content';
import type {AutoBattle,Combatant} from './battle';
import type {ExpeditionState} from './expedition';
import type {GameEvent} from './random';
export class Arena extends Phaser.Scene {
 battle:AutoBattle|null=null;state:ExpeditionState|null=null;reduced=false;
 private regionImage?:Phaser.GameObjects.Image;private region=-1;private sprites=new Map<number,Phaser.GameObjects.Image>();private labels=new Map<number,Phaser.GameObjects.Text>();private bars!:Phaser.GameObjects.Graphics;private effects!:Phaser.GameObjects.Graphics;private flashes=new Map<number,number>();private clock=0;private deaths=new Map<number,number>();
 constructor(){super('arena');}
 preload(){for(let i=0;i<3;i++)this.load.image('region-'+i,`/art/immersive-field${i}-v1.png`);for(const key of new Set(SPECIES.map(s=>s.art)))this.load.image(key,`/art/${key}.png`);}
 create(){
  for(const s of SPECIES){const src=this.textures.get(s.art).getSourceImage() as HTMLImageElement;const texture=this.textures.createCanvas(s.id,192,192)!;const ctx=texture.context;if(s.frame!==undefined){const {columns,rows}=atlasLayout(s.art),w=src.width/columns,h=src.height/rows;ctx.drawImage(src,(s.frame%columns)*w,Math.floor(s.frame/columns)*h,w,h,0,0,192,192);}else ctx.drawImage(src,0,0,192,192);texture.refresh();}
  this.regionImage=this.add.image(400,300,'region-0').setDisplaySize(800,600);const bg=this.add.graphics();bg.fillStyle(0x11251b,.18);bg.fillRect(0,0,800,600);for(let row=0;row<6;row++)for(let col=0;col<8;col++){bg.fillStyle(row<3?0x302c32:0x1c4038,(row+col)%2?.32:.2);bg.fillRoundedRect(col*100+3,row*100+3,94,94,8);bg.lineStyle(1,row<3?0xb498ae:0xadc19d,.38);bg.strokeRoundedRect(col*100+3,row*100+3,94,94,8);}bg.lineStyle(2,0xd8be87,.7);bg.lineBetween(0,300,800,300);
  this.bars=this.add.graphics().setDepth(12);this.effects=this.add.graphics().setDepth(15);
 }
 feedback(events:GameEvent[]){for(const e of events){if(e.type==='hit'&&e.target)this.flashes.set(e.target,this.clock+.13);if(e.type==='death'&&e.source)this.deaths.set(e.source,this.clock);}}
 update(_time:number,dt:number){if(!this.bars)return;const zone=regionOf(this.state?.round??1);if(zone!==this.region){this.region=zone;this.regionImage?.setTexture('region-'+zone);}this.clock+=dt/1000;const units=this.battle?.units??this.preview();const seen=new Set<number>();this.bars.clear();this.effects.clear();for(const u of units){seen.add(u.id);let image=this.sprites.get(u.id);if(!image){image=this.add.image(u.col*100+50,u.row*100+47,u.species).setDisplaySize(84,84);this.sprites.set(u.id,image);const label=this.add.text(0,0,'',{fontFamily:'sans-serif',fontSize:'12px',color:'#f4dc95',stroke:'#102720',strokeThickness:3}).setOrigin(.5).setDepth(14);this.labels.set(u.id,label);}const x=u.col*100+50,y=u.row*100+47;const dead=u.hp<=0;const deathAt=this.deaths.get(u.id)??this.clock;
   if(this.reduced||!this.battle){image.setPosition(x,y);}else{image.x+= (x-image.x)*Math.min(1,dt/70);image.y+=(y-image.y)*Math.min(1,dt/70);}
   const moving=BY_ID[u.species].kingdom==='fauna'&&this.battle?.status==='running';image.setRotation(!this.reduced&&moving?Math.sin(this.clock*13+u.id)*.035:0);image.setDepth(2+u.row);image.setAlpha(dead?(this.reduced?0:Math.max(0,1-(this.clock-deathAt)*2)):1);if((this.flashes.get(u.id)??0)>this.clock)image.setTintFill(0xffffff);else if(u.team===1)image.setTint(0xe7c3d3);else image.clearTint();
   const label=this.labels.get(u.id)!;label.setText(dead?'':'★'.repeat(u.star)).setPosition(x,y+40);
   if(!dead){this.bars.fillStyle(0x061915,1);this.bars.fillRect(x-32,y-39,64,5);this.bars.fillStyle(u.team===0?0x91dcc0:0xe095b1,1);this.bars.fillRect(x-32,y-39,64*u.hp/u.maxHP,5);if(u.shield>0){this.bars.fillStyle(0x8de5ed,.85);this.bars.fillRect(x-32,y-33,Math.min(64,64*u.shield/u.maxHP),2);}if(u.poisonUntil>this.battle?.time!)this.effects.lineStyle(2,0xb387db,.7).strokeCircle(x,y,34);}
  }
  for(const [id,img] of this.sprites)if(!seen.has(id)){img.destroy();this.labels.get(id)?.destroy();this.labels.delete(id);this.sprites.delete(id);this.deaths.delete(id);}
  if(this.battle)for(const p of this.battle.projectiles){const a=units.find(u=>u.id===p.source),b=units.find(u=>u.id===p.target);if(a&&b){const t=1-p.life/.2;this.effects.fillStyle(0xcbea92,1);this.effects.fillCircle((a.col+(b.col-a.col)*t)*100+50,(a.row+(b.row-a.row)*t)*100+47,this.reduced?3:5);}}
 }
 private preview():Combatant[]{if(!this.state)return [];const rows=[...this.state.units.filter(u=>u.row>=0).map(u=>({...u,team:0})),...this.statePreviewEnemies()];return rows.map(u=>({...u,maxHP:BY_ID[u.species].hp,hp:BY_ID[u.species].hp,shield:0,poisonUntil:0})) as Combatant[];}
 private statePreviewEnemies(){return (this.registry.get('enemies')??[]) as Array<any>;}
 clear(){this.battle=null;this.deaths.clear();this.flashes.clear();}
}
export function mountArena(parent:string){const scene=new Arena();const game=new Phaser.Game({type:Phaser.AUTO,parent,width:800,height:600,backgroundColor:'#112f2b',scene:[scene],render:{antialias:true},audio:{noAudio:true},scale:{mode:Phaser.Scale.FIT,autoCenter:Phaser.Scale.CENTER_BOTH}});return {game,scene};}
