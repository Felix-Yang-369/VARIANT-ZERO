/** Local, decorative icons. Their controls retain visible text labels. */
const paths = {
  home: '<path d="m3 10 9-7 9 7v10H3Z"/><path d="M9 20v-7h6v7"/>',
  book: '<path d="M12 5v15M3 4c4-1 6 0 9 2 3-2 5-3 9-2v15c-4-1-6 0-9 2-3-2-5-3-9-2Z"/>',
  flask: '<path d="M9 3h6M10 3v7L4 19q-1 2 2 2h12q3 0 2-2l-6-9V3M8 14h8"/><path d="M10 17h.01M14 18h.01"/>',
  settings: '<path d="m9 3-1 3-3 1-2 3 2 2-1 3 3 2 2 3h4l2-3 3-1 2-3-2-2 1-3-3-2-2-3Z"/><circle cx="11" cy="12" r="3"/>',
  leaf: '<path d="M5 19C-1 5 13 3 21 3c0 9-3 18-14 15M4 21 16 8M8 16v-5M12 12h5"/>',
  dna: '<path d="M6 2c0 10 12 10 12 20M18 2C18 12 6 12 6 22M7 5h10M9 9h6M9 15h6M7 19h10"/>',
  spark: '<path d="m12 3 3 6 6 3-6 3-3 6-3-6-6-3 6-3Z"/>',
  back: '<path d="m10 5-7 7 7 7M3 12h18"/>',
  arrow: '<path d="m14 5 7 7-7 7M3 12h18"/>',
} as const;
export const icon = (name: keyof typeof paths) => `<svg class="ui-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true" focusable="false">${paths[name]}</svg>`;
