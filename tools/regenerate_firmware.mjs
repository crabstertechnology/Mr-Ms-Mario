// ============================================================================
// Luna UI Studio — Automated Firmware Export Script
// Regenerates firmware testing artifacts from Luna UI Studio source of truth
// ============================================================================
import fs from 'fs'
import path from 'path'
import { fileURLToPath } from 'url'
import {
  generateMultiScreenArduinoCode,
  generateHelperHeader,
} from '../luna_ui_studio/src/ui-elements/code-generator.js'
import { LUNA_CANONICAL_SCREENS } from '../luna_ui_studio/src/data/canonicalScreens.js'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)
const rootDir = path.resolve(__dirname, '..')
const targetDir = path.resolve(rootDir, 'firmware testing')

console.log('═════════════════════════════════════════════════════════════════')
console.log('⚡ LUNA UI STUDIO — EXPORTING CANONICAL FIRMWARE ARTIFACTS')
console.log('═════════════════════════════════════════════════════════════════')
console.log(`Source Screens: ${LUNA_CANONICAL_SCREENS.length} Canonical Monolith Screens`)
console.log(`Export Target : ${targetDir}`)

// 1. Generate luna_ui_elements.h
const headerCode = generateHelperHeader()
const headerPath = path.join(targetDir, 'luna_ui_elements.h')
fs.writeFileSync(headerPath, headerCode, 'utf8')
console.log(`✓ Generated: ${path.basename(headerPath)} (${headerCode.length} bytes)`)

// 2. Generate Luna_MultiScreen_App.ino
const multiScreenCode = generateMultiScreenArduinoCode(LUNA_CANONICAL_SCREENS, 'screen_home')
const multiScreenPath = path.join(targetDir, 'Luna_MultiScreen_App.ino')
fs.writeFileSync(multiScreenPath, multiScreenCode, 'utf8')
console.log(`✓ Generated: ${path.basename(multiScreenPath)} (${multiScreenCode.length} bytes)`)

// 3. Generate luna_screens_project.json
const projectJson = JSON.stringify({
  version: '2.5.0',
  system: 'Kinesis / Monolith',
  device: 'Waveshare ESP32-S3 Touch LCD 1.69 (240x280)',
  activeScreenId: 'screen_home',
  screens: LUNA_CANONICAL_SCREENS,
  exportedAt: new Date().toISOString()
}, null, 2)
const projectJsonPath = path.join(targetDir, 'luna_screens_project.json')
fs.writeFileSync(projectJsonPath, projectJson, 'utf8')
console.log(`✓ Generated: ${path.basename(projectJsonPath)} (${projectJson.length} bytes)`)

console.log('═════════════════════════════════════════════════════════════════')
console.log('✨ All firmware testing artifacts successfully regenerated from Studio!')
console.log('═════════════════════════════════════════════════════════════════')
