/**
 * Luna UI Studio — Semantic Render Snapshot Engine (Visual Regression Infrastructure)
 *
 * Simulates the embedded LunaRenderer execution in software and emits a deterministic
 * stream of primitive drawing commands (rectangles, rounded cards, text with measured alignment,
 * images, buttons, and spinners).
 *
 * Used for visual parity regression testing between the Web Canvas and ESP32 hardware.
 */

import crypto from 'node:crypto'
import { hexToRgb565Hex } from '../../compiler/embeddedCompiler.js'
import { measureTextWidth } from '../../compiler/fonts/fontCompiler.js'
import { OUTFIT_16 } from '../../compiler/fonts/fontData.js'

/**
 * Generates the deterministic semantic drawing command stream for a canonical screen.
 *
 * @param {Object} screen - Canonical UIScreen
 * @param {Object} [options]
 * @returns {Array<Object>} List of draw commands
 */
export function generateScreenDrawCommands(screen, options = {}) {
  const commands = []
  if (!screen) return commands

  const bg = screen.background || {}
  const bgColorHex = hexToRgb565Hex(bg.color, 0x0000)

  // 1. Background Fill
  if (bg.pattern === 'stars') {
    commands.push({
      cmd: 'DRAW_STARFIELD',
      backgroundColor: bgColorHex,
      starCount: 12,
    })
  } else {
    commands.push({
      cmd: 'FILL_SCREEN',
      color: bgColorHex,
    })
  }

  // 2. Child Nodes
  const children = screen.children || []
  children.forEach((node) => {
    const layout = node.layout || { x: 0, y: 0, width: 80, height: 40 }
    const style = node.style || {}
    const typo = node.typography || {}
    const props = node.properties || {}

    const bgHex = hexToRgb565Hex(style.backgroundColor, 0x0000)
    const borderHex = hexToRgb565Hex(style.borderColor, 0x0000)
    const textColorHex = hexToRgb565Hex(typo.color, 0xffff)

    const anims = (node.animations && node.animations.length > 0)
      ? node.animations.map((a) => ({
          property: a.property || 'unknown',
          durationMs: a.durationMs || a.duration || 1000,
          from: a.from !== undefined ? a.from : a.fromValue,
          to: a.to !== undefined ? a.to : a.toValue,
          loop: a.loop !== false,
        }))
      : undefined

    // A. Card Node
    if (node.type?.startsWith('card_')) {
      commands.push({
        cmd: 'DRAW_ROUND_RECT',
        nodeId: node.id,
        x: layout.x,
        y: layout.y,
        width: layout.width,
        height: layout.height,
        borderRadius: style.borderRadius || 0,
        backgroundColor: bgHex,
        borderColor: borderHex,
        borderWidth: style.borderWidth || 0,
        animations: anims,
      })

      if (props.title) {
        commands.push({
          cmd: 'DRAW_TEXT',
          nodeId: `${node.id}_title`,
          text: props.title,
          x: layout.x + 12,
          y: layout.y + 14,
          color: textColorHex,
          fontFamily: typo.fontFamily || 'Outfit',
          fontSize: typo.fontSize || 13,
          align: 'left',
        })
      }

      if (props.subtitle) {
        commands.push({
          cmd: 'DRAW_TEXT',
          nodeId: `${node.id}_subtitle`,
          text: props.subtitle,
          x: layout.x + 12,
          y: layout.y + 36,
          color: hexToRgb565Hex(props.subtextColor, 0x8410),
          fontFamily: typo.fontFamily || 'Outfit',
          fontSize: 10,
          align: 'left',
        })
      }
      return
    }

    // B. Button Node
    if (node.type?.includes('btn') || node.type?.includes('button')) {
      const label = props.label || props.text || 'Button'
      const labelWidth = measureTextWidth(label, OUTFIT_16)
      const computedTextX = layout.x + Math.floor((layout.width - labelWidth) / 2)

      commands.push({
        cmd: 'DRAW_BUTTON',
        nodeId: node.id,
        x: layout.x,
        y: layout.y,
        width: layout.width,
        height: layout.height,
        borderRadius: style.borderRadius || 0,
        backgroundColor: bgHex,
        borderColor: borderHex,
        borderWidth: style.borderWidth || 0,
        label,
        computedLabelX: computedTextX,
        computedLabelY: layout.y + Math.floor(layout.height / 2),
        textColor: textColorHex,
        fontFamily: typo.fontFamily || 'Outfit',
        fontSize: typo.fontSize || 14,
        align: typo.align || 'center',
        hasPressedState: !!node.states?.pressed,
        animations: anims,
      })
      return
    }

    // C. Image Node
    if (node.type === 'image' || props.assetId) {
      commands.push({
        cmd: 'DRAW_IMAGE',
        nodeId: node.id,
        assetId: props.assetId || 'unknown_asset',
        x: layout.x,
        y: layout.y,
        width: layout.width,
        height: layout.height,
        animations: anims,
      })
      return
    }

    // D. Spinner Node
    if (node.type === 'loader_spinner') {
      commands.push({
        cmd: 'DRAW_SPINNER',
        nodeId: node.id,
        x: layout.x,
        y: layout.y,
        width: layout.width,
        height: layout.height,
        size: props.size || 30,
        accentColor: hexToRgb565Hex(style.accentColor, 0x07ff),
        trackColor: bgHex,
        label: props.label,
        fontFamily: typo.fontFamily || 'Outfit',
        animations: anims,
      })
      return
    }

    // E. Text Node
    if (node.type === 'custom_label' || node.type === 'digital_clock') {
      const text = props.text || props.timeStr || ''
      const measuredW = measureTextWidth(text, OUTFIT_16)

      let computedX = layout.x
      if (typo.align === 'center') {
        computedX = layout.x + Math.floor((layout.width - measuredW) / 2)
      } else if (typo.align === 'right') {
        computedX = layout.x + layout.width - measuredW
      }

      commands.push({
        cmd: 'DRAW_TEXT',
        nodeId: node.id,
        text,
        x: computedX,
        y: layout.y + Math.floor(layout.height / 2),
        width: layout.width,
        height: layout.height,
        measuredWidth: measuredW,
        align: typo.align || 'left',
        color: textColorHex,
        backgroundColor: bgHex,
        fontFamily: typo.fontFamily || 'Outfit',
        fontSize: typo.fontSize || 14,
        animations: anims,
      })
      return
    }

    // F. Fallback Rectangle
    commands.push({
      cmd: 'DRAW_RECTANGLE',
      nodeId: node.id,
      x: layout.x,
      y: layout.y,
      width: layout.width,
      height: layout.height,
      backgroundColor: bgHex,
      borderColor: borderHex,
      borderWidth: style.borderWidth || 0,
      borderRadius: style.borderRadius || 0,
      animations: anims,
    })
  })

  return commands
}

