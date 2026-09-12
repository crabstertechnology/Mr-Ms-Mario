// ============================================================
// Luna UI Studio — UI Components Registry
// Visual Language: Kinesis / Monolith Design System
// Target Hardware: Waveshare ESP32-S3 Touch LCD 1.69" (240x280)
// ============================================================
import React from 'react'
import styled, { keyframes } from 'styled-components'
import DamithYellowBtn from './elements/buttons/DamithYellowBtn'
import HappyCodingBtn from './elements/buttons/HappyCodingBtn'
import { StarfieldPatternElement } from '../components/Canvas/StarfieldBackground'
import {
  LunaButton,
  LunaIconButton,
  LunaSurface,
  LunaList,
  LunaListItem,
  LunaToggle,
  LunaSlider,
  LunaProgress,
  LunaIndicator,
  LunaDialog,
  LunaToast,
  LunaHeader,
  LunaNavigation,
  LunaTimer,
  LunaNumber,
  LunaStatus
} from '../ui-core/components/index.js'

// ============================================================
// 1. KINESIS / MONOLITH DESIGN SYSTEM PRIMITIVES
// ============================================================

/**
 * 1. MonolithTime
 * Architectural, oversized digital time block with integrated status, battery and date tags.
 */
const MonolithTime = ({
  w = 220, h = 90,
  timeStr = '10:42', secondsStr = ':38', dateStr = 'WED 09 SEP',
  batteryPct = 94, statusText = 'LUNA • READY',
  bgColor = '#121721', borderColor = '#232D3F',
  textColor = '#EAEFF5', accentColor = '#38BDF8', radius = 10
}) => (
  <div style={{
    width: w, height: h, borderRadius: radius,
    background: bgColor, border: `1px solid ${borderColor}`,
    padding: '8px 12px', display: 'flex', flexDirection: 'column',
    justifyContent: 'space-between', boxSizing: 'border-box',
    boxShadow: '0 4px 16px rgba(0,0,0,0.3)', userSelect: 'none'
  }}>
    {/* Header Status & Battery */}
    <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
      <div style={{ display: 'flex', alignItems: 'center', gap: 5 }}>
        <span style={{ width: 6, height: 6, borderRadius: '50%', background: accentColor, display: 'inline-block' }} />
        <span style={{ fontSize: 9, fontWeight: 700, letterSpacing: 1.2, color: accentColor, textTransform: 'uppercase' }}>
          {statusText}
        </span>
      </div>
      <div style={{ display: 'flex', alignItems: 'center', gap: 4, background: 'rgba(255,255,255,0.06)', padding: '2px 6px', borderRadius: 4 }}>
        <span style={{ fontSize: 8, color: '#10B981' }}>⚡</span>
        <span style={{ fontSize: 9, fontWeight: 700, color: textColor, letterSpacing: 0.5 }}>{batteryPct}%</span>
      </div>
    </div>

    {/* Hero Architectural Time */}
    <div style={{ display: 'flex', alignItems: 'baseline', gap: 4, lineHeight: 1 }}>
      <span style={{ fontSize: 38, fontWeight: 900, letterSpacing: -1, color: textColor, fontFamily: 'Outfit, sans-serif' }}>
        {timeStr}
      </span>
      <span style={{ fontSize: 16, fontWeight: 700, color: accentColor, opacity: 0.9 }}>
        {secondsStr}
      </span>
    </div>

    {/* Sub-bar Date & Meridian Ticks */}
    <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', borderTop: `1px solid ${borderColor}66`, paddingTop: 4 }}>
      <span style={{ fontSize: 10, fontWeight: 600, letterSpacing: 1, color: '#8290A4', textTransform: 'uppercase' }}>
        {dateStr}
      </span>
      <div style={{ display: 'flex', gap: 3 }}>
        <span style={{ width: 12, height: 2, background: accentColor, borderRadius: 1 }} />
        <span style={{ width: 4, height: 2, background: `${borderColor}`, borderRadius: 1 }} />
      </div>
    </div>
  </div>
)

/**
 * 2. GlanceBar
 * Dynamic contextual glance strip displaying immediate next-event or ambient state.
 */
const GlanceBar = ({
  w = 220, h = 42,
  icon = '⚡', label = 'NEXT FOCUS', detail = 'Sprint Review @ 11:00',
  bgColor = '#121721', borderColor = '#232D3F',
  textColor = '#EAEFF5', accentColor = '#FF9E3B', radius = 8
}) => (
  <div style={{
    width: w, height: h, borderRadius: radius,
    background: bgColor, border: `1px solid ${borderColor}`,
    padding: '6px 10px', display: 'flex', alignItems: 'center', gap: 9,
    boxSizing: 'border-box', boxShadow: '0 2px 8px rgba(0,0,0,0.25)', userSelect: 'none'
  }}>
    <div style={{
      width: 26, height: 26, borderRadius: 6,
      background: `${accentColor}22`, border: `1px solid ${accentColor}55`,
      display: 'flex', alignItems: 'center', justifyContent: 'center',
      fontSize: 13, color: accentColor, flexShrink: 0
    }}>
      {icon}
    </div>
    <div style={{ flex: 1, minWidth: 0, display: 'flex', flexDirection: 'column' }}>
      <div style={{ fontSize: 8, fontWeight: 700, letterSpacing: 1, color: accentColor, textTransform: 'uppercase' }}>
        {label}
      </div>
      <div style={{ fontSize: 11, fontWeight: 600, color: textColor, whiteSpace: 'nowrap', overflow: 'hidden', textOverflow: 'ellipsis' }}>
        {detail}
      </div>
    </div>
    <div style={{ fontSize: 13, color: '#8290A4', opacity: 0.7 }}>›</div>
  </div>
)

/**
 * 3. TactileButton
 * Commanding primary action block (minimum 44-50px height) designed for foolproof thumb operation.
 */
const TactileButton = ({
  w = 220, h = 48,
  label = 'START FOCUS SESSION', icon = '▶',
  bgColor = '#FF9E3B', borderColor = '#FFB266',
  textColor = '#080A0F', accentColor = '#080A0F', radius = 12
}) => (
  <div style={{
    width: w, height: h, borderRadius: radius,
    background: bgColor, border: `1.5px solid ${borderColor}`,
    display: 'flex', alignItems: 'center', justifyContent: 'center', gap: 8,
    boxSizing: 'border-box', boxShadow: '0 4px 14px rgba(255,158,59,0.3)',
    cursor: 'pointer', userSelect: 'none'
  }}>
    {icon && <span style={{ fontSize: 13, fontWeight: 900, color: textColor }}>{icon}</span>}
    <span style={{ fontSize: 12, fontWeight: 800, letterSpacing: 1.2, color: textColor, textTransform: 'uppercase' }}>
      {label}
    </span>
  </div>
)

