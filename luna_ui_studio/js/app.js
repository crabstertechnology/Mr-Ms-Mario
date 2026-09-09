/**
 * Luna Display Studio - Main Application Controller
 */

let canvas;
let webSerial;

document.addEventListener("DOMContentLoaded", () => {
  initApp();
});

function initApp() {
  // 1. Initialize Canvas
  canvas = new StudioCanvas("screen-canvas", (selectedEl) => {
    updateHUD(selectedEl);
    updateInspector(selectedEl);
    updateLayersList();
  });

  // 2. Initialize Web Serial
  webSerial = new WebSerialBridge((connected, msg) => {
    const btn = document.getElementById("btn-serial");
    if (btn) {
      btn.innerHTML = connected ? `<span>⚡ Connected COM3</span>` : `<span>⚡ Connect COM3</span>`;
      btn.classList.toggle("active", connected);
    }
    showToast(msg);
  });

  // 3. Render Palette Items
  renderPalette();

  // 4. Setup Toolbar Events
  setupToolbar();

  // 5. Setup Inspector Events
  setupInspector();

  // 6. Setup Modal Events
  setupModal();

  // 7. Load default template (Dashboard)
  loadTemplate("dashboard");
}

/* ==========================================================================
   PALETTE RENDERING & FILTERING
   ========================================================================== */
function renderPalette(filterCategory = "all", searchQuery = "") {
  const list = document.getElementById("palette-list");
  list.innerHTML = "";

  const keys = Object.keys(UI_COMPONENTS);
  keys.forEach((key) => {
    const comp = UI_COMPONENTS[key];

    // Filter check
    if (filterCategory !== "all" && comp.category !== filterCategory) return;
    if (searchQuery && !comp.name.toLowerCase().includes(searchQuery.toLowerCase())) return;

    const card = document.createElement("div");
    card.className = "palette-item-card";
    card.draggable = true;
    card.dataset.type = key;

    const previewW = Math.min(comp.defaultProps.w, 310);
    const previewH = comp.defaultProps.h;

    card.innerHTML = `
      <div class="palette-item-header">
        <span class="palette-item-name">${comp.name}</span>
        <span class="palette-item-badge">${comp.category}</span>
      </div>
      <div class="palette-item-preview">
        <div style="width: ${previewW}px; height: ${previewH}px; max-width: 100%; position: relative;">
          ${comp.renderHTML(comp.defaultProps)}
        </div>
      </div>
      <div class="palette-item-footer">
        <span>Click or Drag to add</span>
        <button class="palette-add-btn" type="button">+ Add</button>
      </div>
    `;

    // Click to add to canvas
    card.addEventListener("click", () => {
      canvas.addElement(key, 120, 140);
    });

    // Drag to canvas
    card.addEventListener("dragstart", (e) => {
      e.dataTransfer.setData("text/plain", key);
      e.dataTransfer.effectAllowed = "copy";
    });

    list.appendChild(card);
  });
}

/* ==========================================================================
   TOOLBAR SETUP
   ========================================================================== */
function setupToolbar() {
  // Category tabs
  document.querySelectorAll(".cat-tab").forEach((tab) => {
    tab.addEventListener("click", (e) => {
      document.querySelectorAll(".cat-tab").forEach((t) => t.classList.remove("active"));
      e.target.classList.add("active");
      const cat = e.target.dataset.cat;
      const q = document.getElementById("palette-search").value;
      renderPalette(cat, q);
    });
  });

  // Search input
  const searchInput = document.getElementById("palette-search");
  searchInput.addEventListener("input", (e) => {
    const activeTab = document.querySelector(".cat-tab.active");
    const cat = activeTab ? activeTab.dataset.cat : "all";
    renderPalette(cat, e.target.value);
  });

  // Presets select
  const presetSelect = document.getElementById("preset-select");
  presetSelect.addEventListener("change", (e) => {
    if (e.target.value) {
      loadTemplate(e.target.value);
    }
  });

  // Zoom select
  const zoomSelect = document.getElementById("zoom-select");
  zoomSelect.addEventListener("change", (e) => {
    canvas.setZoom(parseFloat(e.target.value));
  });

  // Grid snap select
  const snapSelect = document.getElementById("snap-select");
  snapSelect.addEventListener("change", (e) => {
    canvas.setGridSnap(e.target.value);
  });

  // Undo / Redo
  document.getElementById("btn-undo").addEventListener("click", () => canvas.undo());
  document.getElementById("btn-redo").addEventListener("click", () => canvas.redo());

  // Delete & Duplicate
  document.getElementById("btn-dup").addEventListener("click", () => canvas.duplicateSelected());
  document.getElementById("btn-del").addEventListener("click", () => canvas.deleteSelected());
  document.getElementById("btn-clear").addEventListener("click", () => {
    if (confirm("Clear all elements from the display canvas?")) {
      canvas.clearAll();
    }
  });

  // Export Code Modal
  document.getElementById("btn-export").addEventListener("click", () => openExportModal());

  // Web Serial button
  const serialBtn = document.getElementById("btn-serial");
  if (serialBtn) {
    serialBtn.addEventListener("click", async () => {
      if (webSerial.isConnected) {
        await webSerial.disconnect();
      } else {
        await webSerial.connect();
      }
    });
  }
}

