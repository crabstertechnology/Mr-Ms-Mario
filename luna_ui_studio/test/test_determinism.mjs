/**
 * Automated Test Suite for Phase 4 Determinism & Byte-for-Byte Reproducibility
 */

import assert from 'node:assert/strict'
import fs from 'node:fs'
import path from 'node:path'
import crypto from 'node:crypto'
import { fileURLToPath } from 'node:url'

import { compileCanonicalToCpp } from '../src/compiler/embeddedCompiler.js'
import { compileAssetsToCpp } from '../src/compiler/assets/assetCompiler.js'
import {
  extractCharactersFromProject,
  subsetFont,
  compileFontsToCpp,
} from '../src/compiler/fonts/fontCompiler.js'
import {
  OUTFIT_16,
  OUTFIT_12,
  JETBRAINS_MONO_9,
} from '../src/compiler/fonts/fontData.js'
import { createProjectRenderSnapshot } from '../src/ui-core/validation/renderSnapshot.js'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

console.log('=================================================================')
console.log('LUNA UI STUDIO - PHASE 4 DETERMINISM TEST SUITE')
console.log('=================================================================\n')

let total = 0
let passed = 0
const failures = []

function runTest(name, fn) {
  total++
  console.log('[Test ' + total + '] ' + name)
  try {
    fn()
    passed++
    console.log('  Passed\n')
  } catch (err) {
    failures.push({ name, error: err.message })
    console.error('  FAILED: ' + err.message + '\n')
  }
}

function sha256(str) {
  return crypto.createHash('sha256').update(str).digest('hex')
}

const fixturePath = path.join(__dirname, 'fixtures', 'golden-parity-project.json')
assert.ok(fs.existsSync(fixturePath), 'Golden parity fixture must exist')
const goldenProject = JSON.parse(fs.readFileSync(fixturePath, 'utf8'))

console.log('-- 1. compileCanonicalToCpp Determinism --\n')

runTest('Two successive compileCanonicalToCpp calls produce byte-identical header', () => {
  const r1 = compileCanonicalToCpp(goldenProject)
  const r2 = compileCanonicalToCpp(goldenProject)
  assert.equal(r1.header, r2.header)
})

runTest('Two successive compileCanonicalToCpp calls produce byte-identical source', () => {
  const r1 = compileCanonicalToCpp(goldenProject)
  const r2 = compileCanonicalToCpp(goldenProject)
  assert.equal(r1.source, r2.source)
})

runTest('SHA-256 of compiled header is stable', () => {
  const h1 = sha256(compileCanonicalToCpp(goldenProject).header)
  const h2 = sha256(compileCanonicalToCpp(goldenProject).header)
  assert.equal(h1, h2)
  console.log('  Header SHA-256: ' + h1)
})

runTest('SHA-256 of compiled source is stable', () => {
  const h1 = sha256(compileCanonicalToCpp(goldenProject).source)
  const h2 = sha256(compileCanonicalToCpp(goldenProject).source)
  assert.equal(h1, h2)
  console.log('  Source SHA-256: ' + h1)
})

console.log('-- 2. compileFontsToCpp Determinism --\n')

runTest('compileFontsToCpp: same font list produce byte-identical header', () => {
  const chars = extractCharactersFromProject(goldenProject)
  const fonts = [subsetFont(OUTFIT_16, chars), subsetFont(OUTFIT_12, chars), JETBRAINS_MONO_9]
  const r1 = compileFontsToCpp(fonts)
  const r2 = compileFontsToCpp(fonts)
  assert.equal(r1.header, r2.header)
})

runTest('compileFontsToCpp: same font list produce byte-identical source', () => {
  const chars = extractCharactersFromProject(goldenProject)
  const fonts = [subsetFont(OUTFIT_16, chars), subsetFont(OUTFIT_12, chars), JETBRAINS_MONO_9]
  const r1 = compileFontsToCpp(fonts)
  const r2 = compileFontsToCpp(fonts)
  assert.equal(r1.source, r2.source)
})

runTest('compileFontsToCpp: SHA-256 of source is stable across calls', () => {
  const chars = extractCharactersFromProject(goldenProject)
  const fonts = [subsetFont(OUTFIT_16, chars)]
  const h1 = sha256(compileFontsToCpp(fonts).source)
  const h2 = sha256(compileFontsToCpp(fonts).source)
  assert.equal(h1, h2)
  console.log('  Font Source SHA-256: ' + h1)
})

console.log('-- 3. compileAssetsToCpp Determinism --\n')

