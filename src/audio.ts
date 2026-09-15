import type { Preferences } from "./settings";
import type { BattleEventType } from "./model";
export type SoundName = BattleEventType | "select" | "invalid";
type Sound = {
  duration: number;
  freq: number;
  end: number;
  wave: "sine" | "square" | "triangle";
  noise?: number;
  notes?: number[];
  priority: number;
};
export const SOUNDS: Partial<Record<SoundName, Sound>> = {
  draft: {
    duration: 0.5,
    freq: 550,
    end: 880,
    wave: "sine",
    notes: [1, 1.25, 1.5],
    priority: 3,
  },
  gene: {
    duration: 0.6,
    freq: 330,
    end: 990,
    wave: "triangle",
    notes: [1, 1.5, 2],
    priority: 3,
  },
  mutate: {
    duration: 0.9,
    freq: 180,
    end: 700,
    wave: "sine",
    noise: 0.2,
    notes: [1, 1.4, 0.8, 2],
    priority: 4,
  },
  select: {
    duration: 0.09,
    freq: 640,
    end: 800,
    wave: "triangle",
    priority: 1,
  },
  invalid: {
    duration: 0.12,
    freq: 160,
    end: 110,
    wave: "triangle",
    priority: 1,
  },
  plant: {
    duration: 0.23,
    freq: 190,
    end: 440,
    wave: "sine",
    noise: 0.15,
    priority: 2,
  },
  collect: {
    duration: 0.3,
    freq: 880,
    end: 1200,
    wave: "sine",
    notes: [1, 1.25, 1.5],
    priority: 2,
  },
  shovel: {
    duration: 0.2,
    freq: 160,
    end: 70,
    wave: "triangle",
    noise: 0.55,
    priority: 2,
  },
  shoot: { duration: 0.11, freq: 480, end: 130, wave: "triangle", priority: 0 },
  hit: {
    duration: 0.09,
    freq: 210,
    end: 65,
    wave: "sine",
    noise: 0.5,
    priority: 0,
  },
  bite: {
    duration: 0.14,
    freq: 120,
    end: 65,
    wave: "square",
    noise: 0.35,
    priority: 0,
  },
  boom: {
    duration: 0.75,
    freq: 100,
    end: 24,
    wave: "sine",
    noise: 0.8,
    priority: 4,
  },
  car: {
    duration: 1.1,
    freq: 85,
    end: 180,
    wave: "square",
    noise: 0.3,
    priority: 4,
  },
  evolve: {
    duration: 0.7,
    freq: 440,
    end: 600,
    wave: "triangle",
    notes: [1, 1.25, 1.5, 2],
    priority: 3,
  },
  fuse: {
    duration: 0.85,
    freq: 330,
    end: 660,
    wave: "sine",
    notes: [1, 1.5, 2, 2.5],
    noise: 0.08,
    priority: 3,
  },
  warning: {
    duration: 0.8,
    freq: 440,
    end: 440,
    wave: "square",
    notes: [1, 0.75, 1, 0.75],
    priority: 5,
  },
  danger: { duration: 0.3, freq: 220, end: 330, wave: "triangle", priority: 4 },
  won: {
    duration: 1.5,
    freq: 440,
    end: 660,
    wave: "triangle",
    notes: [1, 1.25, 1.5, 2, 2],
    priority: 5,
  },
  lost: {
    duration: 1.2,
    freq: 330,
    end: 110,
    wave: "triangle",
    notes: [1, 0.9, 0.75, 0.5],
    priority: 5,
  },
  start: {
    duration: 0.35,
    freq: 330,
    end: 550,
    wave: "triangle",
    notes: [1, 1.5, 2],
    priority: 3,
  },
};
export class AudioManager {
  private ctx?: AudioContext;
  private sfx?: GainNode;
  private music?: GainNode;
  private voices = new Map<AudioBufferSourceNode, { priority: number }>();
  private musicVoices = new Set<AudioBufferSourceNode>();
  private cache = new Map<string, AudioBuffer>();
  private last = new Map<string, number>();
  private step = -1;
  private running = false;
  private unlocked = false;
  private prefs: Preferences;
  failed = false;
  constructor(
    prefs: Preferences,
    private factory: () => AudioContext = () => new AudioContext(),
  ) {
    this.prefs = prefs;
  }
  async unlock() {
    try {
      if (!this.ctx) {
        this.ctx = this.factory();
        this.sfx = this.ctx.createGain();
        this.music = this.ctx.createGain();
        this.sfx.connect(this.ctx.destination);
        this.music.connect(this.ctx.destination);
        this.apply(this.prefs);
      }
      await this.ctx.resume();
      this.unlocked = true;
      this.failed = false;
    } catch {
      this.failed = true;
    }
  }
  apply(prefs: Preferences) {
    this.prefs = { ...prefs };
    const t = this.ctx?.currentTime ?? 0;
    this.sfx?.gain.setTargetAtTime(prefs.muted ? 0 : prefs.sfxVolume, t, 0.02);
    this.music?.gain.setTargetAtTime(
      prefs.muted ? 0 : prefs.musicVolume,
      t,
      0.02,
    );
  }
  private buffer(key: string, s: Sound) {
    let b = this.cache.get(key);
    if (b) return b;
    const ctx = this.ctx!;
    b = ctx.createBuffer(
      1,
      Math.ceil(ctx.sampleRate * s.duration),
      ctx.sampleRate,
    );
    const data = b.getChannelData(0);
    let phase = 0,
      seed = 12345;
    for (let i = 0; i < data.length; i++) {
      const t = i / ctx.sampleRate,
        p = t / s.duration,
        idx = Math.min(
          (s.notes?.length ?? 1) - 1,
          Math.floor(p * (s.notes?.length ?? 1)),
        ),
        note = s.notes?.[idx] ?? 1;
      const local = s.notes ? (p * s.notes.length) % 1 : p;
      const freq = (s.freq + (s.end - s.freq) * p) * note;
      phase += freq / ctx.sampleRate;
      const sin = Math.sin(phase * Math.PI * 2),
        wave =
          s.wave === "sine"
            ? sin
            : s.wave === "square"
              ? Math.tanh(sin * 3)
              : (2 / Math.PI) * Math.asin(sin);
      seed = (seed * 1664525 + 1013904223) >>> 0;
      const noise = seed / 2147483648 - 1;
      const env = Math.min(1, local * 35) * Math.pow(1 - local, 1.7);
      data[i] =
        (wave * (1 - (s.noise ?? 0)) + noise * (s.noise ?? 0)) * env * 0.27;
    }
    this.cache.set(key, b);
    return b;
  }
  play(name: SoundName) {
    const d = SOUNDS[name];
    if (
      !d ||
      !this.ctx ||
      !this.unlocked ||
      this.prefs.muted ||
      this.prefs.sfxVolume === 0
    )
      return false;
    const t = this.ctx.currentTime,
      interval =
        name === "shoot"
          ? 0.085
          : name === "hit"
            ? 0.075
            : name === "bite"
              ? 0.16
              : 0.04;
    if (t - (this.last.get(name) ?? -Infinity) < interval) return false;
    if (this.voices.size >= 12) {
      const victim = [...this.voices].sort(
        (a, b) => a[1].priority - b[1].priority,
      )[0];
      if (victim[1].priority >= d.priority) return false;
      victim[0].stop();
      victim[0].disconnect();
      this.voices.delete(victim[0]);
    }
    this.last.set(name, t);
    const source = this.ctx.createBufferSource();
    source.buffer = this.buffer(name, d);
    source.connect(this.sfx!);
    this.voices.set(source, { priority: d.priority });
    source.onended = () => {
      this.voices.delete(source);
      source.disconnect();
    };
    source.start();
    return true;
  }
  setRunning(running: boolean) {
    if (this.running === running) {
      if (!running) this.stopVoices();
      return;
    }
    this.running = running;
    if (!running) this.stopVoices();
  }
  update(time: number, final: boolean) {
    if (!this.running || !this.ctx || !this.unlocked) return;
    const step = Math.floor(time / 0.25);
    if (step === this.step) return;
    this.step = step;
    if (this.prefs.muted || this.prefs.musicVolume === 0) return;
    const melody = [0, 4, 7, 9, 7, 4, 2, 4, 0, 4, 7, 12, 11, 7, 4, 2],
      root = [48, 53, 55, 48][Math.floor(step / 16) % 4];
    if (step % 2 === 0 || final)
      this.note(
        root + 12 + melody[Math.floor(step / (final ? 1 : 2)) % 16],
        final ? 0.13 : 0.19,
        "triangle",
        final ? 0.16 : 0.2,
      );
    if (step % 4 === 0) this.note(root - 12, 0.36, "sine", 0.35);
    if (final && step % 2 === 1) this.note(root + 7, 0.1, "square", 0.06);
  }
  private note(
    midi: number,
    duration: number,
    wave: Sound["wave"],
    volume: number,
  ) {
    const ctx = this.ctx!;
    const source = ctx.createBufferSource(),
      gain = ctx.createGain(),
      freq = 440 * 2 ** ((midi - 69) / 12);
    source.buffer = this.buffer(`n${midi}-${duration}-${wave}`, {
      duration,
      freq,
      end: freq,
      wave,
      priority: 0,
    });
    gain.gain.value = volume;
    source.connect(gain);
    gain.connect(this.music!);
    this.musicVoices.add(source);
    source.onended = () => {
      this.musicVoices.delete(source);
      source.disconnect();
      gain.disconnect();
    };
    source.start();
  }
  private stopVoices() {
    for (const node of [...this.voices.keys(), ...this.musicVoices]) {
      try {
        node.stop();
        node.disconnect();
      } catch {}
    }
    this.voices.clear();
    this.musicVoices.clear();
  }
  reset() {
    this.stopVoices();
    this.running = false;
    this.step = -1;
    this.last.clear();
  }
  debug() {
    return {
      activeSfx: this.voices.size,
      activeMusic: this.musicVoices.size,
      step: this.step,
      running: this.running,
      unlocked: this.unlocked,
      failed: this.failed,
      contextState: this.ctx?.state ?? "uninitialized",
    };
  }
  dispose() {
    this.reset();
    void this.ctx?.close().catch(() => {});
  }
}
