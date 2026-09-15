/** Fictional field notes for MUTABLOOM. These describe the world, not new combat rules. */
export const WORLD = {
  premise:
    "翠绿网络曾将每一片叶、每一条根连接在一起。如今，网络正在沉寂；失去回应的生命被枯潮裹挟，循着微弱的光走来。",
  mission:
    "你接手了最后一座仍有生命反应的温室。这里保存着三类母体与一册未完成的遗传笔记。种下、杂交、进化，守住这片尚能诞生新生命的土地。",
  stages: [
    {
      title: "01 / 清晨庭院",
      text: "从温室门前开始，守住最先苏醒的幼苗。露光尚未散去，枯潮已经沿着旧石径逼近。",
    },
    {
      title: "02 / 午后追逐",
      text: "跟随断续的根脉信号穿过外庭。这里的枯潮更加躁动，上一场留下的血统，将成为你的下一道防线。",
    },
    {
      title: "03 / 花园实验室",
      text: "旧实验室的晶灯再次亮起。守住最后的培养床，让新的遗传记录回到温室。枯潮的源头仍藏在更深的根系中。",
    },
  ],
};
export const SPECIMENS: Record<
  string,
  {
    family: string;
    name: string;
    habitat: string;
    note: string;
    role: string;
    accent: string;
  }
> = {
  sun: {
    family: "BLOOM / 花系",
    name: "Dawnkeeper",
    habitat: "清晨庭院 · 向光花床",
    note: "翠绿网络沉寂后，它仍会准时朝向日出的方向。园丁相信，最早的光从未离开这片花园。",
    role: "资源母体",
    accent: "#eed080",
  },
  pea: {
    family: "POD / 荚系",
    name: "Podwarden",
    habitat: "清晨庭院 · 旧篱笆",
    note: "它的荚果曾负责把种子送往远方。如今，同样的力量守护着种子还来不及抵达的土地。",
    role: "射击母体",
    accent: "#afd18b",
  },
  wall: {
    family: "ROOT / 根系",
    name: "Rootbound",
    habitat: "温室门廊 · 老树根旁",
    note: "木质外壳里藏着一颗柔软的种仁。它很少移动，却总是站在幼苗和枯潮之间。",
    role: "防守母体",
    accent: "#d6b484",
  },
  solar: {
    family: "BLOOM × POD",
    name: "Heliopod",
    habitat: "温室 · 光照培养床",
    note: "花瓣收拢的光沿荚脉流动。第一株光能射手诞生时，熄灭许久的培养灯忽然亮了一瞬。",
    role: "资源 / 射击",
    accent: "#e7ce78",
  },
  harvest: {
    family: "BLOOM × POD",
    name: "Cornucopia",
    habitat: "温室 · 丰收试验架",
    note: "它把生长的热情倾注在每一粒种子里。园丁为它换过三次标本盒，最后只好留下一张画。",
    role: "高产变体",
    accent: "#f0c576",
  },
  capacitor: {
    family: "BLOOM × POD",
    name: "Lumenheart",
    habitat: "旧实验室 · 蓄光舱",
    note: "透明种腔保存着一小段日光。连枯潮最浓的时刻，你也能看见它胸前那枚没有熄灭的星。",
    role: "蓄能变体",
    accent: "#86d6c9",
  },
  bunker: {
    family: "POD × ROOT",
    name: "Barkbastion",
    habitat: "外庭 · 石阶防线",
    note: "荚果与木甲学会了同一种节奏：向前生长。它是第一份证明“保护”与“反击”可以共存的记录。",
    role: "防守 / 射击",
    accent: "#c6c58d",
  },
  volley: {
    family: "POD × ROOT",
    name: "Twinwhorl",
    habitat: "外庭 · 风口试验田",
    note: "两枚种荚交替呼吸，像风里有问有答的叶哨。它的培养页上留着密密麻麻的计时刻度。",
    role: "连射变体",
    accent: "#a3d2a1",
  },
  sentinel: {
    family: "POD × ROOT",
    name: "Thornsentinel",
    habitat: "外庭 · 断墙根部",
    note: "木甲上的尖刺来自一场意外的生长。园丁没有剪去它们，而是在笔记里写下：“它学会了回答。”",
    role: "反击变体",
    accent: "#cfb57d",
  },
  generator: {
    family: "BLOOM × ROOT",
    name: "Sunhaven",
    habitat: "温室 · 供能花床",
    note: "坚硬果壳托起一圈向光花冠。小苗喜欢聚在它身后，那里既有遮蔽，也有温暖。",
    role: "资源 / 防守",
    accent: "#e1cb88",
  },
  nurse: {
    family: "BLOOM × ROOT",
    name: "Dewmender",
    habitat: "旧实验室 · 修复培养床",
    note: "它用叶片托着一滴青色汁液。谁也说不清那是露水，还是翠绿网络尚未忘记的温柔。",
    role: "修复变体",
    accent: "#9edbc9",
  },
  vault: {
    family: "BLOOM × ROOT",
    name: "Ambervault",
    habitat: "旧实验室 · 种子库",
    note: "层层木甲守着金色的种腔。每一次积蓄都像是在替尚未到来的春天，留下一份余粮。",
    role: "储能变体",
    accent: "#d5b178",
  },
};
export const WITHER = [
  {
    name: "苔行者",
    tag: "残留的生长习惯",
    note: "被枯潮牵引的苔木躯壳，仍沿着旧花径缓慢前行。它似乎已经忘记，自己曾属于哪一片林地。",
  },
  {
    name: "铁壳行者",
    tag: "封闭的生命回声",
    note: "沉积物与老树皮包裹着它的身体。厚重外壳保护了内部，也隔绝了翠绿网络最后的呼唤。",
  },
  {
    name: "疾藤潜行者",
    tag: "失序的向光性",
    note: "枯潮让藤蔓的生长冲动失去了方向。它追逐温室的光，却再也无法在光下安静地扎根。",
  },
  {
    name: "磐木巨像",
    tag: "凝固的古老根脉",
    note: "石质树皮下闪烁着紫色裂隙。它曾是网络中稳固的节点，如今沉重的脚步让花床也跟着颤动。",
  },
];
