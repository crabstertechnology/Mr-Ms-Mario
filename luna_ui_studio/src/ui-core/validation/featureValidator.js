/**
 * Luna UI Studio — Feature & Capability Validator
 *
 * Inspects a canonical UIProject against a target device profile to detect
 * unsupported renderer features (e.g. 3D transforms, unsupported blur,
 * alpha opacity, animations, or touch on non-touch devices).
 *
 * Emits structured diagnostics:
 * {
 *   severity: 'error' | 'warning' | 'info',
 *   nodeId: string,
 *   nodeName: string,
 *   feature: string,
 *   message: string,
 * }
 */

import { getDefaultDevice } from '../device/deviceProfiles.js'

export const SEVERITY = {
  ERROR: 'error',
  WARNING: 'warning',
  INFO: 'info',
}

/**
 * Validates a canonical UIProject against target device capabilities.
 *
 * @param {Object} project - Canonical UIProject
 * @param {Object} [targetDevice] - Target device profile (defaults to project.device or default device)
 * @returns {{ valid: boolean, hasErrors: boolean, hasWarnings: boolean, report: Array, summary: Object }}
 */
export function validateProjectCapabilities(project, targetDevice = null) {
  const device = targetDevice || project?.device || getDefaultDevice()
  const caps = device?.capabilities || {}
  const report = []

  if (!project || !Array.isArray(project.screens)) {
    report.push({
      severity: SEVERITY.ERROR,
      nodeId: 'root',
      nodeName: 'Project',
      feature: 'project.structure',
      message: 'Invalid project structure: project must contain a screens array.',
    })
    return createResult(report)
  }

  // 1. Inspect Project-Level Features
  if (project.screens.length > 1 && !caps.supportsTouch && !caps.supportsPhysicalButtons) {
    report.push({
      severity: SEVERITY.WARNING,
      nodeId: 'project',
      nodeName: project.name || 'Project',
      feature: 'navigation.multi_screen',
      message: 'Multi-screen project configured for a device without touch controller or navigation buttons.',
    })
  }

  // 2. Inspect Screens and Nodes
  project.screens.forEach((screen) => {
    // Screen background checks
    if (screen.background?.type === 'image' && !caps.supportsImages) {
      report.push({
        severity: SEVERITY.WARNING,
        nodeId: screen.id,
        nodeName: screen.name,
        feature: 'background.image',
        message: 'Screen background specifies an image asset, but target renderer lacks image asset stream.',
      })
    }

    if (screen.isScrollable && (screen.maxScrollY || 0) > (device.height || 280) && !caps.supportsDoubleBuffer) {
      report.push({
        severity: SEVERITY.INFO,
        nodeId: screen.id,
        nodeName: screen.name,
        feature: 'screen.scrollable',
        message: 'Scrollable screen on device without PSRAM double-buffer may exhibit tearing during scroll redraws.',
      })
    }

    // Inspect each node recursively
    inspectNodes(screen.children || [], screen, device, caps, report)
  })

  return createResult(report)
}

