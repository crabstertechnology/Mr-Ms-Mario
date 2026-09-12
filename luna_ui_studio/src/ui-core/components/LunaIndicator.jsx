import React from 'react'

/**
 * LunaIndicator — Glance Status Dot & Pulse Beacon
 *
 * @param {Object} props
 * @param {'dot' | 'beacon' | 'pill'} [props.variant='dot']
 * @param {'online' | 'warning' | 'error' | 'syncing' | 'offline'} [props.status='online']
 * @param {string} [props.label]
 * @param {Object} [props.style]
 */
export function LunaIndicator({
  variant = 'dot',
  status = 'online',
  label,
  style = {}
}) {
  const getStatusColor = () => {
    switch (status) {
      case 'online': return '#10B981' // emerald
      case 'warning': return '#FF9E3B' // amber
      case 'error': return '#EF4444' // red
      case 'syncing': return '#38BDF8' // cyan
      case 'offline':
      default: return '#6B7280' // gray
    }
  }

  const col = getStatusColor()

  if (variant === 'pill') {
    return (
      <span
        style={{
          display: 'inline-flex',
          alignItems: 'center',
          gap: '6px',
          padding: '2px 8px',
          backgroundColor: '#121721',
          border: `1px solid ${col}`,
          borderRadius: '12px',
          fontSize: '10px',
          fontWeight: '700',
          color: col,
          fontFamily: 'Inter, sans-serif',
          textTransform: 'uppercase',
          letterSpacing: '0.04em',
          ...style
        }}
      >
        <span style={{ width: '6px', height: '6px', borderRadius: '50%', backgroundColor: col }} />
        {label || status}
      </span>
    )
  }

  return (
    <span
      style={{
        position: 'relative',
        display: 'inline-flex',
        alignItems: 'center',
        justifyContent: 'center',
        width: '10px',
        height: '10px',
        ...style
      }}
    >
      {variant === 'beacon' && (
        <span
          style={{
            position: 'absolute',
            width: '100%',
            height: '100%',
            borderRadius: '50%',
            backgroundColor: col,
            opacity: 0.5,
            animation: 'ping 1.5s cubic-bezier(0, 0, 0.2, 1) infinite'
          }}
        />
      )}
      <span
        style={{
          position: 'relative',
          width: '7px',
          height: '7px',
          borderRadius: '50%',
          backgroundColor: col
        }}
      />
    </span>
  )
}
