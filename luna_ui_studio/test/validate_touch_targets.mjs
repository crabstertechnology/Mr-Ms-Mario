// ============================================================================
// Luna UI Studio — Touch-Target & Ergonomic Validation Test Suite
// Asserts that every interactive element conforms to wearable touch requirements:
//  - Preferred: >= 40x40 px
//  - Absolute Minimum: >= 34x34 px
//  - Screen Bounds: Must fit within 240x280 display width
// ============================================================================
import assert from 'node:assert/strict'
import { LUNA_CANONICAL_SCREENS } from '../src/data/canonicalScreens.js'

console.log('═════════════════════════════════════════════════════════════════')
console.log('🧪 LUNA UI STUDIO — TOUCH-TARGET ERGONOMIC VALIDATION SUITE')
console.log('═════════════════════════════════════════════════════════════════\n')

let totalElements = 0
let interactiveElements = 0
let passedPreferred = 0
let passedMinimum = 0
let failed = 0

const MINIMUM_TOUCH_SIZE = 34
const PREFERRED_TOUCH_SIZE = 40
const DISPLAY_WIDTH = 240

LUNA_CANONICAL_SCREENS.forEach((screen) => {
  console.log(`▶ Screen [${screen.id}] "${screen.name}":`)

  screen.elements.forEach((el) => {
    totalElements++
    const p = el.props || {}
    const w = p.w || 0
    const h = p.h || 0
    const isInteractive = (el.actions && el.actions.length > 0) ||
      el.type.includes('button') ||
      el.type.includes('toggle') ||
      el.type.includes('slider')

    if (isInteractive) {
      interactiveElements++

      // Test 1: Width & Height must meet absolute minimum (34x34)
      const meetsMinW = w >= MINIMUM_TOUCH_SIZE
      const meetsMinH = h >= MINIMUM_TOUCH_SIZE
      const meetsPrefW = w >= PREFERRED_TOUCH_SIZE
      const meetsPrefH = h >= PREFERRED_TOUCH_SIZE

      if (!meetsMinW || !meetsMinH) {
        console.error(`  ✗ FAIL [${el.id}] "${el.name}" (${w}x${h}px) is below minimum touch size (${MINIMUM_TOUCH_SIZE}px)`)
        failed++
      } else {
        passedMinimum++
        if (meetsPrefW && meetsPrefH) {
          passedPreferred++
          console.log(`  ✓ Preferred: [${el.type}] "${el.name}" — ${w}x${h}px (>= 40px)`)
        } else {
          console.log(`  ⚠ Acceptable Minimum: [${el.type}] "${el.name}" — ${w}x${h}px (>= 34px)`)
        }
      }

      // Test 2: Horizontal bounds fit within 240px
      const rightEdge = (p.x || 0) + w
      assert.ok(rightEdge <= DISPLAY_WIDTH, `[${el.id}] extends beyond display width: ${rightEdge} > ${DISPLAY_WIDTH}`)
    }
  })
})

console.log('\n═════════════════════════════════════════════════════════════════')
console.log(`SUMMARY: ${interactiveElements} interactive touch targets analyzed across ${LUNA_CANONICAL_SCREENS.length} screens.`)
console.log(`  Preferred (>=40x40px): ${passedPreferred}/${interactiveElements} (${Math.round((passedPreferred / interactiveElements) * 100)}%)`)
console.log(`  Minimum (>=34x34px)  : ${passedMinimum}/${interactiveElements} (${Math.round((passedMinimum / interactiveElements) * 100)}%)`)
console.log(`  Violations (<34px)   : ${failed}`)
console.log('═════════════════════════════════════════════════════════════════\n')

assert.equal(failed, 0, 'Zero touch-target size violations permitted in canonical screens.')
console.log('🎉 ALL TOUCH-TARGET ERGONOMIC VALIDATIONS PASSED!\n')
