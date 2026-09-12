import React from 'react'
import { LunaProgress } from './LunaProgress.jsx'

/**
 * LunaTimer — Ambient Monolith Focus & Countdown Timer
 *
 * @param {Object} props
 * @param {string} props.timeStr e.g. "24:50"
 * @param {string} [props.label='FOCUS']
 * @param {string} [props.sessionTag='SESSION 1 / 4']
 * @param {number} [props.progress=50] 0 to 100
 * @param {string} [props.accentColor='#FF9E3B']
 * @param {boolean} [props.isRunning=true]
 * @param {Object} [props.style]
 */
export function LunaTimer({
  timeStr = '25:00',
  label = 'FOCUS',
  sessionTag = 'SESSION 1',
  progress = 50,
  accentColor = '#FF9E3B',
  isRunning = true,
  style = {}
}) {
  return (
    <div
      style={{
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        justifyContent: 'center',
        width: '100%',
        padding: '16px 12px',
        backgroundColor: '#121721',
        borderRadius: '12px',
        border: '1px solid #232D3F',
        boxSizing: 'border-box',
        userSelect: 'none',
        ...style
      }}
    >
      <div style={{ display: 'flex', justifyContent: 'space-between', width: '100%', marginBottom: '12px' }}>
        <span style={{ fontSize: '10px', fontWeight: '700', color: accentColor, textTransform: 'uppercase', letterSpacing: '0.06em' }}>
          {label}
        </span>
        <span style={{ fontSize: '10px', fontWeight: '600', color: '#8290A4' }}>
          {sessionTag}
        </span>
      </div>

      {/* Ring Progress with Center Time */}
      <LunaProgress
        type="ring"
        value={progress}
        size={110}
        strokeWidth={8}
        accentColor={accentColor}
        trackColor="#1A2232"
        showLabel={false}
        style={{ margin: '8px 0' }}
      />

      <div style={{ position: 'relative', marginTop: '-76px', marginBottom: '40px', textAlign: 'center' }}>
        <span
          style={{
            fontSize: '28px',
            fontWeight: '800',
            fontFamily: 'Inter, monospace',
            letterSpacing: '-0.02em',
            color: '#EAEFF5'
          }}
        >
          {timeStr}
        </span>
      </div>

      <div style={{ display: 'flex', alignItems: 'center', gap: '6px' }}>
        <span style={{ width: '6px', height: '6px', borderRadius: '50%', backgroundColor: isRunning ? '#10B981' : '#FF9E3B' }} />
        <span style={{ fontSize: '10px', fontWeight: '700', color: isRunning ? '#10B981' : '#FF9E3B', textTransform: 'uppercase' }}>
          {isRunning ? 'ACTIVE' : 'PAUSED'}
        </span>
      </div>
    </div>
  )
}
