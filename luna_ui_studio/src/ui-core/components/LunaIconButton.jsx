import React, { useState } from 'react'

/**
 * LunaIconButton — Space-Efficient Wearable Touch Trigger
 * Guaranteed >= 40x40px touch bounding box (minimum 44x44px standard).
 *
 * @param {Object} props
 * @param {React.ReactNode} props.icon
 * @param {'primary' | 'secondary' | 'ghost' | 'destructive'} [props.variant='secondary']
 * @param {'normal' | 'pressed' | 'disabled' | 'active'} [props.state='normal']
 * @param {string} props.ariaLabel
 * @param {Function} [props.onClick]
 * @param {Object} [props.style]
 */
export function LunaIconButton({
  icon,
  variant = 'secondary',
  state = 'normal',
  ariaLabel,
  onClick,
  style = {}
}) {
  const [isPressing, setIsPressing] = useState(false)
  const isDisabled = state === 'disabled'
  const effectiveState = isPressing && !isDisabled ? 'pressed' : state

  const getColors = () => {
    if (variant === 'primary') {
      return {
        bg: effectiveState === 'pressed' ? '#D97706' : '#FF9E3B',
        border: '#FFB266',
        text: '#080A0F'
      }
    }
    if (variant === 'destructive') {
      return {
        bg: effectiveState === 'pressed' ? '#7F1D1D' : '#991B1B',
        border: '#DC2626',
        text: '#FFFFFF'
      }
    }
    // secondary
    return {
      bg: effectiveState === 'pressed' ? '#1A2232' : '#121721',
      border: effectiveState === 'active' ? '#38BDF8' : '#232D3F',
      text: effectiveState === 'active' ? '#38BDF8' : '#EAEFF5'
    }
  }

  const colors = getColors()

  return (
    <button
      type="button"
      role="button"
      aria-label={ariaLabel}
      disabled={isDisabled}
      aria-disabled={isDisabled}
      onPointerDown={() => !isDisabled && setIsPressing(true)}
      onPointerUp={() => setIsPressing(false)}
      onPointerLeave={() => setIsPressing(false)}
      onClick={!isDisabled ? onClick : undefined}
      style={{
        display: 'inline-flex',
        alignItems: 'center',
        justifyContent: 'center',
        width: '44px',
        height: '44px',
        minWidth: '40px',
        minHeight: '40px',
        borderRadius: '10px',
        border: `1px solid ${colors.border}`,
        backgroundColor: colors.bg,
        color: colors.text,
        fontSize: '18px',
        cursor: isDisabled ? 'not-allowed' : 'pointer',
        userSelect: 'none',
        outline: 'none',
        boxSizing: 'border-box',
        transition: 'background-color 0.12s ease, transform 0.08s ease',
        transform: effectiveState === 'pressed' ? 'scale(0.94)' : 'scale(1)',
        opacity: isDisabled ? 0.45 : 1,
        ...style
      }}
    >
      {icon}
    </button>
  )
}
