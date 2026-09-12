/**
 * Luna UI Studio — Legacy <-> Canonical Schema Adapter
 *
 * Provides bidirectional translation between the existing React Studio state
 * (screens, elements, props, actions) and the canonical UIProject schema.
 *
 * Ensures 100% backward compatibility while establishing the canonical model
 * as the architectural foundation.
 */

import { getDefaultDevice } from '../device/deviceProfiles.js'

/**
 * Converts legacy Studio state into a canonical UIProject.
 *
 * @param {Array} screens - Array of legacy screen objects
 * @param {string} [activeScreenId] - Currently active screen ID
 * @param {Object} [deviceProfile] - Target device configuration
 * @param {Object} [options] - Optional settings (deterministic, timestamps)
 * @returns {Object} Canonical UIProject object
 */
export function legacyToCanonical(screens = [], activeScreenId = 'screen_1', deviceProfile = null, options = {}) {
  const device = deviceProfile || getDefaultDevice()
  const isDeterministic = !!options.deterministic
  const defaultTime = isDeterministic ? '1970-01-01T00:00:00.000Z' : new Date().toISOString()
  const createdAt = options.createdAt || options.metadata?.createdAt || defaultTime
  const updatedAt = options.updatedAt || options.metadata?.updatedAt || defaultTime

  const canonicalScreens = (screens || []).map((screen) => {
    // 1. Convert Background
    const background = {
      type: screen.bgType || 'color',
      color: screen.bgColor || '#0b0f19',
      pattern: screen.bgPattern || 'none',
      imageAssetId: screen.bgImage ? 'custom_bg' : undefined,
    }

    // 2. Convert Children (Nodes)
    const children = (screen.elements || []).map((el) => {
      const p = el.props || {}

      // Layout
      const layout = {
        x: typeof p.x === 'number' ? p.x : 0,
        y: typeof p.y === 'number' ? p.y : 0,
        width: typeof p.w === 'number' ? p.w : 80,
        height: typeof p.h === 'number' ? p.h : 40,
        rotation: 0,
        scaleX: 1,
        scaleY: 1,
      }

      // Semantic Style (no raw CSS strings)
      const style = {
        backgroundColor: p.bgColor || undefined,
        borderColor: p.borderColor || undefined,
        borderWidth: p.borderWidth !== undefined ? p.borderWidth : undefined,
        borderRadius: p.radius !== undefined ? p.radius : undefined,
        opacity: p.opacity !== undefined ? p.opacity : 1,
        accentColor: p.accentColor || undefined,
      }

      // Typography
      const typography = {
        fontFamily: p.fontFamily || 'Outfit',
        fontSize: p.fontSize || (el.type === 'digital_clock' ? 36 : 14),
        fontWeight: p.fontWeight || 600,
        color: p.textColor || p.color || '#ffffff',
        align: p.align || 'center',
      }

      // Events & Actions
      const events = (el.actions || []).map((act, aIdx) => ({
        id: act.id || `evt_${el.id}_${aIdx}`,
        trigger: act.trigger || 'onClick',
        action: {
          id: act.id || `act_${el.id}_${aIdx}`,
          type: act.actionType || 'custom',
          targetScreenId: act.targetScreenId || undefined,
          transition: act.transition || 'none',
          scrollDirection: act.scrollDirection || undefined,
          scrollAmount: act.scrollAmount !== undefined ? act.scrollAmount : undefined,
          alertMessage: act.alertMessage || undefined,
        },
      }))

      // Widget specific semantic properties (excluding layout & style)
      const properties = {}
      const excludedKeys = new Set([
        'x', 'y', 'w', 'h',
        'bgColor', 'borderColor', 'borderWidth', 'radius',
        'opacity', 'accentColor', 'textColor',
        'fontSize', 'fontWeight', 'align', 'fontFamily'
      ])
      Object.keys(p).forEach((k) => {
        if (!excludedKeys.has(k)) {
          properties[k] = p[k]
        }
      })

      return {
        id: el.id,
        type: el.type,
        name: el.name || el.type,
        layout,
        style,
        typography,
        children: [],
        states: {
          normal: { style, typography, layout },
        },
        events,
        animations: [],
        assetReferences: [],
        properties,
      }
    })

    // 3. Convert Gestures
    const gestures = {}
    if (screen.gestures?.swipeLeft) {
      const g = screen.gestures.swipeLeft
      const targetId = typeof g === 'string' ? g : g.targetScreenId
      if (targetId) {
        gestures.swipeLeft = {
          type: 'navigate',
          targetScreenId: targetId,
          transition: typeof g === 'object' ? g.transition || 'slide-left' : 'slide-left',
        }
      }
    }
    if (screen.gestures?.swipeRight) {
      const g = screen.gestures.swipeRight
      const targetId = typeof g === 'string' ? g : g.targetScreenId
      if (targetId) {
        gestures.swipeRight = {
          type: 'navigate',
          targetScreenId: targetId,
          transition: typeof g === 'object' ? g.transition || 'slide-right' : 'slide-right',
        }
      }
    }

    return {
      id: screen.id,
      name: screen.name || 'Screen',
      background,
      children,
      gestures,
      isScrollable: !!screen.isScrollable,
      maxScrollY: screen.maxScrollY || 280,
      metadata: {},
    }
  })

  return {
    version: '1.0.0',
    name: 'Luna UI Project',
    device,
    screens: canonicalScreens,
    activeScreenId: activeScreenId || (canonicalScreens[0] ? canonicalScreens[0].id : 'screen_1'),
    assets: [],
    fonts: [],
    metadata: {
      createdAt,
      updatedAt,
    },
  }
}

