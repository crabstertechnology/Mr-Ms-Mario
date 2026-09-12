import React, { useState } from 'react'

/**
 * LunaListItem — High-Touch Monolith Row
 * Minimum touch height: 48px.
 *
 * @param {Object} props
 * @param {string} props.title
 * @param {string} [props.subtitle]
 * @param {React.ReactNode} [props.leading]
 * @param {React.ReactNode} [props.trailing]
 * @param {string} [props.accentColor]
 * @param {boolean} [props.unread=false]
 * @param {Function} [props.onClick]
 * @param {Object} [props.style]
 */
export function LunaListItem({
  title,
  subtitle,
  leading,
  trailing,
  accentColor,
  unread = false,
  onClick,
  style = {}
}) {
  const [isPressing, setIsPressing] = useState(false)

  return (
    <div
      role="listitem"
      tabIndex={onClick ? 0 : undefined}
      onPointerDown={() => onClick && setIsPressing(true)}
      onPointerUp={() => setIsPressing(false)}
      onPointerLeave={() => setIsPressing(false)}
      onClick={onClick}
      style={{
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'space-between',
        width: '100%',
        minHeight: '48px',
        padding: '10px 12px',
        backgroundColor: isPressing ? '#1A2232' : '#121721',
        borderRadius: '8px',
        border: `1px solid ${unread ? '#38BDF8' : '#232D3F'}`,
        boxSizing: 'border-box',
        cursor: onClick ? 'pointer' : 'default',
        userSelect: 'none',
        transition: 'background-color 0.12s ease',
        ...style
      }}
    >
      <div style={{ display: 'flex', alignItems: 'center', gap: '10px', overflow: 'hidden' }}>
        {unread && (
          <div style={{ width: '4px', height: '24px', borderRadius: '2px', backgroundColor: '#38BDF8', flexShrink: 0 }} />
        )}
        {leading && <div style={{ flexShrink: 0, display: 'flex', alignItems: 'center' }}>{leading}</div>}
        <div style={{ display: 'flex', flexDirection: 'column', overflow: 'hidden' }}>
          <span style={{ color: '#EAEFF5', fontSize: '13px', fontWeight: '600', fontFamily: 'Inter, sans-serif', textOverflow: 'ellipsis', whiteSpace: 'nowrap', overflow: 'hidden' }}>
            {title}
          </span>
          {subtitle && (
            <span style={{ color: '#8290A4', fontSize: '10px', fontFamily: 'Inter, sans-serif', marginTop: '2px', textOverflow: 'ellipsis', whiteSpace: 'nowrap', overflow: 'hidden' }}>
              {subtitle}
            </span>
          )}
        </div>
      </div>
      <div style={{ flexShrink: 0, marginLeft: '8px', display: 'flex', alignItems: 'center' }}>
        {trailing || (
          onClick && <span style={{ color: '#8290A4', fontSize: '16px', fontWeight: 'bold' }}>›</span>
        )}
      </div>
    </div>
  )
}
