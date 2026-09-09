// ============================================================
// Luna Display Studio — UI Components Registry
// Each entry: name, category, defaultProps, component (React)
// ============================================================
import React from 'react'
import styled, { keyframes } from 'styled-components'
import DamithYellowBtn from './elements/buttons/DamithYellowBtn'
import HappyCodingBtn from './elements/buttons/HappyCodingBtn'

// ── Inline micro-components for built-in elements ──────────

// CARDS
const GlassCard = ({ w, h, bgColor, borderColor, textColor, subtextColor, title, subtitle, radius }) => (
  <div style={{
    width: w, height: h, borderRadius: radius,
    background: bgColor, border: `1.5px solid ${borderColor}`,
    boxShadow: '0 8px 32px rgba(0,0,0,0.18)',
    padding: '10px 14px', display: 'flex', flexDirection: 'column', justifyContent: 'center',
    backdropFilter: 'blur(12px)',
  }}>
    <div style={{ fontSize: 13, fontWeight: 700, color: textColor, marginBottom: 3 }}>{title}</div>
    <div style={{ fontSize: 10, color: subtextColor }}>{subtitle}</div>
  </div>
)

const StatCard = ({ w, h, bgColor, borderColor, accentColor, textColor, label, value, unit, radius }) => (
  <div style={{
    width: w, height: h, borderRadius: radius,
    background: bgColor, border: `1.5px solid ${borderColor}`,
    padding: '8px 12px', display: 'flex', flexDirection: 'column', justifyContent: 'center',
    boxShadow: '0 4px 14px rgba(0,0,0,0.15)',
  }}>
    <div style={{ fontSize: 9, color: accentColor, textTransform: 'uppercase', letterSpacing: 1, marginBottom: 2 }}>{label}</div>
    <div style={{ fontSize: 22, fontWeight: 800, color: textColor, lineHeight: 1.1 }}>
      {value}<span style={{ fontSize: 11, fontWeight: 400, color: accentColor, marginLeft: 3 }}>{unit}</span>
    </div>
  </div>
)

const TransactionCard = ({ w, h, bgColor, borderColor, iconBg, textColor, title, radius }) => (
  <div style={{
    width: w, height: h, borderRadius: radius,
    background: bgColor, border: `1px solid ${borderColor}`,
    padding: '6px 12px', display: 'flex', alignItems: 'center', gap: 10,
    boxShadow: '0 4px 14px rgba(0,0,0,0.12)',
  }}>
    <div style={{ width: 32, height: 32, borderRadius: 6, background: iconBg, display: 'flex', alignItems: 'center', justifyContent: 'center', fontSize: 14, flexShrink: 0 }}>$</div>
    <div style={{ flex: 1, fontSize: 12, fontWeight: 600, color: textColor }}>{title}</div>
    <div style={{ fontSize: 14, color: textColor, opacity: 0.5 }}>›</div>
  </div>
)

const RetroCard = ({ w, h, bgColor, borderColor, textColor, title, score }) => (
  <div style={{
    width: w, height: h, background: bgColor,
    border: `3px solid ${borderColor}`, borderRadius: 0,
    padding: '6px 12px', display: 'flex', flexDirection: 'column', justifyContent: 'center',
    fontFamily: "'Press Start 2P', monospace", imageRendering: 'pixelated',
    boxShadow: `4px 4px 0 ${borderColor}`,
  }}>
    <div style={{ fontSize: 8, color: borderColor, marginBottom: 4 }}>{title}</div>
    <div style={{ fontSize: 14, color: textColor }}>SC: {score}</div>
  </div>
)

// BUTTONS
const NeonButton = ({ w, h, bgColor, borderColor, textColor, label, radius }) => (
  <div style={{
    width: w, height: h, borderRadius: radius,
    background: bgColor, border: `2px solid ${borderColor}`,
    display: 'flex', alignItems: 'center', justifyContent: 'center',
    boxShadow: `0 0 12px ${borderColor}55, inset 0 0 12px ${borderColor}22`,
    fontSize: 12, fontWeight: 700, color: textColor, letterSpacing: 2,
    textTransform: 'uppercase', cursor: 'pointer',
  }}>{label}</div>
)

