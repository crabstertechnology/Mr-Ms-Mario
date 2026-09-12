/**
 * Luna UI Studio — Firmware Target Compilation & Synchronization Script
 *
 * Compiles canonical project definitions, image assets, and subsetted font tables into
 * C++ artifacts for the hardware test target in `firmware testing/Phase2_Canonical_Proof/`.
 * Also syncs runtime headers/sources from `runtime/luna_runtime/`.
 */

import fs from 'node:fs'
import path from 'node:path'
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

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

const projectRoot = path.resolve(__dirname, '../..')
const fixturePath = path.join(__dirname, 'fixtures', 'golden-parity-project.json')
const runtimeDir = path.join(projectRoot, 'runtime', 'luna_runtime')
const targetDir = path.join(projectRoot, 'firmware testing', 'Phase2_Canonical_Proof')

console.log('═════════════════════════════════════════════════════════════════')
console.log('🚀 COMPILING FIRMWARE ARTIFACTS FOR ESP32-S3 TARGET')
console.log('═════════════════════════════════════════════════════════════════\n')

// 1. Read Canonical Project
const goldenProject = JSON.parse(fs.readFileSync(fixturePath, 'utf8'))
if (process.env.ACTIVE_SCREEN) {
  goldenProject.activeScreenId = process.env.ACTIVE_SCREEN
} else if (goldenProject.screens.some(s => s.id === 'screen_parity_test')) {
  goldenProject.activeScreenId = 'screen_parity_test'
}
console.log(`[1/5] Loaded Canonical Project: "${goldenProject.name}" (v${goldenProject.version}), Active Screen: "${goldenProject.activeScreenId}"`)

// 2. Compile Assets
const rawAssets = (goldenProject.assets || []).map((a) => {
  return {
    id: a.id,
    width: a.width,
    height: a.height,
    pixels565: new Uint16Array(a.pixels565 || []),
  }
})

const compiledAssets = compileAssetsToCpp(rawAssets)
fs.writeFileSync(path.join(targetDir, 'GeneratedAssets.h'), compiledAssets.header)
fs.writeFileSync(path.join(targetDir, 'GeneratedAssets.cpp'), compiledAssets.source)
console.log(`[2/5] Compiled Assets: ${compiledAssets.assetCount} registered, ${compiledAssets.uniqueCount} unique arrays in flash`)

// 3. Compile Subsetted Fonts
const requiredChars = extractCharactersFromProject(goldenProject)
const subsetO16 = subsetFont(OUTFIT_16, requiredChars)
const subsetO12 = subsetFont(OUTFIT_12, requiredChars)
const subsetJ9 = subsetFont(JETBRAINS_MONO_9, requiredChars)

const compiledFonts = compileFontsToCpp([subsetO16, subsetO12, subsetJ9])
fs.writeFileSync(path.join(targetDir, 'GeneratedFonts.h'), compiledFonts.header)
fs.writeFileSync(path.join(targetDir, 'GeneratedFonts.cpp'), compiledFonts.source)
console.log(`[3/5] Compiled Fonts: 3 subsetted fonts (${compiledFonts.totalGlyphs} total glyphs emitted)`)

// 4. Compile UI Project
const compiledUI = compileCanonicalToCpp(goldenProject)
fs.writeFileSync(path.join(targetDir, 'GeneratedUI.h'), compiledUI.header)
fs.writeFileSync(path.join(targetDir, 'GeneratedUI.cpp'), compiledUI.source)
console.log(`[4/5] Compiled UI: ${goldenProject.screens.length} screens, initial: "${goldenProject.activeScreenId}"`)

// 5. Sync Runtime Source Files & Partitions
const runtimeFiles = fs.readdirSync(runtimeDir)
runtimeFiles.forEach((file) => {
  const src = path.join(runtimeDir, file)
  const dest = path.join(targetDir, file)
  fs.copyFileSync(src, dest)
})
const partitionsSrc = path.join(projectRoot, 'firmware testing', 'partitions.csv')
if (fs.existsSync(partitionsSrc)) {
  fs.copyFileSync(partitionsSrc, path.join(targetDir, 'partitions.csv'))
}
console.log(`[5/5] Synchronized ${runtimeFiles.length} runtime files and partitions.csv from runtime/luna_runtime/ -> firmware testing/Phase2_Canonical_Proof/`)

console.log('\n✅ Firmware target synchronized successfully.')
