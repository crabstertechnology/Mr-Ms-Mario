import React from 'react'

/**
 * LunaToggle — High-Contrast Wearable Binary Switch
 * Track: 48x28px, Thumb: 22x22px (Minimum touch hit target: >= 44px).
 *
 * @param {Object} props
 * @param {boolean} props.checked
 * @param {Function} props.onChange
 * @param {boolean} [props.disabled=false]
 * @param {string} [props.activeColor='#38BDF8']
 * @param {string} [props.inactiveColor='#232D3F']
 * @param {string} [props.thumbColor='#EAEFF5']
 * @param {string} [props.ariaLabel='Toggle']
 * @param {Object} [props.style]
 */
export function LunaToggle({
  checked,
  onChange,
  disabled = false,
  activeColor = '#38BDF8',
  inactiveColor = '#232D3F',
  thumbColor = '#EAEFF5',
  ariaLabel = 'Toggle switch',
  style = {}
}) {
  const handleClick = (e) => {
    e.stopPropagation()
    if (!disabled && onChange) {
      onChange(!checked)
    }
  }

  return (
    <div
      role="switch"
      aria-checked={checked}
      aria-disabled={disabled}
      aria-label={ariaLabel}
      tabIndex={disabled ? -1 : 0}
      onClick={handleClick}
      onKeyDown={(e) => {
        if (!disabled && (e.key === ' ' || e.key === 'Enter')) {
          e.preventDefault()
          onChange && onChange(!checked)
        }
      }}
      style={{
        display: 'inline-flex',
        alignItems: 'center',
        width: '50px',
        height: '28px',
        minWidth: '44px',
        minHeight: '28px',
        backgroundColor: checked ? activeColor : inactiveColor,
        borderRadius: '14px',
        padding: '3px',
        boxSizing: 'border-box',
        cursor: disabled ? 'not-allowed' : 'pointer',
        opacity: disabled ? 0.4 : 1,
        transition: 'background-color 0.15s ease',
        userSelect: 'none',
        outline: 'none',
        ...style
      }}
    >
      <div
        style={{
          width: '22px',
          height: '22px',
          borderRadius: '50%',
          backgroundColor: thumbColor,
          transform: checked ? 'translateX(22px)' : 'translateX(0px)',
          transition: 'transform 0.15s cubic-bezier(0.4, 0, 0.2, 1)',
          boxShadow: '0 1px 3px rgba(0,0,0,0.5)'
        }}
      />
    </div>
  )
}