/* ==========================================================================
   INSPECTOR PROPERTIES PANEL
   ========================================================================== */
function setupInspector() {
  // Tab switching: Properties vs Layers
  document.getElementById("tab-props").addEventListener("click", () => {
    document.getElementById("tab-props").classList.add("active");
    document.getElementById("tab-layers").classList.remove("active");
    document.getElementById("inspector-props").style.display = "flex";
    document.getElementById("inspector-layers").style.display = "none";
  });

  document.getElementById("tab-layers").addEventListener("click", () => {
    document.getElementById("tab-layers").classList.add("active");
    document.getElementById("tab-props").classList.remove("active");
    document.getElementById("inspector-props").style.display = "none";
    document.getElementById("inspector-layers").style.display = "flex";
    updateLayersList();
  });
}

function updateHUD(el) {
  const hudPos = document.getElementById("hud-pos");
  const hudSize = document.getElementById("hud-size");
  if (el) {
    hudPos.textContent = `X: ${el.props.x}, Y: ${el.props.y}`;
    hudSize.textContent = `W: ${el.props.w}, H: ${el.props.h}`;
  } else {
    hudPos.textContent = `X: --, Y: --`;
    hudSize.textContent = `W: --, H: --`;
  }
}

function updateInspector(el) {
  const container = document.getElementById("inspector-props");
  if (!el) {
    container.innerHTML = `
      <div style="text-align: center; color: var(--text-muted); padding: 40px 10px; font-size: 12px;">
        <div style="font-size: 24px; margin-bottom: 8px;">✦</div>
        No element selected.<br>Select or drop an element on the canvas to inspect its properties.
      </div>
    `;
    return;
  }

  const p = el.props;
  let html = `
    <!-- Element Type & Name -->
    <div class="prop-section">
      <div class="prop-section-title">Element Info</div>
      <div class="prop-row">
        <span class="prop-label">Type</span>
        <span style="font-size: 11px; font-family: var(--font-mono); color: var(--accent-cyan);">${el.name}</span>
      </div>
    </div>

    <!-- Geometry -->
    <div class="prop-section">
      <div class="prop-section-title">Geometry (Pixels)</div>
      <div class="prop-input-grid">
        <div class="prop-field">
          <span class="field-prefix">X</span>
          <input type="number" class="prop-input" id="prop-x" value="${p.x}">
        </div>
        <div class="prop-field">
          <span class="field-prefix">Y</span>
          <input type="number" class="prop-input" id="prop-y" value="${p.y}">
        </div>
        <div class="prop-field">
          <span class="field-prefix">W</span>
          <input type="number" class="prop-input" id="prop-w" value="${p.w}">
        </div>
        <div class="prop-field">
          <span class="field-prefix">H</span>
          <input type="number" class="prop-input" id="prop-h" value="${p.h}">
        </div>
      </div>
      ${p.radius !== undefined ? `
        <div class="prop-row" style="margin-top: 6px;">
          <span class="prop-label">Radius</span>
          <div class="prop-field" style="width: 80px;">
            <input type="number" class="prop-input" id="prop-radius" value="${p.radius}">
          </div>
        </div>
      ` : ''}
    </div>

    <!-- Content & Text -->
    <div class="prop-section">
      <div class="prop-section-title">Content & Text</div>
      ${Object.keys(p).filter(k => typeof p[k] === 'string' && !p[k].startsWith('#') && !p[k].startsWith('rgba')).map(k => `
        <div class="prop-row">
          <span class="prop-label">${k}</span>
          <div class="prop-field" style="flex: 1;">
            <input type="text" class="prop-input" data-prop="${k}" value="${escapeHtml(p[k])}">
          </div>
        </div>
      `).join('')}

      ${p.value !== undefined && typeof p.value === 'number' ? `
        <div class="prop-row">
          <span class="prop-label">Value (%)</span>
          <div class="prop-field" style="width: 80px;">
            <input type="number" class="prop-input" id="prop-val-num" value="${p.value}" min="0" max="100">
          </div>
        </div>
      ` : ''}

      ${p.checked !== undefined ? `
        <div class="prop-row">
          <span class="prop-label">Checked</span>
          <input type="checkbox" id="prop-checked" ${p.checked ? 'checked' : ''} style="cursor: pointer;">
        </div>
      ` : ''}
    </div>

    <!-- Colors & Styling -->
    <div class="prop-section">
      <div class="prop-section-title">Colors & 16-Bit RGB565</div>
      ${Object.keys(p).filter(k => typeof p[k] === 'string' && (p[k].startsWith('#') || p[k].startsWith('rgba'))).map(k => {
        const hex = p[k].startsWith('#') ? p[k] : '#00f2fe';
        const rgb565 = CodeGenerator.hexToRGB565(hex);
        return `
          <div class="prop-row" style="margin-bottom: 6px;">
            <span class="prop-label">${k}</span>
            <div class="color-picker-row">
              <div class="color-preview-box" style="background: ${hex};">
                <input type="color" class="color-input-native" data-color-prop="${k}" value="${hex}">
              </div>
              <span class="color-hex-text">${hex}</span>
              <span class="color-rgb565-badge">${rgb565}</span>
            </div>
          </div>
        `;
      }).join('')}

      <!-- Swatches -->
      <div style="font-size: 10px; color: var(--text-muted); margin-top: 4px;">Preset Neon Palettes:</div>
      <div class="swatches-grid">
        ${['#00f2fe', '#4facfe', '#7928ca', '#ec4899', '#f59e0b', '#10b981', '#ef4444', '#ffffff', '#0f172a', '#1e293b'].map(c => `
          <div class="swatch-dot" style="background: ${c};" data-swatch="${c}"></div>
        `).join('')}
      </div>
    </div>
  `;

  container.innerHTML = html;

  // Bind geometry events
  const bindNum = (id, propKey) => {
    const input = document.getElementById(id);
    if (input) {
      input.addEventListener("input", (e) => {
        el.props[propKey] = parseInt(e.target.value, 10) || 0;
        canvas.renderElement(el);
      });
    }
  };

  bindNum("prop-x", "x");
  bindNum("prop-y", "y");
  bindNum("prop-w", "w");
  bindNum("prop-h", "h");
  bindNum("prop-radius", "radius");
  bindNum("prop-val-num", "value");

  // Bind text inputs
  container.querySelectorAll("input[data-prop]").forEach((input) => {
    input.addEventListener("input", (e) => {
      const k = e.target.dataset.prop;
      el.props[k] = e.target.value;
      canvas.renderElement(el);
    });
  });

  // Bind checkbox
  const chk = document.getElementById("prop-checked");
  if (chk) {
    chk.addEventListener("change", (e) => {
      el.props.checked = e.target.checked;
      canvas.renderElement(el);
    });
  }

  // Bind color pickers
  container.querySelectorAll("input[data-color-prop]").forEach((input) => {
    input.addEventListener("input", (e) => {
      const k = e.target.dataset.colorProp;
      el.props[k] = e.target.value;
      canvas.renderElement(el);
      updateInspector(el);
    });
  });

  // Bind swatches to the primary color
  container.querySelectorAll(".swatch-dot").forEach((dot) => {
    dot.addEventListener("click", (e) => {
      const color = e.target.dataset.swatch;
      if (el.props.borderColor !== undefined) el.props.borderColor = color;
      else if (el.props.color !== undefined) el.props.color = color;
      else if (el.props.fillColor !== undefined) el.props.fillColor = color;
      canvas.renderElement(el);
      updateInspector(el);
    });
  });
}