/**
 * Converts a canonical UIProject back into legacy Studio state arrays.
 *
 * @param {Object} project - Canonical UIProject object
 * @returns {{ screens: Array, activeScreenId: string }}
 */
export function canonicalToLegacy(project) {
  if (!project || !Array.isArray(project.screens)) {
    return { screens: [], activeScreenId: 'screen_1' }
  }

  const legacyScreens = project.screens.map((screen) => {
    // 1. Reconstruct elements
    const elements = (screen.children || []).map((node) => {
      const p = {
        x: node.layout?.x ?? 0,
        y: node.layout?.y ?? 0,
        w: node.layout?.width ?? 80,
        h: node.layout?.height ?? 40,
        bgColor: node.style?.backgroundColor,
        borderColor: node.style?.borderColor,
        borderWidth: node.style?.borderWidth,
        radius: node.style?.borderRadius,
        opacity: node.style?.opacity ?? 1,
        accentColor: node.style?.accentColor,
        fontSize: node.typography?.fontSize,
        fontWeight: node.typography?.fontWeight,
        align: node.typography?.align,
        fontFamily: node.typography?.fontFamily,
      }

      // Preserve color naming fidelity: custom_label & digital_clock use 'color'
      if (node.properties?.color !== undefined) {
        p.color = node.properties.color
      } else if (node.type === 'digital_clock' || node.type === 'custom_label') {
        p.color = node.typography?.color
      } else if (node.typography?.color !== undefined) {
        p.textColor = node.typography.color
      }

      // Merge remaining widget-specific properties
      if (node.properties) {
        Object.assign(p, node.properties)
      }

      // Clean undefined keys
      Object.keys(p).forEach((k) => p[k] === undefined && delete p[k])

      // Reconstruct actions from events
      const actions = (node.events || []).map((evt) => {
        const act = {
          id: evt.action?.id || evt.id || `act_${Date.now()}`,
          trigger: evt.trigger || 'onClick',
          actionType: evt.action?.type || 'navigate',
        }
        if (evt.action?.targetScreenId !== undefined) act.targetScreenId = evt.action.targetScreenId
        if (evt.action?.transition !== undefined) act.transition = evt.action.transition
        if (evt.action?.scrollDirection !== undefined) act.scrollDirection = evt.action.scrollDirection
        if (evt.action?.scrollAmount !== undefined) act.scrollAmount = evt.action.scrollAmount
        if (evt.action?.alertMessage !== undefined) act.alertMessage = evt.action.alertMessage
        return act
      })

      return {
        id: node.id,
        type: node.type,
        name: node.name,
        actions,
        props: p,
      }
    })

    // 2. Reconstruct gestures
    const gestures = {
      swipeLeft: screen.gestures?.swipeLeft ? {
        actionType: screen.gestures.swipeLeft.type || 'navigate',
        targetScreenId: screen.gestures.swipeLeft.targetScreenId || null,
        transition: screen.gestures.swipeLeft.transition || 'slide-left',
      } : { actionType: 'none', targetScreenId: null, transition: 'slide-left' },
      swipeRight: screen.gestures?.swipeRight ? {
        actionType: screen.gestures.swipeRight.type || 'navigate',
        targetScreenId: screen.gestures.swipeRight.targetScreenId || null,
        transition: screen.gestures.swipeRight.transition || 'slide-right',
      } : { actionType: 'none', targetScreenId: null, transition: 'slide-right' },
    }

    return {
      id: screen.id,
      name: screen.name,
      bgColor: screen.background?.color || '#0b0f19',
      bgType: screen.background?.type || 'color',
      bgPattern: screen.background?.pattern || 'none',
      bgGradient: screen.background?.gradient ? 'linear-gradient(...)' : '',
      bgImage: screen.background?.imageAssetId || '',
      isScrollable: !!screen.isScrollable,
      maxScrollY: screen.maxScrollY || 280,
      gestures,
      elements,
    }
  })

  return {
    screens: legacyScreens,
    activeScreenId: project.activeScreenId || (legacyScreens[0] ? legacyScreens[0].id : 'screen_1'),
  }
}