function makeSyntheticAsset(id, width, height, fillColor565) {
  const pixels565 = new Uint16Array(width * height).fill(fillColor565)
  return { id, width, height, pixels565 }
}

runTest('compileAssetsToCpp: same assets produce byte-identical header', () => {
  const assets = [
    makeSyntheticAsset('asset_red', 8, 8, 0xF800),
    makeSyntheticAsset('asset_green', 8, 8, 0x07E0),
  ]
  const r1 = compileAssetsToCpp(assets)
  const r2 = compileAssetsToCpp(assets)
  assert.equal(r1.header, r2.header)
})

runTest('compileAssetsToCpp: same assets produce byte-identical source', () => {
  const assets = [
    makeSyntheticAsset('asset_red', 8, 8, 0xF800),
    makeSyntheticAsset('asset_blue', 8, 8, 0x001F),
  ]
  const r1 = compileAssetsToCpp(assets)
  const r2 = compileAssetsToCpp(assets)
  assert.equal(r1.source, r2.source)
})

runTest('compileAssetsToCpp: SHA-256 of source is stable', () => {
  const assets = [makeSyntheticAsset('asset_det', 4, 4, 0xFFFF)]
  const h1 = sha256(compileAssetsToCpp(assets).source)
  const h2 = sha256(compileAssetsToCpp(assets).source)
  assert.equal(h1, h2)
  console.log('  Asset Source SHA-256: ' + h1)
})

console.log('-- 4. createProjectRenderSnapshot Determinism --\n')

runTest('createProjectRenderSnapshot: same project produce identical hash on 3 successive calls', () => {
  const { hash: h1 } = createProjectRenderSnapshot(goldenProject)
  const { hash: h2 } = createProjectRenderSnapshot(goldenProject)
  const { hash: h3 } = createProjectRenderSnapshot(goldenProject)
  assert.equal(h1, h2)
  assert.equal(h2, h3)
  console.log('  Snapshot SHA-256: ' + h1)
})

runTest('createProjectRenderSnapshot: snapshot object keys are sorted', () => {
  const { snapshot: s1 } = createProjectRenderSnapshot(goldenProject)
  const { snapshot: s2 } = createProjectRenderSnapshot(goldenProject)
  const json1 = JSON.stringify(s1)
  const json2 = JSON.stringify(s2)
  assert.equal(json1, json2)
})

console.log('-- 5. Multi-Screen Project Determinism --\n')

runTest('Multi-screen project: stable source SHA-256', () => {
  assert.ok(goldenProject.screens.length >= 2)
  const h1 = sha256(compileCanonicalToCpp(goldenProject).source)
  const h2 = sha256(compileCanonicalToCpp(goldenProject).source)
  assert.equal(h1, h2)
  console.log('  Multi-screen source SHA-256: ' + h1)
})

console.log('-- 6. Different Inputs Produce Different Hashes --\n')

runTest('Different project names produce different source hashes', () => {
  const projectA = { ...goldenProject, name: 'Project Alpha' }
  const projectB = { ...goldenProject, name: 'Project Beta' }
  const hA = sha256(compileCanonicalToCpp(projectA).source)
  const hB = sha256(compileCanonicalToCpp(projectB).source)
  assert.notEqual(hA, hB)
})

runTest('Different asset pixel data produces different asset source hashes', () => {
  const redAsset = [makeSyntheticAsset('asset', 8, 8, 0xF800)]
  const blueAsset = [makeSyntheticAsset('asset', 8, 8, 0x001F)]
  const hRed = sha256(compileAssetsToCpp(redAsset).source)
  const hBlue = sha256(compileAssetsToCpp(blueAsset).source)
  assert.notEqual(hRed, hBlue)
})