/**
 * 4. NotificationBlock
 * Monolithic notification item with unread accent rail, sender pill and short preview.
 */
const NotificationBlock = ({
  w = 220, h = 72,
  sender = 'SARAH CONNOR', timeStr = '4m ago',
  preview = 'Firmware calibration complete. Sensor ready.',
  unread = true,
  bgColor = '#121721', borderColor = '#232D3F',
  textColor = '#EAEFF5', subtextColor = '#8290A4',
  accentColor = '#38BDF8', radius = 10
}) => (
  <div style={{
    width: w, height: h, borderRadius: radius,
    background: bgColor, border: `1px solid ${borderColor}`,
    borderLeft: unread ? `3.5px solid ${accentColor}` : `1px solid ${borderColor}`,
    padding: '8px 10px', display: 'flex', flexDirection: 'column',
    justifyContent: 'space-between', boxSizing: 'border-box',
    boxShadow: '0 3px 10px rgba(0,0,0,0.2)', userSelect: 'none'
  }}>
    <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
      <div style={{ display: 'flex', alignItems: 'center', gap: 5 }}>
        <span style={{ fontSize: 10, fontWeight: 800, letterSpacing: 0.8, color: textColor, textTransform: 'uppercase' }}>
          {sender}
        </span>
        {unread && <span style={{ width: 5, height: 5, borderRadius: '50%', background: accentColor }} />}
      </div>
      <span style={{ fontSize: 9, color: subtextColor }}>{timeStr}</span>
    </div>
    <div style={{
      fontSize: 11, fontWeight: 500, color: textColor, lineHeight: 1.3,
      display: '-webkit-box', WebkitLineClamp: 2, WebkitBoxOrient: 'vertical', overflow: 'hidden'
    }}>
      {preview}
    </div>
    <div style={{ display: 'flex', justifyContent: 'flex-end' }}>
      <span style={{ fontSize: 8, fontWeight: 700, letterSpacing: 0.8, color: accentColor, textTransform: 'uppercase' }}>
        DISMISS ›
      </span>
    </div>
  </div>
)

/**
 * 5. AgendaBlock
 * Glanceable agenda event card with massive countdown pill ("IN 24m").
 */
const AgendaBlock = ({
  w = 220, h = 84,
  countdown = 'IN 24m', timeRange = '10:30 - 11:15',
  title = 'Architecture Sync', location = 'Lab 4 / BLE Orbit',
  bgColor = '#121721', borderColor = '#232D3F',
  textColor = '#EAEFF5', subtextColor = '#8290A4',
  accentColor = '#38BDF8', radius = 10
}) => (
  <div style={{
    width: w, height: h, borderRadius: radius,
    background: bgColor, border: `1px solid ${borderColor}`,
    padding: '8px 12px', display: 'flex', flexDirection: 'column',
    justifyContent: 'space-between', boxSizing: 'border-box',
    boxShadow: '0 3px 12px rgba(0,0,0,0.25)', userSelect: 'none'
  }}>
    <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
      <div style={{
        background: `${accentColor}22`, border: `1px solid ${accentColor}66`,
        color: accentColor, padding: '2px 6px', borderRadius: 4,
        fontSize: 9, fontWeight: 800, letterSpacing: 0.8
      }}>
        {countdown}
      </div>
      <span style={{ fontSize: 9, color: subtextColor }}>{timeRange}</span>
    </div>
    <div style={{ fontSize: 13, fontWeight: 800, color: textColor, marginTop: 2 }}>
      {title}
    </div>
    <div style={{ display: 'flex', alignItems: 'center', gap: 4 }}>
      <span style={{ fontSize: 9, color: accentColor }}>📍</span>
      <span style={{ fontSize: 10, color: subtextColor }}>{location}</span>
    </div>
  </div>
)

/**
 * 6. GameLauncher
 * Monolithic game showcase card with hero identity and launch zone.
 */
const GameLauncher = ({
  w = 220, h = 170,
  title = 'RETRO RUNNER', genre = 'CYBERPLATFORM • 60FPS',
  highScore = '12,480 PTS', icon = '🏃', badgeText = 'READY TO PLAY',
  bgColor = '#121721', borderColor = '#232D3F',
  textColor = '#EAEFF5', accentColor = '#FF9E3B', radius = 12
}) => (
  <div style={{
    width: w, height: h, borderRadius: radius,
    background: bgColor, border: `1.5px solid ${borderColor}`,
    padding: '12px', display: 'flex', flexDirection: 'column',
    justifyContent: 'space-between', boxSizing: 'border-box',
    boxShadow: '0 6px 20px rgba(0,0,0,0.3)', userSelect: 'none'
  }}>
    <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
      <div style={{
        width: 38, height: 38, borderRadius: 10,
        background: `${accentColor}22`, border: `1px solid ${accentColor}55`,
        display: 'flex', alignItems: 'center', justifyContent: 'center',
        fontSize: 20
      }}>
        {icon}
      </div>
      <div style={{
        background: 'rgba(255,255,255,0.06)', border: `1px solid ${borderColor}`,
        color: accentColor, padding: '3px 8px', borderRadius: 6,
        fontSize: 8, fontWeight: 800, letterSpacing: 1
      }}>
        {badgeText}
      </div>
    </div>

    <div>
      <div style={{ fontSize: 16, fontWeight: 900, letterSpacing: 0.5, color: textColor }}>
        {title}
      </div>
      <div style={{ fontSize: 9, fontWeight: 600, color: '#8290A4', letterSpacing: 0.8, marginTop: 2 }}>
        {genre}
      </div>
      <div style={{ fontSize: 11, fontWeight: 700, color: accentColor, marginTop: 6 }}>
        🏆 {highScore}
      </div>
    </div>

    <div style={{
      width: '100%', height: 34, borderRadius: 8,
      background: accentColor, color: '#080A0F',
      display: 'flex', alignItems: 'center', justifyContent: 'center', gap: 6,
      fontSize: 11, fontWeight: 800, letterSpacing: 1, textTransform: 'uppercase',
      boxShadow: `0 3px 10px ${accentColor}44`
    }}>
      <span>PLAY NOW</span>
      <span>▶</span>
    </div>
  </div>
)