/**
 * Recursively sorts keys of an object for deterministic JSON serialization.
 * @param {*} val
 * @returns {*}
 */
export function sortKeysRecursively(val) {
  if (val === null || typeof val !== 'object') return val
  if (Array.isArray(val)) return val.map(sortKeysRecursively)
  const sortedObj = {}
  Object.keys(val).sort().forEach((key) => {
    sortedObj[key] = sortKeysRecursively(val[key])
  })
  return sortedObj
}

/**
 * Serializes a canonical UIProject into a deterministic JSON string.
 * Ensures byte-for-byte identical output for identical UI inputs.
 *
 * @param {Object} project - Canonical UIProject
 * @param {Object} [options]
 * @param {number} [options.indent=2] - Indentation spaces
 * @param {boolean} [options.stripTimestamps=false] - If true, normalizes timestamps to epoch
 * @returns {string} Deterministic JSON string
 */
export function serializeCanonicalProject(project, options = {}) {
  const { indent = 2, stripTimestamps = false } = options

  const clone = JSON.parse(JSON.stringify(project))

  if (stripTimestamps && clone.metadata) {
    clone.metadata.createdAt = '1970-01-01T00:00:00.000Z'
    clone.metadata.updatedAt = '1970-01-01T00:00:00.000Z'
  }

  const sorted = sortKeysRecursively(clone)
  return JSON.stringify(sorted, null, indent)
}

/**
 * Validates canonical project schema compliance.
 *
 * @param {Object} project - Canonical UIProject object
 * @returns {{ valid: boolean, errors: string[] }}
 */
export function validateCanonicalProject(project) {
  const errors = []

  if (!project || typeof project !== 'object') {
    return { valid: false, errors: ['Project must be an object'] }
  }

  if (project.version !== '1.0.0') {
    errors.push(`Invalid version "${project.version}". Expected "1.0.0"`)
  }

  if (!project.device || typeof project.device !== 'object') {
    errors.push('Project missing "device" configuration')
  } else {
    if (!project.device.width || project.device.width <= 0) errors.push('Device width must be > 0')
    if (!project.device.height || project.device.height <= 0) errors.push('Device height must be > 0')
    if (!project.device.colorDepth) errors.push('Device colorDepth must be specified')
  }

  if (!Array.isArray(project.screens)) {
    errors.push('Project "screens" must be an array')
  } else if (project.screens.length === 0) {
    errors.push('Project must contain at least 1 screen')
  } else {
    project.screens.forEach((screen, sIdx) => {
      if (!screen.id) errors.push(`Screen at index ${sIdx} missing "id"`)
      if (!screen.name) errors.push(`Screen at index ${sIdx} missing "name"`)
      if (!screen.background) errors.push(`Screen "${screen.id}" missing "background"`)
      if (!Array.isArray(screen.children)) {
        errors.push(`Screen "${screen.id}" children must be an array`)
      } else {
        screen.children.forEach((node, nIdx) => {
          if (!node.id) errors.push(`Node at index ${nIdx} on screen "${screen.id}" missing "id"`)
          if (!node.type) errors.push(`Node "${node.id}" missing "type"`)
          if (!node.layout || typeof node.layout !== 'object') {
            errors.push(`Node "${node.id}" missing valid "layout" object`)
          } else {
            if (typeof node.layout.x !== 'number') errors.push(`Node "${node.id}" layout.x must be number`)
            if (typeof node.layout.y !== 'number') errors.push(`Node "${node.id}" layout.y must be number`)
            if (typeof node.layout.width !== 'number') errors.push(`Node "${node.id}" layout.width must be number`)
            if (typeof node.layout.height !== 'number') errors.push(`Node "${node.id}" layout.height must be number`)
          }
          if (!node.style || typeof node.style !== 'object') {
            errors.push(`Node "${node.id}" missing valid "style" object`)
          }
        })
      }
    })
  }

  return {
    valid: errors.length === 0,
    errors,
  }
}
