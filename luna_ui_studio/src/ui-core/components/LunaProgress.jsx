import React from 'react'

/**
 * LunaProgress — Deterministic Linear & Ring Progress Indicator
 * Solid fills engineered for 1:1 parity with ESP32-S3 TFT_eSPI routines.
 *
 * @param {Object} props
 * @param {'linear' | 'ring'} [props.type='linear']
 * @param {number} props.value (0 to 100)
 * @param {number} [props.size=60] Diameter for ring
 * @param {number} [props.strokeWidth=6]
 * @param {string} [props.accentColor='#38BDF8']
 * @param {string} [props.trackColor='#1A2232']
 * @param {boolean} [props.showLabel=true]
 * @param {Object} [props.style]
 */
export function LunaProgress({
  type = 'linear',
  value = 0,
  size = 64,
  strokeWidth = 6,
  accentColor = '#38BDF8',
  trackColor = '#1A2232',
  showLabel = true,
  style = {}
}) {
  const pct = Math.min(100, Math.max(0, value))

  if (type === 'ring') {
    const radius = (size - strokeWidth) / 2
    const circumference = 2 * Math.PI * radius
    const offset = circumference - (pct / 100) * circumference

    return (
      <div
        role="progressbar"
        aria-valuenow={pct}
        aria-valuemin={0}
        aria-valuemax={100}
        style={{
          position: 'relative',
          display: 'inline-flex',
          alignItems: 'center',
          justifyContent: 'center',
          width: `${size}px`,
          height: `${size}px`,
          boxSizing: 'border-box',
          ...style
        }}
      >
        <svg width={size} height={size} style={{ transform: 'rotate(-90deg)' }}>
          {/* Track Circle */}
          <circle
            cx={size / 2}
            cy={size / 2}
            r={radius}
            stroke={trackColor}
            strokeWidth={strokeWidth}
            fill="none"
          />
          {/* Active Fill Circle */}
          <circle
            cx={size / 2}
            cy={size / 2}
            r={radius}
            stroke={accentColor}
            strokeWidth={strokeWidth}
            strokeDasharray={circumference}
            strokeDashoffset={offset}
            strokeLinecap="round"
            fill="none"
            style={{ transition: 'stroke-dashoffset 0.2s ease' }}
          />
        </svg>
        {showLabel && (
          <span
            style={{
              position: 'absolute',
              color: '#EAEFF5',
              fontSize: '12px',
              fontWeight: '700',
              fontFamily: 'Inter, sans-serif'
            }}
          >
            {Math.round(pct)}%
          </span>
        )}
      </div>
    )
  }

  // Linear progress
  return (
    <div
      role="progressbar"
      aria-valuenow={pct}
      aria-valuemin={0}
      aria-valuemax={100}
      style={{
        display: 'flex',
        flexDirection: 'column',
        width: '100%',
        gap: '4px',
        boxSizing: 'border-box',
        ...style
      }}
    >
      <div
        style={{
          position: 'relative',
          width: '100%',
          height: `${strokeWidth}px`,
          backgroundColor: trackColor,
          borderRadius: `${strokeWidth / 2}px`,
          border: '1px solid #232D3F',
          overflow: 'hidden'
        }}
      >
        <div
          style={{
            width: `${pct}%`,
            height: '100%',
            backgroundColor: accentColor,
            borderRadius: `${strokeWidth / 2}px`,
            transition: 'width 0.2s ease'
          }}
        />
      </div>
      {showLabel && (
        <div style={{ display: 'flex', justifyContent: 'flex-end', fontSize: '10px', color: '#8290A4', fontFamily: 'Inter, sans-serif' }}>
          <span>{Math.round(pct)}%</span>
        </div>
      )}
    </div>
  )
}
