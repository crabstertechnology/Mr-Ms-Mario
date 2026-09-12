# Luna UI & Firmware Architecture Rules for AI Agents

This repository governs the development and integration of the **Luna UI Studio** and its embedded hardware firmware target (Waveshare ESP32-S3 Touch LCD 1.69").

All AI agents (including Antigravity) working in this repository must strictly adhere to the following architectural boundaries and generation rules.

---

## 1. Architectural Directory Separation

The project maintains a strict conceptual separation of responsibilities:

- **`/src` (`luna_ui_studio/src/`)**:
  Studio source code only. Includes the React editor, Canvas layout engine, state machines, inspector panels, component registry, and canonical schema definitions (`src/ui-core/`).
- **`/generated` (`firmware testing/` or export output)**:
  Generated UI and firmware artifacts output by the Studio compiler. **These are build outputs, not source files.**
- **`/runtime` (`1.69 Luna Firmware/` & future embedded runtime)**:
  Handwritten embedded runtime code, HAL drivers, FreeRTOS tasks, Bluetooth LE, audio synthesizers, sensors, and graphics libraries.
- **`/compiler` (`luna_ui_studio/src/ui-core/` & `code-generator.js`)**:
  UI compilation and code generation pipelines converting the canonical UI schema into embedded target code.
- **`/device` (`luna_ui_studio/src/ui-core/device/`)**:
  Device-specific hardware profiles (display dimensions, color depth, controllers, pinouts, and timing constraints).

---

## 2. The Generated File Rule

> [!IMPORTANT]
> **GENERATED FILES ARE NOT SOURCE FILES.**
> Any generated UI or firmware file must contain the following standard header:
> ```text
> // ============================================================================
> // GENERATED FILE — DO NOT EDIT MANUALLY.
> // SOURCE: Luna UI Studio
> // REGENERATE FROM STUDIO.
> // ============================================================================
> ```

### Enforcement Guidelines for Agents:
1. **Never Manually Edit Generated Code**:
   Antigravity must **NEVER** manually modify generated UI definitions (e.g. `Luna_MultiScreen_App.ino`, `luna_ui_elements.h`, `drawGeneratedScreen.ino`) to "fix" a visual flaw, change coordinates, or add UI elements.
2. **Proper Fix Lifecycle**:
   If the generated hardware UI or build output is incorrect:
   - **Step 1**: Identify the root cause in the Studio, the Canonical UI Schema, the component registry, or the code generator compiler.
   - **Step 2**: Fix the issue in the appropriate source file (e.g. in `luna_ui_studio/src/`).
   - **Step 3**: Re-run the export generator to produce fresh output.
3. **Preserve Source of Truth**:
   The Canonical UI Schema (`UIProject`) is the sole architectural source of truth for UI layouts, styles, and interactions. Generated C++ and JSX are downstream artifacts.

---

## 3. Target Device Profile (Reference)

- **Target Board**: Waveshare ESP32-S3-Touch-LCD-1.69
- **Display Resolution**: 240 × 280 pixels
- **Color Depth**: 16-bit RGB565
- **Panel Offset**: X=0, Y=20
- **Touch Controller**: Hynitron CST816T (I2C address 0x15)
- **Power Hold Pin**: GPIO 41 (Must remain driven HIGH)
- **Display Driver**: ST7789 via 80MHz SPI with double-buffered PSRAM canvas
