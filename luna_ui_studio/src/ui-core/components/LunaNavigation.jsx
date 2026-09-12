import React from 'react'

/**
 * LunaNavigation — Carousel Dots & Segmented Rail
 *
 * @param {Object} props
 * @param {'dots' | 'segmented'} [props.variant='dots']
 * @param {number} props.total
 * @param {number} props.activeIndex
 * @param {Array<string>} [props.labels]
 * @param {Function} [props.onSelect]
 * @param {Object} [props.style]
 */
export function LunaNavigation({
  variant = 'dots',
  total = 8,
  activeIndex = 0,
  labels = [],
  onSelect,
  style = {}
}) {
  if (variant === 'segmented') {
    return (
      <div
        role="tablist"
        style={{
          display: 'flex',
          width: '100%',
          height: '42px',
          backgroundColor: '#121721',
          borderRadius: '10px',
          border: '1px solid #232D3F',
          padding: '3px',
          boxSizing: 'border-box',
          gap: '4px',
          ...style
        }}
      >
        {labels.map((label, idx) => {
          const isActive = idx === activeIndex
          return (
            <button
              key={idx}
              role="tab"
              aria-selected={isActive}
              type="button"
              onClick={() => onSelect && onSelect(idx)}
              style={{
                flex: 1,
                display: 'flex',
                alignItems: 'center',
                justifyContent: 'center',
                height: '100%',
                borderRadius: '7px',
                border: 'none',
                backgroundColor: isActive ? '#38BDF8' : 'transparent',
                color: isActive ? '#080A0F' : '#8290A4',
                fontSize: '11px',
                fontWeight: '700',
                fontFamily: 'Inter, sans-serif',
                textTransform: 'uppercase',
                cursor: 'pointer',
                transition: 'background-color 0.12s ease, color 0.12s ease',
                userSelect: 'none',
                outline: 'none'
              }}
            >
              {label}
            </button>
          )
        })}
      </div>
    )
  }

  // Dots variant: Horizontal dot array
  return (
    <div
      role="navigation"
      aria-label="Carousel pagination"
      style={{
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        gap: '6px',
        padding: '6px 0',
        width: '100%',
        userSelect: 'none',
        ...style
      }}
    >
      {Array.from({ length: total }).map((_, idx) => {
        const isActive = idx === activeIndex
        return (
          <div
            key={idx}
            onClick={() => onSelect && onSelect(idx)}
            style={{
              width: isActive ? '16px' : '6px',
              height: '6px',
              borderRadius: '3px',
              backgroundColor: isActive ? '#38BDF8' : '#232D3F',
              cursor: onSelect ? 'pointer' : 'default',
              transition: 'all 0.15s ease'
            }}
          />
        )
      })}
    </div>
  )
}
