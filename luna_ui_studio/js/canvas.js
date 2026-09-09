/**
 * Luna Display Studio - Interactive Canvas Engine
 * Manages the 240x280 display viewport, dragging, resizing, grid snapping, and selection
 */

class StudioCanvas {
  constructor(canvasContainerId, hudCallback) {
    this.container = document.getElementById(canvasContainerId);
    this.hudCallback = hudCallback;

    this.width = 240;
    this.height = 280;
    this.zoom = 1.5; // Default 150%
    this.gridSnap = 4; // 4px snap

    this.elements = [];
    this.selectedId = null;
    this.activeHandle = null;
    this.isDragging = false;
    this.isResizing = false;

    this.dragStartX = 0;
    this.dragStartY = 0;
    this.initialProps = {};

    this.history = [];
    this.historyIndex = -1;

    this.initDOM();
    this.initEvents();
  }

  initDOM() {
    this.container.innerHTML = `
      <div id="guide-x" class="guide-line-x"></div>
      <div id="guide-y" class="guide-line-y"></div>
      <div id="elements-layer"></div>
    `;
    this.elementsLayer = document.getElementById("elements-layer");
    this.guideX = document.getElementById("guide-x");
    this.guideY = document.getElementById("guide-y");
  }

  initEvents() {
    // Stage drag over & drop from palette
    const stage = document.getElementById("canvas-stage");
    stage.addEventListener("dragover", (e) => {
      e.preventDefault();
      e.dataTransfer.dropEffect = "copy";
    });

    stage.addEventListener("drop", (e) => {
      e.preventDefault();
      const type = e.dataTransfer.getData("text/plain");
      if (type && UI_COMPONENTS[type]) {
        const rect = this.container.getBoundingClientRect();
        let x = Math.round((e.clientX - rect.left) / this.zoom);
        let y = Math.round((e.clientY - rect.top) / this.zoom);
        this.addElement(type, x, y);
      }
    });

    // Deselect on clicking empty canvas
    this.container.addEventListener("pointerdown", (e) => {
      if (e.target === this.container || e.target === this.elementsLayer) {
        this.selectElement(null);
      }
    });

    // Window mouse move & up for smooth dragging outside element bounds
    window.addEventListener("pointermove", (e) => this.onPointerMove(e));
    window.addEventListener("pointerup", (e) => this.onPointerUp(e));

    // Keyboard shortcuts: Arrow keys (nudge), Delete, Ctrl+D (duplicate)
    window.addEventListener("keydown", (e) => this.onKeyDown(e));
  }

  setZoom(factor) {
    this.zoom = factor;
    const frame = document.querySelector(".watch-frame");
    if (frame) {
      frame.style.transform = `scale(${factor})`;
      frame.style.transformOrigin = "center center";
    }
    if (this.hudCallback) this.hudCallback(this.selectedElement());
  }

  setGridSnap(val) {
    this.gridSnap = parseInt(val, 10);
  }

  snap(val) {
    if (this.gridSnap <= 1) return Math.round(val);
    return Math.round(val / this.gridSnap) * this.gridSnap;
  }

  addElement(type, dropX = null, dropY = null) {
    const def = UI_COMPONENTS[type];
    if (!def) return;

    this.saveState();

    const id = "el_" + Date.now() + "_" + Math.floor(Math.random() * 1000);
    const props = JSON.parse(JSON.stringify(def.defaultProps));

    if (dropX !== null && dropY !== null) {
      props.x = this.snap(Math.max(0, Math.min(this.width - props.w, dropX - props.w / 2)));
      props.y = this.snap(Math.max(0, Math.min(this.height - props.h, dropY - props.h / 2)));
    }

    const element = {
      id,
      type,
      name: def.name,
      props
    };

    this.elements.push(element);
    this.renderElement(element);
    this.selectElement(id);
    this.saveState();
  }