/**
 * 7. FocusChamber
 * Ambient Pomodoro focus experience with geometric compression progress.
 */
const FocusChamber = ({
  w = 220, h = 175,
  timeRemaining = '24:50', modeLabel = 'DEEP WORK', sessionTag = 'SESSION 2 / 4',
  progress = 75,
  bgColor = '#121721', borderColor = '#232D3F',
  textColor = '#EAEFF5', subtextColor = '#8290A4',
  accentColor = '#FF9E3B', radius = 12
}) => (
  <div style={{
    width: w, height: h, borderRadius: radius,
    background: bgColor, border: `1.5px solid ${borderColor}`,
    padding: '12px', display: 'flex', flexDirection: 'column',
    justifyContent: 'space-between', boxSizing: 'border-box',
    boxShadow: '0 6px 20px rgba(0,0,0,0.3)', userSelect: 'none'
  }}>
    <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
      <div style={{ display: 'flex', alignItems: 'center', gap: 6 }}>
        <span style={{ width: 7, height: 7, borderRadius: '50%', background: accentColor }} />
        <span style={{ fontSize: 9, fontWeight: 800, letterSpacing: 1.2, color: accentColor }}>
          {modeLabel}
        </span>
      </div>
      <span style={{ fontSize: 9, fontWeight: 700, color: subtextColor }}>{sessionTag}</span>
    </div>

    <div style={{ textAlign: 'center', padding: '6px 0' }}>
      <div style={{ fontSize: 44, fontWeight: 900, letterSpacing: -1, color: textColor, lineHeight: 1, fontFamily: 'Outfit, sans-serif' }}>
        {timeRemaining}
      </div>
      <div style={{ fontSize: 10, color: subtextColor, marginTop: 4, letterSpacing: 0.5 }}>
        MINUTES REMAINING
      </div>
    </div>

    {/* Geometric Monolith Compression Progress Bar */}
    <div>
      <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: 8, color: subtextColor, marginBottom: 4 }}>
        <span>PROGRESS</span>
        <span>{progress}%</span>
      </div>
      <div style={{ width: '100%', height: 6, borderRadius: 3, background: 'rgba(255,255,255,0.08)', overflow: 'hidden' }}>
        <div style={{ width: `${progress}%`, height: '100%', background: accentColor, borderRadius: 3 }} />
      </div>
    </div>

    <div style={{
      width: '100%', height: 32, borderRadius: 8,
      background: 'rgba(255,255,255,0.08)', border: `1px solid ${borderColor}`,
      color: textColor, display: 'flex', alignItems: 'center', justifyContent: 'center',
      fontSize: 10, fontWeight: 800, letterSpacing: 1, textTransform: 'uppercase'
    }}>
      ⏸ PAUSE SESSION
    </div>
  </div>
)

/**
 * 8. QRUtilityCard
 * Digital ID & QR Code matrix card for BLE profile pairing and contact sharing.
 */
