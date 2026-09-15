export class Random {
 constructor(public state:number){this.state=(state>>>0)||0x6d2b79f5;}
 next(){let x=this.state;x^=x<<13;x^=x>>>17;x^=x<<5;this.state=x>>>0;return this.state/4294967296;}
 pick<T>(a:T[]):T{if(!a.length)throw Error('空随机池');return a[Math.floor(this.next()*a.length)];}
 weighted<T>(a:Array<{value:T;weight:number}>):T{let v=this.next()*a.reduce((n,x)=>n+x.weight,0);for(const x of a){v-=x.weight;if(v<0)return x.value;}return a[a.length-1].value;}
}
export interface GameEvent {type:'buy'|'sell'|'merge'|'move'|'shot'|'hit'|'death'|'heal'|'skill'|'income'|'reward'|'experiment'|'round'|'win'|'loss';source?:number;target?:number;value?:number;detail?:string;}
export class Events {private events:GameEvent[]=[];emit(e:GameEvent){this.events.push(e);}drainEvents(){return this.events.splice(0);}}
