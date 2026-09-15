import {
  stats,
  streams,
  applicable,
  phenotypePool,
  weightedPhenotype,
  mutationPool,
  EVOLUTIONS,
  type Family,
  type Gene,
  type Mutation,
  type Evolution,
  type Streams,
} from "./genetics";
export type Kind =
  | "sun"
  | "pea"
  | "wall"
  | "ice"
  | "bomb"
  | "rapid"
  | "double"
  | "frost"
  | "armor"
  | "thorn"
  | "solar"
  | "bunker"
  | "coldwall";
export interface Def {
  name: string;
  cost: number;
  hp: number;
  cd: number;
  damage?: number;
  rate?: number;
  produce?: number;
  slow?: boolean;
  color: string;
  desc: string;
}
export const PLANTS: Record<Kind, Def> = {
  sun: {
    name: "阳光花",
    cost: 50,
    hp: 100,
    cd: 5,
    produce: 12,
    color: "#f6bd42",
    desc: "每 12 秒生产 25 阳光",
  },
  pea: {
    name: "豆射手",
    cost: 100,
    hp: 100,
    cd: 5,
    damage: 20,
    rate: 1.4,
    color: "#74b94c",
    desc: "稳定的同排远程输出",
  },
  wall: {
    name: "护盾果",
    cost: 50,
    hp: 900,
    cd: 12,
    color: "#ba8654",
    desc: "高耐久，保护身后植物",
  },
  ice: {
    name: "冰芽",
    cost: 150,
    hp: 100,
    cd: 8,
    damage: 15,
    rate: 1.8,
    slow: true,
    color: "#7dcbd4",
    desc: "命中减速 30%，持续 2 秒",
  },
  bomb: {
    name: "爆爆花",
    cost: 150,
    hp: 100,
    cd: 25,
    color: "#e97b66",
    desc: "0.5 秒后爆炸，3×3 格伤害 400",
  },
  rapid: {
    name: "速射苗",
    cost: 200,
    hp: 100,
    cd: 10,
    damage: 20,
    rate: 0.7,
    color: "#7aa065",
    desc: "双倍射速，持续压制",
  },
  double: {
    name: "连射射手",
    cost: 150,
    hp: 150,
    cd: 0,
    damage: 25,
    rate: 0.7,
    color: "#4c924e",
    desc: "进化 · 高速单体输出",
  },
  frost: {
    name: "霜冻射手",
    cost: 125,
    hp: 150,
    cd: 0,
    damage: 22,
    rate: 1.3,
    slow: true,
    color: "#70c9dc",
    desc: "进化 · 输出与减速兼备",
  },
  armor: {
    name: "厚甲护盾",
    cost: 100,
    hp: 1800,
    cd: 0,
    color: "#a88661",
    desc: "进化 · 耐久翻倍",
  },
  thorn: {
    name: "反刺护盾",
    cost: 100,
    hp: 1100,
    cd: 0,
    color: "#a0ac59",
    desc: "进化 · 每次被啃咬反伤 25",
  },
  solar: {
    name: "光能射手",
    cost: 50,
    hp: 150,
    cd: 0,
    damage: 20,
    rate: 1.8,
    produce: 16,
    color: "#dbbf4e",
    desc: "杂交 · 同时生产阳光和射击",
  },
  bunker: {
    name: "护盾射手",
    cost: 75,
    hp: 600,
    cd: 0,
    damage: 20,
    rate: 1.8,
    color: "#a1a768",
    desc: "杂交 · 防守与输出合一",
  },
  coldwall: {
    name: "寒霜护盾",
    cost: 75,
    hp: 650,
    cd: 0,
    color: "#8bbcb4",
    desc: "杂交 · 使啃咬者减速",
  },
};
export const BASE: Kind[] = ["sun", "pea", "wall", "ice", "bomb", "rapid"];
export const EVOLVE: Partial<Record<Kind, Kind[]>> = {
  pea: ["double", "frost"],
  wall: ["armor", "thorn"],
};
export const RECIPES: { a: Kind; b: Kind; out: Kind }[] = [
  { a: "sun", b: "pea", out: "solar" },
  { a: "pea", b: "wall", out: "bunker" },
  { a: "ice", b: "wall", out: "coldwall" },
];
export const ENEMIES = [
  { name: "苔行者", hp: 100, speed: 0.16, damage: 20 },
  { name: "铁壳行者", hp: 300, speed: 0.14, damage: 20 },
  { name: "疾藤潜行者", hp: 70, speed: 0.3, damage: 20 },
  { name: "磐木巨像", hp: 700, speed: 0.1, damage: 40 },
];
export const LEVELS = [
  {
    name: "清晨庭院",
    tag: "01 / 初来乍到",
    desc: "种下阳光花，让第一道防线生长。",
    sun: 175,
    cards: BASE.slice(0, 3),
    end: 125,
  },
  {
    name: "午后追逐",
    tag: "02 / 生长的分岔",
    desc: "奔跑者来了。点击射手或护盾，选择进化。",
    sun: 225,
    cards: BASE.filter((k) => k !== "bomb"),
    end: 155,
  },
  {
    name: "花园实验室",
    tag: "03 / 奇妙的组合",
    desc: "用杂交整合防线，迎接磐木巨像。",
    sun: 275,
    cards: BASE,
    end: 180,
  },
];
export interface Plant {
  id: number;
  kind: Kind;
  row: number;
  col: number;
  hp: number;
  attack: number;
  production: number;
  born: number;
  genome?: Gene[];
  primaryFamily?: Family;
  phenotype?: string;
  generation?: number;
  xp?: number;
  evolution?: Evolution;
  evolutionChoices?: Evolution[];
  mutation?: Mutation;
  produced?: number;
  goldenAt?: number;
}
export interface Enemy {
  id: number;
  kind: number;
  row: number;
  x: number;
  hp: number;
  attack: number;
  slowUntil: number;
  hit: number;
}
export interface Bullet {
  id: number;
  row: number;
  x: number;
  damage: number;
  slow: boolean;
  pierce?: number;
  hitIds?: number[];
}
export interface Sun {
  value?: number;
  id: number;
  row: number;
  x: number;
  born: number;
}
export interface Effect {
  id: number;
  row: number;
  x: number;
  kind: "hit" | "plant" | "boom" | "upgrade";
  until: number;
}
export interface Spawn {
  at: number;
  row: number;
  kind: number;
}
export type BattleEventType =
  | "gene"
  | "mutate"
  | "levelup"
  | "stageComplete"
  | "draft"
  | "start"
  | "pause"
  | "resume"
  | "plant"
  | "collect"
  | "shovel"
  | "shoot"
  | "hit"
  | "bite"
  | "boom"
  | "car"
  | "evolve"
  | "fuse"
  | "warning"
  | "finalWave"
  | "danger"
  | "death"
  | "won"
  | "lost";