const QRUtilityCard = ({
  w = 220, h = 160,
  title = 'LUNA ID CARD', subtitle = 'ESP32-S3 • BLE PEER', idTag = 'UID: LN-8842-X',
  bgColor = '#121721', borderColor = '#232D3F',
  textColor = '#EAEFF5', accentColor = '#38BDF8', radius = 12
}) => (
  <div style={{
    width: w, height: h, borderRadius: radius,
    background: bgColor, border: `1px solid ${borderColor}`,
    padding: '10px 12px', display: 'flex', flexDirection: 'column',
    alignItems: 'center', justifyContent: 'space-between', boxSizing: 'border-box',
    boxShadow: '0 4px 16px rgba(0,0,0,0.25)', userSelect: 'none'
  }}>
    <div style={{ width: '100%', display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
      <div>
        <div style={{ fontSize: 11, fontWeight: 800, color: textColor }}>{title}</div>
        <div style={{ fontSize: 8, color: '#8290A4' }}>{subtitle}</div>
      </div>
      <span style={{ fontSize: 8, fontWeight: 700, color: accentColor, background: `${accentColor}22`, padding: '2px 5px', borderRadius: 4 }}>
        {idTag}
      </span>
    </div>

    {/* Graphic QR Code Matrix Representation */}
    <div style={{
      width: 80, height: 80, background: '#FFFFFF', borderRadius: 6,
      display: 'grid', gridTemplateColumns: 'repeat(8, 1fr)', gap: 1, padding: 4, boxSizing: 'border-box'
    }}>
      {/* 64-cell deterministic decorative QR pattern */}
      {[
        1,1,1,1,0,1,1,1,
        1,0,0,1,0,1,0,1,
        1,0,0,1,1,0,0,1,
        1,1,1,1,0,1,1,1,
        0,0,1,0,1,0,1,0,
        1,0,1,1,0,1,0,1,
        1,0,0,0,1,1,0,0,
        1,1,1,0,0,1,1,1
      ].map((cell, idx) => (
        <div key={idx} style={{ background: cell ? '#080A0F' : 'transparent', width: '100%', height: '100%' }} />
      ))}
    </div>

    <div style={{ fontSize: 9, color: '#8290A4', letterSpacing: 0.5 }}>
      SCAN TO PAIR LUNA
    </div>
  </div>
)

/**
 * 9. IMULevelCard
 * Real-time accelerometer & gyroscope tilt analyzer with spirit bubble crosshairs.
 */
const IMULevelCard = ({
  w = 220, h = 140,
  title = 'IMU SPIRIT LEVEL', pitch = '+2.4°', roll = '-0.8°', status = 'LEVEL',
  bgColor = '#121721', borderColor = '#232D3F',
  textColor = '#EAEFF5', subtextColor = '#8290A4',
  accentColor = '#10B981', radius = 12
}) => (
  <div style={{
    width: w, height: h, borderRadius: radius,
    background: bgColor, border: `1px solid ${borderColor}`,
    padding: '10px 12px', display: 'flex', flexDirection: 'column',
    justifyContent: 'space-between', boxSizing: 'border-box',
    boxShadow: '0 4px 16px rgba(0,0,0,0.25)', userSelect: 'none'
  }}>
    <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
      <span style={{ fontSize: 10, fontWeight: 800, letterSpacing: 0.8, color: textColor }}>{title}</span>
      <span style={{ fontSize: 8, fontWeight: 800, color: accentColor, background: `${accentColor}22`, padding: '2px 5px', borderRadius: 4 }}>
        {status}
      </span>
    </div>

    {/* Center Spirit Level Target Crosshair */}
    <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'center', position: 'relative', height: 60 }}>
      <div style={{ width: 56, height: 56, borderRadius: '50%', border: `1px dashed ${borderColor}`, position: 'relative', display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
        <div style={{ width: 28, height: 28, borderRadius: '50%', border: `1px solid ${borderColor}` }} />
        {/* Spirit Bubble */}
        <div style={{ width: 12, height: 12, borderRadius: '50%', background: accentColor, position: 'absolute', transform: 'translate(4px, -3px)', boxShadow: `0 0 8px ${accentColor}` }} />
      </div>
    </div>

    <div style={{ display: 'flex', justifyContent: 'space-around', borderTop: `1px solid ${borderColor}66`, paddingTop: 6 }}>
      <div style={{ textAlign: 'center' }}>
        <div style={{ fontSize: 8, color: subtextColor }}>PITCH</div>
        <div style={{ fontSize: 11, fontWeight: 800, color: textColor }}>{pitch}</div>
      </div>
      <div style={{ textAlign: 'center' }}>
        <div style={{ fontSize: 8, color: subtextColor }}>ROLL</div>
        <div style={{ fontSize: 11, fontWeight: 800, color: textColor }}>{roll}</div>
      </div>
    </div>
  </div>
)

/**
 * 10. SettingRow
 * Tactile, high-contrast system setting row meeting minimum 44px thumb reach requirements.
 */
const SettingRow = ({
  w = 220, h = 48,
  label = 'BRIGHTNESS', valueStr = '85%', sublabel = 'Auto-dim 30s', checked = true,
  bgColor = '#121721', borderColor = '#232D3F',
  textColor = '#EAEFF5', subtextColor = '#8290A4',
  accentColor = '#38BDF8', radius = 8
}) => (
  <div style={{
    width: w, height: h, borderRadius: radius,
    background: bgColor, border: `1px solid ${borderColor}`,
    padding: '8px 12px', display: 'flex', alignItems: 'center',
    justifyContent: 'space-between', boxSizing: 'border-box',
    boxShadow: '0 2px 8px rgba(0,0,0,0.2)', userSelect: 'none'
  }}>
    <div style={{ display: 'flex', flexDirection: 'column' }}>
      <div style={{ fontSize: 11, fontWeight: 800, letterSpacing: 0.5, color: textColor }}>{label}</div>
      {sublabel && <div style={{ fontSize: 9, color: subtextColor }}>{sublabel}</div>}
    </div>
    <div style={{ display: 'flex', alignItems: 'center', gap: 6 }}>
      {valueStr && (
        <span style={{ fontSize: 10, fontWeight: 700, color: accentColor, background: `${accentColor}22`, padding: '2px 6px', borderRadius: 4 }}>
          {valueStr}
        </span>
      )}
      <div style={{
        width: 32, height: 18, borderRadius: 9,
        background: checked ? accentColor : 'rgba(255,255,255,0.1)',
        position: 'relative', display: 'flex', alignItems: 'center', padding: 2, boxSizing: 'border-box'
      }}>
        <div style={{
          width: 14, height: 14, borderRadius: '50%', background: '#FFFFFF',
          transform: checked ? 'translateX(14px)' : 'translateX(0)', transition: 'transform 0.15s ease'
        }} />
      </div>
    </div>
  </div>
)

/**
 * 11. DeviceSpecCard
 * Hardware specs readout showing ESP32-S3 chip, flash, PSRAM and battery status.
 */
const DeviceSpecCard = ({
  w = 220, h = 160,
  deviceName = 'LUNA CORE 1.69"', soc = 'ESP32-S3 Dual 240MHz',
  memory = '16MB Flash • 8MB PSRAM', batteryInfo = 'VBAT 4.12V (94%)',
  status = 'CST816T OK • ST7789 80MHz',
  bgColor = '#121721', borderColor = '#232D3F',
  textColor = '#EAEFF5', subtextColor = '#8290A4',
  accentColor = '#10B981', radius = 12
}) => (
  <div style={{
    width: w, height: h, borderRadius: radius,
    background: bgColor, border: `1px solid ${borderColor}`,
    padding: '10px 12px', display: 'flex', flexDirection: 'column',
    justifyContent: 'space-between', boxSizing: 'border-box',
    boxShadow: '0 4px 16px rgba(0,0,0,0.25)', userSelect: 'none'
  }}>
    <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
      <span style={{ fontSize: 12, fontWeight: 900, color: textColor }}>{deviceName}</span>
      <span style={{ fontSize: 8, fontWeight: 800, color: accentColor, background: `${accentColor}22`, padding: '2px 5px', borderRadius: 4 }}>
        VERIFIED
      </span>
    </div>

    <div style={{ display: 'flex', flexDirection: 'column', gap: 5, padding: '4px 0' }}>
      <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: 9 }}>
        <span style={{ color: subtextColor }}>SoC</span>
        <span style={{ color: textColor, fontWeight: 600 }}>{soc}</span>
      </div>
      <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: 9 }}>
        <span style={{ color: subtextColor }}>Memory</span>
        <span style={{ color: textColor, fontWeight: 600 }}>{memory}</span>
      </div>
      <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: 9 }}>
        <span style={{ color: subtextColor }}>Power</span>
        <span style={{ color: textColor, fontWeight: 600 }}>{batteryInfo}</span>
      </div>
      <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: 9 }}>
        <span style={{ color: subtextColor }}>Buses</span>
        <span style={{ color: accentColor, fontWeight: 700 }}>{status}</span>
      </div>
    </div>

    <div style={{ fontSize: 8, color: subtextColor, textAlign: 'center', borderTop: `1px solid ${borderColor}66`, paddingTop: 5 }}>
      LUNA OS v2.5 • WAVESHARE ESP32-S3
    </div>
  </div>
)

// ============================================================
// 2. LEGACY COMPONENTS (PRESERVED FOR BACKWARD COMPATIBILITY)
// ============================================================
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

const ToggleSwitch = ({ w, h, checked, accentColor, bgColor, label }) => (
  <div style={{ width: w, height: h, display: 'flex', alignItems: 'center', gap: 10, cursor: 'pointer' }}>
    <div style={{
      width: 48, height: 26, borderRadius: 13, background: checked ? accentColor : bgColor,
      position: 'relative', transition: 'background 0.2s', flexShrink: 0,
    }}>
      <div style={{
        width: 20, height: 20, borderRadius: 10, background: '#fff',
        position: 'absolute', top: 3, left: checked ? 25 : 3,
        transition: 'left 0.2s', boxShadow: '0 2px 6px rgba(0,0,0,0.3)',
      }} />
    </div>
    {label && <span style={{ fontSize: 12, color: '#e2e8f0', fontWeight: 600 }}>{label}</span>}
  </div>
)

