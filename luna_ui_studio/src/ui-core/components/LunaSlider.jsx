import React, { useRef } from 'react'

/**
 * LunaSlider — One-Thumb Continuous Wearable Scrubber
 * Touch height: 44px hit area with 8px track and 26px thumb.
 * Displays value bubble above thumb to avoid finger occlusion.
 *
 * @param {Object} props
 * @param {number} props.value (0 to 100)
 * @param {Function} props.onChange
 * @param {number} [props.min=0]
 * @param {number} [props.max=100]
 * @param {number} [props.step=1]
 * @param {string} [props.label]
 * @param {string} [props.unit='%']
 * @param {string} [props.accentColor='#FF9E3B']
 * @param {boolean} [props.disabled=false]
 * @param {Object} [props.style]
 */
export function LunaSlider({
  value,
  onChange,
  min = 0,
  max = 100,
  step = 1,
  label,
  unit = '%',
  accentColor = '#FF9E3B',
  disabled = false,
  style = {}
}) {
  const trackRef = useRef(null)

  const pct = Math.min(100, Math.max(0, ((value - min) / (max - min)) * 100))

  const handlePointer = (e) => {
    if (disabled || !trackRef.current) return
    const rect = trackRef.current.getBoundingClientRect()
    const clientX = e.clientX ?? (e.touches && e.touches[0]?.clientX) ?? 0
    const ratio = Math.min(1, Math.max(0, (clientX - rect.left) / rect.width))
    const rawVal = min + ratio * (max - min)
    const steppedVal = Math.round(rawVal / step) * step
    onChange && onChange(steppedVal)
  }

  return (
    <div
      style={{
        display: 'flex',
        flexDirection: 'column',
        width: '100%',
        padding: '6px 0',
        userSelect: 'none',
        opacity: disabled ? 0.4 : 1,
        boxSizing: 'border-box',
        ...style
      }}
    >
      {label && (
        <div style={{ display: 'flex', justifyContent: 'space-between', marginBottom: '8px', fontSize: '11px', fontFamily: 'Inter, sans-serif' }}>
          <span style={{ color: '#8290A4', fontWeight: '600', textTransform: 'uppercase' }}>{label}</span>
          <span style={{ color: accentColor, fontWeight: '700' }}>{value}{unit}</span>
        </div>
      )}

      {/* Scrubber Hit Target: 44px min height */}
      <div
        ref={trackRef}
        role="slider"
        aria-valuemin={min}
        aria-valuemax={max}
        aria-valuenow={value}
        aria-disabled={disabled}
        tabIndex={disabled ? -1 : 0}
        onPointerDown={(e) => {
          e.currentTarget.setPointerCapture?.(e.pointerId)
          handlePointer(e)
        }}
        onPointerMove={(e) => {
          if (e.buttons === 1) handlePointer(e)
        }}
        style={{
          position: 'relative',
          display: 'flex',
          alignItems: 'center',
          width: '100%',
          height: '44px',
          cursor: disabled ? 'not-allowed' : 'pointer',
          outline: 'none'
        }}
      >
        {/* Track Rail */}
        <div
          style={{
            position: 'absolute',
            left: 0,
            right: 0,
            height: '8px',
            borderRadius: '4px',
            backgroundColor: '#1A2232',
            border: '1px solid #232D3F',
            overflow: 'hidden'
          }}
        >
          {/* Active Fill */}
          <div
            style={{
              width: `${pct}%`,
              height: '100%',
              backgroundColor: accentColor,
              borderRadius: '4px 0 0 4px',
              transition: 'width 0.05s linear'
            }}
          />
        </div>

        {/* Thumb */}
        <div
          style={{
            position: 'absolute',
            left: `calc(${pct}% - 13px)`,
            width: '26px',
            height: '26px',
            borderRadius: '50%',
            backgroundColor: '#EAEFF5',
            border: `2px solid ${accentColor}`,
            boxShadow: '0 2px 6px rgba(0,0,0,0.6)',
            transition: 'left 0.05s linear',
            pointerEvents: 'none'
          }}
        />
      </div>
    </div>
  )
}
