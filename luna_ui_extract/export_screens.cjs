const puppeteer = require('puppeteer-core');
const fs = require('fs');
const path = require('path');

const outDir = path.join(__dirname, '..', 'scratch', 'screens');
if (!fs.existsSync(outDir)) {
  fs.mkdirSync(outDir, { recursive: true });
}

(async () => {
  console.log('[EXPORTER] Launching Chrome...');
  const browser = await puppeteer.launch({
    executablePath: 'C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe',
    headless: true,
    args: ['--no-sandbox', '--disable-setuid-sandbox']
  });

  const page = await browser.newPage();
  await page.setViewport({ width: 1400, height: 1200, deviceScaleFactor: 1 });

  console.log('[EXPORTER] Navigating to http://localhost:5190/?export=1 ...');
  page.on('console', msg => console.log('PAGE LOG:', msg.text()));
  page.on('pageerror', err => console.log('PAGE ERROR:', err.message));
  await page.goto('http://localhost:5190/?export=1', { waitUntil: 'networkidle0', timeout: 30000 });

  const html = await page.content();
  console.log('[EXPORTER] Page HTML length:', html.length);
  const info = await page.evaluate(() => ({
    href: window.location.href,
    search: window.location.search,
    rootSnippet: document.getElementById('root')?.innerHTML?.substring(0, 300)
  }));
  console.log('[EXPORTER] Page info:', JSON.stringify(info));

  // Wait for Google Fonts to be fully loaded
  await page.evaluateHandle('document.fonts.ready');
  await new Promise(r => setTimeout(r, 2000));

  const screens = [
    { id: 'screen-clock-minimal',       name: 'clock_minimal.png' },
    { id: 'screen-clock-minimal-blank', name: 'clock_minimal_blank.png' },
    { id: 'screen-clock-retro',         name: 'clock_retro.png' },
    { id: 'screen-clock-cyber',         name: 'clock_cyber.png' },
    { id: 'screen-clock-analog',        name: 'clock_analog.png' },
    { id: 'screen-notifications',       name: 'notifications.png' },
    { id: 'screen-calendar',            name: 'calendar.png' },
    { id: 'screen-arcade',              name: 'arcade.png' },
    { id: 'screen-pomodoro',            name: 'pomodoro.png' },
    { id: 'screen-navigation',          name: 'navigation.png' },
    { id: 'screen-level',               name: 'level.png' },
    { id: 'screen-settings',            name: 'settings.png' },
    { id: 'screen-orbitron-digits-58',  name: 'orbitron_digits_58.png' },
    { id: 'screen-orbitron-sec-18',     name: 'orbitron_sec_18.png' },
  ];

  for (const s of screens) {
    const el = await page.$(`#${s.id}`);
    if (el) {
      const outPath = path.join(outDir, s.name);
      await el.screenshot({ path: outPath });
      console.log(`[EXPORTER] Captured ${s.id} -> ${outPath}`);
    } else {
      console.error(`[EXPORTER] Could not find #${s.id}`);
    }
  }

  await browser.close();
  console.log('[EXPORTER] Finished successfully.');
})();