const spin = keyframes`from { transform: rotate(0deg); } to { transform: rotate(360deg); }`
const SpinnerLoader = ({ w, h, accentColor, bgColor, label }) => (
  <div style={{ width: w, height: h, display: 'flex', flexDirection: 'column', alignItems: 'center', justifyContent: 'center', gap: 8 }}>
    <div style={{
      width: Math.min(w, h) - 16, height: Math.min(w, h) - 16,
      borderRadius: '50%', border: `3px solid ${bgColor}`,
      borderTopColor: accentColor,
      animation: `${spin} 0.8s linear infinite`,
    }} />
    {label && <div style={{ fontSize: 10, color: '#94a3b8' }}>{label}</div>}
  </div>
)

const bounce = keyframes`0%, 80%, 100% { transform: scale(0); } 40% { transform: scale(1); }`
const DotLoader = ({ w, h, accentColor }) => (
  <div style={{ width: w, height: h, display: 'flex', alignItems: 'center', justifyContent: 'center', gap: 6 }}>
    {[0, 1, 2].map(i => (
      <div key={i} style={{
        width: 10, height: 10, borderRadius: 5, background: accentColor,
        animation: `${bounce} 1.4s ease-in-out infinite both`,
        animationDelay: `${i * 0.16}s`,
      }} />
    ))}
  </div>
)

const pulse = keyframes`0% { transform: scale(0.95); opacity: 0.8; } 50% { transform: scale(1.05); opacity: 0.4; } 100% { transform: scale(0.95); opacity: 0.8; }`
const PulseLoader = ({ w, h, accentColor, bgColor }) => (
  <div style={{ width: w, height: h, display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
    <div style={{
      width: Math.min(w, h), height: Math.min(w, h), borderRadius: '50%',
      background: accentColor, animation: `${pulse} 1.5s ease-in-out infinite`,
      boxShadow: `0 0 20px ${accentColor}88`,
    }} />
  </div>
)

const NeonCheckbox = ({ w, h, label, checked, accentColor, bgColor, textColor }) => (
  <div style={{ width: w, height: h, display: 'flex', alignItems: 'center', gap: 8, cursor: 'pointer' }}>
    <div style={{
      width: 18, height: 18, borderRadius: 4, background: bgColor,
      border: `2px solid ${checked ? accentColor : '#475569'}`,
      display: 'flex', alignItems: 'center', justifyContent: 'center',
      boxShadow: checked ? `0 0 8px ${accentColor}88` : 'none', flexShrink: 0,
    }}>
      {checked && <span style={{ color: accentColor, fontSize: 12, fontWeight: 900 }}>✓</span>}
    </div>
    {label && <span style={{ fontSize: 12, color: textColor }}>{label}</span>}
  </div>
)

const ProgressBar = ({ w, h, label, value, fillColor, bgColor, borderColor, textColor, radius }) => (
  <div style={{ width: w, height: h, display: 'flex', flexDirection: 'column', justifyContent: 'center', gap: 4 }}>
    <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: 10, color: textColor, fontWeight: 600 }}>
      <span>{label}</span>
      <span>{value}%</span>
    </div>
    <div style={{
      width: '100%', height: 8, borderRadius: radius, background: bgColor,
      border: `1px solid ${borderColor}`, overflow: 'hidden',
    }}>
      <div style={{ width: `${value}%`, height: '100%', background: fillColor, borderRadius: radius }} />
    </div>
  </div>
)

const DigitalClock = ({ w, h, timeStr, dateStr, color, dateColor }) => (
  <div style={{
    width: w, height: h, display: 'flex', flexDirection: 'column',
    alignItems: 'center', justifyContent: 'center', userSelect: 'none',
  }}>
    <div style={{ fontSize: 44, fontWeight: 900, color: color, letterSpacing: -2, lineHeight: 1 }}>{timeStr}</div>
    <div style={{ fontSize: 10, color: dateColor, fontWeight: 600, letterSpacing: 2, marginTop: 4 }}>{dateStr}</div>
  </div>
)

const CustomLabel = ({ w, h, text, color, fontSize, fontWeight, bgColor, radius }) => (
  <div style={{
    width: w, height: h, display: 'flex', alignItems: 'center', justifyContent: 'center',
    background: bgColor, borderRadius: radius, color: color,
    fontSize: fontSize || 14, fontWeight: fontWeight || 700, padding: '4px 8px',
  }}>{text}</div>
)

const AlertBadge = ({ w, h, title, message, bgColor, accentColor, textColor, radius }) => (
  <div style={{
    width: w, height: h, borderRadius: radius, background: bgColor,
    border: `1.5px solid ${accentColor}`, padding: '8px 12px',
    display: 'flex', alignItems: 'center', gap: 10,
    boxShadow: `0 0 12px ${accentColor}33`,
  }}>
    <div style={{ width: 8, height: 8, borderRadius: 4, background: accentColor, flexShrink: 0 }} />
    <div style={{ flex: 1 }}>
      <div style={{ fontSize: 11, fontWeight: 700, color: textColor }}>{title}</div>
      <div style={{ fontSize: 9, color: accentColor }}>{message}</div>
    </div>
  </div>
)

const TextInput = ({ w, h, placeholder, bgColor, borderColor, textColor, radius }) => (
  <div style={{
    width: w, height: h, borderRadius: radius, background: bgColor,
    border: `1.5px solid ${borderColor}`, padding: '0 12px',
    display: 'flex', alignItems: 'center',
  }}>
    <span style={{ fontSize: 11, color: textColor, opacity: 0.6 }}>{placeholder}</span>
  </div>
)

// ── Controlled Luna Component Primitives Adapters ──
const LunaButtonElem = (props) => (
  <div style={{ width: props.w || 220, height: props.h || 48 }}>
    <LunaButton {...props} />
  </div>
)

const LunaIconButtonElem = (props) => (
  <div style={{ width: props.w || 44, height: props.h || 44 }}>
    <LunaIconButton {...props} />
  </div>
)