function inspectNodes(nodes, screen, device, caps, report) {
  nodes.forEach((node) => {
    const layout = node.layout || {}
    const style = node.style || {}
    const typography = node.typography || {}
    const events = node.events || []
    const animations = node.animations || []
    const properties = node.properties || {}

    // A. Viewport Layout Boundaries
    const screenWidth = device.width || 240
    const screenHeight = device.height || 280

    if (layout.x < 0 || layout.y < 0 || layout.width > screenWidth || (!screen.isScrollable && (layout.y + layout.height > screenHeight))) {
      report.push({
        severity: SEVERITY.INFO,
        nodeId: node.id,
        nodeName: node.name || node.type,
        feature: 'layout.bounds',
        message: `Node bounds [x:${layout.x}, y:${layout.y}, w:${layout.width}, h:${layout.height}] extend outside target display dimensions (${screenWidth}x${screenHeight}).`,
      })
    }

    // B. Opacity Blending
    if (style.opacity !== undefined && style.opacity < 1 && !caps.supportsOpacity) {
      report.push({
        severity: SEVERITY.WARNING,
        nodeId: node.id,
        nodeName: node.name || node.type,
        feature: 'style.opacity',
        message: 'Alpha opacity blending is not supported by target device renderer; element will render fully opaque.',
      })
    }

    // C. 2D Rotation & Scaling
    if (layout.rotation !== undefined && layout.rotation !== 0 && !caps.supportsRotation) {
      report.push({
        severity: SEVERITY.WARNING,
        nodeId: node.id,
        nodeName: node.name || node.type,
        feature: 'layout.rotation',
        message: `Rotation (${layout.rotation}°) is not supported by target device renderer; element will render at 0°.`,
      })
    }

    if (((layout.scaleX !== undefined && layout.scaleX !== 1) || (layout.scaleY !== undefined && layout.scaleY !== 1)) && !caps.supportsScale) {
      report.push({
        severity: SEVERITY.WARNING,
        nodeId: node.id,
        nodeName: node.name || node.type,
        feature: 'layout.scale',
        message: 'Dynamic 2D scaling is not supported by target device renderer.',
      })
    }

    // D. Backdrop Blur Filter
    if ((node.type === 'card_glass' || style.backdropFilter || properties.backdropFilter) && !caps.supportsBlur) {
      report.push({
        severity: SEVERITY.INFO,
        nodeId: node.id,
        nodeName: node.name || node.type,
        feature: 'style.blur',
        message: 'Backdrop blur filter is not supported by target device renderer; element will render with flat tinted background.',
      })
    }

    // E. Touch Interaction on Non-Touch Target
    if (events.length > 0 && !caps.supportsTouch) {
      report.push({
        severity: SEVERITY.ERROR,
        nodeId: node.id,
        nodeName: node.name || node.type,
        feature: 'input.touch',
        message: 'Touch interactive event defined on a target device without touch input hardware.',
      })
    }

    // F. Animations and 3D Transforms
    if (animations.length > 0 && !caps.supportsAnimation) {
      report.push({
        severity: SEVERITY.WARNING,
        nodeId: node.id,
        nodeName: node.name || node.type,
        feature: 'animation',
        message: 'Target device renderer currently lacks a hardware animation frame loop; element will render statically.',
      })
    }

    // Specific 3D Transforms (e.g. rotateY in Damith Yellow UIverse button or explicit animation property)
    const hasRotateY = node.type === 'uiv_btn_damith_yellow' || animations.some((a) => a.property === 'rotateY' || a.transform?.includes('rotateY'))
    if (hasRotateY && !caps.supports3DTransform) {
      report.push({
        severity: SEVERITY.WARNING,
        nodeId: node.id,
        nodeName: node.name || node.type,
        feature: 'animation.rotateY',
        message: '3D rotateY is not supported by the target device renderer.',
      })
    }

    // G. Custom Font Typography
    if (typography.fontFamily && !['Outfit', 'System', '5x7', 'Press Start 2P'].includes(typography.fontFamily) && !caps.supportsCustomFonts) {
      report.push({
        severity: SEVERITY.INFO,
        nodeId: node.id,
        nodeName: node.name || node.type,
        feature: 'typography.fontFamily',
        message: `Custom font "${typography.fontFamily}" is not bundled in firmware; will fall back to built-in bitmap font.`,
      })
    }

    // Recurse on children if any
    if (Array.isArray(node.children) && node.children.length > 0) {
      inspectNodes(node.children, screen, device, caps, report)
    }
  })
}

function createResult(report) {
  const errors = report.filter((i) => i.severity === SEVERITY.ERROR).length
  const warnings = report.filter((i) => i.severity === SEVERITY.WARNING).length
  const infos = report.filter((i) => i.severity === SEVERITY.INFO).length

  return {
    valid: errors === 0,
    hasErrors: errors > 0,
    hasWarnings: warnings > 0,
    report,
    summary: {
      errors,
      warnings,
      infos,
      total: report.length,
    },
  }
}