/**
 * Deterministically serializes an arbitrary object by recursively sorting all keys.
 * Does NOT filter out keys at nested levels.
 *
 * @param {*} value
 * @returns {string}
 */
export function stableStringify(value) {
  if (value === null || typeof value !== 'object') {
    return JSON.stringify(value)
  }
  if (Array.isArray(value)) {
    return '[' + value.map((item) => stableStringify(item)).join(',') + ']'
  }
  const keys = Object.keys(value).sort()
  const entries = keys.map((k) => JSON.stringify(k) + ':' + stableStringify(value[k]))
  return '{' + entries.join(',') + '}'
}

/**
 * Creates a deterministic visual snapshot hash of an entire canonical UIProject.
 *
 * @param {Object} project - Canonical UIProject
 * @returns {{ snapshot: Object, hash: string }}
 */
export function createProjectRenderSnapshot(project) {
  const screens = project.screens || []
  const snapshot = {
    version: project.version,
    device: project.device?.id || 'unknown',
    screens: {},
  }

  screens.forEach((screen) => {
    snapshot.screens[screen.id] = generateScreenDrawCommands(screen)
  })

  const serialized = stableStringify(snapshot)
  const hash = crypto.createHash('sha256').update(serialized).digest('hex')

  return {
    snapshot,
    hash,
  }
}
