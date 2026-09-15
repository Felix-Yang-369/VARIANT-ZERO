import { defineConfig } from "@playwright/test";
export default defineConfig({
  testDir: "./tests",
  testMatch: "variant.browser.spec.ts",
  timeout: 30000,
  workers: 1,
  reporter: [
    ["list"],
    ["json", { outputFile: ".verification/variant/browser-results.json" }],
  ],
  use: {
    baseURL: "http://127.0.0.1:5173",
    channel: "chrome",
    headless: true,
    viewport: { width: 1366, height: 768 },
    screenshot: "only-on-failure",
    trace: "retain-on-failure",
  },
  outputDir: ".verification/browser-artifacts",
});
