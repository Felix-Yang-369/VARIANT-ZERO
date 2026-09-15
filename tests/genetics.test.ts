import { test } from "node:test";
import assert from "node:assert/strict";
import { Battle, PLANTS, type Kind } from "../src/model";
import { RunState } from "../src/run";
import {
  GENES,
  GENE_IDS,
  PHENOTYPES,
  streams,
  weightedPhenotype,
  mutationPool,
  type Gene,
} from "../src/genetics";
import { defaults, loadSave, saveData } from "../src/settings";
const make = () => {
  const b = new Battle(0, true, streams(123));
  b.start();
  b.sun = 3000;
  b.schedule = [{ at: 9999, row: 0, kind: 0 }];
  b.place("sun", 0, 0);
  b.place("pea", 0, 1);
  b.place("wall", 0, 2);
  b.drainEvents();
  return b;
};
const tick = (b: Battle, seconds: number) => {
  for (let i = 0; i < seconds * 60; i++) {
    b.tick(1 / 60);
    b.drainEvents();
  }
};
test("four slots, applicability, duplicate and sample consumption are atomic", () => {
  const r = new RunState(4);
  r.battle = make();
  r.phase = "battle";
  r.samples = ["frost", "health", "fast", "regen", "power", "split"];
  const b = r.battle,
    p = b.plants[1];
  assert.ok(r.install(0, b.plants[0].id));
  assert.equal(r.samples.length, 6);
  assert.equal(r.install(0, p.id), "");
  assert.equal(r.samples.length, 5);
  assert.equal(b.installGene(p.id, "frost").includes("重复"), true);
  for (const g of ["health", "fast", "regen"] as Gene[])
    assert.equal(b.installGene(p.id, g), "");
  assert.ok(b.installGene(p.id, "power"));
  assert.equal(p.genome!.length, 4);
  const before = b.sun;
  assert.ok(r.install(0, 9999));
  assert.equal(b.sun, before);
});
test("exact probability boundaries and all nine phenotypes", () => {
  assert.equal(Object.keys(PHENOTYPES).length, 9);
  assert.deepEqual(
    [0, 0.59999, 0.6, 0.89999, 0.9, 0.9999].map((r) =>
      weightedPhenotype(["a", "b", "c"], r),
    ),
    ["a", "a", "b", "b", "c", "c"],
  );
});
test("hybrid previews consume no randomness; inheritance ratios and recursive primary family", () => {
  const b = make(),
    [a, q, w] = b.plants;
  a.genome = ["health", "regen", "fast"];
  q.genome = ["health", "power", "frost", "split"];
  a.hp = b.getStats(a).hp * 0.5;
  q.hp = b.getStats(q).hp;
  const state = b.rng.hybrid.state;
  for (let i = 0; i < 10; i++) assert.ok(b.recipe(a.id, q.id));
  assert.equal(b.rng.hybrid.state, state);
  assert.equal(b.fuse(a.id, q.id), "");
  assert.equal(q.primaryFamily, "pea");
  assert.equal(q.generation, 2);
  assert.equal(q.col, 1);
  assert.equal(q.hp, b.getStats(q).hp * 0.75);
  assert.equal(q.genome!.length, 4);
  assert.equal(new Set(q.genome).size, 4);
  assert.equal(b.fuse(q.id, w.id), "");
  assert.equal(w.primaryFamily, "wall");
  assert.equal(w.generation, 3);
  assert.equal(w.xp, 0);
  assert.equal(w.evolution, undefined);
  assert.equal(w.mutation, undefined);
  const rng = b.rng.hybrid.state,
    money = b.sun;
  assert.ok(b.fuse(a.id, w.id));
  assert.equal(b.sun, money);
  assert.equal(b.rng.hybrid.state, rng);
});
test("experience pauses, evolution choices persist and mutation risks exclude invalid capabilities", () => {
  const b = make(),
    p = b.plants[1];
  tick(b, 61);
  assert.ok(p.xp! >= 60);
  assert.equal(p.evolutionChoices?.length, 3);
  const choices = [...p.evolutionChoices!];
  b.pause();
  const xp = p.xp;
  tick(b, 60);
  assert.equal(p.xp, xp);
  b.pause();
  assert.deepEqual(p.evolutionChoices, choices);
  const before = b.sun;
  assert.equal(b.evolveGenetic(p.id, choices[0]), "");
  assert.equal(b.sun, before - 100);
  assert.ok(b.evolveGenetic(p.id, choices[1]));
  assert.equal(mutationPool(b.getStats(p)).length, 6);
  assert.equal(mutationPool(b.getStats(b.plants[0])).length, 5);
  assert.equal(mutationPool(b.getStats(b.plants[2])).length, 4);
  assert.equal(b.mutate(p.id), "");
  const after = b.sun,
    state = b.rng.mutation.state;
  assert.ok(b.mutate(p.id));
  assert.equal(b.sun, after);
  assert.equal(b.rng.mutation.state, state);
  p.mutation = "sterile";
  assert.equal(b.recipe(b.plants[0].id, p.id), undefined);
});
test("draft streams independent and unlocks only affect next expedition", () => {
  const a = new RunState(9),
    b = new RunState(9, ["reserve"]);
  assert.equal(a.pool.length, 8);
  assert.equal(b.pool.length, 9);
  const c = new RunState(9);
  a.rng.combat.next();
  assert.deepEqual(a.choices, c.choices);
  while (a.phase === "draft") {
    assert.equal(new Set(a.choices).size, 3);
    a.pick(a.choices[0]);
  }
  assert.equal(a.samples.length, 3);
  assert.equal(a.pick("fast"), false);
});
test("cross-stage carry retains health, timers and car use; settlement is idempotent", () => {
  const r = new RunState(23),
    save = defaults();
  while (r.phase === "draft") r.pick(r.choices[0]);
  r.battle = make();
  const p = r.battle.plants[1];
  p.genome = ["health"];
  p.hp = r.battle.getStats(p).hp * 0.4;
  p.mutation = "golden";
  p.xp = 47;
  p.generation = 3;
  const before = r.battle.getStats(p).hp;
  r.battle.time = 100;
  p.attack = 101;
  p.production = 105;
  r.battle.cars[2].state = "spent";
  r.battle.status = "won";
  const money = r.battle.sun;
  assert.ok(r.settle(save));
  assert.equal(r.settle(save), false);
  assert.ok(r.next());
  assert.equal(r.next(), false);
  const q = r.battle.plants.find((q) => q.id === p.id)!;
  assert.equal(q.hp, before * 0.7);
  assert.equal(q.attack, 1);
  assert.equal(q.production, 5);
  assert.equal(q.xp, 47);
  assert.equal(q.generation, 3);
  assert.equal(q.mutation, "golden");
  assert.equal(r.battle.cars[2].state, "spent");
  assert.equal(r.battle.sun, money + 100);
  while (r.phase === "draft") r.pick(r.choices[0]);
  r.battle.status = "lost";
  r.settle(save);
  r.settle(save);
  assert.equal(save.records.runs, 1);
});
test("v2 migration preserves all settings and v3 stores no run snapshot", () => {
  const old = {
    version: 2,
    unlocked: 2,
    settings: {
      musicVolume: 0.33,
      sfxVolume: 0.72,
      muted: true,
      reducedMotion: true,
      tutorials: [1, 2],
    },
  };
  const data = new Map([["garden-guardians-v2", JSON.stringify(old)]]);
  const port = {
    getItem: (k: string) => data.get(k) ?? null,
    setItem: (k: string, v: string) => {
      data.set(k, v);
    },
  };
  const save = loadSave(port).data;
  assert.equal(save.version, 3);
  assert.deepEqual(save.settings, old.settings);
  assert.equal(save.unlocked, 2);
  save.genes = ["reserve"];
  save.discoveries = ["plant:solar"];
  saveData(port, save);
  assert.deepEqual(loadSave(port).data, save);
  assert.ok(!JSON.stringify(save).includes("battle"));
});
test("gene damage, production, regen and pierce actually affect combat", () => {
  const b = make(),
    [s, p, w] = b.plants;
  b.installGene(s.id, "photo");
  b.installGene(s.id, "reserve");
  s.produced = 3;
  s.production = 0;
  b.installGene(p.id, "power");
  b.installGene(p.id, "frost");
  b.installGene(p.id, "pierce");
  b.installGene(w.id, "regen");
  w.hp = 100;
  b.enemies = [0, 1].map((x) => ({
    id: 100 + x,
    kind: 0,
    row: 0,
    x: 3 + x,
    hp: 100,
    attack: 999,
    slowUntil: 0,
    hit: 0,
  }));
  p.attack = 0;
  b.tick(0.1);
  assert.equal(b.suns.find((s) => s.value !== undefined)?.value, 62.5);
  assert.ok(w.hp > 100);
  tick(b, 1);
  assert.ok(b.enemies[0].hp < 100);
  assert.ok(b.enemies[1].hp < 100);
  assert.ok(b.enemies[0].slowUntil > b.time);
});
for (const [seed, build] of [
  [1, "damage"],
  [42, "survival"],
  [731, "hybrid"],
] as const)
  test(`three-stage legal expedition ${build} seed ${seed}`, () => {
    const r = new RunState(seed, GENE_IDS),
      save = defaults();
    let experiments = 0;
    for (let stage = 0; stage < 3; stage++) {
      while (r.phase === "draft") r.pick(r.choices[0]);
      const b = r.battle;
      b.start();
      let next = 0;
      for (let i = 0; i < 360 * 60 && b.status === "running"; i++) {
        b.tick(1 / 60);
        b.drainEvents();
        for (const s of [...b.suns]) b.collect(s.id);
        if (b.time < next) continue;
        next = b.time + 0.15;
        for (let n = r.samples.length - 1; n >= 0; n--) {
          const g = r.samples[n];
          for (const p of b.plants) if (!r.install(n, p.id)) break;
        }
        let acted = false;
        const place = (k: Kind, row: number, col: number) => {
          if (
            !acted &&
            !b.plants.some((p) => p.row === row && p.col === col) &&
            !b.place(k, row, col)
          )
            acted = true;
        };
        if (b.plants.filter((p) => p.kind === "sun").length < 5)
          for (let row = 0; row < 5; row++) place("sun", row, 0);
        for (const e of [...b.enemies].sort((a, c) => a.x - c.x))
          if (!b.plants.some((p) => p.row === e.row && b.getStats(p).damage))
            place("pea", e.row, 1);
        for (let col = 1; col < 5; col++)
          for (let row = 0; row < 5; row++) place("pea", row, col);
        for (let row = 0; row < 5; row++) place("wall", row, 6);
        if (b.sun > 350)
          for (const p of b.plants) {
            if (!p.evolution && p.evolutionChoices) {
              const choice = build === "survival" ? "tough" : "expert";
              b.evolveGenetic(
                p.id,
                p.evolutionChoices.includes(choice)
                  ? choice
                  : p.evolutionChoices[0],
              );
              break;
            }
          }
        if (
          build === "hybrid" &&
          b.time > 150 &&
          experiments < 3 &&
          b.sun > 300
        ) {
          const a = b.plants.find((p) => p.kind === "sun"),
            q = b.plants.find((p) => p.kind === "pea" && p.col === 1);
          if (a && q && !b.fuse(a.id, q.id)) experiments++;
        }
      }
      assert.equal(
        b.status,
        "won",
        `stage ${stage}, time ${b.time}, sun ${b.sun}, kills ${b.kills}`,
      );
      assert.ok(r.settle(save));
      if (stage < 2) assert.ok(r.next());
    }
    if (process.env.REPORT_RUNS)
      console.log(
        JSON.stringify({ seed, build, seconds: r.elapsed, kills: r.kills }),
      );
    assert.equal(r.phase, "won");
    assert.equal(save.records.wins, 1);
    assert.ok(r.elapsed >= 690 && r.elapsed < 1080, `duration ${r.elapsed}`);
    if (build === "hybrid") assert.equal(experiments, 3);
  });
