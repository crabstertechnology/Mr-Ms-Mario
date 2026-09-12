import React from 'react'

/**
 * LunaSurface — Elevated Spatial Monolith Container
 * Foundation container for all wearable information zones.
 *
 * @param {Object} props
 * @param {'base' | 'elevated' | 'sunken'} [props.elevation='elevated']
 * @param {string} [props.accentRail] Optional left border accent color (e.g. '#38BDF8', '#FF9E3B', '#10B981')
 * @param {boolean} [props.isSelected=false]
 * @param {number} [props.radius=10]
 * @param {React.ReactNode} props.children
 * @param {Function} [props.onClick]
 * @param {Object} [props.style]
 */
export function LunaSurface({
  elevation = 'elevated',
  accentRail,
  isSelected = false,
  radius = 10,
  children,
  onClick,
  style = {}
}) {
  const getBg = () => {
    switch (elevation) {
      case 'base': return '#080A0F'
      case 'sunken': return '#0C0F17'
      case 'elevated':
      default: return '#121721'
    }
  }

  return (
    <div
      onClick={onClick}
      role={onClick ? 'button' : undefined}
      tabIndex={onClick ? 0 : undefined}
      style={{
        position: 'relative',
        width: '100%',
        backgroundColor: getBg(),
        borderRadius: `${radius}px`,
        border: `1px solid ${isSelected ? '#38BDF8' : '#232D3F'}`,
        boxSizing: 'border-box',
        overflow: 'hidden',
        cursor: onClick ? 'pointer' : 'default',
        transition: 'border-color 0.15s ease, background-color 0.15s ease',
        ...style
      }}
    >
      {accentRail && (
        <div
          style={{
            position: 'absolute',
            left: 0,
            top: 0,
            bottom: 0,
            width: '4px',
            backgroundColor: accentRail
          }}
        />
      )}
      <div style={{ padding: accentRail ? '12px 14px 12px 18px' : '12px 14px' }}>
        {children}
      </div>
    </div>
  )
}
