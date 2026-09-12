import React from 'react'

/**
 * LunaStatus — Wearable Status Badge
 *
 * @param {Object} props
 * @param {'online' | 'syncing' | 'paused' | 'error' | 'standby'} [props.status='online']
 * @param {string} [props.label]
 * @param {Object} [props.style]
 */
export function LunaStatus({
  status = 'online',
  label,
  style = {}
}) {
  const getMeta = () => {
    switch (status) {
      case 'syncing':
        return { color: '#38BDF8', bg: '#0C1B2A', border: '#1E3A5F', defaultLabel: 'SYNCING' }
      case 'paused':
        return { color: '#FF9E3B', bg: '#23180C', border: '#4D3212', defaultLabel: 'PAUSED' }
      case 'error':
        return { color: '#EF4444', bg: '#290E11', border: '#5C1D24', defaultLabel: 'ERROR' }
      case 'standby':
        return { color: '#8290A4', bg: '#121721', border: '#232D3F', defaultLabel: 'STANDBY' }
      case 'online':
      default:
        return { color: '#10B981', bg: '#0D241C', border: '#1C523F', defaultLabel: 'ONLINE' }
    }
  }

  const meta = getMeta()

  return (
    <span
      style={{
        display: 'inline-flex',
        alignItems: 'center',
        gap: '6px',
        padding: '3px 8px',
        borderRadius: '6px',
        backgroundColor: meta.bg,
        border: `1px solid ${meta.border}`,
        color: meta.color,
        fontSize: '10px',
        fontWeight: '700',
        fontFamily: 'Inter, sans-serif',
        textTransform: 'uppercase',
        letterSpacing: '0.04em',
        userSelect: 'none',
        ...style
      }}
    >
      <span style={{ width: '5px', height: '5px', borderRadius: '50%', backgroundColor: meta.color }} />
      <span>{label || meta.defaultLabel}</span>
    </span>
  )
}
