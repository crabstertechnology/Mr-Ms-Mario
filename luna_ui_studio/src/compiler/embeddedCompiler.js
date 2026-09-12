/**
 * Luna UI Studio — Embedded C++ Compiler
 *
 * Compiles a Canonical UIProject into static C++ structures for LunaRuntime.
 *
 * Generated files strictly follow the Generated File Rule:
 * // ============================================================================
 * // GENERATED FILE — DO NOT EDIT MANUALLY.
 * // SOURCE: Luna UI Studio Embedded Compiler
 * // REGENERATE FROM STUDIO.
 * // ============================================================================
 *
 * Fully deterministic: identical input generates byte-for-byte identical C++ output.
 */

import { validateCanonicalProject, validateProjectCapabilities } from '../ui-core/index.js'

/**
 * Converts a hex color string (e.g. "#00f2fe", "#1e293bcc") into a 16-bit RGB565 hex string.
 * @param {string} hex
 * @param {number} [fallback=0x0000]
 * @returns {string} e.g. "0x07FF"
 */
export function hexToRgb565Hex(hex, fallback = 0) {
  if (!hex || typeof hex !== 'string') {
    return `0x${fallback.toString(16).toUpperCase().padStart(4, '0')}`
  }

  let clean = hex.trim().replace(/^#/, '')
  if (clean.length === 3) {
    clean = clean.split('').map((c) => c + c).join('')
  }
  if (clean.length >= 6) {
    const r = parseInt(clean.substring(0, 2), 16)
    const g = parseInt(clean.substring(2, 4), 16)
    const b = parseInt(clean.substring(4, 6), 16)
    const rgb565 = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3)
    return `0x${rgb565.toString(16).toUpperCase().padStart(4, '0')}`
  }

  return `0x${fallback.toString(16).toUpperCase().padStart(4, '0')}`
}

/**
 * Escapes C string literals.
 * @param {string} str
 * @returns {string}
 */
