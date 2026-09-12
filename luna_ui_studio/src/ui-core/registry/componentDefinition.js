/**
 * Luna UI Studio — Component Definition & Capability Contract
 *
 * Defines the unified capability contract for UI components.
 * Every component declares what features it requires (touch, animation, rotation, etc.)
 * so the compiler and validator can match them against device capabilities.
 */

/**
 * Standard Capability Flags for Components
 */
export const COMPONENT_CAPABILITIES = {
  STATIC_RENDER: 'STATIC_RENDER',
  TEXT: 'TEXT',
  IMAGE: 'IMAGE',
  TOUCH: 'TOUCH',
  STATE: 'STATE',
  ANIMATION: 'ANIMATION',
  ROTATION: 'ROTATION',
  SCALE: 'SCALE',
  OPACITY: 'OPACITY',
  CUSTOM_EMBEDDED_RENDERER: 'CUSTOM_EMBEDDED_RENDERER',
}

/**
 * Creates a normalized component definition adhering to the capability contract.
 *
 * @param {Object} config
 * @param {string} config.type - Unique component identifier (e.g. 'card_glass')
 * @param {string} config.displayName - Human-readable name
 * @param {string} config.category - Palette category ('cards', 'buttons', 'text', etc.)
 * @param {string} [config.description] - Component description
 * @param {Object} [config.defaults] - Default layout, style, typography, and properties
 * @param {Object} [config.schema] - Property schema constraints (types, min, max)
 * @param {Array<string>} [config.capabilities] - Array of COMPONENT_CAPABILITIES
 * @param {Array<string>} [config.supportedStates] - e.g. ['normal', 'pressed', 'active', 'disabled']
 * @param {Array<string>} [config.supportedEvents] - e.g. ['onClick', 'onSwipe']
 * @param {Array<string>} [config.supportedAnimations] - e.g. ['rotation', 'opacity', 'scale']
 * @param {React.ComponentType} [config.reactRenderer] - React preview renderer
 * @param {Function} [config.embeddedRenderer] - Embedded C++/LVGL renderer (placeholder for Phase 2)
 * @returns {Object} Normalized component definition
 */
export function defineComponent({
  type,
  displayName,
  name, // backwards compatibility alias for displayName
  category = 'custom',
  description = '',
  defaults = {},
  schema = {},
  capabilities = [COMPONENT_CAPABILITIES.STATIC_RENDER],
  supportedStates = ['normal'],
  supportedEvents = [],
  supportedAnimations = [],
  reactRenderer = null,
  embeddedRenderer = null,
}) {
  const actualName = displayName || name || type

  return {
    type,
    displayName: actualName,
    name: actualName, // alias
    category,
    description,
    defaults: {
      layout: {
        x: 0,
        y: 0,
        width: 80,
        height: 40,
        ...(defaults.layout || {}),
      },
      style: {
        opacity: 1,
        ...(defaults.style || {}),
      },
      typography: {
        fontFamily: 'Outfit',
        fontSize: 14,
        fontWeight: 600,
        color: '#ffffff',
        align: 'center',
        ...(defaults.typography || {}),
      },
      properties: defaults.properties || {},
    },
    schema: {
      properties: schema.properties || {},
      requiredProperties: schema.requiredProperties || [],
    },
    capabilities: Array.from(new Set(capabilities)),
    supportedStates: Array.from(new Set(supportedStates)),
    supportedEvents: Array.from(new Set(supportedEvents)),
    supportedAnimations: Array.from(new Set(supportedAnimations)),
    reactRenderer,
    embeddedRenderer,
  }
}

/**
 * Standard capability profiles for standard element archetypes.
 */
export const ARCHETYPE_PROFILES = {
  BUTTON: {
    capabilities: [
      COMPONENT_CAPABILITIES.STATIC_RENDER,
      COMPONENT_CAPABILITIES.TEXT,
      COMPONENT_CAPABILITIES.TOUCH,
      COMPONENT_CAPABILITIES.STATE,
      COMPONENT_CAPABILITIES.ANIMATION,
    ],
    supportedStates: ['normal', 'pressed', 'active', 'disabled'],
    supportedEvents: ['onClick', 'onPress', 'onRelease'],
    supportedAnimations: ['scale', 'opacity'],
  },
  CARD: {
    capabilities: [
      COMPONENT_CAPABILITIES.STATIC_RENDER,
      COMPONENT_CAPABILITIES.TEXT,
      COMPONENT_CAPABILITIES.TOUCH,
    ],
    supportedStates: ['normal', 'selected'],
    supportedEvents: ['onClick'],
    supportedAnimations: ['opacity'],
  },
  TEXT: {
    capabilities: [
      COMPONENT_CAPABILITIES.STATIC_RENDER,
      COMPONENT_CAPABILITIES.TEXT,
    ],
    supportedStates: ['normal'],
    supportedEvents: [],
    supportedAnimations: ['opacity'],
  },
  LOADER: {
    capabilities: [
      COMPONENT_CAPABILITIES.STATIC_RENDER,
      COMPONENT_CAPABILITIES.ANIMATION,
      COMPONENT_CAPABILITIES.ROTATION,
    ],
    supportedStates: ['normal'],
    supportedEvents: ['onLoad'],
    supportedAnimations: ['rotation', 'scale', 'opacity'],
  },
  PATTERN: {
    capabilities: [
      COMPONENT_CAPABILITIES.STATIC_RENDER,
      COMPONENT_CAPABILITIES.ANIMATION,
    ],
    supportedStates: ['normal'],
    supportedEvents: [],
    supportedAnimations: ['position', 'opacity'],
  },
}
