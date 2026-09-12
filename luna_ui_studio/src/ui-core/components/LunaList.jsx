import React from 'react'

/**
 * LunaList — Wearable Vertical Scrolling Feed Container
 * Optimized for one-thumb vertical swiping with hardware scrollbar indicator.
 *
 * @param {Object} props
 * @param {React.ReactNode} props.children
 * @param {number} [props.gap=8]
 * @param {number} [props.maxHeight]
 * @param {Object} [props.style]
 */
export function LunaList({
  children,
  gap = 8,
  maxHeight,
  style = {}
}) {
  return (
    <div
      role="list"
      style={{
        display: 'flex',
        flexDirection: 'column',
        gap: `${gap}px`,
        width: '100%',
        maxHeight: maxHeight ? `${maxHeight}px` : undefined,
        overflowY: maxHeight ? 'auto' : 'visible',
        boxSizing: 'border-box',
        paddingRight: '2px', // space for scroll thumb
        ...style
      }}
    >
      {children}
    </div>
  )
}
