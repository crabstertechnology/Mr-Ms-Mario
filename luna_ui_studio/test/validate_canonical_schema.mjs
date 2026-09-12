/**
 * Comprehensive Test & Validation Suite for Luna UI Canonical Schema (Phase 1.5)
 *
 * Validates:
 * 1. Device Profile & Hardware Capabilities Specification
 * 2. Schema Versioning & Extensible Migration Infrastructure
 * 3. Deep Round-Trip Fidelity (Legacy -> Canonical -> Legacy) across all archetypes:
 *    - text, digital_clock
 *    - card_glass, card_stat
 *    - button_neon
 *    - uiv_btn_happy_coding
 *    - uiv_btn_damith_yellow
 *    - toggle_switch, checkbox_neon
 *    - gauge_progress
 *    - screen navigation actions
 *    - scroll actions
 *    - custom properties
 * 4. Deterministic Canonical Serialization (byte-for-byte reproducibility & SHA-256 hash)
 * 5. Component Capability Contract & Unsupported-Feature Diagnostic Reporting
 * 6. Golden Parity Project Reference Fixture Validation
 */

import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import crypto from 'node:crypto'
import { fileURLToPath } from 'node:url'

import {
  legacyToCanonical,
  canonicalToLegacy,
  validateCanonicalProject,
  serializeCanonicalProject,
  sortKeysRecursively,
  WAVESHARE_ESP32S3_169,
  WAVESHARE_ESP32C3_13,
  getDefaultDevice,
  getDeviceProfile,
  CURRENT_SCHEMA_VERSION,
  SUPPORTED_SCHEMA_VERSIONS,
  parseSchemaVersion,
  isVersionSupported,
  compareVersions,
  identifySchemaVersion,
  registerMigration,
  migrateProject,
  COMPONENT_CAPABILITIES,
  defineComponent,
  ARCHETYPE_PROFILES,
  validateProjectCapabilities,
  SEVERITY,
} from '../src/ui-core/index.js'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

console.log('═════════════════════════════════════════════════════════════════')
console.log('🧪 LUNA UI STUDIO — PHASE 1.5 ARCHITECTURE HARDENING TEST SUITE')
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

// ── Test 1: Device Capabilities Contract ─────────────────────
runTest('Device Capabilities Contract (WAVESHARE_ESP32S3_169 & ESP32-C3)', () => {
  const s3 = getDefaultDevice()
  assert.equal(s3.id, 'waveshare_esp32s3_touch_lcd_169')
  assert.equal(s3.width, 240)
  assert.equal(s3.height, 280)
  assert.equal(s3.colorDepth, 16)
  assert.equal(s3.touch, true)
  assert.equal(s3.displayController, 'ST7789')
  assert.equal(s3.touchController, 'CST816T')

  // Capabilities contract verification
  const caps = s3.capabilities
  assert.ok(caps, 'Device must define capabilities object')
  assert.equal(caps.maxWidth, 240)
  assert.equal(caps.maxHeight, 280)
  assert.equal(caps.colorDepth, 16)
  assert.equal(caps.supportsTouch, true)
  assert.equal(caps.supportsDoubleBuffer, true)
  assert.equal(caps.supportsImages, true)
  assert.equal(caps.supportsCustomFonts, true)
  assert.equal(caps.supportsOpacity, false)
  assert.equal(caps.supportsRotation, false)
  assert.equal(caps.supportsScale, false)
  assert.equal(caps.supportsAnimation, false)
  assert.equal(caps.supports3DTransform, false)
  assert.equal(caps.supportsBlur, false)

  // Compare with non-touch C3 profile
  const c3 = getDeviceProfile('waveshare_esp32c3_lcd_13')
  assert.equal(c3.touch, false)
  assert.equal(c3.capabilities.supportsTouch, false)
})

