/**
 * Automated Test Suite for Luna Embedded Compiler (Phase 2)
 *
 * Validates:
 * 1. Canonical project loading
 * 2. Device validation & capabilities check
 * 3. Rectangle & Card rendering data compilation
 * 4. Text node compilation
 * 5. Button state compilation (normal & pressed styles)
 * 6. Navigation compilation (semantic actions)
 * 7. Spinner animation data compilation
 * 8. Deterministic generated output (byte-for-byte reproducibility & SHA-256)
 */

import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import crypto from 'node:crypto'
import { fileURLToPath } from 'node:url'

import {
  compileCanonicalToCpp,
  hexToRgb565Hex,
} from '../src/compiler/embeddedCompiler.js'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

console.log('═════════════════════════════════════════════════════════════════')
console.log('🧪 LUNA UI STUDIO — PHASE 2 EMBEDDED COMPILER TEST SUITE')
console.log('═════════════════════════════════════════════════════════════════\n')

let totalTests = 0
let passedTests = 0

function runTest(name, fn) {
  totalTests++
  console.log(`▶ Test ${totalTests}: ${name}`)
  try {
    fn()
    passedTests++
    console.log(`  ✓ Passed\n`)
  } catch (err) {
    console.error(`  ✗ FAILED: ${err.message}\n`)
    throw err
  }
}

const fixturePath = path.join(__dirname, 'fixtures', 'golden-parity-project.json')
assert.ok(fs.existsSync(fixturePath), 'Golden parity fixture must exist')
const goldenProject = JSON.parse(fs.readFileSync(fixturePath, 'utf8'))

// ── Test 1: Canonical Project Loading ─────────────────────────
runTest('Canonical Project Loading & Pre-flight Verification', () => {
  assert.equal(goldenProject.version, '1.0.0')
  assert.equal(goldenProject.name, 'Luna Golden Parity Reference Project')
  assert.equal(goldenProject.screens.length, 3)
  assert.equal(goldenProject.screens[0].id, 'screen_main')
  assert.equal(goldenProject.screens[1].id, 'screen_secondary')
})

// ── Test 2: Device Validation & Capability Check ──────────────
runTest('Device Validation & Capability Diagnostics', () => {
  const result = compileCanonicalToCpp(goldenProject)
  assert.ok(result.header)
  assert.ok(result.source)
  assert.ok(result.diagnostics)
  assert.equal(result.diagnostics.valid, true)
  assert.equal(result.diagnostics.summary.errors, 0)
  console.log(`     Diagnostics: ${result.diagnostics.summary.warnings} warnings, ${result.diagnostics.summary.infos} infos`)
})

// ── Test 3: Rectangle & Card Rendering Data Compilation ───────
runTest('Rectangle & Card Rendering Data Compilation', () => {
  const { source } = compileCanonicalToCpp(goldenProject)

  // Card node compilation verification
  assert.ok(source.includes('"node_card_1"'), 'Must include node_card_1')
  assert.ok(source.includes('LUNA_NODE_CARD'), 'Must map card to LUNA_NODE_CARD')
  assert.ok(source.includes('{ 16, 56, 208, 76 }'), 'Must compile exact layout coordinates [16, 56, 208, 76]')
  assert.ok(source.includes('"Core Reactor"'), 'Must compile title text')
  assert.ok(source.includes('"Nominal • 98.4% Efficiency"'), 'Must compile subtitle text')

  // Color conversion check: #0f172acc -> RGB565 hex
  const cardBgHex = hexToRgb565Hex('#0f172acc')
  assert.ok(source.includes(cardBgHex), `Source must contain card background color ${cardBgHex}`)
})

// ── Test 4: Text Node Compilation ─────────────────────────────
runTest('Text Node Semantic Typography Compilation', () => {
  const { source } = compileCanonicalToCpp(goldenProject)

  assert.ok(source.includes('"node_heading_1"'), 'Must include node_heading_1')
  assert.ok(source.includes('LUNA_NODE_TEXT'), 'Must map label to LUNA_NODE_TEXT')
  assert.ok(source.includes('"LUNA CORE OS"'), 'Must compile heading text')
  assert.ok(source.includes('LUNA_ALIGN_CENTER'), 'Must compile center alignment')
})

