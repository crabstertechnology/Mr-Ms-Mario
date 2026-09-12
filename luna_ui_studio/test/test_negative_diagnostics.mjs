/**
 * Luna UI Studio — Phase 4 Negative Diagnostics Test Suite
 *
 * Tests that the compiler and validator correctly REJECT or WARN about
 * unsupported inputs before they reach the hardware.
 *
 * Each test uses a minimal project that deliberately contains an unsupported
 * feature, then asserts the correct severity and feature tag in the report.
 */

import assert from 'node:assert/strict'
import { validateProjectCapabilities, SEVERITY } from '../src/ui-core/validation/featureValidator.js'
import { WAVESHARE_ESP32S3_169, WAVESHARE_ESP32C3_13 } from '../src/ui-core/device/deviceProfiles.js'
import { compileAssetsToCpp } from '../src/compiler/assets/assetCompiler.js'
import { decodeBmp } from '../src/compiler/assets/pngDecoder.js'

console.log('═════════════════════════════════════════════════════════════════')
console.log('🧪 LUNA UI STUDIO — PHASE 4 NEGATIVE DIAGNOSTICS TEST SUITE')
console.log('═════════════════════════════════════════════════════════════════\n')

let total = 0
let passed = 0
const failures = []

function runTest(name, fn) {
  total++
  console.log(`▶ [${total}] ${name}`)
  try {
    fn()
    passed++
    console.log(`  ✓ Passed\n`)
  } catch (err) {
    failures.push({ name, error: err.message })
    console.error(`  ✗ FAILED: ${err.message}\n`)
  }
}

// ─────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────

function makeProject(screenChildren = [], overrides = {}) {
  return {
    version: '1.0.0',
    name: 'Diagnostic Test Project',
    device: {
      id: 'waveshare_esp32s3_touch_lcd_169',
      width: 240,
      height: 280,
      capabilities: WAVESHARE_ESP32S3_169.capabilities,
    },
    screens: [{
      id: 'screen_test',
      name: 'Test Screen',
      background: { type: 'solid', color: '#000000', pattern: 'none' },
      isScrollable: false,
      maxScrollY: 280,
      children: screenChildren,
    }],
    ...overrides,
  }
}

function makeNode(id, overrides = {}) {
  return {
    id,
    type: 'container',
    layout: { x: 10, y: 10, width: 50, height: 30 },
    style: {},
    typography: {},
    properties: {},
    events: [],
    animations: [],
    ...overrides,
  }
}

function hasReport(report, severity, feature) {
  return report.some(r => r.severity === severity && r.feature && r.feature.includes(feature))
}

function makeBmpBuffer(width, height, bpp, bottomUp = true) {
  const rowSize = Math.floor((bpp * width + 31) / 32) * 4
  const pixelDataSize = rowSize * Math.abs(height)
  const fileSize = 54 + pixelDataSize
  const buf = Buffer.alloc(fileSize, 0)
  buf[0] = 0x42; buf[1] = 0x4d
  buf.writeUInt32LE(fileSize, 2)
  buf.writeUInt32LE(54, 10)
  buf.writeUInt32LE(40, 14)
  buf.writeInt32LE(width, 18)
  buf.writeInt32LE(bottomUp ? height : -height, 22)
  buf.writeUInt16LE(1, 26)
  buf.writeUInt16LE(bpp, 28)
  for (let i = 54; i < fileSize; i++) buf[i] = 0xCC
  return buf
}

// ─────────────────────────────────────────────────────────────────
// 1. Opacity on device without opacity support
// ─────────────────────────────────────────────────────────────────

console.log('── 1. Opacity (unsupported) ─────────────────────────────────────\n')

runTest('opacity < 1 on WAVESHARE_ESP32S3_169 → WARNING style.opacity', () => {
  const node = makeNode('nd_opaque', { style: { opacity: 0.5 } })
  const project = makeProject([node])
  const result = validateProjectCapabilities(project, WAVESHARE_ESP32S3_169)
  assert.ok(hasReport(result.report, SEVERITY.WARNING, 'opacity'),
    'Must emit WARNING for opacity on device without supportsOpacity')
})