// ── Test 2: Schema Versioning & Migrations Infrastructure ─────
runTest('Schema Versioning & Extensible Migration Engine', () => {
  assert.equal(CURRENT_SCHEMA_VERSION, '1.0.0')
  assert.ok(isVersionSupported('1.0.0'))
  assert.equal(isVersionSupported('2.0.0'), false)

  // Version parsing and comparison
  const parsed = parseSchemaVersion('1.0.0')
  assert.deepEqual(parsed, { major: 1, minor: 0, patch: 0 })
  assert.equal(compareVersions('1.0.0', '1.0.0'), 0)
  assert.equal(compareVersions('1.1.0', '1.0.0'), 1)
  assert.equal(compareVersions('1.0.0', '1.2.0'), -1)

  // Schema identification
  assert.equal(identifySchemaVersion({ version: '1.0.0' }), '1.0.0')
  assert.equal(identifySchemaVersion({ screens: [{ elements: [{ props: { x: 10 } }] }] }), 'legacy')
  assert.equal(identifySchemaVersion({}), 'unknown')

  // Migration registration and execution
  registerMigration('1.0.0', '1.1.0', (proj) => {
    return {
      ...proj,
      version: '1.1.0',
      migratedField: true,
    }
  })

  // Mock migration execution
  const dummyProj = { version: '1.0.0', name: 'Test' }
  const migrated = migrateProject(dummyProj, '1.1.0')
  assert.equal(migrated.version, '1.1.0')
  assert.equal(migrated.migratedField, true)

  // Future version error handling
  assert.throws(() => {
    migrateProject({ version: '9.9.9' }, '1.0.0')
  }, /is newer than current supported version/)
})