const LunaSurfaceElem = (props) => (
  <div style={{ width: props.w || 220, height: props.h || 80 }}>
    <LunaSurface {...props}>
      <div style={{ color: '#EAEFF5', fontSize: 12, fontWeight: 700 }}>{props.title || 'Monolith Surface'}</div>
      {props.subtitle && <div style={{ color: '#8290A4', fontSize: 10, marginTop: 4 }}>{props.subtitle}</div>}
    </LunaSurface>
  </div>
)

const LunaListItemElem = (props) => (
  <div style={{ width: props.w || 220, minHeight: props.h || 48 }}>
    <LunaListItem {...props} />
  </div>
)

const LunaToggleElem = (props) => (
  <div style={{ width: props.w || 220, height: props.h || 48, display: 'flex', alignItems: 'center', justifyContent: 'space-between', padding: '0 12px', background: '#121721', borderRadius: 10, border: '1px solid #232D3F', boxSizing: 'border-box' }}>
    <span style={{ color: '#EAEFF5', fontSize: 12, fontWeight: 600 }}>{props.label || 'Toggle Option'}</span>
    <LunaToggle {...props} />
  </div>
)

const LunaSliderElem = (props) => (
  <div style={{ width: props.w || 220, minHeight: props.h || 64, background: '#121721', padding: '10px 14px', borderRadius: 10, border: '1px solid #232D3F', boxSizing: 'border-box' }}>
    <LunaSlider {...props} />
  </div>
)

const LunaProgressElem = (props) => (
  <div style={{ width: props.w || 220, minHeight: props.h || 48, background: '#121721', padding: '10px 14px', borderRadius: 10, border: '1px solid #232D3F', boxSizing: 'border-box' }}>
    <LunaProgress {...props} />
  </div>
)

const LunaIndicatorElem = (props) => (
  <div style={{ width: props.w || 100, height: props.h || 30, display: 'flex', alignItems: 'center' }}>
    <LunaIndicator {...props} />
  </div>
)

const LunaHeaderElem = (props) => (
  <div style={{ width: props.w || 240, height: props.h || 44 }}>
    <LunaHeader {...props} />
  </div>
)

const LunaNavigationElem = (props) => (
  <div style={{ width: props.w || 220, height: props.h || 36 }}>
    <LunaNavigation {...props} />
  </div>
)

const LunaTimerElem = (props) => (
  <div style={{ width: props.w || 220, height: props.h || 175 }}>
    <LunaTimer {...props} />
  </div>
)

const LunaNumberElem = (props) => (
  <div style={{ width: props.w || 100, height: props.h || 44 }}>
    <LunaNumber {...props} />
  </div>
)

const LunaStatusElem = (props) => (
  <div style={{ width: props.w || 90, height: props.h || 26 }}>
    <LunaStatus {...props} />
  </div>
)