/* ==========================================================================
   LAYERS LIST
   ========================================================================== */
function updateLayersList() {
  const container = document.getElementById("inspector-layers");
  if (!container) return;

  if (canvas.elements.length === 0) {
    container.innerHTML = `<div style="text-align: center; color: var(--text-muted); padding: 40px 10px; font-size: 12px;">No layers yet</div>`;
    return;
  }

  let html = `<div class="layers-list">`;
  // Reverse order so top layer is first
  [...canvas.elements].reverse().forEach((el) => {
    const isSelected = el.id === canvas.selectedId;
    html += `
      <div class="layer-item ${isSelected ? 'active' : ''}" data-id="${el.id}">
        <div class="layer-left">
          <span style="color: var(--accent-cyan); font-size: 11px;">✦</span>
          <span style="font-weight: 600;">${el.name}</span>
        </div>
        <div class="layer-actions">
          <button class="layer-action-btn" title="Bring Forward" onclick="canvas.bringForward()">▲</button>
          <button class="layer-action-btn" title="Send Backward" onclick="canvas.sendBackward()">▼</button>
          <button class="layer-action-btn" title="Delete" onclick="canvas.deleteSelected()">✕</button>
        </div>
      </div>
    `;
  });
  html += `</div>`;
  container.innerHTML = html;

  container.querySelectorAll(".layer-item").forEach((item) => {
    item.addEventListener("click", (e) => {
      if (!e.target.classList.contains("layer-action-btn")) {
        canvas.selectElement(item.dataset.id);
      }
    });
  });
}