// ── Test 3: Deep Round-Trip Fidelity ─────────────────────────
runTest('Deep Round-Trip Fidelity (Legacy -> Canonical -> Legacy)', () => {
  const legacyScreens = [
    {
      id: 'screen_1',
      name: 'Primary Screen',
      bgColor: '#0b0f19',
      bgType: 'pattern',
      bgPattern: 'stars',
      isScrollable: true,
      maxScrollY: 560,
      gestures: {
        swipeLeft: { actionType: 'navigate', targetScreenId: 'screen_2', transition: 'slide-left' },
        swipeRight: { actionType: 'none', targetScreenId: null, transition: 'slide-right' },
      },
      elements: [
        // 1. Text (custom_label)
        {
          id: 'el_header',
          type: 'custom_label',
          name: 'Main Header',
          actions: [],
          props: {
            x: 16,
            y: 12,
            w: 208,
            h: 36,
            text: 'MISSION CONTROL',
            color: '#38bdf8',
            fontSize: 16,
            fontWeight: 800,
            bgColor: '#1e293b',
            radius: 8,
            customTag: 'HDR_01', // Custom property
          },
        },
        // 2. Digital Clock
        {
          id: 'el_clock',
          type: 'digital_clock',
          name: 'Telemetry Clock',
          actions: [],
          props: {
            x: 20,
            y: 54,
            w: 200,
            h: 60,
            timeStr: '14:22:08',
            dateStr: 'THU, 10 SEP',
            color: '#00f2fe',
            dateColor: '#94a3b8',
          },
        },
        // 3. Card (card_glass)
        {
          id: 'el_glass_card',
          type: 'card_glass',
          name: 'Glass Container',
          actions: [],
          props: {
            x: 16,
            y: 120,
            w: 208,
            h: 70,
            title: 'Telemetry Stream',
            subtitle: 'Nominal 60Hz',
            bgColor: '#1e293bcc',
            borderColor: '#00f2fe44',
            textColor: '#ffffff',
            subtextColor: '#94a3b8',
            radius: 14,
          },
        },
        // 4. Card (card_stat)
        {
          id: 'el_stat_card',
          type: 'card_stat',
          name: 'Battery Metric',
          actions: [],
          props: {
            x: 16,
            y: 196,
            w: 96,
            h: 56,
            label: 'BATTERY',
            value: '94',
            unit: '%',
            bgColor: '#0f172a',
            borderColor: '#1e293b',
            accentColor: '#10b981',
            textColor: '#ffffff',
            radius: 10,
          },
        },
        // 5. Standard Button (button_neon)
        {
          id: 'el_btn_neon',
          type: 'button_neon',
          name: 'Neon Trigger Button',
          actions: [
            {
              id: 'act_scroll_down',
              trigger: 'onClick',
              actionType: 'scroll',
              scrollDirection: 'down',
              scrollAmount: 120,
            },
          ],
          props: {
            x: 120,
            y: 196,
            w: 104,
            h: 56,
            label: 'SCROLL',
            bgColor: '#0f172a',
            borderColor: '#00f2fe',
            textColor: '#00f2fe',
            radius: 8,
          },
        },
        // 6. Happy Coding UIverse Button
        {
          id: 'el_uiv_happy',
          type: 'uiv_btn_happy_coding',
          name: 'Happy Coding Button',
          actions: [
            {
              id: 'act_nav_screen_2',
              trigger: 'onClick',
              actionType: 'navigate',
              targetScreenId: 'screen_2',
              transition: 'slide-left',
              alertMessage: 'Switching to Screen 2...',
            },
          ],
          props: {
            x: 20,
            y: 260,
            w: 200,
            h: 48,
            label: 'Happy Coding!',
            bgColor: '#EEF2FF',
            borderColor: '#536DFE',
            textColor: '#536DFE',
          },
        },
        // 7. Damith Yellow UIverse Button
        {
          id: 'el_uiv_damith',
          type: 'uiv_btn_damith_yellow',
          name: 'Damith 3D Button',
          actions: [],
          props: {
            x: 20,
            y: 316,
            w: 200,
            h: 48,
            label: 'SYSTEM VERIFY',
            bgColor: '#363636',
            borderColor: '#ffff00',
            textColor: '#ffff00',
          },
        },
        // 8. Toggle Switch
        {
          id: 'el_toggle',
          type: 'toggle_switch',
          name: 'WiFi Toggle',
          actions: [],
          props: {
            x: 20,
            y: 372,
            w: 140,
            h: 30,
            label: 'WiFi Link',
            checked: true,
            accentColor: '#10b981',
            bgColor: '#1e293b',
          },
        },
        // 9. Checkbox
        {
          id: 'el_checkbox',
          type: 'checkbox_neon',
          name: 'Telemetry Checkbox',
          actions: [],
          props: {
            x: 20,
            y: 410,
            w: 160,
            h: 28,
            label: 'Logging Active',
            checked: false,
            accentColor: '#10b981',
            bgColor: '#0f172a',
            textColor: '#e2e8f0',
          },
        },
        // 10. Gauge (gauge_progress)
        {
          id: 'el_gauge',
          type: 'gauge_progress',
          name: 'CPU Gauge',
          actions: [],
          props: {
            x: 16,
            y: 446,
            w: 208,
            h: 40,
            label: 'CPU Core 0',
            value: 68,
            fillColor: '#38bdf8',
            bgColor: '#1e293b',
            borderColor: '#334155',
            textColor: '#ffffff',
            radius: 6,
            telemetrySensorId: 'SENS_TEMP_0', // Custom property
          },
        },
      ],
    },
    {
      id: 'screen_2',
      name: 'Secondary Screen',
      bgColor: '#000000',
      bgType: 'color',
      bgPattern: 'none',
      isScrollable: false,
      maxScrollY: 280,
      gestures: {
        swipeLeft: { actionType: 'none', targetScreenId: null, transition: 'slide-left' },
        swipeRight: { actionType: 'navigate', targetScreenId: 'screen_1', transition: 'slide-right' },
      },
      elements: [
        {
          id: 'el_back_btn',
          type: 'button_neon',
          name: 'Return Button',
          actions: [
            {
              id: 'act_return_home',
              trigger: 'onClick',
              actionType: 'navigate',
              targetScreenId: 'screen_1',
              transition: 'slide-right',
            },
          ],
          props: {
            x: 40,
            y: 200,
            w: 160,
            h: 44,
            label: 'RETURN',
            bgColor: '#0f172a',
            borderColor: '#00f2fe',
            textColor: '#00f2fe',
            radius: 8,
          },
        },
      ],
    },
  ]

  // Step 1: Legacy -> Canonical
  const canonical = legacyToCanonical(legacyScreens, 'screen_1', WAVESHARE_ESP32S3_169, { deterministic: true })
  const validation = validateCanonicalProject(canonical)
  assert.equal(validation.valid, true, `Canonical project invalid: ${validation.errors.join(', ')}`)

  // Step 2: Canonical -> Legacy
  const reconstructed = canonicalToLegacy(canonical)

  // Step 3: Deep comparison of all meaningful user-editable values
  assert.equal(reconstructed.screens.length, legacyScreens.length, 'Screen count mismatch')
  assert.equal(reconstructed.activeScreenId, 'screen_1')

  const discrepancies = []

  legacyScreens.forEach((origScreen, sIdx) => {
    const recScreen = reconstructed.screens[sIdx]
    assert.equal(recScreen.id, origScreen.id, `Screen ID mismatch at ${sIdx}`)
    assert.equal(recScreen.name, origScreen.name, `Screen name mismatch at ${sIdx}`)
    assert.equal(recScreen.bgColor, origScreen.bgColor, `Screen bgColor mismatch at ${sIdx}`)
    assert.equal(recScreen.bgType, origScreen.bgType, `Screen bgType mismatch at ${sIdx}`)
    assert.equal(recScreen.bgPattern, origScreen.bgPattern, `Screen bgPattern mismatch at ${sIdx}`)
    assert.equal(recScreen.isScrollable, origScreen.isScrollable, `Screen isScrollable mismatch at ${sIdx}`)
    assert.equal(recScreen.maxScrollY, origScreen.maxScrollY, `Screen maxScrollY mismatch at ${sIdx}`)

    // Compare Gestures
    assert.deepEqual(recScreen.gestures.swipeLeft, origScreen.gestures.swipeLeft)
    assert.deepEqual(recScreen.gestures.swipeRight, origScreen.gestures.swipeRight)

    assert.equal(recScreen.elements.length, origScreen.elements.length, `Element count mismatch on ${origScreen.id}`)

    origScreen.elements.forEach((origEl, eIdx) => {
      const recEl = recScreen.elements[eIdx]
      assert.equal(recEl.id, origEl.id, `Element id mismatch at ${origEl.id}`)
      assert.equal(recEl.type, origEl.type, `Element type mismatch at ${origEl.id}`)
      assert.equal(recEl.name, origEl.name, `Element name mismatch at ${origEl.id}`)

      // Compare actions
      assert.equal(recEl.actions.length, origEl.actions.length, `Actions count mismatch for ${origEl.id}`)
      origEl.actions.forEach((origAct, aIdx) => {
        const recAct = recEl.actions[aIdx]
        assert.equal(recAct.trigger, origAct.trigger)
        assert.equal(recAct.actionType, origAct.actionType)
        if (origAct.targetScreenId !== undefined) assert.equal(recAct.targetScreenId, origAct.targetScreenId)
        if (origAct.transition !== undefined) assert.equal(recAct.transition, origAct.transition)
        if (origAct.scrollDirection !== undefined) assert.equal(recAct.scrollDirection, origAct.scrollDirection)
        if (origAct.scrollAmount !== undefined) assert.equal(recAct.scrollAmount, origAct.scrollAmount)
        if (origAct.alertMessage !== undefined) assert.equal(recAct.alertMessage, origAct.alertMessage)
      })

      // Compare every property in props
      Object.keys(origEl.props).forEach((propKey) => {
        const origVal = origEl.props[propKey]
        const recVal = recEl.props[propKey]

        if (recVal === undefined) {
          discrepancies.push({
            elementId: origEl.id,
            elementType: origEl.type,
            property: propKey,
            issue: 'LOST',
            originalValue: origVal,
            reconstructedValue: recVal,
          })
        } else if (recVal !== origVal) {
          discrepancies.push({
            elementId: origEl.id,
            elementType: origEl.type,
            property: propKey,
            issue: 'COERCED_OR_MODIFIED',
            originalValue: origVal,
            reconstructedValue: recVal,
          })
        }
      })
    })
  })

  if (discrepancies.length > 0) {
    console.error('  ⚠️  DISCREPANCIES DETECTED IN ROUND-TRIP FIDELITY:')
    discrepancies.forEach((d) => console.error(`     - [${d.issue}] ${d.elementType}.${d.property}: ${d.originalValue} -> ${d.reconstructedValue}`))
    assert.fail(`Round-trip fidelity check failed with ${discrepancies.length} property discrepancies.`)
  } else {
    console.log('     ✓ 10/10 component archetypes preserved 100% of user-editable properties with zero loss or coercion.')
  }
})

