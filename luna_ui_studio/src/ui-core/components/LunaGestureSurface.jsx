import React, { useRef } from 'react'

/**
 * LunaGestureSurface — Hardware-Faithful Touch Gesture Router
 * Implements the CST816T deadzone, classification gate, and transition debounce.
 *
 * @param {Object} props
 * @param {React.ReactNode} props.children
 * @param {Function} [props.onSwipeLeft]
 * @param {Function} [props.onSwipeRight]
 * @param {Function} [props.onScrollVertical]
 * @param {Function} [props.onTap]
 * @param {Object} [props.style]
 */
export function LunaGestureSurface({
  children,
  onSwipeLeft,
  onSwipeRight,
  onScrollVertical,
  onTap,
  style = {}
}) {
  const touchState = useRef({
    isDown: false,
    startX: 0,
    startY: 0,
    startTime: 0,
    lastX: 0,
    lastY: 0,
    state: 'IDLE' // IDLE, TOUCH_DOWN, TRACKING, SCROLL_VERTICAL, SWIPE_HORIZONTAL
  })

  const handlePointerDown = (e) => {
    touchState.current = {
      isDown: true,
      startX: e.clientX,
      startY: e.clientY,
      startTime: Date.now(),
      lastX: e.clientX,
      lastY: e.clientY,
      state: 'TOUCH_DOWN'
    }
  }

  const handlePointerMove = (e) => {
    const s = touchState.current
    if (!s.isDown) return

    const dx = e.clientX - s.startX
    const dy = e.clientY - s.startY
    const absX = Math.abs(dx)
    const absY = Math.abs(dy)

    // 10px deadzone before leaving initial candidate tap
    if (s.state === 'TOUCH_DOWN') {
      if (absX >= 10 || absY >= 10) {
        s.state = 'TRACKING'
      }
    }

    if (s.state === 'TRACKING') {
      if (absY >= 16 && absY > absX) {
        s.state = 'SCROLL_VERTICAL'
      } else if (absX >= 50 && absX > absY * 1.5) {
        s.state = 'SWIPE_HORIZONTAL'
      }
    }

    if (s.state === 'SCROLL_VERTICAL') {
      const deltaY = s.lastY - e.clientY
      onScrollVertical && onScrollVertical(deltaY)
      s.lastY = e.clientY
    }
  }

  const handlePointerUp = (e) => {
    const s = touchState.current
    if (!s.isDown) return
    s.isDown = false

    const dx = e.clientX - s.startX
    const dy = e.clientY - s.startY
    const absX = Math.abs(dx)
    const absY = Math.abs(dy)
    const duration = Date.now() - s.startTime

    // Clean tap: movement stayed within 10px deadzone and held < 500ms
    if (s.state === 'TOUCH_DOWN' && absX < 10 && absY < 10 && duration < 500) {
      onTap && onTap(e)
      return
    }

    // Horizontal swipe: dx >= 50px and dx > 1.5 * dy
    if ((s.state === 'SWIPE_HORIZONTAL' || s.state === 'TRACKING') && absX >= 50 && absX > absY * 1.5) {
      if (dx < 0) {
        onSwipeLeft && onSwipeLeft()
      } else {
        onSwipeRight && onSwipeRight()
      }
    }

    s.state = 'IDLE'
  }

  return (
    <div
      onPointerDown={handlePointerDown}
      onPointerMove={handlePointerMove}
      onPointerUp={handlePointerUp}
      onPointerCancel={handlePointerUp}
      style={{
        position: 'relative',
        width: '100%',
        height: '100%',
        touchAction: 'none',
        userSelect: 'none',
        ...style
      }}
    >
      {children}
    </div>
  )
}