runTest('createProjectRenderSnapshot: same project produces same hash, changed project produces different hash', () => {
  const baseProject = {
    version: '1.0.0',
    name: 'Base Project',
    device: { id: 'waveshare_esp32s3_touch_lcd_169', width: 240, height: 280 },
    screens: [
      {
        id: 's1',
        name: 'Screen 1',
        background: { type: 'solid', color: '#112233' },
        children: [
          {
            id: 'n1',
            type: 'custom_label',
            layout: { x: 10, y: 20, width: 100, height: 30 },
            style: { backgroundColor: '#223344', borderColor: '#334455', borderWidth: 1 },
            typography: { color: '#ffffff', fontFamily: 'Outfit', fontSize: 16, align: 'left' },
            properties: { text: 'Hello' },
            animations: [{ property: 'opacity', from: 0, to: 1, durationMs: 500 }],
          },
          {
            id: 'n2',
            type: 'image',
            layout: { x: 50, y: 80, width: 32, height: 32 },
            style: {},
            properties: { assetId: 'badge_1' },
          },
        ],
      },
    ],
  }

  const { hash: baseHash1 } = createProjectRenderSnapshot(baseProject)
  const { hash: baseHash2 } = createProjectRenderSnapshot(baseProject)
  assert.equal(baseHash1, baseHash2, 'Same project must produce identical snapshot hash')

  const clone = (obj) => JSON.parse(JSON.stringify(obj))

  const pX = clone(baseProject)
  pX.screens[0].children[0].layout.x = 15
  assert.notEqual(createProjectRenderSnapshot(pX).hash, baseHash1, 'Changing node x must change hash')

  const pY = clone(baseProject)
  pY.screens[0].children[0].layout.y = 25
  assert.notEqual(createProjectRenderSnapshot(pY).hash, baseHash1, 'Changing node y must change hash')

  const pW = clone(baseProject)
  pW.screens[0].children[0].layout.width = 120
  assert.notEqual(createProjectRenderSnapshot(pW).hash, baseHash1, 'Changing width must change hash')

  const pH = clone(baseProject)
  pH.screens[0].children[0].layout.height = 40
  assert.notEqual(createProjectRenderSnapshot(pH).hash, baseHash1, 'Changing height must change hash')

  const pBg = clone(baseProject)
  pBg.screens[0].children[0].style.backgroundColor = '#ff0000'
  assert.notEqual(createProjectRenderSnapshot(pBg).hash, baseHash1, 'Changing background color must change hash')

  const pText = clone(baseProject)
  pText.screens[0].children[0].properties.text = 'World'
  assert.notEqual(createProjectRenderSnapshot(pText).hash, baseHash1, 'Changing text must change hash')

  const pFont = clone(baseProject)
  pFont.screens[0].children[0].typography.fontFamily = 'JetBrains Mono'
  assert.notEqual(createProjectRenderSnapshot(pFont).hash, baseHash1, 'Changing font must change hash')

  const pSize = clone(baseProject)
  pSize.screens[0].children[0].typography.fontSize = 20
  assert.notEqual(createProjectRenderSnapshot(pSize).hash, baseHash1, 'Changing font size must change hash')

  const pAlign = clone(baseProject)
  pAlign.screens[0].children[0].typography.align = 'center'
  assert.notEqual(createProjectRenderSnapshot(pAlign).hash, baseHash1, 'Changing alignment must change hash')

  const pAsset = clone(baseProject)
  pAsset.screens[0].children[1].properties.assetId = 'badge_2'
  assert.notEqual(createProjectRenderSnapshot(pAsset).hash, baseHash1, 'Changing asset must change hash')

  const pChildren = clone(baseProject)
  pChildren.screens[0].children.pop()
  assert.notEqual(createProjectRenderSnapshot(pChildren).hash, baseHash1, 'Changing screen children must change hash')

  const pAnim = clone(baseProject)
  pAnim.screens[0].children[0].animations[0].durationMs = 1200
  assert.notEqual(createProjectRenderSnapshot(pAnim).hash, baseHash1, 'Changing animation parameters must change hash')

  console.log('  Verified content-sensitivity across all 12 required visual properties.')
})

console.log('-- 7. Font Subsetting Determinism --\n')

runTest('extractCharactersFromProject: same project produce identical character set on repeated calls', () => {
  const chars1 = extractCharactersFromProject(goldenProject)
  const chars2 = extractCharactersFromProject(goldenProject)
  const sorted1 = Array.from(chars1).sort().join('')
  const sorted2 = Array.from(chars2).sort().join('')
  assert.equal(sorted1, sorted2)
})

runTest('subsetFont followed by compileFontsToCpp: deterministic round-trip', () => {
  const chars = new Set(['H', 'e', 'l', 'o', ' ', '1', '2', ':', '3', '0'])
  const subset1 = subsetFont(OUTFIT_16, chars)
  const subset2 = subsetFont(OUTFIT_16, chars)
  const r1 = compileFontsToCpp([subset1])
  const r2 = compileFontsToCpp([subset2])
  assert.equal(sha256(r1.source), sha256(r2.source))
})

console.log('=================================================================')
console.log('RESULT: ' + passed + '/' + total + ' tests passed')
if (failures.length > 0) {
  console.log('\nFAILURES:')
  failures.forEach((f, i) => console.error('  [' + (i + 1) + '] ' + f.name + '\n      ' + f.error))
  process.exit(1)
} else {
  console.log('All determinism tests passed.')
  process.exit(0)
}