runTest('opacity === 1 (fully opaque) → no opacity warning', () => {
  const node = makeNode('nd_fully_opaque', { style: { opacity: 1 } })
  const project = makeProject([node])
  const result = validateProjectCapabilities(project, WAVESHARE_ESP32S3_169)
  const opacityWarnings = result.report.filter(r => r.feature?.includes('opacity'))
  assert.equal(opacityWarnings.length, 0, 'Fully opaque elements must not trigger opacity warnings')
})

// ─────────────────────────────────────────────────────────────────
// 2. Backdrop blur filter
// ─────────────────────────────────────────────────────────────────

console.log('── 2. Backdrop Blur (unsupported) ───────────────────────────────\n')

runTest('backdropFilter property → INFO style.blur', () => {
  const node = makeNode('nd_blur', { style: { backdropFilter: 'blur(10px)' } })
  const project = makeProject([node])
  const result = validateProjectCapabilities(project, WAVESHARE_ESP32S3_169)
  assert.ok(hasReport(result.report, SEVERITY.INFO, 'blur'),
    'Must emit INFO for backdrop blur on device without supportsBlur')
})

runTest('card_glass node type → INFO style.blur (glass implies blur)', () => {
  const node = makeNode('nd_glass', { type: 'card_glass' })
  const project = makeProject([node])
  const result = validateProjectCapabilities(project, WAVESHARE_ESP32S3_169)
  assert.ok(hasReport(result.report, SEVERITY.INFO, 'blur'),
    'card_glass type must trigger blur info diagnostic')
})

// ─────────────────────────────────────────────────────────────────
// 3. 3D transform (rotateY)
// ─────────────────────────────────────────────────────────────────

console.log('── 3. 3D Transform rotateY (unsupported) ────────────────────────\n')

runTest('animation with rotateY property → WARNING animation.rotateY', () => {
  const node = makeNode('nd_3d', {
    animations: [{ property: 'rotateY', fromValue: 0, toValue: 180, durationMs: 400, loop: false }]
  })
  const project = makeProject([node])
  const result = validateProjectCapabilities(project, WAVESHARE_ESP32S3_169)
  assert.ok(hasReport(result.report, SEVERITY.WARNING, 'rotateY'),
    'Must emit WARNING for rotateY animation')
})

runTest('uiv_btn_damith_yellow node type → WARNING animation.rotateY (implicit 3D)', () => {
  const node = makeNode('nd_damith', { type: 'uiv_btn_damith_yellow' })
  const project = makeProject([node])
  const result = validateProjectCapabilities(project, WAVESHARE_ESP32S3_169)
  assert.ok(hasReport(result.report, SEVERITY.WARNING, 'rotateY'),
    'Damith Yellow UIverse button must trigger rotateY warning')
})

// ─────────────────────────────────────────────────────────────────
// 4. Animation on device without animation support
// ─────────────────────────────────────────────────────────────────

console.log('── 4. Animation (unsupported on ESP32-C3) ───────────────────────\n')

runTest('animation on WAVESHARE_ESP32C3_13 → WARNING animation', () => {
  const node = makeNode('nd_anim', {
    animations: [{ property: 'rotation', fromValue: 0, toValue: 360, durationMs: 1000, loop: true }]
  })
  const project = makeProject([node], {
    device: {
      id: 'waveshare_esp32c3_lcd_13',
      width: 240, height: 240,
      capabilities: WAVESHARE_ESP32C3_13.capabilities,
    }
  })
  const result = validateProjectCapabilities(project, WAVESHARE_ESP32C3_13)
  assert.ok(hasReport(result.report, SEVERITY.WARNING, 'animation'),
    'Must emit WARNING for animation on device with supportsAnimation=false')
})

// ─────────────────────────────────────────────────────────────────
// 5. Touch on non-touch device
// ─────────────────────────────────────────────────────────────────

console.log('── 5. Touch Events on Non-Touch Device ──────────────────────────\n')

runTest('onClick event on WAVESHARE_ESP32C3_13 (no touch) → ERROR input.touch', () => {
  const node = makeNode('nd_btn_no_touch', {
    events: [{ trigger: 'onClick', action: { type: 'navigate', targetScreenId: 'x' } }]
  })
  const project = makeProject([node], {
    device: {
      id: 'waveshare_esp32c3_lcd_13',
      width: 240, height: 240,
      capabilities: WAVESHARE_ESP32C3_13.capabilities,
    }
  })
  const result = validateProjectCapabilities(project, WAVESHARE_ESP32C3_13)
  assert.ok(result.hasErrors, 'Must have errors for touch on non-touch device')
  assert.ok(hasReport(result.report, SEVERITY.ERROR, 'touch'),
    'Must emit ERROR for touch event on non-touch device')
})

