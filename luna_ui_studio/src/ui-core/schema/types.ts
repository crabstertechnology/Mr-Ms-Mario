/**
 * Luna UI Studio — Canonical UI Schema Types
 *
 * This file defines the hardware-independent, declarative data model for UI projects.
 * It serves as the single source of truth for both React preview and embedded code generation.
 *
 * RULE: This schema must NOT depend on browser DOM or CSS concepts (no flexbox, CSS grid,
 * DOM positioning, or CSS strings). It must represent pure semantic UI structures that
 * can be compiled to ESP32 / Arduino / LVGL graphics backends.
 */

export interface UIDevice {
  id: string
  name: string
  width: number
  height: number
  colorDepth: 16 | 24 | 32
  orientation: 'portrait' | 'landscape'
  touch: boolean
  mcu?: string
  displayController?: string
  touchController?: string
  panelOffsetX?: number
  panelOffsetY?: number
}

export interface UILayout {
  x: number
  y: number
  width: number
  height: number
  rotation?: number
  scaleX?: number
  scaleY?: number
  zIndex?: number
}

export interface UIStyle {
  backgroundColor?: string
  borderColor?: string
  borderWidth?: number
  borderRadius?: number
  opacity?: number
  clipChildren?: boolean
  accentColor?: string
  shadowColor?: string
  shadowBlur?: number
}

export interface UITypography {
  fontFamily: string
  fontSize: number
  fontWeight: number | string
  color: string
  align: 'left' | 'center' | 'right'
  lineHeight?: number
  letterSpacing?: number
}

export type UIStateName = 'normal' | 'pressed' | 'active' | 'disabled' | 'selected'

export interface UIStateStyle {
  style?: Partial<UIStyle>
  typography?: Partial<UITypography>
  layout?: Partial<UILayout>
}

export type UIEventTrigger = 'onClick' | 'onPress' | 'onRelease' | 'onSwipe' | 'onLoad' | 'onChange'

export type UIActionType = 'navigate' | 'scroll' | 'alert' | 'toggle' | 'setState' | 'custom'

export interface UIAction {
  id?: string
  type: UIActionType
  targetScreenId?: string
  transition?: 'none' | 'slide-left' | 'slide-right' | 'fade'
  scrollDirection?: 'up' | 'down' | 'left' | 'right'
  scrollAmount?: number
  alertMessage?: string
  stateKey?: string
  stateValue?: any
  customCommand?: string
}

export interface UIEvent {
  id?: string
  trigger: UIEventTrigger
  action: UIAction
}

export type UIEasing = 'linear' | 'easeIn' | 'easeOut' | 'easeInOut' | 'step'

export interface UIAnimation {
  id?: string
  property: string
  from: number | string
  to: number | string
  duration: number
  delay?: number
  easing: UIEasing
  repeat: 'once' | 'infinite' | number
  direction: 'normal' | 'reverse' | 'alternate'
  trigger: 'onLoad' | 'onClick' | 'onStateChange'
}

export interface UIAssetReference {
  id: string
  name: string
  type: 'image' | 'font' | 'binary'
  format: string
  sourcePath: string
  metadata?: Record<string, any>
}

export interface UIFontReference {
  id: string
  family: string
  weight: number | string
  size: number
  sourcePath?: string
  isSystem?: boolean
}

export interface UIBackground {
  type: 'color' | 'gradient' | 'pattern' | 'image'
  color?: string
  gradient?: {
    type: 'linear' | 'radial'
    stops: Array<{ offset: number; color: string }>
    angle?: number
  }
  pattern?: 'none' | 'stars' | 'grid' | 'dots' | 'scanlines' | 'carbon' | 'hex'
  imageAssetId?: string
}

export interface UINode {
  id: string
  type: string
  name: string
  layout: UILayout
  style: UIStyle
  typography?: UITypography
  children?: UINode[]
  states?: Partial<Record<UIStateName, UIStateStyle>>
  events?: UIEvent[]
  animations?: UIAnimation[]
  assetReferences?: UIAssetReference[]
  properties: Record<string, any>
}

export interface UIScreenGestures {
  swipeLeft?: UIAction
  swipeRight?: UIAction
  swipeUp?: UIAction
  swipeDown?: UIAction
}

export interface UIScreen {
  id: string
  name: string
  background: UIBackground
  children: UINode[]
  gestures?: UIScreenGestures
  isScrollable?: boolean
  maxScrollY?: number
  metadata?: Record<string, any>
}

export interface UIProject {
  version: '1.0.0'
  name: string
  device: UIDevice
  screens: UIScreen[]
  activeScreenId: string
  assets: UIAssetReference[]
  fonts: UIFontReference[]
  metadata: {
    createdAt: string
    updatedAt: string
    author?: string
    description?: string
  }
}