const RetroButton = ({ w, h, bgColor, borderColor, textColor, label }) => (
  <div style={{
    width: w, height: h, background: bgColor,
    border: `3px solid ${borderColor}`, borderRadius: 0,
    display: 'flex', alignItems: 'center', justifyContent: 'center',
    fontFamily: "'Press Start 2P', monospace",
    fontSize: 8, color: textColor, letterSpacing: 1,
    boxShadow: `4px 4px 0 ${borderColor}`, cursor: 'pointer',
    textTransform: 'uppercase',
  }}>{label}</div>
)

const DownloadButton = ({ w, h, bgColor, textColor, label, radius }) => (
  <div style={{
    width: w, height: h, borderRadius: radius, background: bgColor,
    display: 'flex', alignItems: 'center', justifyContent: 'center', gap: 6,
    fontSize: 12, fontWeight: 700, color: textColor, cursor: 'pointer',
    boxShadow: '0 4px 14px rgba(0,0,0,0.2)',
  }}>
    <span style={{ fontSize: 14 }}>↓</span>{label}
  </div>
)

// TOGGLES
const ToggleSwitch = ({ w, h, bgColor, accentColor, checked, label }) => (
  <div style={{ display: 'flex', alignItems: 'center', gap: 8, width: w, height: h }}>
    <div style={{
      width: 44, height: 24, borderRadius: 12, background: checked ? accentColor : '#ccc',
      position: 'relative', cursor: 'pointer', transition: 'background 0.2s', flexShrink: 0,
    }}>
      <div style={{
        width: 20, height: 20, borderRadius: 10, background: '#fff',
        position: 'absolute', top: 2, left: checked ? 22 : 2,
        transition: 'left 0.2s', boxShadow: '0 1px 4px rgba(0,0,0,0.3)',
      }} />
    </div>
    <span style={{ fontSize: 12, fontWeight: 600, color: bgColor }}>{label}</span>
  </div>
)

const NeonToggle = ({ accentColor, checked, label, w, h }) => (
  <div style={{ display: 'flex', alignItems: 'center', gap: 8, width: w, height: h }}>
    <div style={{
      width: 48, height: 26, borderRadius: 13,
      background: checked ? `${accentColor}33` : '#1e293b',
      border: `2px solid ${checked ? accentColor : '#334155'}`,
      position: 'relative', cursor: 'pointer',
      boxShadow: checked ? `0 0 10px ${accentColor}66` : 'none',
      flexShrink: 0,
    }}>
      <div style={{
        width: 18, height: 18, borderRadius: 9,
        background: checked ? accentColor : '#475569',
        position: 'absolute', top: 2, left: checked ? 24 : 2,
        transition: 'left 0.2s, background 0.2s',
        boxShadow: checked ? `0 0 8px ${accentColor}` : 'none',
      }} />
    </div>
    <span style={{ fontSize: 11, color: checked ? accentColor : '#64748b', fontWeight: 600 }}>{label}</span>
  </div>
)

// LOADERS with proper styled-components keyframes
const spinAnim = keyframes`
  0% { transform: rotate(0deg); }
  100% { transform: rotate(360deg); }
`

const dotBounceAnim = keyframes`
  from { transform: translateY(0); opacity: 1; }
  to { transform: translateY(-8px); opacity: 0.35; }
`

const pulseAnim = keyframes`
  0%, 100% { transform: scale(1); opacity: 0.9; }
  50% { transform: scale(1.3); opacity: 0.4; }
`

const SpinnerRing = styled.div`
  width: ${p => p.$size}px;
  height: ${p => p.$size}px;
  border-radius: 50%;
  border: 3px solid ${p => p.$bg};
  border-top-color: ${p => p.$accent};
  animation: ${spinAnim} 0.8s linear infinite;
`

const DotBubble = styled.div`
  width: 10px;
  height: 10px;
  border-radius: 50%;
  background: ${p => p.$accent};
  animation: ${dotBounceAnim} 0.8s ease-in-out ${p => p.$delay}s infinite alternate;
`

const PulseRing = styled.div`
  width: 40px;
  height: 40px;
  border-radius: 50%;
  background: ${p => p.$accent}33;
  border: 2px solid ${p => p.$accent};
  animation: ${pulseAnim} 1.2s ease-in-out infinite;
`