// ─────────────────────────────────────────────────────────────────
// 6. Layout out of bounds
// ─────────────────────────────────────────────────────────────────

console.log('── 6. Layout Bounds Violations ──────────────────────────────────\n')

runTest('node with width > screen width → INFO layout.bounds', () => {
  // featureValidator checks `layout.width > screenWidth` (250 > 240 → true)
  const node = makeNode('nd_wide', { layout: { x: 0, y: 10, width: 250, height: 30 } })
  const project = makeProject([node])
  const result = validateProjectCapabilities(project, WAVESHARE_ESP32S3_169)
  assert.ok(hasReport(result.report, SEVERITY.INFO, 'bounds'),
    'Must emit INFO for node with width > display width')
})

runTest('node with negative x → INFO layout.bounds', () => {
  const node = makeNode('nd_neg_x', { layout: { x: -5, y: 10, width: 50, height: 30 } })
  const project = makeProject([node])
  const result = validateProjectCapabilities(project, WAVESHARE_ESP32S3_169)
  assert.ok(hasReport(result.report, SEVERITY.INFO, 'bounds'),
    'Must emit INFO for negative x coordinate')
})

runTest('node within bounds → no bounds INFO', () => {
  const node = makeNode('nd_safe', { layout: { x: 10, y: 10, width: 100, height: 50 } })
  const project = makeProject([node])
  const result = validateProjectCapabilities(project, WAVESHARE_ESP32S3_169)
  const boundsInfos = result.report.filter(r => r.feature?.includes('bounds'))
  assert.equal(boundsInfos.length, 0, 'In-bounds node must not trigger layout warnings')
})

// ─────────────────────────────────────────────────────────────────
// 7. Image on device without image support (ESP32-C3)
// ─────────────────────────────────────────────────────────────────

console.log('── 7. Image Asset on Non-Image Device ───────────────────────────\n')

runTest('image background on non-image device → WARNING background.image', () => {
  const project = {
    version: '1.0.0',
    name: 'Image Device Test',
    device: { id: 'waveshare_esp32c3_lcd_13', width: 240, height: 240, capabilities: WAVESHARE_ESP32C3_13.capabilities },
    screens: [{
      id: 'screen_img',
      name: 'Image Screen',
      background: { type: 'image', color: '#000000', assetId: 'some_img' },
      isScrollable: false,
      maxScrollY: 240,
      children: []
    }]
  }
  const result = validateProjectCapabilities(project, WAVESHARE_ESP32C3_13)
  assert.ok(hasReport(result.report, SEVERITY.WARNING, 'image'),
    'Must emit WARNING for image background on device without supportsImages')
})

// ─────────────────────────────────────────────────────────────────
// 8. Unknown/custom font on non-custom-font device
// ─────────────────────────────────────────────────────────────────

console.log('── 8. Custom Font on Non-Custom-Font Device ─────────────────────\n')

runTest('unknown font on device without supportsCustomFonts → INFO typography.fontFamily', () => {
  const node = makeNode('nd_font', {
    typography: { fontFamily: 'Roboto', fontSize: 14, color: '#ffffff', align: 'left' }
  })
  const project = makeProject([node], {
    device: { id: 'waveshare_esp32c3_lcd_13', width: 240, height: 240, capabilities: WAVESHARE_ESP32C3_13.capabilities }
  })
  const result = validateProjectCapabilities(project, WAVESHARE_ESP32C3_13)
  assert.ok(hasReport(result.report, SEVERITY.INFO, 'fontFamily'),
    'Must emit INFO for unknown font family on device without supportsCustomFonts')
})

runTest('Outfit font on device with supportsCustomFonts → no font warning', () => {
  const node = makeNode('nd_outfit', {
    typography: { fontFamily: 'Outfit', fontSize: 14, color: '#ffffff', align: 'left' }
  })
  const project = makeProject([node])
  const result = validateProjectCapabilities(project, WAVESHARE_ESP32S3_169)
  const fontInfos = result.report.filter(r => r.feature?.includes('fontFamily'))
  assert.equal(fontInfos.length, 0, 'Outfit (supported font) must not trigger font warning')
})