// ============================================================
// 3. UI COMPONENTS REGISTRY
// ============================================================
export const UI_COMPONENTS = {
  // ── KINESIS / MONOLITH SYSTEM (PRIMARY) ──────────────────
  monolith_time: {
    name: 'Monolith Time Hero', category: 'glance',
    defaultProps: {
      x: 10, y: 15, w: 220, h: 90,
      timeStr: '10:42', secondsStr: ':38', dateStr: 'WED 09 SEP',
      batteryPct: 94, statusText: 'LUNA • READY',
      bgColor: '#121721', borderColor: '#232D3F',
      textColor: '#EAEFF5', accentColor: '#38BDF8', radius: 10
    },
    component: MonolithTime,
  },

  glance_bar: {
    name: 'Context Glance Bar', category: 'glance',
    defaultProps: {
      x: 10, y: 112, w: 220, h: 42,
      icon: '⚡', label: 'NEXT FOCUS', detail: 'Sprint Review @ 11:00',
      bgColor: '#121721', borderColor: '#232D3F',
      textColor: '#EAEFF5', accentColor: '#FF9E3B', radius: 8
    },
    component: GlanceBar,
  },

  tactile_button: {
    name: 'Tactile Action Block', category: 'actions',
    defaultProps: {
      x: 10, y: 210, w: 220, h: 50,
      label: 'START FOCUS SESSION', icon: '▶',
      bgColor: '#FF9E3B', borderColor: '#FFB266',
      textColor: '#080A0F', accentColor: '#080A0F', radius: 12
    },
    component: TactileButton,
  },

  notification_block: {
    name: 'Notification Monolith', category: 'streams',
    defaultProps: {
      x: 10, y: 20, w: 220, h: 72,
      sender: 'SARAH CONNOR', timeStr: '4m ago',
      preview: 'Firmware calibration complete. Sensor ready.',
      unread: true,
      bgColor: '#121721', borderColor: '#232D3F',
      textColor: '#EAEFF5', subtextColor: '#8290A4',
      accentColor: '#38BDF8', radius: 10
    },
    component: NotificationBlock,
  },

  agenda_block: {
    name: 'Agenda Glance Item', category: 'streams',
    defaultProps: {
      x: 10, y: 20, w: 220, h: 84,
      countdown: 'IN 24m', timeRange: '10:30 - 11:15',
      title: 'Architecture Sync', location: 'Lab 4 / BLE Orbit',
      bgColor: '#121721', borderColor: '#232D3F',
      textColor: '#EAEFF5', subtextColor: '#8290A4',
      accentColor: '#38BDF8', radius: 10
    },
    component: AgendaBlock,
  },

  game_launcher: {
    name: 'Game Hero Launcher', category: 'experiences',
    defaultProps: {
      x: 10, y: 20, w: 220, h: 170,
      title: 'RETRO RUNNER', genre: 'CYBERPLATFORM • 60FPS',
      highScore: '12,480 PTS', icon: '🏃', badgeText: 'READY TO PLAY',
      bgColor: '#121721', borderColor: '#232D3F',
      textColor: '#EAEFF5', accentColor: '#FF9E3B', radius: 12
    },
    component: GameLauncher,
  },

  focus_chamber: {
    name: 'Focus / Pomodoro Chamber', category: 'experiences',
    defaultProps: {
      x: 10, y: 15, w: 220, h: 175,
      timeRemaining: '24:50', modeLabel: 'DEEP WORK', sessionTag: 'SESSION 2 / 4',
      progress: 75,
      bgColor: '#121721', borderColor: '#232D3F',
      textColor: '#EAEFF5', subtextColor: '#8290A4',
      accentColor: '#FF9E3B', radius: 12
    },
    component: FocusChamber,
  },

  qr_utility_card: {
    name: 'Digital ID / QR Card', category: 'system',
    defaultProps: {
      x: 10, y: 20, w: 220, h: 160,
      title: 'LUNA ID CARD', subtitle: 'ESP32-S3 • BLE PEER', idTag: 'UID: LN-8842-X',
      bgColor: '#121721', borderColor: '#232D3F',
      textColor: '#EAEFF5', accentColor: '#38BDF8', radius: 12
    },
    component: QRUtilityCard,
  },

  imu_level_card: {
    name: 'IMU Spirit Level', category: 'system',
    defaultProps: {
      x: 10, y: 20, w: 220, h: 140,
      title: 'IMU SPIRIT LEVEL', pitch: '+2.4°', roll: '-0.8°', status: 'LEVEL',
      bgColor: '#121721', borderColor: '#232D3F',
      textColor: '#EAEFF5', subtextColor: '#8290A4',
      accentColor: '#10B981', radius: 12
    },
    component: IMULevelCard,
  },

  setting_row: {
    name: 'Tactile Setting Row', category: 'system',
    defaultProps: {
      x: 10, y: 20, w: 220, h: 48,
      label: 'BRIGHTNESS', valueStr: '85%', sublabel: 'Auto-dim 30s', checked: true,
      bgColor: '#121721', borderColor: '#232D3F',
      textColor: '#EAEFF5', subtextColor: '#8290A4',
      accentColor: '#38BDF8', radius: 8
    },
    component: SettingRow,
  },

  device_spec_card: {
    name: 'Device Spec Readout', category: 'system',
    defaultProps: {
      x: 10, y: 20, w: 220, h: 160,
      deviceName: 'LUNA CORE 1.69"', soc: 'ESP32-S3 Dual 240MHz',
      memory: '16MB Flash • 8MB PSRAM', batteryInfo: 'VBAT 4.12V (94%)',
      status: 'CST816T OK • ST7789 80MHz',
      bgColor: '#121721', borderColor: '#232D3F',
      textColor: '#EAEFF5', subtextColor: '#8290A4',
      accentColor: '#10B981', radius: 12
    },
    component: DeviceSpecCard,
  },

  // ── LUNA COMPONENT PRIMITIVES ────────────────────────────
  luna_button: {
    name: 'Luna Tactile Button', category: 'actions',
    defaultProps: {
      x: 10, y: 20, w: 220, h: 48,
      label: 'CONFIRM ACTION', variant: 'primary', state: 'normal'
    },
    component: LunaButtonElem,
  },

  luna_icon_button: {
    name: 'Luna Icon Button', category: 'actions',
    defaultProps: {
      x: 10, y: 20, w: 44, h: 44,
      icon: '⚙', variant: 'secondary', state: 'normal', ariaLabel: 'Settings'
    },
    component: LunaIconButtonElem,
  },

  luna_surface: {
    name: 'Luna Monolith Surface', category: 'glance',
    defaultProps: {
      x: 10, y: 20, w: 220, h: 80,
      title: 'Monolith Surface', subtitle: 'Elevated spatial block', elevation: 'elevated', radius: 10
    },
    component: LunaSurfaceElem,
  },

  luna_list_item: {
    name: 'Luna List Item', category: 'streams',
    defaultProps: {
      x: 10, y: 20, w: 220, h: 52,
      title: 'List Row Item', subtitle: 'Secondary context line', unread: true
    },
    component: LunaListItemElem,
  },

  luna_toggle: {
    name: 'Luna Binary Toggle', category: 'actions',
    defaultProps: {
      x: 10, y: 20, w: 220, h: 48,
      label: 'TOUCH VIBRATION', checked: true, activeColor: '#38BDF8', inactiveColor: '#232D3F'
    },
    component: LunaToggleElem,
  },

  luna_slider: {
    name: 'Luna Touch Slider', category: 'actions',
    defaultProps: {
      x: 10, y: 20, w: 220, h: 64,
      value: 75, min: 0, max: 100, step: 5, label: 'BACKLIGHT INTENSITY', unit: '%', accentColor: '#FF9E3B'
    },
    component: LunaSliderElem,
  },

  luna_progress: {
    name: 'Luna Progress Gauge', category: 'glance',
    defaultProps: {
      x: 10, y: 20, w: 220, h: 48,
      type: 'linear', value: 68, strokeWidth: 6, accentColor: '#38BDF8', trackColor: '#1A2232'
    },
    component: LunaProgressElem,
  },

  luna_indicator: {
    name: 'Luna Status Indicator', category: 'glance',
    defaultProps: {
      x: 10, y: 20, w: 100, h: 30,
      variant: 'pill', status: 'online', label: 'ONLINE'
    },
    component: LunaIndicatorElem,
  },

  luna_header: {
    name: 'Luna Screen Header', category: 'glance',
    defaultProps: {
      x: 0, y: 0, w: 240, h: 44,
      title: 'SUB-SYSTEM'
    },
    component: LunaHeaderElem,
  },

  luna_navigation: {
    name: 'Luna Carousel Dots', category: 'glance',
    defaultProps: {
      x: 10, y: 20, w: 220, h: 36,
      variant: 'dots', total: 8, activeIndex: 0
    },
    component: LunaNavigationElem,
  },

  luna_timer: {
    name: 'Luna Ambient Timer', category: 'experiences',
    defaultProps: {
      x: 10, y: 15, w: 220, h: 175,
      timeStr: '25:00', label: 'FOCUS', sessionTag: 'SESSION 1', progress: 50, isRunning: true, accentColor: '#FF9E3B'
    },
    component: LunaTimerElem,
  },

  luna_number: {
    name: 'Luna Tabular Number', category: 'system',
    defaultProps: {
      x: 10, y: 20, w: 100, h: 44,
      value: '240', unit: 'MHz', label: 'CORE CLK', accentColor: '#38BDF8', size: 'md'
    },
    component: LunaNumberElem,
  },

  luna_status: {
    name: 'Luna Status Badge', category: 'system',
    defaultProps: {
      x: 10, y: 20, w: 90, h: 26,
      status: 'online', label: 'ONLINE'
    },
    component: LunaStatusElem,
  },

  // ── LEGACY RETRO & UIVERSE ELEMENTS (MAINTAINED FOR COMPAT) ──
  card_glass: {
    name: 'Glass Card', category: 'legacy',
    defaultProps: { x: 20, y: 20, w: 200, h: 80, bgColor: 'rgba(255,255,255,0.07)', borderColor: 'rgba(255,255,255,0.18)', textColor: '#ffffff', subtextColor: '#94a3b8', title: 'Glass Card', subtitle: 'Frosted blur effect', radius: 14 },
    component: GlassCard,
  },
  card_stat: {
    name: 'Stat Card', category: 'legacy',
    defaultProps: { x: 20, y: 20, w: 200, h: 75, bgColor: '#1e293b', borderColor: '#334155', accentColor: '#38bdf8', textColor: '#ffffff', label: 'Heart Rate', value: '128', unit: 'bpm', radius: 10 },
    component: StatCard,
  },
  card_transaction: {
    name: 'Transaction Card', category: 'legacy',
    defaultProps: { x: 20, y: 20, w: 200, h: 60, bgColor: '#1e293b', borderColor: '#334155', iconBg: '#059669', textColor: '#ffffff', title: 'Salary Deposit', radius: 12 },
    component: TransactionCard,
  },
  card_retro: {
    name: 'Retro Arcade Box', category: 'legacy',
    defaultProps: { x: 20, y: 20, w: 200, h: 70, bgColor: '#000000', borderColor: '#00ff66', textColor: '#ffffff', title: 'STAGE 01', score: '04200' },
    component: RetroCard,
  },
  button_neon: {
    name: 'Neon Glow Button', category: 'legacy',
    defaultProps: { x: 30, y: 30, w: 180, h: 44, bgColor: '#060d1f', borderColor: '#00f2fe', textColor: '#00f2fe', label: 'CONNECT', radius: 8 },
    component: NeonButton,
  },
  button_retro: {
    name: 'Retro Arcade Button', category: 'legacy',
    defaultProps: { x: 30, y: 30, w: 180, h: 40, bgColor: '#111827', borderColor: '#f59e0b', textColor: '#f59e0b', label: 'START' },
    component: RetroButton,
  },
  button_download: {
    name: 'Download Pill Button', category: 'legacy',
    defaultProps: { x: 30, y: 30, w: 180, h: 42, bgColor: '#2563eb', textColor: '#ffffff', label: 'Export Data', radius: 18 },
    component: DownloadButton,
  },
  uiv_btn_damith_yellow: {
    name: 'Damith Yellow Button', category: 'legacy',
    defaultProps: { x: 30, y: 30, w: 180, h: 46, label: 'SUBMIT', bgColor: '#facc15', borderColor: '#eab308', textColor: '#000000' },
    component: DamithYellowBtn,
  },
  uiv_btn_happy_coding: {
    name: 'Happy Coding Button', category: 'legacy',
    defaultProps: { x: 30, y: 30, w: 180, h: 46, label: 'Happy Coding!', bgColor: '#0f172a', borderColor: '#38bdf8', textColor: '#38bdf8' },
    component: HappyCodingBtn,
  },
  toggle_switch: {
    name: 'Toggle Switch', category: 'legacy',
    defaultProps: { x: 20, y: 20, w: 160, h: 36, checked: true, accentColor: '#2563eb', bgColor: '#334155', label: 'Bluetooth' },
    component: ToggleSwitch,
  },
  toggle_neon: {
    name: 'Neon Glow Switch', category: 'legacy',
    defaultProps: { x: 20, y: 20, w: 160, h: 36, checked: true, accentColor: '#00f2fe', bgColor: '#1e293b', label: 'Display Active' },
    component: ToggleSwitch,
  },
  loader_spinner: {
    name: 'Ring Spinner Loader', category: 'legacy',
    defaultProps: { x: 80, y: 100, w: 80, h: 80, accentColor: '#00f2fe', bgColor: 'rgba(255,255,255,0.1)', label: 'Syncing...' },
    component: SpinnerLoader,
  },
  loader_dots: {
    name: 'Dot Loader', category: 'legacy',
    defaultProps: { x: 70, y: 120, w: 100, h: 40, accentColor: '#7c3aed' },
    component: DotLoader,
  },
  loader_pulse: {
    name: 'Pulse Loader', category: 'legacy',
    defaultProps: { x: 90, y: 110, w: 60, h: 60, accentColor: '#ec4899', bgColor: '#1e293b' },
    component: PulseLoader,
  },
  checkbox_neon: {
    name: 'Neon Checkbox', category: 'legacy',
    defaultProps: { x: 20, y: 130, w: 160, h: 28, label: 'Enable feature', checked: true, accentColor: '#10b981', bgColor: '#0f172a', textColor: '#e2e8f0' },
    component: NeonCheckbox,
  },
  gauge_progress: {
    name: 'Progress Bar', category: 'legacy',
    defaultProps: { x: 16, y: 120, w: 208, h: 40, label: 'Battery', value: 72, fillColor: '#10b981', bgColor: '#1e293b', borderColor: '#334155', textColor: '#ffffff', radius: 6 },
    component: ProgressBar,
  },
  digital_clock: {
    name: 'Digital Clock', category: 'legacy',
    defaultProps: { x: 20, y: 30, w: 200, h: 70, timeStr: '10:45', dateStr: 'WED, SEP 9', color: '#00f2fe', dateColor: '#94a3b8' },
    component: DigitalClock,
  },
  custom_label: {
    name: 'Custom Label / Header', category: 'legacy',
    defaultProps: { x: 16, y: 20, w: 208, h: 44, text: 'LUNA SMART WATCH', color: '#ffffff', fontSize: 14, fontWeight: 800, bgColor: '#1e293b', radius: 8 },
    component: CustomLabel,
  },
  alert_badge: {
    name: 'Alert Badge', category: 'legacy',
    defaultProps: { x: 16, y: 20, w: 208, h: 44, title: 'System Alert', message: 'Update available', bgColor: '#0f172a', accentColor: '#f59e0b', textColor: '#ffffff', radius: 10 },
    component: AlertBadge,
  },
  text_input: {
    name: 'Text Input Field', category: 'legacy',
    defaultProps: { x: 16, y: 100, w: 208, h: 36, placeholder: 'Enter value...', bgColor: '#1e293b', borderColor: '#334155', textColor: '#e2e8f0', radius: 8 },
    component: TextInput,
  },
}

export const CATEGORIES = [
  { id: 'all', label: 'All' },
  { id: 'primitives', label: '💎 Luna Primitives' },
  { id: 'glance', label: '✨ Glance & Time' },
  { id: 'actions', label: '⚡ Actions' },
  { id: 'streams', label: '📜 Streams' },
  { id: 'experiences', label: '🎮 Experiences' },
  { id: 'system', label: '🛠 System & Specs' },
  { id: 'patterns', label: 'Patterns' },
  { id: 'legacy', label: 'Legacy' },
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