const SpinnerLoader = ({ accentColor = '#00f2fe', bgColor = '#1e293b', size = 30, label = 'LOADING', w, h }) => (
  <div style={{ width: w, height: h, display: 'flex', flexDirection: 'column', alignItems: 'center', justifyContent: 'center', gap: 6 }}>
    <SpinnerRing $size={size} $bg={bgColor} $accent={accentColor} />
    <span style={{ fontSize: 9, color: accentColor, fontFamily: 'JetBrains Mono, monospace', letterSpacing: 1 }}>{label}</span>
  </div>
)

const DotLoader = ({ accentColor = '#7c3aed', w, h }) => (
  <div style={{ width: w, height: h, display: 'flex', alignItems: 'center', justifyContent: 'center', gap: 6 }}>
    {[0, 1, 2].map(i => (
      <DotBubble key={i} $accent={accentColor} $delay={i * 0.15} />
    ))}
  </div>
)

const PulseLoader = ({ accentColor = '#ec4899', bgColor = '#1e293b', w, h }) => (
  <div style={{ width: w, height: h, display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
    <PulseRing $accent={accentColor} />
  </div>
)

// CHECKBOXES
const NeonCheckbox = ({ accentColor, bgColor, textColor, checked, label, w, h }) => (
  <div style={{ display: 'flex', alignItems: 'center', gap: 8, width: w, height: h }}>
    <div style={{
      width: 20, height: 20, borderRadius: 4,
      border: `2px solid ${checked ? accentColor : '#475569'}`,
      background: checked ? `${accentColor}22` : bgColor,
      display: 'flex', alignItems: 'center', justifyContent: 'center',
      boxShadow: checked ? `0 0 8px ${accentColor}66` : 'none',
      flexShrink: 0, cursor: 'pointer',
    }}>
      {checked && <span style={{ color: accentColor, fontSize: 14, fontWeight: 800, lineHeight: 1 }}>✓</span>}
    </div>
    <span style={{ fontSize: 12, color: textColor, fontWeight: 500 }}>{label}</span>
  </div>
)

// GAUGES
const ProgressBar = ({ w, h, bgColor, fillColor, borderColor, value, label, radius }) => (
  <div style={{ width: w, height: h, display: 'flex', flexDirection: 'column', justifyContent: 'center', gap: 4 }}>
    <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: 10, color: fillColor }}>
      <span style={{ fontWeight: 600 }}>{label}</span>
      <span>{value}%</span>
    </div>
    <div style={{ width: '100%', height: 10, borderRadius: radius, background: bgColor, border: `1px solid ${borderColor}` }}>
      <div style={{
        width: `${Math.min(100, Math.max(0, value))}%`, height: '100%',
        borderRadius: radius, background: fillColor,
        boxShadow: `0 0 6px ${fillColor}88`,
        transition: 'width 0.3s',
      }} />
    </div>
  </div>
)

// TEXT
const DigitalClock = ({ w, h, timeStr, dateStr, color, dateColor }) => (
  <div style={{ width: w, height: h, display: 'flex', flexDirection: 'column', alignItems: 'center', justifyContent: 'center' }}>
    <div style={{ fontSize: 36, fontWeight: 800, color, fontFamily: 'JetBrains Mono, monospace', lineHeight: 1 }}>{timeStr}</div>
    <div style={{ fontSize: 11, color: dateColor, marginTop: 4, letterSpacing: 1 }}>{dateStr}</div>
  </div>
)

const CustomLabel = ({ w, h, text, color, fontSize, fontWeight, bgColor, radius }) => (
  <div style={{
    width: w, height: h, borderRadius: radius, background: bgColor,
    display: 'flex', alignItems: 'center', justifyContent: 'center',
    fontSize, fontWeight, color, letterSpacing: 0.5,
    fontFamily: 'Outfit, sans-serif',
  }}>{text}</div>
)

// NOTIFICATION
const AlertBadge = ({ w, h, bgColor, accentColor, textColor, title, message, radius }) => (
  <div style={{
    width: w, height: h, borderRadius: radius,
    background: bgColor, border: `1.5px solid ${accentColor}`,
    padding: '6px 12px', display: 'flex', alignItems: 'center', gap: 8,
    boxShadow: `0 0 12px ${accentColor}44`,
  }}>
    <div style={{ width: 8, height: 8, borderRadius: '50%', background: accentColor, flexShrink: 0, boxShadow: `0 0 6px ${accentColor}` }} />
    <div>
      <div style={{ fontSize: 11, fontWeight: 700, color: textColor }}>{title}</div>
      <div style={{ fontSize: 9, color: accentColor }}>{message}</div>
    </div>
  </div>
)