  renderElement(element) {
    let elDiv = document.getElementById(element.id);
    if (!elDiv) {
      elDiv = document.createElement("div");
      elDiv.id = element.id;
      elDiv.className = "canvas-element";
      elDiv.innerHTML = `
        <div class="element-content" style="width: 100%; height: 100%;"></div>
        <div class="resize-handle handle-nw" data-handle="nw"></div>
        <div class="resize-handle handle-ne" data-handle="ne"></div>
        <div class="resize-handle handle-sw" data-handle="sw"></div>
        <div class="resize-handle handle-se" data-handle="se"></div>
        <div class="resize-handle handle-n"  data-handle="n"></div>
        <div class="resize-handle handle-s"  data-handle="s"></div>
        <div class="resize-handle handle-w"  data-handle="w"></div>
        <div class="resize-handle handle-e"  data-handle="e"></div>
      `;

      elDiv.addEventListener("pointerdown", (e) => this.onElementPointerDown(e, element.id));
      this.elementsLayer.appendChild(elDiv);
    }

    const p = element.props;
    elDiv.style.left = p.x + "px";
    elDiv.style.top = p.y + "px";
    elDiv.style.width = p.w + "px";
    elDiv.style.height = p.h + "px";

    const contentDiv = elDiv.querySelector(".element-content");
    const def = UI_COMPONENTS[element.type];
    if (def) {
      contentDiv.innerHTML = def.renderHTML(p);
    }
  }

  selectElement(id) {
    this.selectedId = id;
    document.querySelectorAll(".canvas-element").forEach((el) => {
      el.classList.toggle("selected", el.id === id);
    });

    if (this.hudCallback) {
      this.hudCallback(this.selectedElement());
    }
  }

  selectedElement() {
    return this.elements.find((el) => el.id === this.selectedId) || null;
  }

  onElementPointerDown(e, id) {
    e.stopPropagation();
    this.selectElement(id);

    const el = this.selectedElement();
    if (!el) return;

    this.initialProps = { ...el.props };
    this.dragStartX = e.clientX;
    this.dragStartY = e.clientY;

    if (e.target.dataset.handle) {
      this.isResizing = true;
      this.activeHandle = e.target.dataset.handle;
    } else {
      this.isDragging = true;
    }
  }

  onPointerMove(e) {
    if (!this.isDragging && !this.isResizing) return;
    const el = this.selectedElement();
    if (!el) return;

    const dx = (e.clientX - this.dragStartX) / this.zoom;
    const dy = (e.clientY - this.dragStartY) / this.zoom;

    if (this.isDragging) {
      let newX = this.snap(this.initialProps.x + dx);
      let newY = this.snap(this.initialProps.y + dy);

      // Clamp inside screen bounds
      newX = Math.max(0, Math.min(this.width - el.props.w, newX));
      newY = Math.max(0, Math.min(this.height - el.props.h, newY));

      el.props.x = newX;
      el.props.y = newY;
      this.renderElement(el);
      this.checkAlignmentGuides(el);
    } else if (this.isResizing) {
      this.applyResize(el, dx, dy, this.activeHandle);
    }

    if (this.hudCallback) this.hudCallback(el);
  }

  applyResize(el, dx, dy, handle) {
    let p = el.props;
    let init = this.initialProps;
    const minW = 24;
    const minH = 16;

    if (handle.includes("e")) {
      p.w = this.snap(Math.max(minW, Math.min(this.width - init.x, init.w + dx)));
    }
    if (handle.includes("s")) {
      p.h = this.snap(Math.max(minH, Math.min(this.height - init.y, init.h + dy)));
    }
    if (handle.includes("w")) {
      let right = init.x + init.w;
      let newX = this.snap(Math.max(0, Math.min(right - minW, init.x + dx)));
      p.x = newX;
      p.w = right - newX;
    }
    if (handle.includes("n")) {
      let bottom = init.y + init.h;
      let newY = this.snap(Math.max(0, Math.min(bottom - minH, init.y + dy)));
      p.y = newY;
      p.h = bottom - newY;
    }

    this.renderElement(el);
  }

  checkAlignmentGuides(el) {
    const cx = el.props.x + el.props.w / 2;
    const cy = el.props.y + el.props.h / 2;

    // Center guides
    if (Math.abs(cx - this.width / 2) < 3) {
      this.guideY.style.left = this.width / 2 + "px";
      this.guideY.style.display = "block";
    } else {
      this.guideY.style.display = "none";
    }

    if (Math.abs(cy - this.height / 2) < 3) {
      this.guideX.style.top = this.height / 2 + "px";
      this.guideX.style.display = "block";
    } else {
      this.guideX.style.display = "none";
    }
  }

  onPointerUp(e) {
    if (this.isDragging || this.isResizing) {
      this.isDragging = false;
      this.isResizing = false;
      this.activeHandle = null;
      this.guideX.style.display = "none";
      this.guideY.style.display = "none";
      this.saveState();
    }
  }

