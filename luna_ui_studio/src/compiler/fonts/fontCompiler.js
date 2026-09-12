/**
 * Luna UI Studio — Embedded Font Compiler & Subsetting Pipeline
 *
 * Scans Canonical UI projects, determines the exact character subset used across all UI screens,
 * and compiles proportional font tables for LunaRenderer.
 */

import { OUTFIT_16, OUTFIT_12, JETBRAINS_MONO_9 } from './fontData.js'

/**
 * Calculates authoritative text width in pixels based on font glyph metrics.
 *
 * @param {string} text
 * @param {Object} font
 * @returns {number} Width in pixels
 */
export function measureTextWidth(text, font = OUTFIT_16) {
  if (!text || typeof text !== 'string') return 0
  let width = 0

  for (let i = 0; i < text.length; i++) {
    const ch = text[i]
    const glyph = font.glyphs ? font.glyphs[ch] : null
    if (glyph) {
      width += glyph.xAdvance
    } else {
      width += Math.round(font.size / 2) // fallback default advance
    }
  }

  return width
}

/**
 * Extracts all unique characters used in a canonical UIProject.
 *
 * @param {Object} project - Canonical UIProject
 * @returns {Set<string>}
 */
export function extractCharactersFromProject(project) {
  const chars = new Set([' '])

  if (!project || !Array.isArray(project.screens)) return chars

  project.screens.forEach((screen) => {
    if (screen.name) {
      for (const c of screen.name) chars.add(c)
    }

    const children = screen.children || []
    children.forEach((node) => {
      const props = node.properties || {}
      const textFields = [props.text, props.label, props.title, props.subtitle, props.timeStr, props.dateStr]
      textFields.forEach((tf) => {
        if (typeof tf === 'string') {
          for (const c of tf) chars.add(c)
        }
      })
    })
  })

  return chars
}

/**
 * Subsets a font definition to include only required glyphs.
 *
 * @param {Object} masterFont
 * @param {Set<string>|string} requiredChars
 * @returns {Object} Subsetted font
 */
export function subsetFont(masterFont, requiredChars) {
  const charSet = requiredChars instanceof Set ? requiredChars : new Set(requiredChars)
  const subsettedGlyphs = {}

  Object.keys(masterFont.glyphs || {}).forEach((ch) => {
    if (charSet.has(ch) || ch === ' ') {
      subsettedGlyphs[ch] = masterFont.glyphs[ch]
    }
  })

  return {
    ...masterFont,
    glyphs: subsettedGlyphs,
    glyphCount: Object.keys(subsettedGlyphs).length,
  }
}

/**
 * Compiles subsetted font tables into C++ header and source code.
 *
 * @param {Array<Object>} fontList
 * @returns {{ header: string, source: string, totalGlyphs: number }}
 */
export function compileFontsToCpp(fontList = [OUTFIT_16, OUTFIT_12, JETBRAINS_MONO_9]) {
  const headerLines = [
    '// ============================================================================',
    '// GENERATED FILE — DO NOT EDIT MANUALLY.',
    '// SOURCE: Luna UI Studio Font Compiler',
    '// REGENERATE FROM STUDIO.',
    '// ============================================================================',
    '#ifndef GENERATED_FONTS_H',
    '#define GENERATED_FONTS_H',
    '',
    '#include "LunaTypes.h"',
    '',
    '#ifdef __cplusplus',
    'extern "C" {',
    '#endif',
    '',
  ]

  const sourceLines = [
    '// ============================================================================',
    '// GENERATED FILE — DO NOT EDIT MANUALLY.',
    '// SOURCE: Luna UI Studio Font Compiler',
    '// REGENERATE FROM STUDIO.',
    '// ============================================================================',
    '#include "GeneratedFonts.h"',
    '#include <pgmspace.h>',
    '',
  ]

  let totalGlyphs = 0

  fontList.forEach((font, fIdx) => {
    const fontSafeName = `${font.family.toLowerCase().replace(/[^a-z0-9]/g, '_')}_${font.size}`
    const glyphEntries = Object.entries(font.glyphs || {})
    totalGlyphs += glyphEntries.length

    headerLines.push(`extern const LunaFont LUNA_FONT_${fontSafeName.toUpperCase()};`)

    // 1. Emit 1-bit bitmap arrays for each glyph
    glyphEntries.forEach(([ch, g], gIdx) => {
      const charCode = ch.charCodeAt(0)
      const bmSymbol = `font_${fontSafeName}_g${gIdx}_c${charCode}_bm`
      sourceLines.push(`// Glyph: '${ch === '\\' ? '\\\\' : ch}' (0x${charCode.toString(16).toUpperCase()})`)
      if (g.bitmap && g.bitmap.length > 0) {
        sourceLines.push(`static const uint8_t PROGMEM ${bmSymbol}[${g.bitmap.length}] = { ${g.bitmap.map((b) => `0x${b.toString(16).toUpperCase().padStart(2, '0')}`).join(', ')} };`)
      } else {
        sourceLines.push(`static const uint8_t PROGMEM ${bmSymbol}[1] = { 0x00 };`)
      }
    })
    sourceLines.push('')

    // 2. Emit glyph structures array
    sourceLines.push(`static const LunaGlyph ${fontSafeName}_glyphs[${glyphEntries.length}] = {`)
    glyphEntries.forEach(([ch, g], gIdx) => {
      const charCode = ch.codePointAt(0)
      const bmSymbol = `font_${fontSafeName}_g${gIdx}_c${charCode}_bm`
      sourceLines.push(`  { 0x${charCode.toString(16).toUpperCase()}, ${g.width}, ${g.height}, ${g.xAdvance}, ${g.xOffset}, ${g.yOffset}, ${bmSymbol} },`)
    })
    sourceLines.push('};')
    sourceLines.push('')

    // 3. Emit LunaFont structure
    sourceLines.push(`const LunaFont LUNA_FONT_${fontSafeName.toUpperCase()} = {`)
    sourceLines.push(`  "${font.family}",`)
    sourceLines.push(`  ${font.size},`)
    sourceLines.push(`  ${font.weight || 400},`)
    sourceLines.push(`  ${font.baseline || 12},`)
    sourceLines.push(`  ${font.lineHeight || 16},`)
    sourceLines.push(`  ${fontSafeName}_glyphs,`)
    sourceLines.push(`  ${glyphEntries.length}`)
    sourceLines.push('};')
    sourceLines.push('')
  })

  headerLines.push('')
  headerLines.push('#ifdef __cplusplus')
  headerLines.push('}')
  headerLines.push('#endif')
  headerLines.push('')
  headerLines.push('#endif // GENERATED_FONTS_H')
  headerLines.push('')

  return {
    header: headerLines.join('\n'),
    source: sourceLines.join('\n'),
    totalGlyphs,
  }
}