// ── Test 5: Button State Compilation (Normal & Pressed) ───────
runTest('Button State Compilation (Normal & Pressed States)', () => {
  const { source } = compileCanonicalToCpp(goldenProject)

  assert.ok(source.includes('"node_btn_navigate"'), 'Must include node_btn_navigate')
  assert.ok(source.includes('LUNA_NODE_BUTTON'), 'Must map button to LUNA_NODE_BUTTON')
  assert.ok(source.includes('"Go to Subsystems"'), 'Must compile button label')

  // Pressed style verification
  assert.ok(source.includes('s0_screen_main_n2_node_btn_navigate_pressedStyle'), 'Must declare pressed style struct')
  const pressedBgHex = hexToRgb565Hex('#c7d2fe')
  assert.ok(source.includes(pressedBgHex), `Must compile pressed background color ${pressedBgHex}`)
})

// ── Test 6: Navigation Compilation (Semantic Actions) ─────────
runTest('Navigation Compilation (Semantic Event & Action Mapping)', () => {
  const { source } = compileCanonicalToCpp(goldenProject)

  // Screen 1 Button -> Screen 2
  assert.ok(source.includes('s0_screen_main_n2_node_btn_navigate_events'), 'Must declare navigation event array')
  assert.ok(source.includes('LUNA_ACTION_NAVIGATE'), 'Must declare LUNA_ACTION_NAVIGATE')
  assert.ok(source.includes('"screen_secondary"'), 'Must target screen_secondary')

  // Screen 2 Return Button -> Screen 1
  assert.ok(source.includes('s1_screen_secondary_n1_node_btn_return_events'), 'Must declare return event array')
  assert.ok(source.includes('"screen_main"'), 'Must target screen_main')
})

// ── Test 7: Spinner Animation Data Compilation ────────────────
runTest('Spinner Animation Parameter Compilation', () => {
  const { source } = compileCanonicalToCpp(goldenProject)

  assert.ok(source.includes('s0_screen_main_n3_node_spinner_1_anims'), 'Must declare spinner animation array')
  assert.ok(source.includes('"rotation"'), 'Animation property must be rotation')
  assert.ok(source.includes('1200'), 'Animation duration must be 1200ms')
  assert.ok(source.includes('true'), 'Animation must loop')
  assert.ok(source.includes('"SYNCING"'), 'Must include spinner label')
})

// ── Test 8: Deterministic Generated Output ────────────────────
runTest('Deterministic Compilation (Byte-for-byte identical output)', () => {
  const res1 = compileCanonicalToCpp(goldenProject)
  const res2 = compileCanonicalToCpp(goldenProject)

  assert.equal(res1.header, res2.header, 'Generated headers must be byte-for-byte identical')
  assert.equal(res1.source, res2.source, 'Generated sources must be byte-for-byte identical')

  const hash1 = crypto.createHash('sha256').update(res1.source).digest('hex')
  const hash2 = crypto.createHash('sha256').update(res2.source).digest('hex')
  assert.equal(hash1, hash2, 'SHA-256 hashes must be identical')

  // Verify Mandatory Generated File Header
  const expectedHeader = '// ============================================================================\n// GENERATED FILE — DO NOT EDIT MANUALLY.'
  assert.ok(res1.header.startsWith(expectedHeader), 'Header must contain standard Generated File header')
  assert.ok(res1.source.startsWith(expectedHeader), 'Source must contain standard Generated File header')

  console.log(`     Deterministic Source SHA-256: ${hash1}`)
})

console.log('═════════════════════════════════════════════════════════════════')
console.log(`🎉 ALL PHASE 2 EMBEDDED COMPILER TESTS PASSED (${passedTests}/${totalTests})`)
console.log('═════════════════════════════════════════════════════════════════\n')
