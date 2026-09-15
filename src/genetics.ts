import type { Def, Plant } from "./model";
export type Family = "sun" | "pea" | "wall";
export const FAMILIES: Family[] = ["sun", "pea", "wall"];
export const GENES = {
  fast: { name: "速生", desc: "攻击、生产间隔 −20%", needs: "active" },
  health: { name: "厚壁", desc: "最大耐久 +35%", needs: "any" },
  regen: { name: "再生", desc: "每秒恢复 1% 最大耐久", needs: "any" },
  power: { name: "强击", desc: "攻击伤害 +25%", needs: "attack" },
  frost: { name: "寒霜", desc: "攻击减速 30%，持续 2 秒", needs: "attack" },
  pierce: {
    name: "穿透",
    desc: "额外穿透 1 个目标，后续伤害 50%",
    needs: "attack",
  },
  split: {
    name: "分裂",
    desc: "25% 概率追加半伤害子弹，不递归触发",
    needs: "attack",
  },
  photo: {
    name: "光合",
    desc: "阳光产量 +50%，最大耐久 −15%",
    needs: "produce",
  },
  reserve: {
    name: "储能",
    desc: "每生产 4 次，额外产生 25 阳光",
    needs: "produce",
  },
  thorns: { name: "荆棘", desc: "啃咬反伤实际伤害的 30%", needs: "any" },
  shell: { name: "甲壳", desc: "所受伤害 −20%", needs: "any" },
  symbiosis: {
    name: "共生",
    desc: "相邻植物攻击、生产间隔 −10%，不叠加",
    needs: "any",
  },
} as const;
export type Gene = keyof typeof GENES;
export const GENE_IDS = Object.keys(GENES) as Gene[];
export const EVOLUTIONS = {
  tough: { name: "强韧", desc: "最大耐久 +50%" },
  agile: { name: "敏捷", desc: "攻击、生产间隔 −25%；纯防守植物减伤 15%" },
  healing: { name: "再生", desc: "每秒恢复 1.5% 最大耐久" },
  expert: { name: "专精", desc: "伤害、阳光产量 +35%；纯防守植物减伤 25%" },
} as const;
export type Evolution = keyof typeof EVOLUTIONS;
export const MUTATIONS = {
  giant: {
    name: "巨化",
    desc: "耐久 +80%；攻击、生产间隔 +25%；纯防守植物额外承伤 25%",
  },
  haste: { name: "急生", desc: "攻击、生产间隔 −40%；最大耐久 −35%" },
  pods: { name: "多荚", desc: "每次两枚子弹；单枚伤害 −35%（仅射击植物）" },
  parasite: {
    name: "寄生",
    desc: "每秒吸取一株邻居 2% 最大耐久来治疗自身，可能致死",
  },
  golden: {
    name: "金化",
    desc: "每 20 秒产 25 阳光；攻击、生产间隔 +30%；纯防守植物耐久 −15%",
  },
  sterile: {
    name: "不育",
    desc: "耐久、伤害、阳光产量 +40%；永久失去杂交资格",
  },
} as const;
export type Mutation = keyof typeof MUTATIONS;
export interface Phenotype {
  name: string;
  pair: Family[];
  hp: number;
  damage?: number;
  rate?: number;
  produce?: number;
  extra?: "charge" | "repair" | "store" | "counter";
}
export const PHENOTYPES: Record<string, Phenotype> = {
  solar: {
    name: "光能射手",
    pair: ["sun", "pea"],
    hp: 180,
    damage: 20,
    rate: 1.8,
    produce: 16,
  },
  harvest: {
    name: "丰收射手",
    pair: ["sun", "pea"],
    hp: 140,
    damage: 15,
    rate: 1.8,
    produce: 9,
  },
  capacitor: {
    name: "辉光蓄能者",
    pair: ["sun", "pea"],
    hp: 200,
    damage: 40,
    rate: 2.4,
    produce: 16,
    extra: "charge",
  },
  bunker: {
    name: "种荚堡垒",
    pair: ["pea", "wall"],
    hp: 600,
    damage: 20,
    rate: 1.8,
  },
  volley: {
    name: "连射堡垒",
    pair: ["pea", "wall"],
    hp: 450,
    damage: 18,
    rate: 0.9,
  },
  sentinel: {
    name: "反击守卫",
    pair: ["pea", "wall"],
    hp: 750,
    damage: 20,
    rate: 1.8,
    extra: "counter",
  },
  generator: { name: "供能壁垒", pair: ["sun", "wall"], hp: 650, produce: 16 },
  nurse: {
    name: "修复花盾",
    pair: ["sun", "wall"],
    hp: 550,
    produce: 18,
    extra: "repair",
  },
  vault: {
    name: "储能圣果",
    pair: ["sun", "wall"],
    hp: 800,
    produce: 12,
    extra: "store",
  },
};
export class Random {
  constructor(public state: number) {
    this.state = state >>> 0;
  }
  next() {
    this.state = (this.state * 1664525 + 1013904223) >>> 0;
    return this.state / 4294967296;
  }
  sample<T>(values: readonly T[], count: number): T[] {
    const a = [...values];
    const out: T[] = [];
    while (a.length && out.length < count)
      out.push(a.splice(Math.floor(this.next() * a.length), 1)[0]);
    return out;
  }
}
export interface Streams {
  draft: Random;
  hybrid: Random;
  mutation: Random;
  evolution: Random;
  combat: Random;
}
export function streams(seed: number): Streams {
  return {
    draft: new Random(seed ^ 0x14321),
    hybrid: new Random(seed ^ 0x72431),
    mutation: new Random(seed ^ 0x38791),
    evolution: new Random(seed ^ 0x46293),
    combat: new Random(seed ^ 0x92317),
  };
}
export function phenotypePool(a: Family, b: Family) {
  return a === b
    ? []
    : Object.keys(PHENOTYPES).filter(
        (k) => PHENOTYPES[k].pair.includes(a) && PHENOTYPES[k].pair.includes(b),
      );
}
export function weightedPhenotype(pool: string[], roll: number) {
  return pool[roll < 0.6 ? 0 : roll < 0.9 ? 1 : 2];
}
export function applicable(
  gene: Gene,
  d: { damage?: number; produce?: number },
) {
  const n = GENES[gene].needs;
  return (
    n === "any" ||
    (n === "attack" && !!d.damage) ||
    (n === "produce" && !!d.produce) ||
    (n === "active" && !!(d.damage || d.produce))
  );
}
export function mutationPool(d: { damage?: number; produce?: number }) {
  return (Object.keys(MUTATIONS) as Mutation[])
    .filter((m) => m !== "pods" || !!d.damage)
    .filter((m) => m !== "haste" || !!(d.damage || d.produce));
}
export interface Stats extends Def {
  yield: number;
  regen: number;
  reduction: number;
  thorns: number;
  pierce: number;
  split: boolean;
  pods: boolean;
  extra?: Phenotype["extra"];
}
export function stats(p: Plant, base: Def, neighbors: Plant[] = []): Stats {
  const ph = p.phenotype ? PHENOTYPES[p.phenotype] : undefined;
  const d: Stats = {
    ...base,
    ...ph,
    yield: 25,
    regen: 0,
    reduction: 0,
    thorns: 0,
    pierce: 0,
    split: false,
    pods: false,
  };
  let hp = 1,
    damage = 1,
    interval = 1,
    production = 1;
  for (const g of p.genome ?? []) {
    if (!applicable(g, d)) continue;
    switch (g) {
      case "fast":
        interval *= 0.8;
        break;
      case "health":
        hp *= 1.35;
        break;
      case "regen":
        d.regen += 0.01;
        break;
      case "power":
        damage *= 1.25;
        break;
      case "frost":
        d.slow = true;
        break;
      case "pierce":
        d.pierce = 1;
        break;
      case "split":
        d.split = true;
        break;
      case "photo":
        production *= 1.5;
        hp *= 0.85;
        break;
      case "thorns":
        d.thorns = 0.3;
        break;
      case "shell":
        d.reduction += 0.2;
        break;
    }
  }
  switch (p.evolution) {
    case "tough":
      hp *= 1.5;
      break;
    case "agile":
      interval *= 0.75;
      if (!d.damage && !d.produce) d.reduction += 0.15;
      break;
    case "healing":
      d.regen += 0.015;
      break;
    case "expert":
      damage *= 1.35;
      production *= 1.35;
      if (!d.damage && !d.produce) d.reduction += 0.25;
      break;
  }
  switch (p.mutation) {
    case "giant":
      hp *= 1.8;
      interval *= 1.25;
      if (!d.damage && !d.produce) d.reduction -= 0.25;
      break;
    case "haste":
      interval *= 0.6;
      hp *= 0.65;
      break;
    case "pods":
      d.pods = true;
      damage *= 0.65;
      break;
    case "golden":
      interval *= 1.3;
      if (!d.damage && !d.produce) hp *= 0.85;
      break;
    case "sterile":
      hp *= 1.4;
      damage *= 1.4;
      production *= 1.4;
      break;
  }
  if (
    neighbors.some(
      (q) =>
        q.id !== p.id &&
        q.hp > 0 &&
        Math.abs(q.row - p.row) + Math.abs(q.col - p.col) === 1 &&
        q.genome?.includes("symbiosis"),
    )
  )
    interval *= 0.9;
  interval = Math.max(0.3, interval);
  d.hp = Math.round(d.hp * hp);
  if (d.damage) d.damage *= damage;
  if (d.rate) d.rate = Math.max(0.2, d.rate * interval);
  if (d.produce) d.produce = Math.max(3, d.produce * interval);
  d.yield = Math.round(25 * production * 10) / 10;
  d.reduction = Math.min(0.7, d.reduction);
  return d;
}
