/** Atlas geometry shared by CSS cards and the Phaser renderer. */
export function atlasLayout(key:string){
 if(key==='vz-founders-divine-v2')return {columns:3,rows:2};
 if(key==='vz-founders')return {columns:4,rows:2.5};
 return {columns:4,rows:3};
}
