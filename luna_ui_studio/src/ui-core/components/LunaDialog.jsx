import React from 'react'
import { LunaButton } from './LunaButton.jsx'

/**
 * LunaDialog — Full-Screen Wearable Confirmation Overlay
 * Isolates high-stakes decisions with massive touch targets.
 *
 * @param {Object} props
 * @param {boolean} props.isOpen
 * @param {string} props.title
 * @param {string} props.description
 * @param {string} [props.confirmLabel='CONFIRM']
 * @param {string} [props.cancelLabel='CANCEL']
 * @param {'primary' | 'destructive'} [props.confirmVariant='primary']
 * @param {Function} props.onConfirm
 * @param {Function} props.onCancel
 */
export function LunaDialog({
  isOpen,
  title,
  description,
  confirmLabel = 'CONFIRM',
  cancelLabel = 'CANCEL',
  confirmVariant = 'primary',
  onConfirm,
  onCancel
}) {
  if (!isOpen) return null

  return (
    <div
      role="dialog"
      aria-modal="true"
      style={{
        position: 'absolute',
        top: 0,
        left: 0,
        width: '240px',
        height: '280px',
        backgroundColor: 'rgba(8, 10, 15, 0.95)',
        display: 'flex',
        flexDirection: 'column',
        justifyContent: 'space-between',
        padding: '24px 14px',
        boxSizing: 'border-box',
        zIndex: 9999,
        fontFamily: 'Inter, sans-serif'
      }}
    >
      {/* Header Info */}
      <div style={{ display: 'flex', flexDirection: 'column', gap: '8px', textAlign: 'center', marginTop: '12px' }}>
        <h2 style={{ margin: 0, fontSize: '15px', fontWeight: '800', color: '#EAEFF5', letterSpacing: '0.04em' }}>
          {title}
        </h2>
        <p style={{ margin: 0, fontSize: '12px', color: '#8290A4', lineHeight: 1.4 }}>
          {description}
        </p>
      </div>

      {/* Dual Slab Action Triggers */}
      <div style={{ display: 'flex', flexDirection: 'column', gap: '10px' }}>
        <LunaButton
          variant={confirmVariant}
          label={confirmLabel}
          onClick={onConfirm}
        />
        <LunaButton
          variant="secondary"
          label={cancelLabel}
          onClick={onCancel}
        />
      </div>
    </div>
  )
}
