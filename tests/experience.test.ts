import { test } from "node:test";
import assert from "node:assert/strict";
import { Battle, PLANTS } from "../src/model";
import { Interaction } from "../src/interaction";
import {
  loadSave,
  parseSave,
  saveData,
  defaults,
  SAVE_KEY,
} from "../src/settings";
import { AudioManager, type SoundName } from "../src/audio";
test("events drain once without changing game state and terminal event is emitted once", () => {
  const b = new Battle();
  b.start();
  b.place("pea", 0, 0);
  const hp = b.plants[0].hp,
    money = b.sun;
  const events = b.drainEvents();
  assert.deepEqual(
    events.map((e) => e.type),
    ["start", "plant"],
  );
  assert.equal(new Set(events.map((e) => e.eventId)).size, 2);
  assert.deepEqual(b.drainEvents(), []);
  assert.equal(b.plants[0].hp, hp);
  assert.equal(b.sun, money);
  b.schedule = [];
  b.tick(1 / 60);
  assert.equal(b.drainEvents().filter((e) => e.type === "won").length, 1);
  b.tick(100);
  assert.deepEqual(b.drainEvents(), []);
});
test("wave and lane danger events are emitted once", () => {
  const b = new Battle();
  b.start();
  b.drainEvents();
  b.time = 119.99;
  b.enemies = [
    {
      id: 777,
      kind: 0,
      row: 2,
      x: 1.4,
      hp: 100,
      attack: 999,
      slowUntil: 0,
      hit: 0,
    },
  ];
  b.tick(0.02);
  const events = b.drainEvents();
  assert.equal(events.filter((e) => e.type === "warning").length, 1);
  assert.equal(events.filter((e) => e.type === "danger").length, 1);
  b.tick(0.02);
  assert.ok(
    !b.drainEvents().some((e) => e.type === "warning" || e.type === "danger"),
  );
});
test("tools take precedence over sun; fusion invalidation cannot charge resources", () => {
  const b = new Battle(2),
    i = new Interaction();
  b.start();
  b.sun = 1000;
  i.choose("sun");
  assert.equal(i.canCollect, false);
  i.click(b, 0, 0);
  i.choose("pea");
  i.click(b, 0, 1);
  assert.equal(i.canCollect, true);
  i.tool("fusion");
  assert.equal(i.canCollect, false);
  i.click(b, 0, 0);
  i.click(b, 0, 1);
  b.remove(b.plants[0].id);
  const money = b.sun;
  i.reconcile(b);
  assert.deepEqual(i.state, { type: "fusion", first: null, second: null });
  assert.ok(i.confirm(b));
  assert.equal(b.sun, money);
});
test("failed planting retains selection; successful planting clears it", () => {
  const b = new Battle(),
    i = new Interaction();
  b.start();
  i.choose("pea");
  b.sun = 0;
  assert.ok(i.click(b, 0, 0));
  assert.deepEqual(i.state, { type: "plant", kind: "pea" });
  b.sun = 100;
  assert.equal(i.click(b, 0, 0), "");
  assert.equal(i.state.type, "browse");
});
test("legacy save retains unlock and mute; volume defaults and reduced motion migrate", () => {
  const data = loadSave(
    {
      getItem: (k) =>
        k === "garden-guardians-v1"
          ? JSON.stringify({ version: 1, unlocked: 2, muted: true })
          : null,
      setItem() {},
    },
    true,
  );
  assert.equal(data.data.unlocked, 2);
  assert.deepEqual(data.data.settings, {
    musicVolume: 0.25,
    sfxVolume: 0.6,
    muted: true,
    reducedMotion: true,
    tutorials: [],
  });
  const malformed = parseSave({
    version: 2,
    unlocked: 99,
    settings: { musicVolume: 4, sfxVolume: -2, tutorials: [0, 0, 2, 8, "1"] },
  });
  assert.equal(malformed.settings.musicVolume, 1);
  assert.equal(malformed.settings.sfxVolume, 0);
  assert.deepEqual(malformed.settings.tutorials, [0, 2]);
});
test("corrupt or unavailable storage safely falls back; settings roundtrip", () => {
  const store = new Map<string, string>([
    [SAVE_KEY, "broken"],
    ["garden-guardians-v1", '{"unlocked":1,"muted":true}'],
  ]);
  const port = {
    getItem: (k: string) => store.get(k) ?? null,
    setItem: (k: string, v: string) => {
      store.set(k, v);
    },
  };
  assert.equal(loadSave(port).data.unlocked, 1);
  const d = defaults();
  d.settings.tutorials = [1, 2];
  d.settings.musicVolume = 0.4;
  assert.ok(saveData(port, d));
  assert.deepEqual(loadSave(port).data, d);
  assert.equal(
    saveData(
      {
        getItem: () => null,
        setItem: () => {
          throw Error();
        },
      },
      d,
    ),
    false,
  );
});
class FakeSource {
  onended: (() => void) | null = null;
  buffer: unknown;
  connected = false;
  stopped = false;
  connect() {
    this.connected = true;
  }
  disconnect() {
    this.connected = false;
  }
  start() {}
  stop() {
    this.stopped = true;
    this.onended?.();
  }
}
class FakeContext {
  currentTime = 10;
  sampleRate = 1000;
  destination = {};
  state = "running";
  sources: FakeSource[] = [];
  createGain() {
    return {
      gain: { value: 0, setTargetAtTime() {} },
      connect() {},
      disconnect() {},
    };
  }
  createBuffer(_c: number, length: number) {
    return { getChannelData: () => new Float32Array(length) };
  }
  createBufferSource() {
    const source = new FakeSource();
    this.sources.push(source);
    return source;
  }
  async resume() {}
  async close() {
    this.state = "closed";
  }
}
test("audio starts only after unlock; merged effects cap at 12 with priority replacement", async () => {
  const ctx = new FakeContext(),
    a = new AudioManager(
      defaults().settings,
      () => ctx as unknown as AudioContext,
    );
  assert.equal(a.play("shoot"), false);
  await a.unlock();
  assert.equal(a.play("shoot"), true);
  assert.equal(a.play("shoot"), false);
  for (let n = 0; n < 11; n++) {
    ctx.currentTime += 0.1;
    assert.equal(a.play("shoot"), true);
  }
  assert.equal(a.debug().activeSfx, 12);
  ctx.currentTime += 0.2;
  assert.equal(a.play("shoot"), false);
  assert.equal(a.play("boom"), true);
  assert.equal(a.debug().activeSfx, 12);
  assert.ok(ctx.sources[0].stopped);
});
test("pause stops voices and freezes music position; reset clears transport and old notes", async () => {
  const ctx = new FakeContext(),
    a = new AudioManager(
      defaults().settings,
      () => ctx as unknown as AudioContext,
    );
  await a.unlock();
  a.setRunning(true);
  a.update(2, false);
  a.play("car");
  assert.ok(a.debug().activeMusic > 0);
  const pos = a.debug().step;
  a.setRunning(false);
  assert.equal(a.debug().activeSfx, 0);
  assert.equal(a.debug().activeMusic, 0);
  a.update(5, true);
  assert.equal(a.debug().step, pos);
  a.setRunning(true);
  a.update(2.25, true);
  assert.notEqual(a.debug().step, pos);
  a.reset();
  assert.equal(a.debug().step, -1);
  assert.equal(a.debug().activeMusic, 0);
  assert.equal(a.debug().activeSfx, 0);
  assert.ok(ctx.sources.every((s) => s.stopped));
});
