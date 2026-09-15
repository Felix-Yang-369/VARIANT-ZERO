import { test } from "node:test";
import assert from "node:assert/strict";
import { Battle, PLANTS, type Kind } from "../src/model";
const running = (level = 0) => {
  const b = new Battle(level);
  b.start();
  return b;
};
function advance(b: Battle, seconds: number) {
  for (let i = 0; i < seconds * 60; i++) b.tick(1 / 60);
}
test("invalid placement is atomic and cooldown is not consumed", () => {
  const b = running();
  assert.ok(b.place("pea", -1, 0));
  assert.equal(b.sun, 175);
  assert.equal(b.place("pea", 0, 0), "");
  const balance = b.sun;
  assert.ok(b.place("pea", 0, 0));
  assert.equal(b.sun, balance);
  assert.ok(b.place("pea", 0, 1));
  assert.equal(b.plants.length, 1);
});
test("sun cannot be collected twice; pause freezes all model time", () => {
  const b = running();
  advance(b, 5);
  const id = b.suns[0].id;
  const old = b.sun;
  assert.ok(b.collect(id));
  assert.equal(b.collect(id), false);
  assert.equal(b.sun, old + 25);
  b.pause();
  const before = JSON.stringify(b);
  advance(b, 10);
  assert.equal(JSON.stringify(b), before);
});
test("evolution and fusion preserve health ratios and reject recursive use", () => {
  const b = running(2);
  b.sun = 2000;
  b.place("pea", 0, 0);
  const p = b.plants[0];
  p.hp = 50;
  assert.equal(b.evolve(p.id, "double"), "");
  assert.equal(p.hp, PLANTS.double.hp / 2);
  assert.ok(b.evolve(p.id, "frost"));
  b.place("sun", 1, 0);
  b.place("wall", 1, 1);
  assert.ok(b.fuse(b.plants[1].id, b.plants[2].id));
  advance(b, 6);
  b.place("pea", 1, 2);
  const q = b.plants[3],
    s = b.plants[1];
  s.hp = 50;
  q.hp = 100;
  const money = b.sun;
  assert.equal(b.fuse(s.id, q.id), "");
  assert.equal(q.kind, "solar");
  assert.equal(q.hp, PLANTS.solar.hp * 0.75);
  assert.equal(q.col, 2);
  assert.equal(b.sun, money - 50);
  assert.ok(b.fuse(s.id, q.id));
  assert.ok(b.fuse(p.id, q.id));
});
test("fusion refuses stale material without spending", () => {
  const b = running(2);
  b.sun = 1000;
  b.place("sun", 0, 0);
  b.place("pea", 0, 1);
  const [a, c] = b.plants;
  assert.ok(b.recipe(a.id, c.id));
  b.remove(a.id);
  const before = b.sun;
  assert.ok(b.fuse(a.id, c.id));
  assert.equal(b.sun, before);
  assert.equal(b.plants.length, 1);
});
test("swept bullet collision hits only first enemy", () => {
  const b = running();
  b.schedule = [{ at: 999, row: 0, kind: 0 }];
  b.enemies = [1, 2].map((id, i) => ({
    id,
    kind: 0,
    row: 0,
    x: 2 + i * 0.5,
    hp: 100,
    attack: 0,
    slowUntil: 0,
    hit: 0,
  }));
  b.bullets = [{ id: 3, row: 0, x: 1, damage: 20, slow: true }];
  b.tick(0.5);
  assert.equal(b.enemies[0].hp, 80);
  assert.equal(b.enemies[1].hp, 100);
  assert.equal(b.bullets.length, 0);
  assert.ok(b.enemies[0].slowUntil > b.time);
});
test("one-time defense car works and exhausted lane can lose", () => {
  const b = running();
  b.schedule = [{ at: 999, row: 0, kind: 0 }];
  const enemy = (id: number) => ({
    id,
    kind: 0,
    row: 0,
    x: -0.4,
    hp: 100,
    attack: 0,
    slowUntil: 0,
    hit: 0,
  });
  b.enemies = [enemy(1)];
  advance(b, 0.2);
  assert.equal(b.enemies.length, 0);
  assert.equal(b.cars[0].state, "active");
  advance(b, 2);
  assert.equal(b.cars[0].state, "spent");
  b.enemies = [{ ...enemy(2), x: -1.01 }];
  b.tick(1 / 60);
  assert.equal(b.status, "lost");
});
test("victory waits for last enemy and terminal state is stable", () => {
  const b = running();
  b.schedule = [];
  b.enemies = [
    { id: 1, kind: 0, row: 0, x: 3, hp: 1, attack: 0, slowUntil: 0, hit: 0 },
  ];
  b.tick(1 / 60);
  assert.equal(b.status, "running");
  b.enemies[0].hp = 0;
  b.tick(1 / 60);
  assert.equal(b.status, "won");
  const before = JSON.stringify(b);
  advance(b, 3);
  assert.equal(JSON.stringify(b), before);
});
test("fresh battle resets all temporary state", () => {
  const b = running();
  b.place("pea", 0, 0);
  advance(b, 30);
  const fresh = new Battle();
  assert.equal(fresh.time, 0);
  assert.equal(fresh.plants.length, 0);
  assert.equal(fresh.cooldowns.pea, undefined);
  assert.ok(fresh.cars.every((c) => c.state === "ready"));
});
for (let level = 0; level < 3; level++)
  test(`level ${level + 1} can be won with legal resources and card cooldowns`, () => {
    const b = running(level);
    let nextDecision = 0;
    for (let step = 0; step < 360 * 60 && b.status === "running"; step++) {
      b.tick(1 / 60);
      for (const s of [...b.suns]) b.collect(s.id);
      if (b.time < nextDecision) continue;
      nextDecision = b.time + 0.15;
      const empty = (r: number, c: number) =>
        !b.plants.some((p) => p.row === r && p.col === c);
      let acted = false;
      const tryPlace = (k: Kind, r: number, c: number) => {
        if (!acted && empty(r, c) && !b.place(k, r, c)) acted = true;
      };
      // Establish production, then protect the most advanced threat first.
      if (b.plants.filter((p) => p.kind === "sun").length < 5) {
        for (let r = 0; r < 5; r++) tryPlace("sun", r, 0);
      }
      for (const e of [...b.enemies].sort((a, c) => a.x - c.x)) {
        if (!b.plants.some((p) => p.row === e.row && PLANTS[p.kind].damage))
          tryPlace("pea", e.row, 1);
      }
      for (let r = 0; r < 5; r++) tryPlace("pea", r, 1);
      for (let c = 2; c < 5; c++)
        for (let r = 0; r < 5; r++) tryPlace("pea", r, c);
      for (let r = 0; r < 5; r++) tryPlace("wall", r, 6);
    }
    assert.equal(
      b.status,
      "won",
      `level=${level} time=${b.time} kills=${b.kills} resources=${b.sun}`,
    );
  });