// INPUT
const TextInput = ({ w, h, bgColor, borderColor, textColor, placeholder, radius }) => (
  <div style={{
    width: w, height: h, borderRadius: radius,
    background: bgColor, border: `1.5px solid ${borderColor}`,
    display: 'flex', alignItems: 'center', padding: '0 10px',
    fontSize: 12, color: `${textColor}66`,
    boxShadow: 'inset 2px 2px 6px rgba(0,0,0,0.2)',
  }}>{placeholder}</div>
)

// ── REGISTRY ───────────────────────────────────────────────
export const UI_COMPONENTS = {
  // CARDS
  card_glass: {
    name: 'Glass Card', category: 'cards',
    defaultProps: { x: 16, y: 30, w: 208, h: 80, title: 'Status', subtitle: 'System online • v2.0', bgColor: '#1e293bcc', borderColor: '#00f2fe44', textColor: '#ffffff', subtextColor: '#94a3b8', radius: 14 },
    component: GlassCard,
  },
  card_stat: {
    name: 'Stat Card', category: 'cards',
    defaultProps: { x: 16, y: 30, w: 100, h: 60, label: 'CPU', value: '72', unit: '%', bgColor: '#0f172a', borderColor: '#1e293b', accentColor: '#00f2fe', textColor: '#ffffff', radius: 10 },
    component: StatCard,
  },
  card_transaction: {
    name: 'Transaction Card', category: 'cards',
    defaultProps: { x: 16, y: 36, w: 208, h: 56, title: 'New Transaction', bgColor: '#ffffff', iconBg: '#10b981', textColor: '#0f172a', radius: 12, borderColor: '#e2e8f0', borderWidth: 1 },
    component: TransactionCard,
  },
  card_retro: {
    name: 'Retro Arcade Card', category: 'cards',
    defaultProps: { x: 16, y: 36, w: 180, h: 70, title: 'GAME OVER', score: '38420', bgColor: '#0b0014', borderColor: '#ff00ff', textColor: '#ffffff' },
    component: RetroCard,
  },

  // BUTTONS
  button_neon: {
    name: 'Neon Glow Button', category: 'buttons',
    defaultProps: { x: 60, y: 130, w: 120, h: 36, label: 'ACTIVATE', bgColor: '#0f172a', borderColor: '#00f2fe', textColor: '#00f2fe', radius: 8 },
    component: NeonButton,
  },
  button_retro: {
    name: '8-Bit Arcade Button', category: 'buttons',
    defaultProps: { x: 40, y: 120, w: 160, h: 44, label: 'PRESS START', bgColor: '#cc0000', borderColor: '#ff4444', textColor: '#ffffff' },
    component: RetroButton,
  },
  button_download: {
    name: 'Download Button', category: 'buttons',
    defaultProps: { x: 60, y: 120, w: 120, h: 36, label: 'Download', bgColor: '#2563eb', textColor: '#ffffff', radius: 18 },
    component: DownloadButton,
  },
  uiv_btn_damith_yellow: {
    name: 'Neon Yellow Border Btn (Damith)', category: 'buttons',
    description: 'UIverse.io by Damithkumara — slide + rotateY 360° hover. Exact React component.',
    defaultProps: { x: 40, y: 120, w: 160, h: 48, label: 'BUTTON', borderColor: '#ffff00', bgColor: '#363636', textColor: '#ffff00' },
    component: DamithYellowBtn,
  },
  uiv_btn_happy_coding: {
    name: 'Happy Coding Button', category: 'buttons',
    description: 'UIverse.io Neumorphic 3D Indigo Button with tactile press animation. Exact React component.',
    defaultProps: { x: 25, y: 110, w: 190, h: 50, label: 'Happy Coding!', bgColor: '#EEF2FF', borderColor: '#536DFE', textColor: '#536DFE' },
    component: HappyCodingBtn,
  },

  // TOGGLES
  toggle_switch: {
    name: 'Toggle Switch', category: 'toggles',
    defaultProps: { x: 20, y: 130, w: 140, h: 30, label: 'WiFi', checked: true, accentColor: '#10b981', bgColor: '#1e293b' },
    component: ToggleSwitch,
  },
  toggle_neon: {
    name: 'Neon Toggle', category: 'toggles',
    defaultProps: { x: 20, y: 130, w: 140, h: 30, label: 'Bluetooth', checked: true, accentColor: '#7c3aed' },
    component: NeonToggle,
  },

  // LOADERS
  loader_spinner: {
    name: 'Spinner Loader', category: 'loaders',
    defaultProps: { x: 90, y: 110, w: 60, h: 60, size: 30, label: 'LOADING', accentColor: '#00f2fe', bgColor: '#1e293b' },
    component: SpinnerLoader,
  },
  loader_dots: {
    name: 'Dot Loader', category: 'loaders',
    defaultProps: { x: 70, y: 120, w: 100, h: 40, accentColor: '#7c3aed' },
    component: DotLoader,
  },
  loader_pulse: {
    name: 'Pulse Loader', category: 'loaders',
    defaultProps: { x: 90, y: 110, w: 60, h: 60, accentColor: '#ec4899', bgColor: '#1e293b' },
    component: PulseLoader,
  },

  // CHECKBOXES
  checkbox_neon: {
    name: 'Neon Checkbox', category: 'checkboxes',
    defaultProps: { x: 20, y: 130, w: 160, h: 28, label: 'Enable feature', checked: true, accentColor: '#10b981', bgColor: '#0f172a', textColor: '#e2e8f0' },
    component: NeonCheckbox,
  },

  // GAUGES
  gauge_progress: {
    name: 'Progress Bar', category: 'gauges',
    defaultProps: { x: 16, y: 120, w: 208, h: 40, label: 'Battery', value: 72, fillColor: '#10b981', bgColor: '#1e293b', borderColor: '#334155', textColor: '#ffffff', radius: 6 },
    component: ProgressBar,
  },

  // TEXT
  digital_clock: {
    name: 'Digital Clock', category: 'text',
    defaultProps: { x: 20, y: 30, w: 200, h: 70, timeStr: '10:45', dateStr: 'WED, SEP 9', color: '#00f2fe', dateColor: '#94a3b8' },
    component: DigitalClock,
  },
  custom_label: {
    name: 'Custom Label / Header', category: 'text',
    defaultProps: { x: 16, y: 20, w: 208, h: 44, text: 'LUNA SMART WATCH', color: '#ffffff', fontSize: 14, fontWeight: 800, bgColor: '#1e293b', radius: 8 },
    component: CustomLabel,
  },

  // NOTIFICATIONS
  alert_badge: {
    name: 'Alert Badge', category: 'notifications',
    defaultProps: { x: 16, y: 20, w: 208, h: 44, title: 'System Alert', message: 'Update available', bgColor: '#0f172a', accentColor: '#f59e0b', textColor: '#ffffff', radius: 10 },
    component: AlertBadge,
  },

  // INPUTS
  text_input: {
    name: 'Text Input Field', category: 'inputs',
    defaultProps: { x: 16, y: 100, w: 208, h: 36, placeholder: 'Enter value...', bgColor: '#1e293b', borderColor: '#334155', textColor: '#e2e8f0', radius: 8 },
    component: TextInput,
  },
}

export const CATEGORIES = [
  { id: 'all', label: 'All' },
  { id: 'cards', label: 'Cards' },
  { id: 'buttons', label: 'Buttons' },
  { id: 'toggles', label: 'Toggles' },
  { id: 'loaders', label: 'Loaders' },
  { id: 'checkboxes', label: 'Checkboxes' },
  { id: 'notifications', label: 'Alerts' },
  { id: 'gauges', label: 'Gauges' },
  { id: 'inputs', label: 'Inputs' },
  { id: 'text', label: 'Text' },
  { id: 'custom', label: 'Compiled' },
]

// ── Dynamic Registry for Runtime-Compiled Components ───────
export const DYNAMIC_REGISTRY = {}
const registryListeners = new Set()

export function subscribeRegistry(listener) {
  registryListeners.add(listener)
  return () => registryListeners.delete(listener)
}

export function registerCustomElement(key, definition) {
  DYNAMIC_REGISTRY[key] = definition
  UI_COMPONENTS[key] = definition
  registryListeners.forEach(fn => fn(UI_COMPONENTS))
}

