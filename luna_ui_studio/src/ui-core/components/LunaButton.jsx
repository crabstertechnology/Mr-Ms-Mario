import React, { useState } from 'react'

/**
 * LunaButton — Tactical Monolithic Wearable Action Slab
 * Inspired by shadcn/ui state model, adapted for 240x280 one-thumb operation.
 * Minimum touch target: 44px height (preferred 48px).
 *
 * @param {Object} props
 * @param {'primary' | 'secondary' | 'ghost' | 'destructive'} [props.variant='primary']
 * @param {'normal' | 'pressed' | 'disabled' | 'loading' | 'success' | 'error'} [props.state='normal']
 * @param {string} props.label
 * @param {React.ReactNode} [props.icon]
 * @param {Function} [props.onClick]
 * @param {string} [props.className]
 * @param {Object} [props.style]
 */
export function LunaButton({
  variant = 'primary',
  state = 'normal',
  label,
  icon,
  onClick,
  className = '',
  style = {}
}) {
  const [isPressing, setIsPressing] = useState(false)

  const isDisabled = state === 'disabled' || state === 'loading'
  const effectiveState = isPressing && !isDisabled ? 'pressed' : state

  // Variant color definitions
  const getColors = () => {
    switch (variant) {
      case 'secondary':
        return {
          bg: effectiveState === 'pressed' ? '#1A2232' : '#121721',
          border: '#232D3F',
          text: effectiveState === 'disabled' ? '#4B5563' : '#EAEFF5',
          accent: '#38BDF8'
        }
      case 'destructive':
        return {
          bg: effectiveState === 'pressed' ? '#7F1D1D' : '#991B1B',
          border: '#DC2626',
          text: effectiveState === 'disabled' ? '#6B7280' : '#FFFFFF',
          accent: '#F87171'
        }
      case 'ghost':
        return {
          bg: effectiveState === 'pressed' ? '#121721' : 'transparent',
          border: effectiveState === 'pressed' ? '#232D3F' : 'transparent',
          text: effectiveState === 'disabled' ? '#4B5563' : '#38BDF8',
          accent: '#38BDF8'
        }
      case 'primary':
      default:
        return {
          bg: effectiveState === 'pressed' ? '#D97706' : '#FF9E3B',
          border: effectiveState === 'pressed' ? '#B45309' : '#FFB266',
          text: effectiveState === 'disabled' ? '#6B7280' : '#080A0F',
          accent: '#080A0F'
        }
    }
  }

  const colors = getColors()

  const handlePointerDown = () => {
    if (!isDisabled) setIsPressing(true)
  }

  const handlePointerUp = () => {
    setIsPressing(false)
  }

  return (
    <button
      type="button"
      role="button"
      disabled={isDisabled}
      aria-disabled={isDisabled}
      onPointerDown={handlePointerDown}
      onPointerUp={handlePointerUp}
      onPointerLeave={handlePointerUp}
      onClick={!isDisabled ? onClick : undefined}
      className={`luna-button ${className}`}
      style={{
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        width: '100%',
        minHeight: '44px',
        height: '48px',
        padding: '0 16px',
        borderRadius: '12px',
        border: `1px solid ${colors.border}`,
        backgroundColor: colors.bg,
        color: colors.text,
        fontFamily: 'Inter, -apple-system, sans-serif',
        fontSize: '13px',
        fontWeight: '700',
        letterSpacing: '0.05em',
        textTransform: 'uppercase',
        cursor: isDisabled ? 'not-allowed' : 'pointer',
        userSelect: 'none',
        outline: 'none',
        boxSizing: 'border-box',
        transition: 'background-color 0.12s ease, transform 0.08s ease',
        transform: effectiveState === 'pressed' ? 'scale(0.98)' : 'scale(1)',
        opacity: effectiveState === 'disabled' ? 0.45 : 1,
        ...style
      }}
    >
      {effectiveState === 'loading' ? (
        <span style={{ display: 'flex', gap: '6px', alignItems: 'center' }}>
          <span style={{ width: '6px', height: '6px', borderRadius: '50%', backgroundColor: colors.text, animation: 'pulse 0.8s infinite alternate' }} />
          <span style={{ width: '6px', height: '6px', borderRadius: '50%', backgroundColor: colors.text, animation: 'pulse 0.8s infinite alternate 0.2s' }} />
          <span style={{ width: '6px', height: '6px', borderRadius: '50%', backgroundColor: colors.text, animation: 'pulse 0.8s infinite alternate 0.4s' }} />
        </span>
      ) : effectiveState === 'success' ? (
        <span style={{ display: 'flex', alignItems: 'center', gap: '8px', color: '#10B981' }}>
          ✓ DONE
        </span>
      ) : (
        <span style={{ display: 'flex', alignItems: 'center', gap: '8px', overflow: 'hidden', textOverflow: 'ellipsis', whiteSpace: 'nowrap' }}>
          {icon && <span style={{ display: 'flex', alignItems: 'center', fontSize: '15px' }}>{icon}</span>}
          <span>{label}</span>
        </span>
      )}
    </button>
  )
}