// ── Test 4: Deterministic Canonical Serialization ────────────
runTest('Deterministic Compilation & Serialization (Byte-for-byte identical output)', () => {
  const fixturePath = path.join(__dirname, 'fixtures', 'golden-parity-project.json')
  const goldenRaw = JSON.parse(fs.readFileSync(fixturePath, 'utf8'))

  // Serialize twice
  const serialized1 = serializeCanonicalProject(goldenRaw, { indent: 2, stripTimestamps: true })
  const serialized2 = serializeCanonicalProject(goldenRaw, { indent: 2, stripTimestamps: true })

  // Byte-for-byte check
  assert.equal(serialized1, serialized2, 'Canonical serialization must be byte-for-byte identical')

  // Hash check
  const hash1 = crypto.createHash('sha256').update(serialized1).digest('hex')
  const hash2 = crypto.createHash('sha256').update(serialized2).digest('hex')
  assert.equal(hash1, hash2, 'SHA-256 hashes must be identical')

  // Key order stability: shuffle keys in copy and ensure sorted output is identical
  const shuffled = {
    metadata: { ...goldenRaw.metadata },
    screens: goldenRaw.screens,
    fonts: goldenRaw.fonts || [],
    version: goldenRaw.version,
    assets: goldenRaw.assets || [],
    device: goldenRaw.device,
    name: goldenRaw.name,
    activeScreenId: goldenRaw.activeScreenId,
  }

  const serializedShuffled = serializeCanonicalProject(shuffled, { indent: 2, stripTimestamps: true })
  assert.equal(serializedShuffled, serialized1, 'Serialization must be invariant to object property insertion order')
  console.log(`     ✓ Deterministic SHA-256: ${hash1}`)
})