export interface BattleEvent {
  eventId: number;
  type: BattleEventType;
  time: number;
  row?: number;
  x?: number;
  entityId?: number;
  targetId?: number;
  texture?: string;
  value?: number;
  detail?: string;
}
export class Battle {
  level: number;
  genetic = false;
  rng: Streams;
  endTime: number;
  get cards() {
    return this.genetic
      ? (["sun", "pea", "wall"] as Kind[])
      : LEVELS[this.level].cards;
  }
  getStats(p: Plant) {
    return stats(p, PLANTS[p.kind], this.genetic ? this.plants : []);
  }
  status: "ready" | "running" | "paused" | "won" | "lost" = "ready";
  time = 0;
  sun: number;
  plants: Plant[] = [];
  enemies: Enemy[] = [];
  bullets: Bullet[] = [];
  suns: Sun[] = [];
  effects: Effect[] = [];
  cars = Array.from({ length: 5 }, () => ({ state: "ready", x: -0.65 }));
  cooldowns: Partial<Record<Kind, number>> = {};
  schedule: Spawn[] = [];
  spawned = 0;
  kills = 0;
  nextSun = 4;
  private serial = 0;
  private eventSerial = 0;
  private pendingEvents: BattleEvent[] = [];
  private warned = false;
  private finalWave = false;
  private dangerRows = new Set<number>();
  emit(
    type: BattleEventType,
    data: Omit<BattleEvent, "type" | "time" | "eventId"> = {},
  ) {
    this.pendingEvents.push({
      ...data,
      type,
      time: this.time,
      eventId: ++this.eventSerial,
    });
  }
  drainEvents(): BattleEvent[] {
    const events = this.pendingEvents;
    this.pendingEvents = [];
    return events;
  }
  constructor(level = 0, genetic = false, random = streams(1)) {
    this.genetic = genetic;
    this.rng = random;
    this.level = Math.max(0, Math.min(2, level));
    this.sun = LEVELS[this.level].sun;
    const end = (this.endTime = genetic ? 225 : LEVELS[this.level].end);
    for (let t = 18, i = 0; t <= end; t += this.level === 0 ? 7 : 5, i++) {
      const row = (i * 3 + Math.floor(i / 5)) % 5;
      let kind = 0;
      if (i > 4 && i % 4 === 0) kind = 1;
      if (this.level > 0 && i % 5 === 3) kind = 2;
      if (this.level === 2 && i > 15 && i % 9 === 0) kind = 3;
      this.schedule.push({ at: t, row, kind });
    }
    for (let r = 0; r < 5; r++)
      this.schedule.push({
        at: end + 5 + r * 0.8,
        row: r,
        kind: this.level === 2 ? 1 : 0,
      });
    this.schedule.sort((a, b) => a.at - b.at);
  }
  id() {
    return ++this.serial;
  }
  start() {
    if (this.status === "ready") {
      this.status = "running";
      this.emit("start");
    }
  }
  pause() {
    if (this.status === "running") {
      this.status = "paused";
      this.emit("pause");
    } else if (this.status === "paused") {
      this.status = "running";
      this.emit("resume");
    }
  }
  addEffect(row: number, x: number, kind: Effect["kind"]) {
    this.effects.push({
      id: this.id(),
      row,
      x,
      kind,
      until: this.time + (kind === "boom" ? 0.65 : 0.35),
    });
  }
  placementError(kind: Kind, row: number, col: number): string {
    if (this.status !== "running") return "请先开始或继续游戏";
    if (!this.cards.includes(kind)) return "本关尚未开放这张植物卡";
    if (
      row < 0 ||
      row > 4 ||
      col < 0 ||
      col > 8 ||
      !Number.isInteger(row) ||
      !Number.isInteger(col)
    )
      return "请种在草地上";
    if (this.plants.some((p) => p.row === row && p.col === col))
      return "这个格子已经有植物了";
    const d = PLANTS[kind];
    if (this.sun < d.cost) return "阳光不足，先收集阳光";
    if ((this.cooldowns[kind] ?? 0) > this.time) return "植物卡正在冷却";
    return "";
  }
  place(kind: Kind, row: number, col: number): string {
    const error = this.placementError(kind, row, col);
    if (error) return error;
    const d = PLANTS[kind];
    this.sun -= d.cost;
    this.cooldowns[kind] = this.time + d.cd;
    this.plants.push({
      id: this.id(),
      kind,
      row,
      col,
      hp: d.hp,
      attack: this.time + 0.5,
      production: this.time + (d.produce ?? 0),
      born: this.time,
      ...(this.genetic
        ? {
            genome: [],
            primaryFamily: kind as Family,
            generation: 1,
            xp: 0,
            produced: 0,
            goldenAt: this.time + 20,
          }
        : {}),
    });
    this.addEffect(row, col, "plant");
    this.emit("plant", {
      row,
      x: col,
      entityId: this.plants[this.plants.length - 1].id,
      texture: kind,
    });
    return "";
  }
  collect(id: number) {
    if (this.status !== "running") return false;
    const i = this.suns.findIndex((s) => s.id === id);
    if (i < 0) return false;
    const collected = this.suns[i];
    this.suns.splice(i, 1);
    this.emit("collect", {
      row: collected.row,
      x: collected.x,
      entityId: id,
      value: collected.value ?? 25,
    });
    this.sun = Math.round((this.sun + (collected.value ?? 25)) * 10) / 10;
    return true;
  }
  remove(id: number) {
    if (this.status !== "running") return false;
    const p = this.plants.find((p) => p.id === id);
    if (!p) return false;
    this.plants = this.plants.filter((p) => p.id !== id);
    this.emit("shovel", {
      row: p.row,
      x: p.col,
      entityId: id,
      texture: p.phenotype ?? p.kind,
    });
    return true;
  }
  evolve(id: number, to: Kind): string {
    if (this.status !== "running") return "请先继续游戏";
    if (this.level < 1) return "第二关开放进化";
    const p = this.plants.find((p) => p.id === id);
    if (!p || !(EVOLVE[p.kind] ?? []).includes(to))
      return "这株植物无法进行该进化";
    if (this.sun < PLANTS[to].cost) return "进化需要更多阳光";
    this.sun -= PLANTS[to].cost;
    this.transform(p, to, p.hp / PLANTS[p.kind].hp);
    this.emit("evolve", { row: p.row, x: p.col, entityId: p.id, texture: to });
    return "";
  }
  recipe(
    a: number,
    b: number,
  ): { a: Kind; b: Kind; out: Kind; pool?: string[] } | undefined {
    const p = this.plants.find((p) => p.id === a),
      q = this.plants.find((p) => p.id === b);
    if (!p || !q || a === b) return;
    if (this.genetic) {
      if (
        p.hp <= 0 ||
        q.hp <= 0 ||
        p.mutation === "sterile" ||
        q.mutation === "sterile"
      )
        return;
      const pool = phenotypePool(p.primaryFamily!, q.primaryFamily!);
      return pool.length
        ? {
            a: p.kind,
            b: q.kind,
            out: (pool[0] === "generator" ? "coldwall" : pool[0]) as Kind,
            pool,
          }
        : undefined;
    }
    return RECIPES.find(
      (r) =>
        (r.a === p.kind && r.b === q.kind) ||
        (r.b === p.kind && r.a === q.kind),
    );
  }
  fuse(a: number, b: number): string {
    if (this.status !== "running") return "请先继续游戏";
    if (this.genetic) return this.hybridize(a, b);
    if (this.level < 2) return "第三关开放杂交";
    const r = this.recipe(a, b);
    if (!r) return "材料已失效或不符合配方";
    if (this.sun < PLANTS[r.out].cost) return "融合需要更多阳光";
    const p = this.plants.find((p) => p.id === a)!,
      q = this.plants.find((p) => p.id === b)!;
    const ratio = (p.hp / PLANTS[p.kind].hp + q.hp / PLANTS[q.kind].hp) / 2;
    this.sun -= PLANTS[r.out].cost;
    this.plants = this.plants.filter((p) => p.id !== a);
    this.transform(q, r.out, ratio);
    this.emit("fuse", { row: q.row, x: q.col, entityId: q.id, texture: r.out });
    return "";
  }
  installGene(id: number, gene: Gene): string {
    if (this.status !== "running") return "请先继续战斗";
    const p = this.plants.find((p) => p.id === id && p.hp > 0);
    if (!p) return "材料已失效";
    if (!applicable(gene, this.getStats(p)))
      return "这株植物没有该基因需要的能力";
    if ((p.genome ?? []).length >= 4) return "四个基因槽已满";
    if (p.genome?.includes(gene)) return "不能安装重复基因";
    const ratio = p.hp / this.getStats(p).hp;
    p.genome = [...(p.genome ?? []), gene];
    p.hp = ratio * this.getStats(p).hp;
    this.emit("gene", { entityId: id, row: p.row, x: p.col, detail: gene });
    this.addEffect(p.row, p.col, "upgrade");
    return "";
  }
  evolveGenetic(id: number, choice: Evolution): string {
    const p = this.plants.find((p) => p.id === id && p.hp > 0);
    if (this.status !== "running" || !p) return "植物已失效或战斗暂停";
    if (
      (p.xp ?? 0) < 60 ||
      p.evolution ||
      !p.evolutionChoices?.includes(choice)
    )
      return "需要三级植物和有效进化方向";
    if (this.sun < 100) return "进化需要 100 阳光";
    const ratio = p.hp / this.getStats(p).hp;
    this.sun -= 100;
    p.evolution = choice;
    p.hp = ratio * this.getStats(p).hp;
    this.emit("evolve", { entityId: id, row: p.row, x: p.col, detail: choice });
    this.addEffect(p.row, p.col, "upgrade");
    return "";
  }
  mutate(id: number): string {
    const p = this.plants.find((p) => p.id === id && p.hp > 0);
    if (this.status !== "running" || !p) return "植物已失效或战斗暂停";
    if (p.mutation) return "每株植物只能突变一次";
    if (this.sun < 125) return "突变需要 125 阳光";
    const ratio = p.hp / this.getStats(p).hp;
    const pool = mutationPool(this.getStats(p));
    p.mutation = pool[Math.floor(this.rng.mutation.next() * pool.length)];
    this.sun -= 125;
    p.hp = ratio * this.getStats(p).hp;
    p.goldenAt = this.time + 20;
    this.emit("mutate", {
      entityId: id,
      row: p.row,
      x: p.col,
      detail: p.mutation,
    });
    this.addEffect(p.row, p.col, "upgrade");
    return "";
  }
  hybridize(a: number, b: number): string {
    const r = this.recipe(a, b);
    if (!r || !("pool" in r) || !r.pool) return "材料已失效、主系相同或已不育";
    if (this.status !== "running") return "请先继续战斗";
    if (this.sun < 100) return "杂交需要 100 阳光";
    const p = this.plants.find((p) => p.id === a)!,
      q = this.plants.find((p) => p.id === b)!;
    const ratio = (p.hp / this.getStats(p).hp + q.hp / this.getStats(q).hp) / 2;
    const phenotype = weightedPhenotype(r.pool, this.rng.hybrid.next());
    const genome = this.rng.hybrid.sample(
      [...new Set([...(p.genome ?? []), ...(q.genome ?? [])])],
      4,
    );
    this.sun -= 100;
    this.plants = this.plants.filter((p) => p.id !== a);
    q.kind = (r.pool[0] === "generator" ? "coldwall" : r.pool[0]) as Kind;
    q.phenotype = phenotype;
    q.genome = genome;
    q.generation = Math.max(p.generation ?? 1, q.generation ?? 1) + 1;
    q.xp = 0;
    q.evolution = undefined;
    q.evolutionChoices = undefined;
    q.mutation = undefined;
    q.produced = 0;
    q.born = this.time;
    const d = this.getStats(q);
    q.hp = d.hp * ratio;
    q.attack = this.time + (d.rate ?? 1);
    q.production = this.time + (d.produce ?? 0);
    this.emit("fuse", {
      entityId: q.id,
      row: q.row,
      x: q.col,
      texture: phenotype,
      detail: phenotype,
    });
    this.addEffect(q.row, q.col, "upgrade");
    return "";
  }
  inherit(previous: Battle) {
    this.plants = previous.plants.map((p) => ({
      ...p,
      genome: [...(p.genome ?? [])],
      attack: Math.max(0.1, p.attack - previous.time),
      production: Math.max(0.1, p.production - previous.time),
      born: p.born - previous.time,
      goldenAt: Math.max(
        0.1,
        (p.goldenAt ?? previous.time + 20) - previous.time,
      ),
    }));
    this.sun = previous.sun + 100;
    this.cars = previous.cars.map((c) => ({
      ...c,
      state: c.state === "active" ? "spent" : c.state,
    }));
    this.serial = Math.max(0, ...this.plants.map((p) => p.id));
    for (const p of this.plants)
      p.hp = Math.min(this.getStats(p).hp, p.hp + this.getStats(p).hp * 0.3);
    for (const [kind, time] of Object.entries(previous.cooldowns))
      this.cooldowns[kind as Kind] = Math.max(0, time! - previous.time);
  }
  private transform(p: Plant, to: Kind, ratio: number) {
    p.kind = to;
    p.hp = PLANTS[to].hp * ratio;
    p.attack = this.time + (PLANTS[to].rate ?? 1);
    p.production = this.time + (PLANTS[to].produce ?? 0);
    p.born = this.time;
    this.addEffect(p.row, p.col, "upgrade");
  }
  tick(dt: number) {
    if (this.status !== "running") return;
    this.time += dt;
    if (!this.warned && this.time >= this.endTime - 5) {
      this.warned = true;
      this.emit("warning");
    }
    if (!this.finalWave && this.time >= this.endTime) {
      this.finalWave = true;
      this.emit("finalWave");
    }
    while (
      this.spawned < this.schedule.length &&
      this.schedule[this.spawned].at <= this.time
    ) {
      const s = this.schedule[this.spawned++];
      this.enemies.push({
        id: this.id(),
        kind: s.kind,
        row: s.row,
        x: 9.3,
        hp: ENEMIES[s.kind].hp,
        attack: 0,
        slowUntil: 0,
        hit: 0,
      });
    }
    if (this.time >= this.nextSun) {
      this.suns.push({
        id: this.id(),
        row: (Math.floor(this.time / 8) * 3) % 5,
        x: 1 + ((Math.floor(this.time / 8) * 7) % 7),
        born: this.time,
      });
      this.nextSun = this.time + 8;
    }
    this.suns = this.suns.filter((s) => this.time - s.born < 15);
    for (const p of this.plants) {
      if (p.hp <= 0) continue;
      const d = this.getStats(p);
      if (this.genetic) {
        const oldLevel = 1 + Math.floor((p.xp ?? 0) / 30);
        p.xp = (p.xp ?? 0) + dt;
        if (1 + Math.floor(p.xp / 30) > oldLevel) {
          if (p.xp >= 60 && !p.evolutionChoices)
            p.evolutionChoices = this.rng.evolution.sample(
              Object.keys(EVOLUTIONS) as Evolution[],
              3,
            );
          this.emit("levelup", { entityId: p.id, row: p.row, x: p.col });
        }
        p.hp = Math.min(d.hp, p.hp + d.hp * d.regen * dt);
        if (d.extra === "repair")
          for (const q of this.plants)
            if (
              q.hp > 0 &&
              Math.abs(q.row - p.row) + Math.abs(q.col - p.col) === 1
            )
              q.hp = Math.min(this.getStats(q).hp, q.hp + 3 * dt);
        if (p.mutation === "parasite" && p.hp < d.hp) {
          const q = this.plants.find(
            (q) =>
              q.hp > 0 &&
              q.id !== p.id &&
              Math.abs(q.row - p.row) + Math.abs(q.col - p.col) === 1,
          );
          if (q) {
            const amount = Math.min(
              q.hp,
              this.getStats(q).hp * 0.02 * dt,
              d.hp - p.hp,
            );
            q.hp -= amount;
            p.hp += amount;
          }
        }
        if (p.mutation === "golden" && this.time >= (p.goldenAt ?? 0)) {
          this.suns.push({
            id: this.id(),
            row: p.row,
            x: p.col + 0.2,
            born: this.time,
            value: 25,
          });
          p.goldenAt = this.time + 20;
        }
      }
      if (d.produce && p.production <= this.time) {
        this.suns.push({
          id: this.id(),
          row: p.row,
          x: p.col + 0.22,
          born: this.time,
          value:
            d.yield +
            (((p.produced ?? 0) + 1) % 4 === 0 &&
            (p.genome?.includes("reserve") || d.extra === "store")
              ? 25
              : 0),
        });
        p.produced = (p.produced ?? 0) + 1;
        p.production = this.time + d.produce;
      }
      if (p.kind === "bomb" && this.time - p.born >= 0.5) {
        for (const e of this.enemies)
          if (Math.abs(e.row - p.row) <= 1 && Math.abs(e.x - p.col) <= 1.5)
            e.hp -= 400;
        p.hp = 0;
        this.addEffect(p.row, p.col, "boom");
        this.emit("boom", { row: p.row, x: p.col, entityId: p.id });
      }
      if (
        d.damage &&
        p.attack <= this.time &&
        this.enemies.some((e) => e.hp > 0 && e.row === p.row && e.x > p.col)
      ) {
        this.bullets.push({
          id: this.id(),
          row: p.row,
          x: p.col + 0.38,
          damage:
            d.damage *
            (d.extra === "charge" && (p.produced ?? 0) % 4 === 3 ? 1.5 : 1),
          slow: !!d.slow,
          pierce: d.pierce,
        });
        if (d.pods)
          this.bullets.push({
            id: this.id(),
            row: p.row,
            x: p.col + 0.08,
            damage: d.damage,
            slow: !!d.slow,
            pierce: d.pierce,
          });
        if (d.split && this.rng.combat.next() < 0.25)
          this.bullets.push({
            id: this.id(),
            row: p.row,
            x: p.col - 0.08,
            damage: d.damage * 0.5,
            slow: !!d.slow,
            pierce: d.pierce,
          });
        p.attack = this.time + (d.rate ?? 1);
        this.emit("shoot", { row: p.row, x: p.col, entityId: p.id });
      }
    }
    for (const e of this.enemies) {
      if (e.hp <= 0) continue;
      const d = ENEMIES[e.kind];
      const target = this.plants
        .filter((p) => p.hp > 0 && p.row === e.row && p.col <= e.x + 0.2)
        .sort((a, b) => b.col - a.col)[0];
      const move = d.speed * (e.slowUntil > this.time ? 0.7 : 1) * dt;
      if (target && e.x - move <= target.col + 0.55) {
        e.x = Math.max(target.col + 0.55, e.x - move);
        if (e.attack <= this.time) {
          const defense = this.getStats(target);
          const dealt = d.damage * (1 - defense.reduction);
          target.hp -= dealt;
          e.hp -= dealt * defense.thorns;
          if (defense.extra === "counter") e.hp -= 20;
          this.emit("bite", {
            row: e.row,
            x: e.x,
            entityId: e.id,
            targetId: target.id,
          });
          e.attack = this.time + 1;
          if (target.kind === "thorn") e.hp -= 25;
          if (target.kind === "coldwall") e.slowUntil = this.time + 2;
        }
      } else e.x -= move;
      if (e.x < 1.5 && !this.dangerRows.has(e.row)) {
        this.dangerRows.add(e.row);
        this.emit("danger", { row: e.row, x: e.x });
      }
      const car = this.cars[e.row];
      if (e.x < -0.35 && car.state === "ready") {
        car.state = "active";
        car.x = -0.65;
        this.emit("car", { row: e.row, x: car.x });
      }
    }
    for (const b of this.bullets) {
      const next = b.x + 5 * dt;
      const e = this.enemies
        .filter(
          (e) =>
            e.hp > 0 &&
            e.row === b.row &&
            !b.hitIds?.includes(e.id) &&
            e.x + 0.24 >= b.x &&
            e.x - 0.24 <= next,
        )
        .sort((a, b) => a.x - b.x)[0];
      if (e) {
        e.hp -= b.damage;
        e.hit = this.time + 0.12;
        if (b.slow) e.slowUntil = this.time + 2;
        this.addEffect(e.row, e.x, "hit");
        this.emit("hit", { row: e.row, x: e.x, entityId: e.id });
        if (b.pierce) {
          b.pierce--;
          b.hitIds = [...(b.hitIds ?? []), e.id];
          b.damage *= 0.5;
          b.x = e.x;
        } else b.x = 20;
      } else b.x = next;
    }
    this.cars.forEach((c, r) => {
      if (c.state === "active") {
        const next = c.x + 7 * dt;
        for (const e of this.enemies)
          if (e.row === r && e.x >= c.x - 0.5 && e.x <= next + 0.5) e.hp = 0;
        c.x = next;
        if (c.x > 10) c.state = "spent";
      }
    });
    for (const e of this.enemies)
      if (e.hp <= 0) {
        this.kills++;
        this.emit("death", {
          row: e.row,
          x: e.x,
          entityId: e.id,
          texture: "enemy" + e.kind,
        });
      }
    for (const p of this.plants)
      if (p.hp <= 0)
        this.emit("death", {
          row: p.row,
          x: p.col,
          entityId: p.id,
          texture: p.phenotype ?? p.kind,
        });
    this.enemies = this.enemies.filter((e) => e.hp > 0);
    this.plants = this.plants.filter((p) => p.hp > 0);
    this.bullets = this.bullets.filter((b) => b.x < 10);
    this.effects = this.effects.filter((e) => e.until > this.time);
    if (this.enemies.some((e) => e.x < -1)) {
      this.status = "lost";
      this.emit("lost");
    } else if (
      this.spawned === this.schedule.length &&
      this.enemies.length === 0
    ) {
      this.status = "won";
      if (this.genetic) this.emit("stageComplete");
      this.emit("won");
    }
  }
}
