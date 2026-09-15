import { PLANTS, type Kind } from "./model";
const wrap = (s: string) =>
  `<svg xmlns="http://www.w3.org/2000/svg" width="120" height="120" viewBox="0 0 120 120">${s}</svg>`;
const eyes =
  '<ellipse cx="51" cy="46" rx="4" ry="6" fill="#263c2c"/><ellipse cx="73" cy="46" rx="4" ry="6" fill="#263c2c"/><circle cx="52" cy="44" r="1.4" fill="white"/><circle cx="74" cy="44" r="1.4" fill="white"/>';
export function plantSvg(k: Kind) {
  const c = PLANTS[k].color;
  let body = "";
  const leaves =
    '<path d="M60 105 C22 108 17 88 29 86 Q49 86 59 100 Q70 78 95 87 Q100 105 60 105" fill="#4a8b4c" stroke="#315c37" stroke-width="3"/><path d="M60 96 L60 62" stroke="#427747" stroke-width="8"/>';
  if (k === "sun") {
    body =
      Array.from(
        { length: 10 },
        (_, i) =>
          `<ellipse cx="60" cy="24" rx="12" ry="19" transform="rotate(${i * 36} 60 49)" fill="#ffc651" stroke="#daa032" stroke-width="2"/>`,
      ).join("") +
      '<circle cx="60" cy="49" r="25" fill="#925a36"/>' +
      eyes.replaceAll("#263c2c", "#fff4d1") +
      '<path d="M53 60 Q61 68 70 60" fill="none" stroke="#fff4d1" stroke-width="3"/>';
  } else if (["wall", "armor", "thorn", "coldwall"].includes(k)) {
    body = `<path d="M31 92 Q20 39 40 24 Q61 10 80 27 Q99 47 91 94 Q60 106 31 92" fill="${c}" stroke="#6b573c" stroke-width="4"/><path d="M38 38 Q30 56 37 77 M84 43 L88 57" fill="none" stroke="#ffffff" stroke-opacity=".22" stroke-width="6"/>${eyes}<path d="M54 64 Q62 69 70 64" fill="none" stroke="#674a32" stroke-width="3"/>${k === "armor" ? '<path d="M26 42 L33 24 L60 14 L87 25 L94 42Z" fill="#658183" stroke="#355359" stroke-width="3"/><circle cx="60" cy="28" r="5" fill="#d3e0d8"/>' : ""}${k === "thorn" ? '<path d="M28 40 L14 30 L24 57 L9 66 L26 73 M91 40 L109 28 L99 59 L113 71 L95 78" fill="#63874b" stroke="#456238" stroke-width="3"/>' : ""}${k === "coldwall" ? '<path d="M47 82 H75 M61 69 V95 M51 72 L71 92 M51 92 L71 72" stroke="#e9ffff" stroke-width="3"/>' : ""}`;
  } else if (k === "bomb") {
    body =
      '<path d="M60 36 Q71 18 88 20" stroke="#466f36" stroke-width="7" fill="none"/><path d="M84 19 L93 10 M86 20 L100 23" stroke="#edb54b" stroke-width="4"/><circle cx="60" cy="65" r="35" fill="#e47863" stroke="#a3453e" stroke-width="4"/>' +
      eyes.replaceAll("46", "59").replaceAll("44", "57") +
      '<path d="M51 77 Q60 72 73 79" fill="none" stroke="#7d3932" stroke-width="3"/>';
  } else {
    body = `${["bunker", "solar"].includes(k) ? `<ellipse cx="59" cy="83" rx="29" ry="22" fill="${k === "solar" ? "#f3cb57" : "#b39062"}" stroke="#667547" stroke-width="3"/>` : ""}<path d="M28 53 Q21 25 49 20 Q77 12 87 39 L103 36 L108 64 L83 63 Q66 87 40 74 Q29 66 28 53Z" fill="${c}" stroke="#3f7149" stroke-width="4"/><ellipse cx="103" cy="50" rx="10" ry="15" fill="#3f7149"/><ellipse cx="106" cy="48" rx="4" ry="8" fill="#23492d"/><ellipse cx="52" cy="42" rx="5" ry="7" fill="#213e31"/><circle cx="53" cy="40" r="1.6" fill="white"/><path d="M34 35 Q42 25 55 25" stroke="#fff" stroke-opacity=".3" stroke-width="5" fill="none"/>${["ice", "frost"].includes(k) ? '<path d="M46 19 L42 6 L56 16 L63 2 L69 18 L81 9 L79 26" fill="#d4f8fc" stroke="#62a9be" stroke-width="2"/>' : ""}${["rapid", "double"].includes(k) ? '<path d="M32 25 L37 13 L78 13 L86 31Z" fill="#446c45"/><path d="M38 18 H78" stroke="#bdd184" stroke-width="5"/>' : ""}`;
  }
  return wrap(
    `<ellipse cx="61" cy="109" rx="37" ry="7" fill="#203e2a" opacity=".15"/>${leaves}${body}`,
  );
}
export function enemySvg(k: number) {
  return wrap(
    `<ellipse cx="59" cy="112" rx="32" ry="6" fill="#24462c" opacity=".18"/><path d="M46 86 L40 108 L54 108 L62 87 M68 85 L77 107 L91 107 L82 81" fill="#425563" stroke="#293e44" stroke-width="4"/><path d="M37 57 L25 82 L34 88 L48 72 M78 58 L99 75 L92 84 L76 75" fill="#899e75" stroke="#52684f" stroke-width="4"/><path d="M42 52 L76 53 L85 91 L37 91Z" fill="${["#89716b", "#887b65", "#c87a57", "#65777c"][k]}" stroke="#4b5145" stroke-width="4"/><path d="M56 56 L65 72 L57 85 L51 72Z" fill="#dca28a"/><path d="M35 29 Q34 6 64 8 Q87 9 85 34 L80 54 Q59 65 39 49Z" fill="#9daa81" stroke="#52684f" stroke-width="4"/><ellipse cx="46" cy="31" rx="7" ry="9" fill="#f4f0d5"/><ellipse cx="70" cy="30" rx="8" ry="9" fill="#f4f0d5"/><circle cx="44" cy="33" r="3" fill="#344936"/><circle cx="68" cy="32" r="3" fill="#344936"/><path d="M44 46 L72 45" stroke="#435b42" stroke-width="5"/><path d="M51 44 V50 M62 44 V49" stroke="#f7f1db" stroke-width="4"/>${k === 1 ? '<path d="M30 26 L35 3 L79 3 L86 26Z" fill="#a2adb0" stroke="#5c7175" stroke-width="4"/><path d="M41 8 H72" stroke="#dbe2df" stroke-width="4"/>' : ""}${k === 2 ? '<path d="M32 20 Q51 2 78 16 L96 24 L31 25Z" fill="#d77952" stroke="#854c39" stroke-width="3"/>' : ""}${k === 3 ? '<path d="M35 63 L79 63 L81 87 L38 87Z" fill="#819397" stroke="#465d65" stroke-width="3"/><path d="M38 71 H77 M38 79 H77" stroke="#bac8c6" stroke-width="3"/>' : ""}`,
  );
}
export const dataUri = (svg: string) =>
  "data:image/svg+xml;charset=utf-8," + encodeURIComponent(svg);
