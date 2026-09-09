/**
 * Luna Display Studio - UI Component Registry
 * Curated with EXACT authentic Uiverse.io UI components & CSS Scoping
 * Tailored for ESP32-S3 1.69" 240x280 Displays
 */

/**
 * Strictly scopes any CSS string to a container class so it NEVER leaks to the document
 */
function scopeUiverseCSS(scopeClass, cssText) {
  if (!cssText) return '';
  const clean = cssText.replace(/\/\*[\s\S]*?\*\//g, '');
  const keyframes = [];
  const cleanNoKf = clean.replace(/@(-webkit-)?keyframes[^{]+{(?:[^{}]+{[^{}]+})+[^{}]*}/g, (m) => {
    keyframes.push(m);
    return '';
  });
  const prefixed = cleanNoKf.replace(/([^{}]+)\s*\{/g, (match, selPart) => {
    const trimmed = selPart.trim();
    if (!trimmed || trimmed.startsWith('@')) return match;
    const sels = trimmed.split(',').map(s => s.trim()).filter(Boolean);
    return sels.map(s => `.${scopeClass} ${s}`).join(', ') + ' {';
  });
  return `@scope (.${scopeClass}) {\n${keyframes.join('\n')}\n${prefixed}\n}`;
}

const UI_COMPONENTS = {
  // ─────────────────────────────────────────────────────────────
  // ─── CARDS ───
  // ─────────────────────────────────────────────────────────────
  uiverse_push_card: {
    name: "Push Alert Card (vinod)",
    category: "cards",
    description: "Exact Uiverse.io Push notifications card by vinodjangid07",
    defaultProps: {
      x: 16, y: 30, w: 208, h: 104,
      title: "Push notifications",
      bgColor: "#f5f5f5",
      textColor: "#000000"
    },
    renderHTML: (p) => {
      const scope = "uiv-card-deer";
      const exactCss = `/* From Uiverse.io by vinodjangid07 - Tags: notification, card, popup */
.notificationCard {
  width: 220px;
  height: 280px;
  background: rgb(245, 245, 245);
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 20px 35px;
  gap: 10px;
  box-shadow: 5px 5px 10px rgba(0, 0, 0, 0.123);
  border-radius: 20px;
}

.bellIcon {
  width: 50px;
  margin: 20px 0px;
}

.bellIcon path {
  fill: rgb(168, 131, 255);
}

.notificationHeading {
  color: black;
  font-weight: 600;
  font-size: 0.8em;
}

.notificationPara {
  color: rgb(133, 133, 133);
  font-size: 0.6em;
  font-weight: 600;
  text-align: center;
}

.buttonContainer {
  display: flex;
  flex-direction: column;
  gap: 5px;
}

.AllowBtn {
  width: 120px;
  height: 25px;
  background-color: rgb(168, 131, 255);
  color: white;
  border: none;
  border-radius: 20px;
  font-size: 0.7em;
  font-weight: 600;
  cursor: pointer;
}

.NotnowBtn {
  width: 120px;
  height: 25px;
  color: rgb(168, 131, 255);
  border: none;
  background-color: transparent;
  font-weight: 600;
  font-size: 0.7em;
  cursor: pointer;
  border-radius: 20px;
}

.NotnowBtn:hover {
  background-color: rgb(239, 227, 255);
}

.AllowBtn:hover {
  background-color: rgb(153, 110, 255);
}`;
      const exactHtml = `<div class="notificationCard">
  <p class="notificationHeading">Push notifications</p>
  <svg class="bellIcon" viewBox="0 0 448 512"><path d="M224 0c-17.7 0-32 14.3-32 32V51.2C119 66 64 130.6 64 208v18.8c0 47-17.3 92.4-48.5 127.6l-7.4 8.3c-8.4 9.4-10.4 22.9-5.3 34.4S19.4 416 32 416H416c12.6 0 24-7.4 29.2-18.9s3.1-25-5.3-34.4l-7.4-8.3C401.3 319.2 384 273.9 384 226.8V208c0-77.4-55-142-128-156.8V32c0-17.7-14.3-32-32-32zm45.3 493.3c12-12 18.7-28.3 18.7-45.3H224 160c0 17 6.7 33.3 18.7 45.3s28.3 18.7 45.3 18.7s33.3-6.7 45.3-18.7z"></path></svg>
  <p class="notificationPara">Allow push notifications so you will get latest updates</p>
  <div class="buttonContainer">
    <button class="AllowBtn">Allow</button>
    <button class="NotnowBtn">Now now</button>
  </div>
</div>`;
      return `
        <div class="${scope}" style="width: 100%; height: 100%; display: flex; align-items: center; justify-content: center; position: relative; overflow: hidden; transform-origin: center center;">
          <style>${scopeUiverseCSS(scope, exactCss)}</style>
          <div style="transform: scale(0.68); transform-origin: center center;">
            ${exactHtml}
          </div>
        </div>
      `;
    }
  },

  card_transaction: {
    name: "Transaction Card (uiverse)",
    category: "cards",
    description: "Uiverse.io Card with green icon pill and chevron",
    defaultProps: {
      x: 16, y: 36, w: 208, h: 56,
      title: "New Transaction",
      bgColor: "#ffffff",
      iconBg: "#10b981",
      textColor: "#0f172a",
      radius: 12,
      borderWidth: 1,
      borderColor: "#e2e8f0"
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        background: ${p.bgColor};
        border: ${p.borderWidth}px solid ${p.borderColor};
        border-radius: ${p.radius}px;
        padding: 6px 14px;
        display: flex; align-items: center; justify-content: space-between;
        box-shadow: 0 4px 14px rgba(0, 0, 0, 0.15);
      ">
        <div style="display: flex; align-items: center; gap: 10px;">
          <div style="
            width: 36px; height: 32px;
            background: ${p.iconBg};
            border-radius: 8px;
            display: flex; align-items: center; justify-content: center;
            color: #ffffff; font-size: 15px; font-weight: bold;
          ">💳</div>
          <span style="
            font-size: 11px;
            font-weight: 700;
            color: ${p.textColor};
          ">${p.title}</span>
        </div>
        <span style="font-size: 16px; font-weight: bold; color: #64748b;">›</span>
      </div>
    `
  },

  dialog_order_valid: {
    name: "Order Validated (uiverse)",
    category: "cards",
    description: "Uiverse.io Success Card with circular checkmark",
    defaultProps: {
      x: 16, y: 36, w: 208, h: 96,
      title: "Order validated",
      subtitle: "Thank you for your purchase! Ready to play.",
      bgColor: "#ffffff",
      accentColor: "#10b981",
      textColor: "#0f172a",
      subtextColor: "#64748b",
      radius: 14
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        background: ${p.bgColor};
        border-radius: ${p.radius}px;
        padding: 10px;
        display: flex; flex-direction: column; align-items: center; text-align: center;
        box-shadow: 0 8px 24px rgba(0, 0, 0, 0.15);
        position: relative;
      ">
        <div style="
          width: 30px; height: 30px;
          border-radius: 50%;
          background: #ecfdf5;
          border: 2px solid ${p.accentColor};
          color: ${p.accentColor};
          display: flex; align-items: center; justify-content: center;
          font-size: 15px; font-weight: 900;
          margin-bottom: 4px;
        ">✓</div>
        <div style="font-size: 12px; font-weight: 800; color: ${p.textColor};">${p.title}</div>
        <div style="font-size: 8.5px; color: ${p.subtextColor}; margin-top: 2px; line-height: 1.3;">${p.subtitle}</div>
      </div>
    `
  },

  uiverse_weather_card: {
    name: "Weather Forecast Card",
    category: "cards",
    description: "Uiverse.io ambient weather widget card",
    defaultProps: {
      x: 16, y: 32, w: 208, h: 84,
      city: "San Francisco",
      temp: "24°C",
      condition: "Partly Cloudy",
      bgColor: "#1e293b",
      textColor: "#ffffff",
      subtextColor: "#94a3b8",
      accentColor: "#38bdf8",
      radius: 14
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        background: ${p.bgColor};
        border-radius: ${p.radius}px;
        padding: 10px 14px;
        display: flex; align-items: center; justify-content: space-between;
        box-shadow: 0 6px 20px rgba(0,0,0,0.25);
        border: 1px solid rgba(56, 189, 248, 0.2);
      ">
        <div>
          <div style="font-size: 9px; font-weight: bold; color: ${p.subtextColor}; text-transform: uppercase;">${p.city}</div>
          <div style="font-size: 24px; font-weight: 900; color: ${p.textColor}; font-family: var(--font-mono); margin: 2px 0;">${p.temp}</div>
          <div style="font-size: 8.5px; color: ${p.accentColor}; font-weight: 600;">${p.condition}</div>
        </div>
        <div style="font-size: 32px; filter: drop-shadow(0 0 8px rgba(56,189,248,0.5));">⛅</div>
      </div>
    `
  },

  uiverse_music_card: {
    name: "Music Player Card",
    category: "cards",
    description: "Uiverse.io Mini audio player with playback bar",
    defaultProps: {
      x: 16, y: 36, w: 208, h: 88,
      track: "Midnight City",
      artist: "Luna Synth",
      progress: 65,
      bgColor: "#090d16",
      accentColor: "#ec4899",
      textColor: "#ffffff",
      radius: 12
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        background: ${p.bgColor};
        border-radius: ${p.radius}px;
        padding: 10px 12px;
        display: flex; flex-direction: column; justify-content: space-between;
        border: 1px solid rgba(236, 72, 153, 0.3);
        box-shadow: 0 4px 16px rgba(0,0,0,0.4);
      ">
        <div style="display: flex; align-items: center; gap: 10px;">
          <div style="width: 32px; height: 32px; border-radius: 50%; background: linear-gradient(135deg, #ec4899, #8b5cf6); display: flex; align-items: center; justify-content: center; font-size: 14px; box-shadow: 0 0 10px rgba(236,72,153,0.5);">🎵</div>
          <div style="overflow: hidden; flex: 1;">
            <div style="font-size: 11px; font-weight: 800; color: ${p.textColor}; white-space: nowrap; text-overflow: ellipsis; overflow: hidden;">${p.track}</div>
            <div style="font-size: 8.5px; color: #94a3b8;">${p.artist}</div>
          </div>
          <span style="font-size: 16px; color: #ec4899;">▶</span>
        </div>
        <div>
          <div style="width: 100%; height: 4px; background: #1e293b; border-radius: 2px; overflow: hidden;">
            <div style="width: ${p.progress}%; height: 100%; background: ${p.accentColor};"></div>
          </div>
          <div style="display: flex; justify-content: space-between; font-size: 7.5px; color: #64748b; margin-top: 3px; font-family: var(--font-mono);">
            <span>02:14</span><span>03:45</span>
          </div>
        </div>
      </div>
    `
  },

  uiverse_profile_card: {
    name: "Cyber Rank Profile Card",
    category: "cards",
    description: "Uiverse.io Gamer profile with rank badge and XP",
    defaultProps: {
      x: 16, y: 36, w: 208, h: 80,
      username: "MARIO_ESP32",
      rank: "DIAMOND IV",
      level: "LVL 42",
      bgColor: "#0f172a",
      accentColor: "#00f2fe",
      textColor: "#ffffff",
      radius: 12
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        background: ${p.bgColor};
        border-radius: ${p.radius}px;
        padding: 10px 12px;
        display: flex; align-items: center; gap: 10px;
        border: 1px solid ${p.accentColor};
        box-shadow: 0 0 14px ${p.accentColor}30;
      ">
        <div style="
          width: 42px; height: 42px;
          border-radius: 50%;
          background: #1e293b;
          border: 2px solid ${p.accentColor};
          display: flex; align-items: center; justify-content: center;
          font-size: 20px;
        ">👾</div>
        <div style="flex: 1; overflow: hidden;">
          <div style="display: flex; justify-content: space-between; align-items: center;">
            <span style="font-size: 11px; font-weight: 900; color: ${p.textColor}; font-family: var(--font-mono);">${p.username}</span>
            <span style="font-size: 8px; font-weight: 800; background: ${p.accentColor}22; color: ${p.accentColor}; padding: 2px 5px; border-radius: 4px;">${p.level}</span>
          </div>
          <div style="font-size: 8px; font-weight: 700; color: #94a3b8; margin-top: 2px; letter-spacing: 0.5px;">${p.rank}</div>
          <div style="width: 100%; height: 3px; background: #1e293b; border-radius: 2px; margin-top: 6px; overflow: hidden;">
            <div style="width: 75%; height: 100%; background: ${p.accentColor};"></div>
          </div>
        </div>
      </div>
    `
  },

  card_glass: {
    name: "Glassmorphic Card",
    category: "cards",
    description: "Translucent rounded card with subtle gradient border",
    defaultProps: {
      x: 16, y: 32, w: 208, h: 84,
      title: "LUNA SYSTEM",
      subtitle: "Status: Optimal",
      bgColor: "#141c2e",
      borderColor: "#00f2fe",
      textColor: "#ffffff",
      subtextColor: "#94a3b8",
      radius: 12,
      borderWidth: 1,
      glow: true
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        background: ${p.bgColor};
        border: ${p.borderWidth}px solid ${p.borderColor};
        border-radius: ${p.radius}px;
        padding: 10px 12px;
        display: flex; flex-direction: column; justify-content: space-between;
        box-shadow: ${p.glow ? `0 0 12px ${p.borderColor}40` : 'none'};
      ">
        <div style="font-size: 11px; font-weight: 800; color: ${p.textColor}; letter-spacing: 0.5px;">${p.title}</div>
        <div style="font-size: 9px; color: ${p.subtextColor}; font-family: var(--font-mono);">${p.subtitle}</div>
        <div style="height: 2px; width: 100%; background: ${p.borderColor}; border-radius: 2px; opacity: 0.6;"></div>
      </div>
    `
  },

  card_stat: {
    name: "Metric Stat Card",
    category: "cards",
    description: "Sensor telemetry or battery metric card",
    defaultProps: {
      x: 16, y: 32, w: 100, h: 78,
      label: "HEART RATE",
      value: "128",
      unit: "BPM",
      icon: "heart",
      bgColor: "#161b26",
      borderColor: "#ec4899",
      accentColor: "#ec4899",
      textColor: "#ffffff",
      radius: 10,
      borderWidth: 1
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        background: ${p.bgColor};
        border: ${p.borderWidth}px solid ${p.borderColor};
        border-radius: ${p.radius}px;
        padding: 8px;
        display: flex; flex-direction: column; justify-content: space-between;
      ">
        <div style="display: flex; align-items: center; justify-content: space-between;">
          <span style="font-size: 8px; font-weight: 800; color: #94a3b8;">${p.label}</span>
          <span style="font-size: 10px; color: ${p.accentColor};">♥</span>
        </div>
        <div style="display: flex; align-items: baseline; gap: 4px;">
          <span style="font-size: 20px; font-weight: 900; color: ${p.textColor}; font-family: var(--font-mono);">${p.value}</span>
          <span style="font-size: 8.5px; color: ${p.accentColor}; font-weight: bold;">${p.unit}</span>
        </div>
        <div style="height: 3px; width: 70%; background: ${p.accentColor}; border-radius: 2px;"></div>
      </div>
    `
  },

  card_retro: {
    name: "Arcade Bevel Box",
    category: "cards",
    description: "Retro pixelated box with double borders",
    defaultProps: {
      x: 20, y: 36, w: 200, h: 74,
      title: "HIGH SCORE",
      score: "99450",
      bgColor: "#090d16",
      borderColor: "#f59e0b",
      textColor: "#f59e0b",
      radius: 0
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        background: ${p.bgColor};
        border: 2px solid ${p.borderColor};
        outline: 1px solid ${p.borderColor};
        outline-offset: -5px;
        padding: 8px;
        display: flex; flex-direction: column; align-items: center; justify-content: center;
        gap: 6px;
      ">
        <div style="font-family: var(--font-pixel); font-size: 7.5px; color: ${p.textColor};">${p.title}</div>
        <div style="font-family: var(--font-pixel); font-size: 12px; color: #ffffff;">${p.score}</div>
      </div>
    `
  },

  // ─────────────────────────────────────────────────────────────
  // ─── EXACT UIVERSE BUTTONS (Scoped & Verified) ───
  // ─────────────────────────────────────────────────────────────
  adamgiebl_plastic_parrot: {
    name: "Cloud Download (adamgiebl)",
    category: "buttons",
    description: "Exact Uiverse button by adamgiebl with gradient & SVG cloud",
    defaultProps: {
      x: 24, y: 130, w: 192, h: 48,
      label: "Download",
      bgColor: "#4d36d0",
      textColor: "#ffffff"
    },
    renderHTML: (p) => {
      const scope = "uiv-btn-parrot";
      const exactCss = `/* From Uiverse.io by adamgiebl - Tags: icon, button */
.cssbuttons-io-button {
  display: flex;
  align-items: center;
  font-family: inherit;
  font-weight: 500;
  font-size: 17px;
  padding: 0.8em 1.5em 0.8em 1.2em;
  color: white;
  background: #ad5389;
  background: linear-gradient(0deg, rgba(77,54,208,1) 0%, rgba(132,116,254,1) 100%);
  border: none;
  box-shadow: 0 0.7em 1.5em -0.5em #4d36d0be;
  letter-spacing: 0.05em;
  border-radius: 20em;
}

.cssbuttons-io-button svg {
  margin-right: 8px;
}

.cssbuttons-io-button:hover {
  box-shadow: 0 0.5em 1.5em -0.5em #4d36d0be;
}

.cssbuttons-io-button:active {
  box-shadow: 0 0.3em 1em -0.5em #4d36d0be;
}`;
      const exactHtml = `<button class="cssbuttons-io-button">
  <svg height="24" width="24" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><path d="M0 0h24v24H0z" fill="none"></path><path d="M1 14.5a6.496 6.496 0 0 1 3.064-5.519 8.001 8.001 0 0 1 15.872 0 6.5 6.5 0 0 1-2.936 12L7 21c-3.356-.274-6-3.078-6-6.5zm15.848 4.487a4.5 4.5 0 0 0 2.03-8.309l-.807-.503-.12-.942a6.001 6.001 0 0 0-11.903 0l-.12.942-.805.503a4.5 4.5 0 0 0 2.029 8.309l.173.013h9.35l.173-.013zM13 12h3l-4 5-4-5h3V8h2v4z" fill="currentColor"></path></svg>
  <span>Download</span>
</button>`;
      return `
        <div class="${scope}" style="width: 100%; height: 100%; display: flex; align-items: center; justify-content: center;">
          <style>${scopeUiverseCSS(scope, exactCss)}</style>
          ${exactHtml}
        </div>
      `;
    }
  },

  sarthak_hungry_penguin: {
    name: "CTA Slide Arrow (0x-Sarthak)",
    category: "buttons",
    description: "Exact Uiverse button by 0x-Sarthak with sliding arrow",
    defaultProps: {
      x: 24, y: 130, w: 192, h: 44,
      label: "Start Session",
      bgColor: "#b1dae7",
      textColor: "#234567"
    },
    renderHTML: (p) => {
      const scope = "uiv-btn-penguin";
      const exactCss = `/* From Uiverse.io by 0x-Sarthak  - Tags: button, hover, rounded, animated, hover button */
.cta {
  position: relative;
  margin: auto;
  padding: 11.5px 18px;
  transition: all 0.2s ease;
  border: 3px solid #552da8;
  border-radius: 50px;
  background: #552da8;
  cursor: pointer;
}

.cta:before {
  content: "";
  position: absolute;
  top: 0;
  right: 0;
  display: block;
  border-radius: 50px;
  background: white;
  width: 45px;
  height: 45px;
  transition: all 0.8s ease;
}

.cta span {
  position: relative;
  font-family: Montserrat;
  font-size: 18px;
  color: white;
  font-weight: 400;
  letter-spacing: 0.05em;
}

.cta svg {
  position: relative;
  top: 0;
  margin-left: 10px;
  fill: none;
  stroke-linecap: round;
  stroke-linejoin: round;
  stroke: white;
  stroke-width: 2;
  transform: translateX(-5px);
  transition: all 0.5s ease;
}

.cta:hover:before {
  width: 100%;
  background: #1c1c1c;
}

.cta:hover svg {
  transform: translateX(0);
  transition: all 2s ease;
}

.cta:active {
  transform: scale(0.95);
  transition: all 2s ease;
}`;
      const exactHtml = `<button class="cta">
  <span>Contact Us &nbsp;</span>
  <svg viewBox="0 0 13 10" height="10px" width="15px">
    <path d="M1,5 L11,5"></path>
    <polyline points="8 1 12 5 8 9"></polyline>
  </svg>
</button>`;
      return `
        <div class="${scope}" style="width: 100%; height: 100%; display: flex; align-items: center; justify-content: center;">
          <style>${scopeUiverseCSS(scope, exactCss)}</style>
          ${exactHtml}
        </div>
      `;
    }
  },

  gharsh_ancient_elephant: {
    name: "Neon Gradient (gharsh)",
    category: "buttons",
    description: "Exact Uiverse neon button by gharsh11032000 with glowing border",
    defaultProps: {
      x: 24, y: 130, w: 192, h: 44,
      label: "SYSTEM ONLINE",
      bgColor: "transparent",
      textColor: "#ffffff"
    },
    renderHTML: (p) => {
      const scope = "uiv-btn-elephant";
      const exactCss = `/* From Uiverse.io by gharsh11032000 - Tags: button, hover effect, hoverme */
.button {
  text-decoration: none;
  color: white;
  font-weight: 600;
  font-size: 20px;
  text-transform: uppercase;
  letter-spacing: 2px;
  padding: 15px 30px;
  display: block;
  background-color: transparent;
  border: none;
  position: relative;
  cursor: pointer;
  transition: all 0.6s cubic-bezier(0.23, 1, 0.320, 1);
}

.button span {
  background: linear-gradient(-45deg, #63A4FF 0%, #83EAF1 100% );
  background-clip: text;
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
}

.button:hover span {
  -webkit-text-fill-color: white;
}

.button::before {
  position: absolute;
  content: "";
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  transform: scaleY(2);
  border: 3px solid;
  border-image: linear-gradient(-45deg, #63A4FF 0%, #83EAF1 100% );
  border-image-slice: 1;
  border-width: 2px 0 2px 0;
  opacity: 0;
  pointer-events: none;
  transition: all 0.6s cubic-bezier(0.23, 1, 0.320, 1);
  z-index: -1;
}

.button:hover::before {
  transform: scaleY(0);
  opacity: 1;
}

.button::after {
  position: absolute;
  content: "";
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  background: linear-gradient(-45deg, #63A4FF 0%, #83EAF1 100% );
  transform: scale(0);
  pointer-events: none;
  transition: all 0.6s cubic-bezier(0.23, 1, 0.320, 1);
  z-index: -1;
}

.button:hover::after {
  transform: scaleY(1);
}

.button:active {
  scale: 0.90;
}`;
      const exactHtml = `<button class="button">
  <span>Hover me</span>
</button>`;
      return `
        <div class="${scope}" style="width: 100%; height: 100%; display: flex; align-items: center; justify-content: center;">
          <style>${scopeUiverseCSS(scope, exactCss)}</style>
          ${exactHtml}
        </div>
      `;
    }
  },

  adamgiebl_fluffy_liger: {
    name: "Flow Gradient (adamgiebl)",
    category: "buttons",
    description: "Exact Uiverse animated flowing multi-color button by adamgiebl",
    defaultProps: {
      x: 24, y: 130, w: 192, h: 44,
      label: "Launch ESP32",
      bgColor: "#3f00b5",
      textColor: "#ffffff"
    },
    renderHTML: (p) => {
      const scope = "uiv-btn-liger";
      const exactCss = `/* From Uiverse.io by adamgiebl - Tags: button */
button {
  background: linear-gradient(-45deg, #3f00b5, #9f69fe, #27c8b7, #3f00b5);
  background-size: 800% 400%;
  padding: 1em 2em;
  display: inline-block;
  border: none;
  border-radius: 10px;
  font-size: 17px;
  font-weight: 700;
  color: white;
  transition: all .5s ease-in-out;
  animation: gradient 10s infinite cubic-bezier(.62, .28, .23, .99) both;
}

button:hover {
  animation: gradient 3s infinite;
  transform: scale(1.05);
}

button:active {
  animation: gradient 3s infinite;
  transform: scale(0.8);
}

@keyframes gradient {
  0% {
    background-position: 0% 50%;
  }

  50% {
    background-position: 100% 50%;
  }

  100% {
    background-position: 0% 50%;
  }
}`;
      const exactHtml = `<button> Button
</button>`;
      return `
        <div class="${scope}" style="width: 100%; height: 100%; display: flex; align-items: center; justify-content: center;">
          <style>${scopeUiverseCSS(scope, exactCss)}</style>
          ${exactHtml}
        </div>
      `;
    }
  },

  adamgiebl_lucky_donkey: {
    name: "Tactile Neumorphic (adamgiebl)",
    category: "buttons",
    description: "Exact Uiverse Neumorphic button by adamgiebl with inset depth",
    defaultProps: {
      x: 24, y: 130, w: 192, h: 44,
      label: "Confirm Action",
      bgColor: "#e8e8e8",
      textColor: "#090909"
    },
    renderHTML: (p) => {
      const scope = "uiv-btn-donkey";
      const exactCss = `/* From Uiverse.io by adamgiebl - Tags: neumorphism, button */
button {
  color: #090909;
  padding: 0.7em 1.7em;
  font-size: 18px;
  border-radius: 0.5em;
  background: #e8e8e8;
  border: 1px solid #e8e8e8;
  transition: all .3s;
  box-shadow: 6px 6px 12px #c5c5c5,
             -6px -6px 12px #ffffff;
}

button:hover {
  border: 1px solid white;
}

button:active {
  box-shadow: 4px 4px 12px #c5c5c5,
             -4px -4px 12px #ffffff;
}`;
      const exactHtml = `<button> Button
</button>`;
      return `
        <div class="${scope}" style="width: 100%; height: 100%; display: flex; align-items: center; justify-content: center;">
          <style>${scopeUiverseCSS(scope, exactCss)}</style>
          ${exactHtml}
        </div>
      `;
    }
  },

  vinod_afraid_falcon: {
    name: "Scroll Top Pill (vinod)",
    category: "buttons",
    description: "Exact Uiverse dark rounded icon button by vinodjangid07",
    defaultProps: {
      x: 45, y: 130, w: 150, h: 44,
      label: "Top View",
      bgColor: "#141414",
      textColor: "#ffffff"
    },
    renderHTML: (p) => {
      const scope = "uiv-btn-falcon";
      const exactCss = `/* From Uiverse.io by vinodjangid07  - Tags: animation, black, button, arrow, hover effect, back to top, scroll to top */
.button {
  width: 50px;
  height: 50px;
  border-radius: 50%;
  background-color: rgb(20, 20, 20);
  border: none;
  font-weight: 600;
  display: flex;
  align-items: center;
  justify-content: center;
  box-shadow: 0px 0px 0px 4px rgba(180, 160, 255, 0.253);
  cursor: pointer;
  transition-duration: 0.3s;
  overflow: hidden;
  position: relative;
}

.svgIcon {
  width: 12px;
  transition-duration: 0.3s;
}

.svgIcon path {
  fill: white;
}

.button:hover {
  width: 140px;
  border-radius: 50px;
  transition-duration: 0.3s;
  background-color: rgb(181, 160, 255);
  align-items: center;
}

.button:hover .svgIcon {
  /* width: 20px; */
  transition-duration: 0.3s;
  transform: translateY(-200%);
}

.button::before {
  position: absolute;
  bottom: -20px;
  content: "Back to Top";
  color: white;
  /* transition-duration: .3s; */
  font-size: 0px;
}

.button:hover::before {
  font-size: 13px;
  opacity: 1;
  bottom: unset;
  /* transform: translateY(-30px); */
  transition-duration: 0.3s;
}`;
      const exactHtml = `<button class="button">
  <svg class="svgIcon" viewBox="0 0 384 512">
    <path
      d="M214.6 41.4c-12.5-12.5-32.8-12.5-45.3 0l-160 160c-12.5 12.5-12.5 32.8 0 45.3s32.8 12.5 45.3 0L160 141.2V448c0 17.7 14.3 32 32 32s32-14.3 32-32V141.2L329.4 246.6c12.5 12.5 32.8 12.5 45.3 0s12.5-32.8 0-45.3l-160-160z"
    ></path>
  </svg>
</button>`;
      return `
        <div class="${scope}" style="width: 100%; height: 100%; display: flex; align-items: center; justify-content: center;">
          <style>${scopeUiverseCSS(scope, exactCss)}</style>
          ${exactHtml}
        </div>
      `;
    }
  },

  btn_happy_coding: {
    name: "Happy Coding (uiverse)",
    category: "buttons",
    description: "Uiverse.io Neumorphic Button with purple/cyan glow",
    defaultProps: {
      x: 24, y: 140, w: 192, h: 44,
      label: "Happy Coding!",
      bgColor: "#131b2e",
      borderColor: "#6366f1",
      textColor: "#818cf8",
      radius: 12,
      borderWidth: 1.5,
      glow: true
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        background: ${p.bgColor};
        border: ${p.borderWidth}px solid ${p.borderColor};
        border-radius: ${p.radius}px;
        display: flex; align-items: center; justify-content: center;
        box-shadow: 0 4px 18px rgba(99, 102, 241, 0.4);
        cursor: pointer;
      ">
        <span style="
          color: ${p.textColor};
          font-size: 11px;
          font-weight: 700;
          letter-spacing: 0.5px;
        ">${p.label}</span>
      </div>
    `
  },

  btn_get_started: {
    name: "Get Started Button (uiverse)",
    category: "buttons",
    description: "Uiverse.io Vibrant Blue Button with badge icon",
    defaultProps: {
      x: 28, y: 140, w: 184, h: 42,
      label: "Get started",
      badge: "G",
      bgColor: "#2563eb",
      textColor: "#ffffff",
      radius: 10
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        background: ${p.bgColor};
        border-radius: ${p.radius}px;
        display: flex; align-items: center; justify-content: center;
        gap: 8px;
        box-shadow: 0 4px 16px rgba(37, 99, 235, 0.45);
      ">
        <span style="font-size: 11px; font-weight: 700; color: ${p.textColor};">${p.label}</span>
        <span style="
          background: rgba(255, 255, 255, 0.25);
          color: #fff;
          font-size: 9px;
          font-weight: 800;
          padding: 2px 6px;
          border-radius: 4px;
        ">${p.badge}</span>
      </div>
    `
  },

  button_retro: {
    name: "8-Bit Arcade Button",
    category: "buttons",
    description: "Chunky retro pixel button with drop shadow",
    defaultProps: {
      x: 30, y: 150, w: 180, h: 40,
      label: "PRESS START",
      bgColor: "#e11d48",
      textColor: "#ffffff",
      borderColor: "#881337",
      radius: 0
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        background: ${p.bgColor};
        border: 2px solid ${p.borderColor};
        box-shadow: inset -2px -2px 0px #881337, inset 2px 2px 0px #fda4af;
        display: flex; align-items: center; justify-content: center;
      ">
        <span style="
          font-family: var(--font-pixel);
          font-size: 8px;
          color: ${p.textColor};
        ">${p.label}</span>
      </div>
    `
  },

  // ─────────────────────────────────────────────────────────────
  // ─── EXACT UIVERSE TOGGLES & SWITCHES ───
  // ─────────────────────────────────────────────────────────────
  adamgiebl_grumpy_moth: {
    name: "Fluid Pill Switch (adamgiebl)",
    category: "toggles",
    description: "Exact Uiverse toggle switch by adamgiebl with sliding knob",
    defaultProps: {
      x: 36, y: 100, w: 168, h: 36,
      label: "BLUETOOTH",
      checked: true,
      textColor: "#ffffff"
    },
    renderHTML: (p) => {
      const scope = "uiv-toggle-moth";
      const exactCss = `/* From Uiverse.io by adamgiebl - Tags: switch */
/* The switch - the box around the slider */
.switch {
  font-size: 17px;
  position: relative;
  display: inline-block;
  width: 3.5em;
  height: 2em;
}

/* Hide default HTML checkbox */
.switch input {
  opacity: 0;
  width: 0;
  height: 0;
}

/* The slider */
.slider {
  position: absolute;
  cursor: pointer;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-color: #ccc;
  transition: .4s;
  border-radius: 30px;
}

.slider:before {
  position: absolute;
  content: "";
  height: 1.4em;
  width: 1.4em;
  border-radius: 20px;
  left: 0.3em;
  bottom: 0.3em;
  background-color: white;
  transition: .4s;
}

input:checked + .slider {
  background-color: #2196F3;
}

input:focus + .slider {
  box-shadow: 0 0 1px #2196F3;
}

input:checked + .slider:before {
  transform: translateX(1.5em);
}`;
      const exactHtml = `<label class="switch">
  <input type="checkbox">
  <span class="slider"></span>
</label>`;
      return `
        <div class="${scope}" style="width: 100%; height: 100%; display: flex; align-items: center; justify-content: space-between; padding: 0 10px;">
          <style>${scopeUiverseCSS(scope, exactCss)}</style>
          <span style="font-size: 10px; font-weight: 700; color: ${p.textColor};">${p.label}</span>
          ${exactHtml}
        </div>
      `;
    }
  },

  vinod_speaker_switch: {
    name: "Speaker Audio Switch (vinod)",
    category: "toggles",
    description: "Exact Uiverse audio mute switch by vinodjangid07",
    defaultProps: {
      x: 36, y: 100, w: 168, h: 42,
      label: "AUDIO OUTPUT",
      checked: true,
      textColor: "#ffffff"
    },
    renderHTML: (p) => {
      const scope = "uiv-toggle-speaker";
      const exactCss = `/* From Uiverse.io by vinodjangid07 - Tags: switch, toggle, volume, click effect */
/* The switch - the box around the speaker*/
.toggleSwitch {
  width: 50px;
  height: 50px;
  position: relative;
  display: flex;
  align-items: center;
  justify-content: center;
  background-color: rgb(39, 39, 39);
  border-radius: 50%;
  cursor: pointer;
  transition-duration: .3s;
  box-shadow: 2px 2px 10px rgba(0, 0, 0, 0.13);
  overflow: hidden;
}

/* Hide default HTML checkbox */
#checkboxInput {
  display: none;
}

.bell {
  width: 18px;
}

.bell path {
  fill: white;
}

.speaker {
  width: 100%;
  height: 100%;
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 2;
  transition-duration: .3s;
}

.speaker svg {
  width: 18px;
}

.mute-speaker {
  position: absolute;
  width: 100%;
  height: 100%;
  display: flex;
  align-items: center;
  justify-content: center;
  opacity: 0;
  z-index: 3;
  transition-duration: .3s;
}

.mute-speaker svg {
  width: 18px;
}

#checkboxInput:checked +.toggleSwitch .speaker {
  opacity: 0;
  transition-duration: .3s;
}

#checkboxInput:checked +.toggleSwitch .mute-speaker {
  opacity: 1;
  transition-duration: .3s;
}

#checkboxInput:active + .toggleSwitch {
  transform: scale(0.7);
}

#checkboxInput:hover + .toggleSwitch {
  background-color: rgb(61, 61, 61);
}`;
      const exactHtml = `<input type="checkbox" id="checkboxInput">
    <label for="checkboxInput" class="toggleSwitch">

<div class="speaker"><svg xmlns="http://www.w3.org/2000/svg" version="1.0" viewBox="0 0 75 75">
<path d="M39.389,13.769 L22.235,28.606 L6,28.606 L6,47.699 L21.989,47.699 L39.389,62.75 L39.389,13.769z" style="stroke:#fff;stroke-width:5;stroke-linejoin:round;fill:#fff;"></path>
<path d="M48,27.6a19.5,19.5 0 0 1 0,21.4M55.1,20.5a30,30 0 0 1 0,35.6M61.6,14a38.8,38.8 0 0 1 0,48.6" style="fill:none;stroke:#fff;stroke-width:5;stroke-linecap:round"></path>
</svg></div>

<div class="mute-speaker"><svg version="1.0" viewBox="0 0 75 75" stroke="#fff" stroke-width="5">
<path d="m39,14-17,15H6V48H22l17,15z" fill="#fff" stroke-linejoin="round"></path>
<path d="m49,26 20,24m0-24-20,24" fill="#fff" stroke-linecap="round"></path>
</svg></div>

    </label>`;
      return `
        <div class="${scope}" style="width: 100%; height: 100%; display: flex; align-items: center; justify-content: space-between; padding: 0 10px;">
          <style>${scopeUiverseCSS(scope, exactCss)}</style>
          <span style="font-size: 10px; font-weight: 700; color: ${p.textColor};">${p.label}</span>
          ${exactHtml}
        </div>
      `;
    }
  },

  uiverse_sun_moon: {
    name: "Day / Night Sun Moon Switch",
    category: "toggles",
    description: "Uiverse.io Day/Night theme toggle with celestial icons",
    defaultProps: {
      x: 36, y: 100, w: 168, h: 36,
      label: "DARK THEME",
      checked: true,
      textColor: "#ffffff"
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        display: flex; align-items: center; justify-content: space-between;
        padding: 0 4px;
      ">
        <span style="font-size: 10px; font-weight: 700; color: ${p.textColor};">${p.label}</span>
        <div style="
          width: 52px; height: 28px;
          background: ${p.checked ? '#1e293b' : '#38bdf8'};
          border-radius: 14px;
          position: relative;
          border: 1px solid rgba(255,255,255,0.2);
          display: flex; align-items: center; justify-content: space-between;
          padding: 0 6px;
        ">
          <span style="font-size: 10px;">🌙</span>
          <span style="font-size: 10px;">☀️</span>
          <div style="
            width: 22px; height: 22px;
            background: #ffffff;
            border-radius: 50%;
            position: absolute;
            top: 2px;
            left: ${p.checked ? '27px' : '3px'};
            box-shadow: 0 2px 5px rgba(0,0,0,0.3);
          "></div>
        </div>
      </div>
    `
  },

  action_dock: {
    name: "Action Dock (uiverse)",
    category: "toggles",
    description: "Uiverse.io Dark Action Toolbar with icon buttons",
    defaultProps: {
      x: 36, y: 160, w: 168, h: 40,
      bgColor: "#0f172a",
      borderColor: "#334155",
      iconColor: "#e2e8f0",
      radius: 10
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        background: ${p.bgColor};
        border: 1px solid ${p.borderColor};
        border-radius: ${p.radius}px;
        display: flex; align-items: center; justify-content: space-around;
        padding: 0 10px;
        box-shadow: 0 6px 18px rgba(0, 0, 0, 0.4);
      ">
        <span style="font-size: 14px; color: ${p.iconColor};">☁</span>
        <div style="width: 1px; height: 16px; background: rgba(255, 255, 255, 0.1);"></div>
        <span style="font-size: 14px; color: ${p.iconColor};">🗑</span>
        <div style="width: 1px; height: 16px; background: rgba(255, 255, 255, 0.1);"></div>
        <span style="font-size: 14px; color: ${p.iconColor};">⛶</span>
      </div>
    `
  },

  // ─────────────────────────────────────────────────────────────
  // ─── EXACT UIVERSE LOADERS ───
  // ─────────────────────────────────────────────────────────────
  alexruix_big_octopus: {
    name: "Animated Dot Loader (alexruix)",
    category: "loaders",
    description: "Exact Uiverse loader by alexruix with purple pulsing dot and text",
    defaultProps: {
      x: 80, y: 110, w: 80, h: 60,
      textColor: "#ffffff"
    },
    renderHTML: (p) => {
      const scope = "uiv-loader-octopus";
      const exactCss = `/* From Uiverse.io by alexruix - Tags: loader */
.loader {
  width: 80px;
  height: 50px;
  position: relative;
}

.loader-text {
  position: absolute;
  top: 0;
  padding: 0;
  margin: 0;
  color: #C8B6FF;
  animation: text_713 3.5s ease both infinite;
  font-size: .8rem;
  letter-spacing: 1px;
}

.load {
  background-color: #9A79FF;
  border-radius: 50px;
  display: block;
  height: 16px;
  width: 16px;
  bottom: 0;
  position: absolute;
  transform: translateX(64px);
  animation: loading_713 3.5s ease both infinite;
}

.load::before {
  position: absolute;
  content: "";
  width: 100%;
  height: 100%;
  background-color: #D1C2FF;
  border-radius: inherit;
  animation: loading2_713 3.5s ease both infinite;
}

@keyframes text_713 {
  0% {
    letter-spacing: 1px;
    transform: translateX(0px);
  }

  40% {
    letter-spacing: 2px;
    transform: translateX(26px);
  }

  80% {
    letter-spacing: 1px;
    transform: translateX(32px);
  }

  90% {
    letter-spacing: 2px;
    transform: translateX(0px);
  }

  100% {
    letter-spacing: 1px;
    transform: translateX(0px);
  }
}

@keyframes loading_713 {
  0% {
    width: 16px;
    transform: translateX(0px);
  }

  40% {
    width: 100%;
    transform: translateX(0px);
  }

  80% {
    width: 16px;
    transform: translateX(64px);
  }

  90% {
    width: 100%;
    transform: translateX(0px);
  }

  100% {
    width: 16px;
    transform: translateX(0px);
  }
}

@keyframes loading2_713 {
  0% {
    transform: translateX(0px);
    width: 16px;
  }

  40% {
    transform: translateX(0%);
    width: 80%;
  }

  80% {
    width: 100%;
    transform: translateX(0px);
  }

  90% {
    width: 80%;
    transform: translateX(15px);
  }

  100% {
    transform: translateX(0px);
    width: 16px;
  }
}`;
      const exactHtml = `<div class="loader">
    <span class="loader-text">loading</span>
      <span class="load"></span>
  </div>`;
      return `
        <div class="${scope}" style="width: 100%; height: 100%; display: flex; align-items: center; justify-content: center;">
          <style>${scopeUiverseCSS(scope, exactCss)}</style>
          ${exactHtml}
        </div>
      `;
    }
  },

  shoh_ancient_lionfish: {
    name: "3D Coin Flip (Shoh2008)",
    category: "loaders",
    description: "Exact Uiverse 3D golden coin flip loader by Shoh2008",
    defaultProps: {
      x: 85, y: 110, w: 70, h: 70,
      bgColor: "#ffd700",
      textColor: "#78350f"
    },
    renderHTML: (p) => {
      const scope = "uiv-loader-coin";
      const exactCss = `/* From Uiverse.io by Shoh2008 - Tags: loader */
.loader {
  transform: translateZ(1px);
}

.loader:after {
  content: '$';
  display: inline-block;
  width: 48px;
  height: 48px;
  border-radius: 50%;
  text-align: center;
  line-height: 40px;
  font-size: 32px;
  font-weight: bold;
  background: #FFD700;
  color: #DAA520;
  border: 4px double;
  box-sizing: border-box;
  box-shadow: 2px 2px 2px 1px rgba(0, 0, 0, .1);
  animation: coin-flip 4s cubic-bezier(0, 0.2, 0.8, 1) infinite;
}

@keyframes coin-flip {
  0%, 100% {
    animation-timing-function: cubic-bezier(0.5, 0, 1, 0.5);
  }

  0% {
    transform: rotateY(0deg);
  }

  50% {
    transform: rotateY(1800deg);
    animation-timing-function: cubic-bezier(0, 0.5, 0.5, 1);
  }

  100% {
    transform: rotateY(3600deg);
  }
}`;
      const exactHtml = `<div class="loader"></div>`;
      return `
        <div class="${scope}" style="width: 100%; height: 100%; display: flex; align-items: center; justify-content: center;">
          <style>${scopeUiverseCSS(scope, exactCss)}</style>
          ${exactHtml}
        </div>
      `;
    }
  },

  uiverse_orbit_loader: {
    name: "Orbiting Dots Loader",
    category: "loaders",
    description: "Uiverse.io glowing orbital particle loading spinner",
    defaultProps: {
      x: 80, y: 110, w: 80, h: 80,
      label: "LOADING...",
      color: "#00f2fe",
      textColor: "#94a3b8"
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        display: flex; flex-direction: column; align-items: center; justify-content: center; gap: 8px;
      ">
        <div style="
          width: 36px; height: 36px;
          border-radius: 50%;
          border: 3px solid rgba(0, 242, 254, 0.2);
          border-top-color: ${p.color};
          animation: spin 1s linear infinite;
          box-shadow: 0 0 10px ${p.color}60;
        "></div>
        <span style="font-size: 8px; font-weight: 800; color: ${p.textColor}; letter-spacing: 1px;">${p.label}</span>
      </div>
    `
  },

  // ─────────────────────────────────────────────────────────────
  // ─── EXACT UIVERSE CHECKBOXES & RADIOS ───
  // ─────────────────────────────────────────────────────────────
  adamgiebl_new_dingo: {
    name: "Soft Inset Checkbox (adamgiebl)",
    category: "checkboxes",
    description: "Exact Uiverse Neumorphic inset checkbox by adamgiebl",
    defaultProps: {
      x: 36, y: 120, w: 168, h: 36,
      label: "Checkbox",
      textColor: "#000000"
    },
    renderHTML: (p) => {
      const scope = "uiv-check-dingo";
      const exactCss = `/* From Uiverse.io by adamgiebl - Tags: neumorphism, skeuomorphism, checkbox */
.checkbox {
  display: flex;
  align-items: center;
  margin: 10px;
  font-family: Arial, sans-serif;
  color: black;
}

.checkbox input {
  display: none;
}

.checkbox .checkmark {
  width: 28px;
  height: 28px;
  border-radius: 10px;
  background-color: #ffffff2b;
  box-shadow: rgba(0, 0, 0, 0.62) 0px 0px 5px inset, rgba(0, 0, 0, 0.21) 0px 0px 0px 24px inset,
        #22cc3f 0px 0px 0px 0px inset, rgba(224, 224, 224, 0.45) 0px 1px 0px 0px;
  cursor: pointer;
  position: relative;
}

.checkbox .checkmark::after {
  content: "";
  width: 18px;
  height: 18px;
  border-radius: 5px;
  background-color: #e3e3e3;
  box-shadow: transparent 0px 0px 0px 2px, rgba(0, 0, 0, 0.3) 0px 6px 6px;
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  transition: background-color 0.3s ease-in-out;
}

.checkbox input:checked + .checkmark {
  background-color: #22cc3f;
  box-shadow: rgba(0, 0, 0, 0.62) 0px 0px 5px inset, #22cc3f 0px 0px 0px 2px inset, #22cc3f 0px 0px 0px 24px inset,
        rgba(224, 224, 224, 0.45) 0px 1px 0px 0px;
}

.checkbox input:checked + .checkmark::after {
  background-color: white;
}

.checkbox .label {
  margin-right: 10px;
  user-select: none;
  font-weight: 700;
  cursor: pointer;
}`;
      const exactHtml = `<label class="checkbox" for="checkbox1">
  <span class="label">Checkbox</span>
  <input checked="" id="checkbox1" type="checkbox">
  <span class="checkmark"></span>
</label>`;
      return `
        <div class="${scope}" style="width: 100%; height: 100%; display: flex; align-items: center; justify-content: center;">
          <style>${scopeUiverseCSS(scope, exactCss)}</style>
          ${exactHtml}
        </div>
      `;
    }
  },

  checkbox: {
    name: "Modern Checkbox",
    category: "checkboxes",
    description: "Checkbox with rounded border and checkmark",
    defaultProps: {
      x: 36, y: 130, w: 168, h: 28,
      label: "AUTO SLEEP",
      checked: true,
      color: "#10b981",
      textColor: "#ffffff"
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        display: flex; align-items: center; gap: 10px;
      ">
        <div style="
          width: 20px; height: 20px;
          border-radius: 5px;
          background: ${p.checked ? p.color : 'transparent'};
          border: 1.5px solid ${p.checked ? p.color : '#64748b'};
          display: flex; align-items: center; justify-content: center;
          font-size: 12px; color: #000; font-weight: bold;
        ">
          ${p.checked ? '✓' : ''}
        </div>
        <span style="font-size: 10px; font-weight: 700; color: ${p.textColor};">${p.label}</span>
      </div>
    `
  },

  // ─────────────────────────────────────────────────────────────
  // ─── NOTIFICATIONS ───
  // ─────────────────────────────────────────────────────────────
  alexruix_gentle_octopus: {
    name: "Level Up Toast (alexruix)",
    category: "notifications",
    description: "Exact Uiverse achievement notification by alexruix",
    defaultProps: {
      x: 16, y: 36, w: 208, h: 56,
      textColor: "#000000"
    },
    renderHTML: (p) => {
      const scope = "uiv-notif-octopus";
      const exactCss = `/* From Uiverse.io by alexruix - Tags: notification */
.notification {
  display: flex;
  width: 0;
  background: #ddd;
  border-radius: 10px;
  animation: slideIn 1s ease-in-out 1.25s forwards;
}

.notification-info {
  display: inline-flex;
  overflow: hidden;
  background-color: #181818;
  color: #fff;
  width: 100%;
  border-radius: 12px 8px 8px 12px;
  padding-left: 16px;
  align-items: center;
}

.notification-text {
  opacity: 0;
  padding: 10px 8px;
  white-space: nowrap;
  animation: textInfo 1s ease-in-out 1.5s forwards;
}

.highlight {
  color: #ffc300;
}

.box-avatar {
  background-color: #181818;
  padding: 6px;
  width: 4.5em;
  height: 4.5em;
  border-radius: 50%;
  margin-right: -20px;
  box-shadow: 6.2px 3.1px 38.2px -6px rgba(0, 0, 0, 0.2), 63px 32px 176px -6px rgba(0, 0, 0, 0.1);
  z-index: 1;
}

.avatar {
  background-color: #FDA203;
  padding: .6em;
  border-radius: 50%;
}

.avatar-icon {
  width: 100%;
  height: 100%;
  fill: white;
  animation: grow 2s ease-in-out 3;
}

/*Animations*/
@keyframes slideIn {
  0% {
    width: 0;
    padding: 2px;
  }

  100% {
    width: 250px;
    padding: 2px;
  }
}

@keyframes textInfo {
  0% {
    opacity: 0%;
    margin-left: -20em;
  }

  100% {
    opacity: 100%;
    margin-left: 0;
  }
}

@keyframes grow {
  0% {
    transform: scale(100%) translateY(0%);
  }

  50% {
    transform: scale(100%) translateY(-10%);
  }

  100% {
    transform: scale(100%) translateY(0%);
  }
}`;
      const exactHtml = `<div class="box-avatar">
  <div class="avatar">
      <svg class="avatar-icon" viewBox="0 0 17 15">
        <path d="M3 14s-1 0-1-1 1-4 6-4 6 3 6 4-1 1-1 1H3Zm5-6a3 3 0 1 0 0-6 3 3 0 0 0 0 6Z"></path>
      </svg>
    </div>
</div>
  <div class="notification">    
    <div class="notification-info">
      <p class="notification-text"> <span class="highlight">Player</span> reached <b>level 15!</b> </p>
    </div>
  </div>`;
      return `
        <div class="${scope}" style="width: 100%; height: 100%; display: flex; align-items: center; justify-content: center; position: relative;">
          <style>${scopeUiverseCSS(scope, exactCss)}</style>
          ${exactHtml}
        </div>
      `;
    }
  },

  card_notification: {
    name: "Notification Card",
    category: "notifications",
    description: "Incoming alert/message card with sender badge",
    defaultProps: {
      x: 16, y: 36, w: 208, h: 68,
      sender: "WHATSAPP",
      title: "Alex Mario",
      message: "Hey! Firmware update is ready.",
      time: "10:42",
      bgColor: "#111827",
      borderColor: "#22c55e",
      badgeColor: "#22c55e",
      textColor: "#ffffff",
      radius: 10,
      borderWidth: 1
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        background: ${p.bgColor};
        border: ${p.borderWidth}px solid ${p.borderColor};
        border-radius: ${p.radius}px;
        padding: 8px 10px;
        display: flex; flex-direction: column; justify-content: space-between;
      ">
        <div style="display: flex; justify-content: space-between; align-items: center;">
          <span style="background: ${p.badgeColor}; color: #000; font-size: 8px; font-weight: 900; padding: 2px 6px; border-radius: 4px;">${p.sender}</span>
          <span style="font-size: 8px; color: #94a3b8; font-family: var(--font-mono);">${p.time}</span>
        </div>
        <div style="font-size: 10px; font-weight: bold; color: ${p.textColor}; margin-top: 2px;">${p.title}</div>
        <div style="font-size: 8.5px; color: #94a3b8; overflow: hidden; text-overflow: ellipsis; white-space: nowrap;">${p.message}</div>
      </div>
    `
  },

  // ─────────────────────────────────────────────────────────────
  // ─── GAUGES ───
  // ─────────────────────────────────────────────────────────────
  progress_bar: {
    name: "Gradient Progress Bar",
    category: "gauges",
    description: "Linear progress meter with percentage display",
    defaultProps: {
      x: 20, y: 160, w: 200, h: 36,
      label: "CHARGING",
      value: 78,
      fillColor: "#00f2fe",
      bgColor: "#1e293b",
      textColor: "#ffffff",
      radius: 6
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        display: flex; flex-direction: column; justify-content: space-between;
      ">
        <div style="display: flex; justify-content: space-between; font-size: 9px; font-weight: bold; color: ${p.textColor};">
          <span>${p.label}</span>
          <span style="font-family: var(--font-mono); color: ${p.fillColor};">${p.value}%</span>
        </div>
        <div style="
          width: 100%; height: 12px;
          background: ${p.bgColor};
          border-radius: ${p.radius}px;
          overflow: hidden;
          padding: 2px;
        ">
          <div style="
            width: ${p.value}%; height: 100%;
            background: ${p.fillColor};
            border-radius: ${Math.max(2, p.radius - 2)}px;
            box-shadow: 0 0 8px ${p.fillColor}80;
          "></div>
        </div>
      </div>
    `
  },

  progress_ring: {
    name: "Radial Dial / Pomodoro",
    category: "gauges",
    description: "Circular clock dial with progress arc and tick marks",
    defaultProps: {
      x: 45, y: 40, w: 150, h: 150,
      label: "FOCUS",
      time: "24:59",
      value: 75,
      arcColor: "#ef4444",
      trackColor: "#1e293b",
      textColor: "#ffffff"
    },
    renderHTML: (p) => {
      const radius = 60;
      const circ = 2 * Math.PI * radius;
      const offset = circ - (p.value / 100) * circ;
      return `
        <div style="
          width: 100%; height: 100%;
          position: relative;
          display: flex; align-items: center; justify-content: center;
        ">
          <svg width="100%" height="100%" viewBox="0 0 150 150">
            <circle cx="75" cy="75" r="${radius}" fill="none" stroke="${p.trackColor}" stroke-width="8" />
            <circle cx="75" cy="75" r="${radius}" fill="none" stroke="${p.arcColor}" stroke-width="8"
              stroke-dasharray="${circ}" stroke-dashoffset="${offset}"
              stroke-linecap="round" transform="rotate(-90 75 75)" />
          </svg>
          <div style="position: absolute; text-align: center;">
            <div style="font-size: 8px; font-weight: 800; color: #94a3b8; letter-spacing: 1px;">${p.label}</div>
            <div style="font-size: 20px; font-weight: 900; color: ${p.textColor}; font-family: var(--font-mono);">${p.time}</div>
          </div>
        </div>
      `;
    }
  },

  battery_gauge: {
    name: "Battery Status Pill",
    category: "gauges",
    description: "4-segment battery level indicator",
    defaultProps: {
      x: 60, y: 20, w: 120, h: 28,
      percent: 85,
      isCharging: true,
      color: "#22c55e",
      textColor: "#ffffff"
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        background: #111827;
        border: 1px solid rgba(255, 255, 255, 0.1);
        border-radius: 14px;
        padding: 4px 10px;
        display: flex; align-items: center; justify-content: space-between;
      ">
        <div style="display: flex; align-items: center; gap: 4px;">
          ${p.isCharging ? '<span style="color: #f59e0b; font-size: 11px;">⚡</span>' : ''}
          <span style="font-size: 10px; font-weight: bold; font-family: var(--font-mono); color: ${p.textColor};">${p.percent}%</span>
        </div>
        <div style="
          width: 32px; height: 14px;
          border: 1.5px solid ${p.color};
          border-radius: 3px;
          padding: 1.5px;
          display: flex; gap: 1.5px;
          position: relative;
        ">
          <div style="width: 20%; height: 100%; background: ${p.percent >= 20 ? p.color : 'transparent'};"></div>
          <div style="width: 20%; height: 100%; background: ${p.percent >= 40 ? p.color : 'transparent'};"></div>
          <div style="width: 20%; height: 100%; background: ${p.percent >= 60 ? p.color : 'transparent'};"></div>
          <div style="width: 20%; height: 100%; background: ${p.percent >= 80 ? p.color : 'transparent'};"></div>
          <div style="position: absolute; right: -4px; top: 3px; width: 2px; height: 5px; background: ${p.color}; border-radius: 0 1px 1px 0;"></div>
        </div>
      </div>
    `
  },

  // ─────────────────────────────────────────────────────────────
  // ─── INPUTS ───
  // ─────────────────────────────────────────────────────────────
  uiverse_neu_input: {
    name: "Soft Inset Input Box",
    category: "inputs",
    description: "Uiverse.io Neumorphic inset text input field",
    defaultProps: {
      x: 20, y: 120, w: 200, h: 40,
      placeholder: "Enter Wi-Fi SSID...",
      textColor: "#0f172a",
      bgColor: "#f1f5f9",
      radius: 10
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        background: ${p.bgColor};
        border-radius: ${p.radius}px;
        box-shadow: inset 2px 2px 5px #d1d5db, inset -2px -2px 5px #ffffff;
        padding: 0 12px;
        display: flex; align-items: center;
        color: #94a3b8;
        font-size: 10px;
        font-family: var(--font-sans);
      ">
        ${p.placeholder}
      </div>
    `
  },

  uiverse_cyber_search: {
    name: "Cyber Search Bar",
    category: "inputs",
    description: "Uiverse.io Glowing futuristic search bar",
    defaultProps: {
      x: 20, y: 120, w: 200, h: 38,
      placeholder: "Search BLE beacons...",
      borderColor: "#00f2fe",
      bgColor: "#090d16",
      textColor: "#ffffff",
      radius: 19
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        background: ${p.bgColor};
        border: 1px solid ${p.borderColor};
        border-radius: ${p.radius}px;
        box-shadow: 0 0 10px ${p.borderColor}40;
        padding: 0 12px;
        display: flex; align-items: center; gap: 8px;
        color: #94a3b8;
        font-size: 10px;
      ">
        <span style="color: ${p.borderColor};">🔍</span>
        <span style="color: ${p.textColor}; opacity: 0.7;">${p.placeholder}</span>
      </div>
    `
  },

  // ─────────────────────────────────────────────────────────────
  // ─── TYPOGRAPHY & STATUS ───
  // ─────────────────────────────────────────────────────────────
  digital_clock: {
    name: "Digital Cyberpunk Clock",
    category: "text",
    description: "Prominent digital time with date and seconds",
    defaultProps: {
      x: 20, y: 36, w: 200, h: 72,
      time: "10:45",
      seconds: "28",
      date: "WED, SEP 9",
      color: "#00f2fe",
      dateColor: "#94a3b8"
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        display: flex; flex-direction: column; align-items: center; justify-content: center;
      ">
        <div style="display: flex; align-items: baseline; gap: 4px;">
          <span style="
            font-size: 38px;
            font-weight: 900;
            font-family: var(--font-mono);
            color: ${p.color};
            letter-spacing: -1px;
            text-shadow: 0 0 16px ${p.color}80;
          ">${p.time}</span>
          <span style="
            font-size: 14px;
            font-weight: bold;
            font-family: var(--font-mono);
            color: ${p.color};
            opacity: 0.7;
          ">${p.seconds}</span>
        </div>
        <div style="
          font-size: 9px;
          font-weight: 800;
          color: ${p.dateColor};
          letter-spacing: 1.5px;
          margin-top: 2px;
        ">${p.date}</div>
      </div>
    `
  },

  status_badge: {
    name: "Pill Status Badge",
    category: "text",
    description: "Compact status indicator badge",
    defaultProps: {
      x: 70, y: 24, w: 100, h: 22,
      text: "CONNECTED",
      color: "#10b981",
      textColor: "#ffffff"
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        background: ${p.color}22;
        border: 1px solid ${p.color};
        border-radius: 11px;
        display: flex; align-items: center; justify-content: center; gap: 5px;
      ">
        <div style="width: 5px; height: 5px; border-radius: 50%; background: ${p.color}; box-shadow: 0 0 6px ${p.color};"></div>
        <span style="font-size: 8px; font-weight: 800; color: ${p.textColor}; letter-spacing: 0.5px;">${p.text}</span>
      </div>
    `
  },

  label_text: {
    name: "Custom Label / Header",
    category: "text",
    description: "Customizable text label with font sizes",
    defaultProps: {
      x: 20, y: 20, w: 200, h: 30,
      text: "LUNA SMART WATCH",
      color: "#ffffff",
      fontSize: 12,
      align: "center",
      weight: "bold"
    },
    renderHTML: (p) => `
      <div style="
        width: 100%; height: 100%;
        display: flex; align-items: center;
        justify-content: ${p.align === 'left' ? 'flex-start' : p.align === 'right' ? 'flex-end' : 'center'};
      ">
        <span style="
          font-size: ${p.fontSize}px;
          font-weight: ${p.weight};
          color: ${p.color};
          letter-spacing: 0.5px;
        ">${p.text}</span>
      </div>
    `
  }
};

/**
 * Register a dynamic element imported from the Galaxy Library
 */
function registerGalaxyComponent(item) {
  if (UI_COMPONENTS[item.id]) return item.id;
  
  const scopedId = `galaxy-comp-${item.id.replace(/[^a-zA-Z0-9_-]/g, '')}`;
  
  UI_COMPONENTS[item.id] = {
    name: item.name + ` (${item.author})`,
    category: item.category || "cards",
    description: `Imported from Uiverse.io by ${item.author} (${item.tags || 'galaxy'})`,
    isGalaxy: true,
    author: item.author,
    defaultProps: {
      x: 20, y: 50, w: 200, h: 64,
      title: item.name,
      bgColor: "#1e293b",
      textColor: "#ffffff"
    },
    renderHTML: (p) => `
      <div class="${scopedId}" style="
        width: 100%; height: 100%;
        display: flex; align-items: center; justify-content: center;
        overflow: hidden; position: relative;
        transform-origin: center center;
      ">
        <style>${scopeUiverseCSS(scopedId, item.css)}</style>
        ${item.html}
      </div>
    `
  };
  
  return item.id;
}
