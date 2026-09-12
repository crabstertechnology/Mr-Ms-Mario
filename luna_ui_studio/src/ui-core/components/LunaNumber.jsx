import React from 'react'

/**
 * LunaNumber — Monospaced Tabular Metric Readout
 * Zero layout shift across dynamic telemetry updates.
 *
 * @param {Object} props
 * @param {number | string} props.value
 * @param {string} [props.unit]
 * @param {string} [props.label]
 * @param {string} [props.accentColor='#EAEFF5']
 * @param {'sm' | 'md' | 'lg' | 'hero'} [props.size='md']
 * @param {Object} [props.style]
 */
export function LunaNumber({
  value,
  unit,
  label,
  accentColor = '#EAEFF5',
  size = 'md',
  style = {}
}) {
  const getFontSize = () => {
    switch (size) {
      case 'sm': return { num: '14px', unit: '10px' }
      case 'lg': return { num: '24px', unit: '12px' }
      case 'hero': return { num: '34px', unit: '14px' }
      case 'md':
      default: return { num: '18px', unit: '11px' }
    }
  }

  const dims = getFontSize()

  return (
    <div
      style={{
        display: 'inline-flex',
        flexDirection: 'column',
        fontFamily: 'Inter, monospace',
        userSelect: 'none',
        ...style
      }}
    >
      {label && (
        <span style={{ fontSize: '10px', fontWeight: '600', color: '#8290A4', textTransform: 'uppercase', marginBottom: '2px' }}>
          {label}
        </span>
      )}
      <div style={{ display: 'flex', alignItems: 'baseline', gap: '4px' }}>
        <span
          style={{
            fontSize: dims.num,
            fontWeight: '800',
            color: accentColor,
            fontVariantNumeric: 'tabular-nums',
            letterSpacing: '-0.02em'
          }}
        >
          {value}
        </span>
        {unit && (
          <span style={{ fontSize: dims.unit, fontWeight: '600', color: '#8290A4' }}>
            {unit}
          </span>
        )}
      </div>
    </div>
  )
}