// ── Test 5: Component Capability Contract & Feature Diagnostics
runTest('Component Capability Contract & Unsupported-Feature Diagnostics', () => {
  // Check Component Capabilities enumeration
  assert.ok(COMPONENT_CAPABILITIES.STATIC_RENDER)
  assert.ok(COMPONENT_CAPABILITIES.TEXT)
  assert.ok(COMPONENT_CAPABILITIES.IMAGE)
  assert.ok(COMPONENT_CAPABILITIES.TOUCH)
  assert.ok(COMPONENT_CAPABILITIES.STATE)
  assert.ok(COMPONENT_CAPABILITIES.ANIMATION)
  assert.ok(COMPONENT_CAPABILITIES.ROTATION)
  assert.ok(COMPONENT_CAPABILITIES.SCALE)
  assert.ok(COMPONENT_CAPABILITIES.OPACITY)
  assert.ok(COMPONENT_CAPABILITIES.CUSTOM_EMBEDDED_RENDERER)

  // Define component with capability contract
  const customDef = defineComponent({
    type: 'my_custom_dial',
    displayName: 'Rotary Dial',
    capabilities: [
      COMPONENT_CAPABILITIES.STATIC_RENDER,
      COMPONENT_CAPABILITIES.TOUCH,
      COMPONENT_CAPABILITIES.ROTATION,
    ],
    supportedStates: ['normal', 'active'],
    supportedEvents: ['onClick', 'onRotate'],
    supportedAnimations: ['rotation'],
  })

  assert.equal(customDef.type, 'my_custom_dial')
  assert.ok(customDef.capabilities.includes(COMPONENT_CAPABILITIES.ROTATION))
  assert.ok(customDef.supportedEvents.includes('onRotate'))

  // Validate Project with unsupported features:
  // 1. Damith Yellow 3D rotateY on ESP32-S3
  // 2. Opacity < 1
  // 3. Touch event on non-touch ESP32-C3 device
  const testProject = {
    version: '1.0.0',
    device: WAVESHARE_ESP32S3_169,
    screens: [
      {
        id: 'screen_diag',
        name: 'Diagnostics Screen',
        children: [
          {
            id: 'node_damith',
            type: 'uiv_btn_damith_yellow',
            name: '3D Damith Button',
            layout: { x: 10, y: 10, width: 100, height: 40 },
            style: { opacity: 0.8 }, // Opacity unsupported on hardware
            events: [{ id: 'e1', trigger: 'onClick', action: { type: 'custom' } }],
          },
        ],
      },
    ],
  }

  // Diagnostic report against standard S3
  const s3Report = validateProjectCapabilities(testProject, WAVESHARE_ESP32S3_169)
  assert.ok(s3Report.hasWarnings, 'Should report warnings for unsupported hardware features')

  const rotateYIssue = s3Report.report.find((r) => r.feature === 'animation.rotateY')
  assert.ok(rotateYIssue, 'Must detect 3D rotateY transform')
  assert.equal(rotateYIssue.severity, SEVERITY.WARNING)
  assert.equal(rotateYIssue.nodeId, 'node_damith')
  assert.equal(rotateYIssue.message, '3D rotateY is not supported by the target device renderer.')

  const opacityIssue = s3Report.report.find((r) => r.feature === 'style.opacity')
  assert.ok(opacityIssue, 'Must detect unsupported opacity blending')
  assert.equal(opacityIssue.severity, SEVERITY.WARNING)

  // Diagnostic report against non-touch C3 device
  const c3Report = validateProjectCapabilities(testProject, WAVESHARE_ESP32C3_13)
  assert.ok(c3Report.hasErrors, 'Should report ERROR for touch events on non-touch device')
  const touchIssue = c3Report.report.find((r) => r.feature === 'input.touch')
  assert.ok(touchIssue, 'Must detect touch event on non-touch hardware')
  assert.equal(touchIssue.severity, SEVERITY.ERROR)

  console.log(`     ✓ Diagnostics correctly generated ${s3Report.summary.warnings} warnings and caught non-touch error.`)
})