  onKeyDown(e) {
    const el = this.selectedElement();
    if (!el) return;

    // Ignore if typing in text input
    if (e.target.tagName === "INPUT" || e.target.tagName === "TEXTAREA") return;

    const step = e.shiftKey ? 8 : 1;

    if (e.key === "ArrowLeft") {
      e.preventDefault();
      el.props.x = Math.max(0, el.props.x - step);
      this.renderElement(el);
      this.saveState();
    } else if (e.key === "ArrowRight") {
      e.preventDefault();
      el.props.x = Math.min(this.width - el.props.w, el.props.x + step);
      this.renderElement(el);
      this.saveState();
    } else if (e.key === "ArrowUp") {
      e.preventDefault();
      el.props.y = Math.max(0, el.props.y - step);
      this.renderElement(el);
      this.saveState();
    } else if (e.key === "ArrowDown") {
      e.preventDefault();
      el.props.y = Math.min(this.height - el.props.h, el.props.y + step);
      this.renderElement(el);
      this.saveState();
    } else if (e.key === "Delete" || e.key === "Backspace") {
      e.preventDefault();
      this.deleteSelected();
    } else if (e.ctrlKey && e.key.toLowerCase() === "d") {
      e.preventDefault();
      this.duplicateSelected();
    } else if (e.ctrlKey && e.key.toLowerCase() === "z") {
      e.preventDefault();
      if (e.shiftKey) this.redo();
      else this.undo();
    }
  }

  duplicateSelected() {
    const el = this.selectedElement();
    if (!el) return;
    const newProps = JSON.parse(JSON.stringify(el.props));
    newProps.x = Math.min(this.width - newProps.w, newProps.x + 8);
    newProps.y = Math.min(this.height - newProps.h, newProps.y + 8);

    const id = "el_" + Date.now() + "_" + Math.floor(Math.random() * 1000);
    const newEl = { id, type: el.type, name: el.name + " Copy", props: newProps };
    this.elements.push(newEl);
    this.renderElement(newEl);
    this.selectElement(id);
    this.saveState();
  }

  deleteSelected() {
    if (!this.selectedId) return;
    const div = document.getElementById(this.selectedId);
    if (div) div.remove();
    this.elements = this.elements.filter((el) => el.id !== this.selectedId);
    this.selectElement(null);
    this.saveState();
  }

  clearAll() {
    this.elementsLayer.innerHTML = "";
    this.elements = [];
    this.selectElement(null);
    this.saveState();
  }

  // Z-Index ordering
  bringForward() {
    const idx = this.elements.findIndex((el) => el.id === this.selectedId);
    if (idx < this.elements.length - 1) {
      const temp = this.elements[idx];
      this.elements[idx] = this.elements[idx + 1];
      this.elements[idx + 1] = temp;
      this.rebuildDOMOrder();
      this.saveState();
    }
  }

  sendBackward() {
    const idx = this.elements.findIndex((el) => el.id === this.selectedId);
    if (idx > 0) {
      const temp = this.elements[idx];
      this.elements[idx] = this.elements[idx - 1];
      this.elements[idx - 1] = temp;
      this.rebuildDOMOrder();
      this.saveState();
    }
  }

  rebuildDOMOrder() {
    this.elements.forEach((el) => {
      const div = document.getElementById(el.id);
      if (div) this.elementsLayer.appendChild(div);
    });
  }

  // History Undo/Redo
  saveState() {
    const snapshot = JSON.stringify(this.elements);
    if (this.historyIndex >= 0 && this.history[this.historyIndex] === snapshot) return;
    this.history = this.history.slice(0, this.historyIndex + 1);
    this.history.push(snapshot);
    this.historyIndex = this.history.length - 1;
  }

  undo() {
    if (this.historyIndex > 0) {
      this.historyIndex--;
      this.loadSnapshot(this.history[this.historyIndex]);
    }
  }

  redo() {
    if (this.historyIndex < this.history.length - 1) {
      this.historyIndex++;
      this.loadSnapshot(this.history[this.historyIndex]);
    }
  }

  loadSnapshot(jsonStr) {
    this.elementsLayer.innerHTML = "";
    this.elements = JSON.parse(jsonStr);
    this.elements.forEach((el) => this.renderElement(el));
    this.selectElement(this.elements.length > 0 ? this.elements[this.elements.length - 1].id : null);
  }
}
