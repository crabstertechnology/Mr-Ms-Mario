import React, { useEffect } from 'react'

/**
 * LunaToast — Transient Upper-Third Feedback Pill
 * Non-blocking status notification with auto-dismiss.
 *
 * @param {Object} props
 * @param {boolean} props.isOpen
 * @param {string} props.message
 * @param {'info' | 'success' | 'warning' | 'error'} [props.type='info']
 * @param {number} [props.durationMs=2500]
 * @param {Function} props.onClose
 */
export function LunaToast({
  isOpen,
  message,
  type = 'info',
  durationMs = 2500,
  onClose
}) {
  useEffect(() => {
    if (isOpen && durationMs > 0) {
      const timer = setTimeout(() => {
        onClose && onClose()
      }, durationMs)
      return () => clearTimeout(timer)
    }
  }, [isOpen, durationMs, onClose])

  if (!isOpen) return null

  const getAccent = () => {
    switch (type) {
      case 'success': return '#10B981'
      case 'warning': return '#FF9E3B'
      case 'error': return '#EF4444'
      case 'info':
      default: return '#38BDF8'
    }
  }

  const accent = getAccent()

  return (
    <div
      role="status"
      onClick={onClose}
      style={{
        position: 'absolute',
        top: '16px',
        left: '14px',
        width: '212px',
        minHeight: '40px',
        backgroundColor: '#121721',
        borderRadius: '10px',
        border: '1px solid #232D3F',
        borderLeft: `4px solid ${accent}`,
        display: 'flex',
        alignItems: 'center',
        padding: '8px 12px',
        boxSizing: 'border-box',
        cursor: 'pointer',
        boxShadow: '0 4px 12px rgba(0,0,0,0.8)',
        zIndex: 8888,
        userSelect: 'none',
        animation: 'fadeInDown 0.18s ease-out'
      }}
    >
      <span
        style={{
          color: '#EAEFF5',
          fontSize: '11px',
          fontWeight: '600',
          fontFamily: 'Inter, sans-serif',
          overflow: 'hidden',
          textOverflow: 'ellipsis',
          whiteSpace: 'nowrap'
        }}
      >
        {message}
      </span>
    </div>
  )
}
