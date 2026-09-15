import { GENE_IDS, type Gene } from "./genetics";
export interface Preferences {
  musicVolume: number;
  sfxVolume: number;
  muted: boolean;
  reducedMotion: boolean;
  tutorials: number[];
}
export interface SaveData {
  version: 3;
  discoveries: string[];
  genes: Gene[];
  lessons: string[];
  records: {
    runs: number;
    wins: number;
    bestStage: number;
    bestKills: number;
    bestTime: number;
  };
  unlocked: number;
  settings: Preferences;
}
export interface StoragePort {
  getItem(key: string): string | null;
  setItem(key: string, value: string): void;
}
export const SAVE_KEY = "mutabloom-v3";
export function defaults(reduced = false): SaveData {
  return {
    version: 3,
    discoveries: [],
    genes: [],
    lessons: [],
    records: { runs: 0, wins: 0, bestStage: 0, bestKills: 0, bestTime: 0 },
    unlocked: 0,
    settings: {
      musicVolume: 0.25,
      sfxVolume: 0.6,
      muted: false,
      reducedMotion: reduced,
      tutorials: [],
    },
  };
}
export function parseSave(raw: unknown, reduced = false): SaveData {
  const out = defaults(reduced);
  if (!raw || typeof raw !== "object") return out;
  const v = raw as Record<string, unknown>;
  if (Number.isInteger(v.unlocked))
    out.unlocked = Math.max(0, Math.min(2, v.unlocked as number));
  const prefs = (
    (v.version === 2 || v.version === 3) &&
    v.settings &&
    typeof v.settings === "object"
      ? v.settings
      : v
  ) as Record<string, unknown>;
  for (const key of ["musicVolume", "sfxVolume"] as const)
    if (typeof prefs[key] === "number" && Number.isFinite(prefs[key]))
      out.settings[key] = Math.max(0, Math.min(1, prefs[key]));
  for (const key of ["muted", "reducedMotion"] as const)
    if (typeof prefs[key] === "boolean") out.settings[key] = prefs[key];
  if (Array.isArray(prefs.tutorials))
    out.settings.tutorials = [
      ...new Set(
        prefs.tutorials.filter(
          (x): x is number => Number.isInteger(x) && x >= 0 && x <= 2,
        ),
      ),
    ];
  if (v.version === 3) {
    if (Array.isArray(v.discoveries))
      out.discoveries = [
        ...new Set(
          v.discoveries.filter(
            (x): x is string => typeof x === "string" && x.length < 80,
          ),
        ),
      ];
    if (Array.isArray(v.lessons))
      out.lessons = [
        ...new Set(
          v.lessons.filter(
            (x): x is string => typeof x === "string" && x.length < 40,
          ),
        ),
      ];
    if (Array.isArray(v.genes))
      out.genes = GENE_IDS.filter((g) => (v.genes as unknown[]).includes(g));
    if (v.records && typeof v.records === "object")
      for (const k of Object.keys(
        out.records,
      ) as (keyof typeof out.records)[]) {
        const n = (v.records as Record<string, unknown>)[k];
        if (typeof n === "number" && Number.isFinite(n) && n >= 0)
          out.records[k] = n;
      }
  }
  return out;
}
export function loadSave(
  storage: StoragePort,
  reduced = false,
): { data: SaveData; ok: boolean } {
  try {
    let ok = true;
    for (const key of [
      SAVE_KEY,
      "garden-guardians-v2",
      "garden-guardians-v1",
    ]) {
      const raw = storage.getItem(key);
      if (!raw) continue;
      try {
        return { data: parseSave(JSON.parse(raw), reduced), ok };
      } catch {
        ok = false;
      }
    }
    return { data: defaults(reduced), ok };
  } catch {
    return { data: defaults(reduced), ok: false };
  }
}
export function saveData(storage: StoragePort, data: SaveData) {
  try {
    storage.setItem(SAVE_KEY, JSON.stringify(data));
    return true;
  } catch {
    return false;
  }
}
