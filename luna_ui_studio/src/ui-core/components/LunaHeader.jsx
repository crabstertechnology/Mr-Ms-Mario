import React from 'react'
import { LunaIconButton } from './LunaIconButton.jsx'

/**
 * LunaHeader — Standard Screen Context Header
 * Features guaranteed >= 40px touch back button and bold screen title.
 *
 * @param {Object} props
 * @param {string} props.title
 * @param {Function} [props.onBack]
 * @param {React.ReactNode} [props.rightAccessory]
 * @param {Object} [props.style]
 */
export function LunaHeader({
  title,
  onBack,
  rightAccessory,
  style = {}
}) {
  return (
    <div
      role="banner"
      style={{
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'space-between',
        width: '100%',
        height: '44px',
        minHeight: '44px',
        padding: '0 8px',
        boxSizing: 'border-box',
        backgroundColor: '#080A0F',
        borderBottom: '1px solid #1A2232',
        userSelect: 'none',
        ...style
      }}
    >
      <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
        {onBack && (
          <LunaIconButton
            icon="‹"
            variant="ghost"
            ariaLabel="Back"
            onClick={onBack}
            style={{ width: '38px', height: '38px', minWidth: '38px', minHeight: '38px', fontSize: '24px', fontWeight: 'bold' }}
          />
        )}
        <span
          style={{
            color: '#EAEFF5',
            fontSize: '13px',
            fontWeight: '800',
            letterSpacing: '0.06em',
            textTransform: 'uppercase',
            fontFamily: 'Inter, sans-serif'
          }}
        >
          {title}
        </span>
      </div>
      {rightAccessory && (
        <div style={{ display: 'flex', alignItems: 'center' }}>
          {rightAccessory}
        </div>
      )}
    </div>
  )
}