// ─────────────────────────────────────────────────────────────────
// 9. Asset Compile-Time: throws on missing pixel data
// ─────────────────────────────────────────────────────────────────

console.log('── 9. Asset Compile-Time Error: Missing Pixel Data ──────────────\n')

runTest('compileAssetsToCpp: asset with no buffer AND no pixels565 → throws', () => {
  assert.throws(
    () => compileAssetsToCpp([{ id: 'bad_asset' }]),
    /missing valid pixel data/i,
    'Must throw descriptive error for asset with no pixel data'
  )
})

runTest('compileAssetsToCpp: asset with zero-length pixels565 → throws', () => {
  assert.throws(
    () => compileAssetsToCpp([{ id: 'empty_asset', width: 0, height: 0, pixels565: new Uint16Array(0) }]),
    /missing valid pixel data/i,
    'Must throw descriptive error for zero-dimension asset'
  )
})

// ─────────────────────────────────────────────────────────────────
// 10. BMP: 16bpp silently produces black (documented defect)
// ─────────────────────────────────────────────────────────────────

console.log('── 10. BMP: 16bpp Silently Produces Black ───────────────────────\n')

runTest('16bpp BMP throws informative unsupported error', () => {
  const buf = makeBmpBuffer(4, 4, 16, true)
  assert.throws(() => decodeBmp(buf), /Unsupported BMP bit depth: 16 bpp/)
  console.log('  ✓ 16bpp BMP correctly rejected with clear diagnostic error.')
})

// ─────────────────────────────────────────────────────────────────
// 11. Rotation on unsupported device
// ─────────────────────────────────────────────────────────────────

console.log('── 11. 2D Rotation (unsupported) ────────────────────────────────\n')

runTest('layout.rotation !== 0 on WAVESHARE_ESP32S3_169 → WARNING layout.rotation', () => {
  const node = makeNode('nd_rotated', { layout: { x: 10, y: 10, width: 50, height: 30, rotation: 45 } })
  const project = makeProject([node])
  const result = validateProjectCapabilities(project, WAVESHARE_ESP32S3_169)
  assert.ok(hasReport(result.report, SEVERITY.WARNING, 'rotation'),
    'Must emit WARNING for non-zero rotation on device without supportsRotation')
})

runTest('layout.rotation === 0 → no rotation warning', () => {
  const node = makeNode('nd_no_rotate', { layout: { x: 10, y: 10, width: 50, height: 30, rotation: 0 } })
  const project = makeProject([node])
  const result = validateProjectCapabilities(project, WAVESHARE_ESP32S3_169)
  const rotWarnings = result.report.filter(r => r.feature?.includes('rotation'))
  assert.equal(rotWarnings.length, 0, 'Zero-rotation element must not trigger rotation warning')
})

// ─────────────────────────────────────────────────────────────────
// 12. Valid minimal project → no diagnostics
// ─────────────────────────────────────────────────────────────────

console.log('── 12. Valid Project → Zero Diagnostics ─────────────────────────\n')

runTest('Minimal valid project → valid=true, no errors, no warnings', () => {
  const project = makeProject([
    makeNode('clean_btn', {
      type: 'btn_standard',
      layout: { x: 10, y: 10, width: 80, height: 30 },
      typography: { fontFamily: 'Outfit', fontSize: 14, color: '#ffffff', align: 'center' },
      events: [{ trigger: 'onClick', action: { type: 'navigate', targetScreenId: 'screen_test' } }]
    })
  ])
  const result = validateProjectCapabilities(project, WAVESHARE_ESP32S3_169)
  assert.ok(result.valid, 'Minimal valid project must be valid')
  assert.ok(!result.hasErrors, 'Must have no errors')
})

// ─────────────────────────────────────────────────────────────────
// Summary
// ─────────────────────────────────────────────────────────────────

console.log('═════════════════════════════════════════════════════════════════')
console.log(`RESULT: ${passed}/${total} tests passed`)
if (failures.length > 0) {
  console.log('\nFAILURES:')
  failures.forEach((f, i) => console.error(`  [${i + 1}] ${f.name}\n      ${f.error}`))
  process.exit(1)
} else {
  console.log('✅ All negative diagnostics tests passed.')
  process.exit(0)
}