// ── Test 6: Golden Parity Project Reference Fixture ───────────
runTest('Golden Parity Project Reference Fixture Validation', () => {
  const fixturePath = path.join(__dirname, 'fixtures', 'golden-parity-project.json')
  assert.ok(fs.existsSync(fixturePath), 'Golden fixture file must exist')

  const golden = JSON.parse(fs.readFileSync(fixturePath, 'utf8'))
  const validResult = validateCanonicalProject(golden)
  assert.equal(validResult.valid, true, `Golden fixture must be strictly valid schema: ${validResult.errors.join(', ')}`)

  assert.equal(golden.version, '1.0.0')
  assert.equal(golden.screens.length, 3)
  assert.equal(golden.screens[0].children.length, 7)
  assert.equal(golden.screens[1].children.length, 3)

  // Verify elements in golden fixture
  const s1Types = golden.screens[0].children.map((c) => c.type)
  assert.ok(s1Types.includes('custom_label'), 'Must contain heading text')
  assert.ok(s1Types.includes('card_glass'), 'Must contain card')
  assert.ok(s1Types.includes('uiv_btn_happy_coding'), 'Must contain button')
  assert.ok(s1Types.includes('loader_spinner'), 'Must contain spinner')
  assert.ok(s1Types.includes('image'), 'Must contain image emblem badge')

  const s2Types = golden.screens[1].children.map((c) => c.type)
  assert.ok(s2Types.includes('custom_label'), 'Must contain Screen 2 text')
  assert.ok(s2Types.includes('button_neon'), 'Must contain Screen 2 back button')

  // Verify navigation action
  const navBtn = golden.screens[0].children.find((c) => c.id === 'node_btn_navigate')
  assert.equal(navBtn.events[0].action.type, 'navigate')
  assert.equal(navBtn.events[0].action.targetScreenId, 'screen_secondary')

  const backBtn = golden.screens[1].children.find((c) => c.id === 'node_btn_return')
  assert.equal(backBtn.events[0].action.type, 'navigate')
  assert.equal(backBtn.events[0].action.targetScreenId, 'screen_main')

  // Verify state and animation definition
  assert.ok(navBtn.states.pressed, 'Button must have pressed state definition')
  const spinner = golden.screens[0].children.find((c) => c.id === 'node_spinner_1')
  assert.ok(spinner.animations.length > 0, 'Spinner must have animation definition')
  assert.equal(spinner.animations[0].property, 'rotation')

  // Verify asset presence
  assert.equal(golden.assets.length, 2)
  assert.equal(golden.assets[0].id, 'asset_luna_badge')

  // Convert Golden -> Legacy -> Canonical
  const legacyFromGolden = canonicalToLegacy(golden)
  const canonicalAgain = legacyToCanonical(legacyFromGolden.screens, legacyFromGolden.activeScreenId, golden.device, { deterministic: true })
  assert.equal(canonicalAgain.screens.length, 3)
  assert.equal(canonicalAgain.screens[0].children.length, 7)
})

console.log('═════════════════════════════════════════════════════════════════')
console.log(`🎉 ALL PHASE 1.5 ARCHITECTURE-HARDENING TESTS PASSED (${passedTests}/${totalTests})`)
console.log('═════════════════════════════════════════════════════════════════\n')