/* ==========================================================================
   CODE EXPORT MODAL & TEMPLATES
   ========================================================================== */
function setupModal() {
  document.getElementById("modal-close").addEventListener("click", closeExportModal);
  document.getElementById("modal-backdrop").addEventListener("click", (e) => {
    if (e.target.id === "modal-backdrop") closeExportModal();
  });

  // Modal tab switching
  document.querySelectorAll(".modal-tab").forEach((tab) => {
    tab.addEventListener("click", (e) => {
      document.querySelectorAll(".modal-tab").forEach((t) => t.classList.remove("active"));
      e.target.classList.add("active");
      const targetTab = e.target.dataset.tab;
      renderModalCode(targetTab);
    });
  });

  // Copy code
  document.getElementById("btn-copy-code").addEventListener("click", () => {
    const text = document.getElementById("code-output").value;
    navigator.clipboard.writeText(text).then(() => {
      showToast("✓ Code copied to clipboard!");
    });
  });

  // Download file
  document.getElementById("btn-download-code").addEventListener("click", () => {
    const activeTab = document.querySelector(".modal-tab.active").dataset.tab;
    const text = document.getElementById("code-output").value;
    let filename = "drawGeneratedScreen.ino";
    if (activeTab === "header") filename = "luna_ui_elements.h";
    else if (activeTab === "json") filename = "luna_screen_layout.json";

    const blob = new Blob([text], { type: "text/plain" });
    const a = document.createElement("a");
    a.href = URL.createObjectURL(blob);
    a.download = filename;
    a.click();
    showToast(`✓ Downloaded ${filename}`);
  });
}

function openExportModal() {
  document.getElementById("modal-backdrop").classList.add("open");
  const activeTab = document.querySelector(".modal-tab.active").dataset.tab || "arduino";
  renderModalCode(activeTab);
}

function closeExportModal() {
  document.getElementById("modal-backdrop").classList.remove("open");
}

function renderModalCode(tab) {
  const codeArea = document.getElementById("code-output");
  if (tab === "arduino") {
    codeArea.value = CodeGenerator.generateArduinoCode(canvas.elements);
  } else if (tab === "header") {
    codeArea.value = CodeGenerator.generateHelperHeader();
  } else if (tab === "json") {
    codeArea.value = JSON.stringify(canvas.elements, null, 2);
  }
}

async function loadTemplate(name) {
  try {
    const resp = await fetch(`templates/${name}.json`);
    if (!resp.ok) throw new Error("Template not found");
    const elements = await resp.json();
    canvas.loadSnapshot(JSON.stringify(elements));
    showToast(`Loaded "${name}" template`);
  } catch (e) {
    console.warn("Could not load template file directly, using fallback preset:", e);
    // Fallback built-in preset
    if (name === "dashboard") {
      canvas.clearAll();
      canvas.addElement("digital_clock", 120, 50);
      canvas.addElement("card_glass", 120, 140);
      canvas.addElement("button_neon", 120, 220);
    }
  }
}

function showToast(msg) {
  let toast = document.getElementById("studio-toast");
  if (!toast) {
    toast = document.createElement("div");
    toast.id = "studio-toast";
    toast.style.cssText = `
      position: fixed; bottom: 24px; right: 24px;
      background: rgba(15, 23, 42, 0.95);
      border: 1px solid var(--accent-cyan);
      color: #fff; padding: 10px 18px; border-radius: 8px;
      font-size: 12px; font-weight: 600; z-index: 2000;
      box-shadow: 0 4px 20px rgba(0, 242, 254, 0.3);
      transition: opacity 0.2s; pointer-events: none;
    `;
    document.body.appendChild(toast);
  }
  toast.textContent = msg;
  toast.style.opacity = "1";
  setTimeout(() => { toast.style.opacity = "0"; }, 2500);
}

function escapeHtml(str) {
  return String(str).replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;").replace(/"/g, "&quot;");
}