function escapeCString(str) {
  if (str === null || str === undefined) return 'nullptr'
  const escaped = String(str)
    .replace(/\\/g, '\\\\')
    .replace(/"/g, '\\"')
    .replace(/\n/g, '\\n')
    .replace(/\r/g, '\\r')
  return `"${escaped}"`
}

/**
 * Maps canonical node type to LunaNodeType enum.
 */
function mapNodeTypeEnum(type) {
  switch (type) {
    case 'card_glass':
    case 'card_stat':
    case 'card_retro':
    case 'card_transaction':
      return 'LUNA_NODE_CARD'
    case 'button_neon':
    case 'button_retro':
    case 'button_download':
    case 'uiv_btn_happy_coding':
    case 'uiv_btn_damith_yellow':
      return 'LUNA_NODE_BUTTON'
    case 'custom_label':
    case 'digital_clock':
      return 'LUNA_NODE_TEXT'
    case 'loader_spinner':
    case 'loader_dots':
    case 'loader_pulse':
      return 'LUNA_NODE_SPINNER'
    case 'image':
      return 'LUNA_NODE_IMAGE'
    default:
      return 'LUNA_NODE_CONTAINER'
  }
}


/**
 * Maps alignment to LunaTextAlign enum.
 */
function mapTextAlignEnum(align) {
  if (align === 'center') return 'LUNA_ALIGN_CENTER'
  if (align === 'right') return 'LUNA_ALIGN_RIGHT'
  return 'LUNA_ALIGN_LEFT'
}

/**
 * Compiles a canonical UIProject into deterministic C++ header and source code.
 *
 * @param {Object} project - Canonical UIProject
 * @param {Object} [options]
 * @returns {{ header: string, source: string, diagnostics: Object }}
 */
export function compileCanonicalToCpp(project, options = {}) {
  // 1. Validate Schema & Device Capabilities
  const valResult = validateCanonicalProject(project)
  if (!valResult.valid) {
    throw new Error(`Canonical project validation failed: ${valResult.errors.join(', ')}`)
  }

  const diagResult = validateProjectCapabilities(project, project.device)

  // 2. Prepare C++ Header File
  const headerLines = [
    '// ============================================================================',
    '// GENERATED FILE — DO NOT EDIT MANUALLY.',
    '// SOURCE: Luna UI Studio Embedded Compiler',
    '// REGENERATE FROM STUDIO.',
    '// ============================================================================',
    '#ifndef GENERATED_UI_H',
    '#define GENERATED_UI_H',
    '',
    '#include "LunaTypes.h"',
    '#include "GeneratedAssets.h"',
    '#include "GeneratedFonts.h"',
    '',
    '#ifdef __cplusplus',
    'extern "C" {',
    '#endif',

    '',
    '// Compiled Canonical UI Project Data',
    'extern const LunaProjectDef LUNA_COMPILED_PROJECT;',
    '',
    '#ifdef __cplusplus',
    '}',
    '#endif',
    '',
    '#endif // GENERATED_UI_H',
    '',
  ]

  // 3. Prepare C++ Source File
  const sourceLines = [
    '// ============================================================================',
    '// GENERATED FILE — DO NOT EDIT MANUALLY.',
    '// SOURCE: Luna UI Studio Embedded Compiler',
    '// REGENERATE FROM STUDIO.',
    '// ============================================================================',
    '#include "GeneratedUI.h"',
    '',
  ]

  const screens = project.screens || []

  // Compile individual nodes for each screen
  screens.forEach((screen, sIdx) => {
    const screenPrefix = `s${sIdx}_${screen.id.replace(/[^a-zA-Z0-9_]/g, '_')}`
    const children = screen.children || []

    children.forEach((node, nIdx) => {
      const nodePrefix = `${screenPrefix}_n${nIdx}_${node.id.replace(/[^a-zA-Z0-9_]/g, '_')}`
      const layout = node.layout || {}
      const style = node.style || {}
      const typo = node.typography || {}
      const props = node.properties || {}
      const events = node.events || []
      const anims = node.animations || []

      // Events Array
      if (events.length > 0) {
        sourceLines.push(`static const LunaEvent ${nodePrefix}_events[${events.length}] = {`)
        events.forEach((evt) => {
          const act = evt.action || {}
          let actType = 'LUNA_ACTION_NONE'
          if (act.type === 'navigate') actType = 'LUNA_ACTION_NAVIGATE'
          else if (act.type === 'scroll') actType = 'LUNA_ACTION_SCROLL'

          sourceLines.push(`  { ${escapeCString(evt.trigger || 'onClick')}, { ${actType}, ${escapeCString(act.targetScreenId)}, ${act.scrollAmount || 0} } },`)
        })
        sourceLines.push('};')
        sourceLines.push('')
      }

      // Animations Array
      if (anims.length > 0) {
        sourceLines.push(`static const LunaAnimation ${nodePrefix}_anims[${anims.length}] = {`)
        anims.forEach((a) => {
          sourceLines.push(`  { ${escapeCString(a.property || 'rotation')}, ${a.from || a.fromValue || 0}, ${a.to || a.toValue || 360}, ${a.durationMs || a.duration || 1000}, ${a.loop !== false ? 'true' : 'false'} },`)
        })
        sourceLines.push('};')
        sourceLines.push('')
      }

      // Optional Pressed Style
      const pressedState = node.states?.pressed
      if (pressedState?.style) {
        const ps = pressedState.style
        sourceLines.push(`static const LunaStyle ${nodePrefix}_pressedStyle = {`)
        sourceLines.push(`  ${hexToRgb565Hex(ps.backgroundColor || style.backgroundColor, 0x0000)},`)
        sourceLines.push(`  ${hexToRgb565Hex(ps.borderColor || style.borderColor, 0x0000)},`)
        sourceLines.push(`  ${ps.borderWidth !== undefined ? ps.borderWidth : (style.borderWidth || 0)},`)
        sourceLines.push(`  ${ps.borderRadius !== undefined ? ps.borderRadius : (style.borderRadius || 0)},`)
        sourceLines.push(`  ${ps.opacity !== undefined ? Math.round(ps.opacity * 255) : 255},`)
        sourceLines.push(`  ${hexToRgb565Hex(ps.accentColor || style.accentColor, 0x0000)},`)
        sourceLines.push(`  ${ps.backgroundColor ? 'true' : 'false'},`)
        sourceLines.push(`  ${ps.borderColor && (ps.borderWidth || style.borderWidth) ? 'true' : 'false'}`)
        sourceLines.push('};')
        sourceLines.push('')
      }
    })

    // Screen Nodes Array
    sourceLines.push(`static const LunaNodeDef ${screenPrefix}_nodes[${children.length}] = {`)
    children.forEach((node, nIdx) => {
      const nodePrefix = `${screenPrefix}_n${nIdx}_${node.id.replace(/[^a-zA-Z0-9_]/g, '_')}`
      const layout = node.layout || {}
      const style = node.style || {}
      const typo = node.typography || {}
      const props = node.properties || {}
      const events = node.events || []
      const anims = node.animations || []
      const hasPressedStyle = !!node.states?.pressed?.style

      const textVal = props.text || props.label || props.title || props.timeStr || null
      const subtitleVal = props.subtitle || props.dateStr || null
      const subtextColorHex = hexToRgb565Hex(props.subtextColor || props.dateColor, 0x8410)

      const spinnerSize = props.size || 30

      sourceLines.push('  {')
      sourceLines.push(`    ${escapeCString(node.id)},`)
      sourceLines.push(`    ${escapeCString(node.type)},`)
      sourceLines.push(`    ${mapNodeTypeEnum(node.type)},`)
      sourceLines.push(`    { ${layout.x || 0}, ${layout.y || 0}, ${layout.width || 80}, ${layout.height || 40} },`)
      sourceLines.push('    {')
      sourceLines.push(`      ${hexToRgb565Hex(style.backgroundColor, 0x0000)},`)
      sourceLines.push(`      ${hexToRgb565Hex(style.borderColor, 0x0000)},`)
      sourceLines.push(`      ${style.borderWidth || 0},`)
      sourceLines.push(`      ${style.borderRadius || 0},`)
      sourceLines.push(`      ${style.opacity !== undefined ? Math.round(style.opacity * 255) : 255},`)
      sourceLines.push(`      ${hexToRgb565Hex(style.accentColor, 0x0000)},`)
      sourceLines.push(`      ${style.backgroundColor ? 'true' : 'false'},`)
      sourceLines.push(`      ${style.borderColor && style.borderWidth ? 'true' : 'false'}`)
      sourceLines.push('    },')
      sourceLines.push('    {')
      sourceLines.push(`      ${hexToRgb565Hex(typo.color, 0xFFFF)},`)
      sourceLines.push(`      ${typo.fontSize || 14},`)
      sourceLines.push(`      ${mapTextAlignEnum(typo.align)},`)
      sourceLines.push(`      ${escapeCString(typo.fontFamily || 'Outfit')}`)
      sourceLines.push('    },')
      sourceLines.push(`    ${escapeCString(textVal)},`)
      sourceLines.push(`    ${escapeCString(subtitleVal)},`)
      sourceLines.push(`    ${subtextColorHex},`)
      sourceLines.push(`    ${spinnerSize},`)

      // Proportional Font selection
      let fontRef = 'nullptr'
      if (typo.fontFamily === 'JetBrains Mono' || props.fontFamily === 'JetBrains Mono') {
        fontRef = '&LUNA_FONT_JETBRAINS_MONO_9'
      } else if (typo.fontFamily === 'Outfit' || !typo.fontFamily) {
        if (typo.fontSize && typo.fontSize <= 12) {
          fontRef = '&LUNA_FONT_OUTFIT_12'
        } else {
          fontRef = '&LUNA_FONT_OUTFIT_16'
        }
      }

      sourceLines.push(`    ${escapeCString(props.assetId || null)},`)
      sourceLines.push(`    ${fontRef},`)
      sourceLines.push(`    ${hasPressedStyle ? `&${nodePrefix}_pressedStyle` : 'nullptr'},`)
      sourceLines.push(`    ${events.length > 0 ? `${nodePrefix}_events` : 'nullptr'},`)
      sourceLines.push(`    ${events.length},`)
      sourceLines.push(`    ${anims.length > 0 ? `${nodePrefix}_anims` : 'nullptr'},`)
      sourceLines.push(`    ${anims.length}`)

      sourceLines.push('  },')
    })
    sourceLines.push('};')
    sourceLines.push('')
  })

  // Screens Array
  sourceLines.push(`static const LunaScreenDef COMPILED_SCREENS[${screens.length}] = {`)
  screens.forEach((screen, sIdx) => {
    const screenPrefix = `s${sIdx}_${screen.id.replace(/[^a-zA-Z0-9_]/g, '_')}`
    const bg = screen.background || {}
    const leftTarget = screen.gestures?.swipeLeft?.targetScreenId || null
    const rightTarget = screen.gestures?.swipeRight?.targetScreenId || null

    sourceLines.push('  {')
    sourceLines.push(`    ${escapeCString(screen.id)},`)
    sourceLines.push(`    ${escapeCString(screen.name)},`)
    sourceLines.push(`    ${hexToRgb565Hex(bg.color, 0x0000)},`)
    sourceLines.push(`    ${escapeCString(bg.pattern || 'none')},`)
    sourceLines.push(`    ${screen.isScrollable ? 'true' : 'false'},`)
    sourceLines.push(`    ${screen.maxScrollY || 280},`)
    sourceLines.push(`    ${screenPrefix}_nodes,`)
    sourceLines.push(`    ${(screen.children || []).length},`)
    sourceLines.push(`    ${escapeCString(leftTarget)},`)
    sourceLines.push(`    ${escapeCString(rightTarget)}`)
    sourceLines.push('  },')
  })
  sourceLines.push('};')
  sourceLines.push('')

  // Project Definition
  const dev = project.device || {}
  sourceLines.push('const LunaProjectDef LUNA_COMPILED_PROJECT = {')
  sourceLines.push(`  ${escapeCString(project.version || '1.0.0')},`)
  sourceLines.push(`  ${escapeCString(project.name || 'Luna UI Project')},`)
  sourceLines.push(`  ${dev.width || 240},`)
  sourceLines.push(`  ${dev.height || 280},`)
  sourceLines.push(`  ${escapeCString(project.activeScreenId || (screens[0] ? screens[0].id : 'screen_1'))},`)
  sourceLines.push('  COMPILED_SCREENS,')
  sourceLines.push(`  ${screens.length}`)
  sourceLines.push('};')
  sourceLines.push('')

  return {
    header: headerLines.join('\n'),
    source: sourceLines.join('\n'),
    diagnostics: diagResult,
  }
}
