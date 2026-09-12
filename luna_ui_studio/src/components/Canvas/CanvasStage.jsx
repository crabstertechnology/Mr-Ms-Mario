import React, { useRef, useState, useCallback, useEffect } from 'react'
import styled, { keyframes } from 'styled-components'
import { UI_COMPONENTS } from '../../ui-elements/registry'
import ScreenTabs from './ScreenTabs'
import { StarfieldBackground } from './StarfieldBackground'

const DISPLAY_W = 240
const DISPLAY_H = 280

// ── Wireframe category colours ──
const WIRE_COLORS = {
  cards:        { stroke: '#6366f1', fill: 'rgba(99,102,241,0.08)',  text: '#6366f1' },
  buttons:      { stroke: '#10b981', fill: 'rgba(16,185,129,0.08)', text: '#10b981' },
  toggles:      { stroke: '#f59e0b', fill: 'rgba(245,158,11,0.08)', text: '#f59e0b' },
  loaders:      { stroke: '#ec4899', fill: 'rgba(236,72,153,0.08)', text: '#ec4899' },
  checkboxes:   { stroke: '#14b8a6', fill: 'rgba(20,184,166,0.08)', text: '#14b8a6' },
  gauges:       { stroke: '#8b5cf6', fill: 'rgba(139,92,246,0.08)', text: '#8b5cf6' },
  text:         { stroke: '#64748b', fill: 'rgba(100,116,139,0.08)', text: '#64748b' },
  inputs:       { stroke: '#0ea5e9', fill: 'rgba(14,165,233,0.08)', text: '#0ea5e9' },
  notifications:{ stroke: '#f97316', fill: 'rgba(249,115,22,0.08)', text: '#f97316' },
  default:      { stroke: '#94a3b8', fill: 'rgba(148,163,184,0.08)', text: '#94a3b8' },
}

export default function CanvasStage({
  screens = [],
  activeScreenId = 'screen_1',
  elements = [],
  selectedId,
  zoom,
  gridSnap,
  mode = 'design', // 'design' | 'wireframe' | 'test'
  setMode,
  onSelectScreen,
  onAddScreen,
  onDeleteScreen,
  onDuplicateScreen,
  onRenameScreen,
  onOpenFlowMap,
  onSelect,
  onAddDrop,
  onMove,
  onResize,
  onUpdateElement
}) {
  const containerRef = useRef(null)
  const isTestMode = mode === 'test'
  const isWireframe = mode === 'wireframe'

  const [scrollY, setScrollY] = useState(0)
  const [transitionAnim, setTransitionAnim] = useState('none')
  const [watchAlert, setWatchAlert] = useState(null)
  const [actionLog, setActionLog] = useState([])

  const activeScreen = screens.find(s => s.id === activeScreenId) || screens[0]

  const pointerStartRef = useRef({ x: 0, y: 0, time: 0, isDown: false, startScrollY: 0, hasScrolled: false })

  // Reset scroll on screen change
  useEffect(() => {
    setScrollY(0)
    setTransitionAnim('slideIn')
    const timer = setTimeout(() => setTransitionAnim('none'), 350)
    return () => clearTimeout(timer)
  }, [activeScreenId])

  const logEvent = useCallback((msg) => {
    setActionLog(prev => [msg, ...prev.slice(0, 4)])
  }, [])

  const handlePointerDown = (e) => {
    pointerStartRef.current = {
      x: e.clientX,
      y: e.clientY,
      time: Date.now(),
      isDown: true,
      startScrollY: scrollY,
      hasScrolled: false
    }
  }

  const handlePointerMove = (e) => {
    if (!pointerStartRef.current.isDown) return
    const dx = (pointerStartRef.current.x - e.clientX) / zoom
    const dy = (pointerStartRef.current.y - e.clientY) / zoom

    // Full-display vertical scroll
    if (activeScreen?.isScrollable && Math.abs(dy) > 4 && Math.abs(dy) > Math.abs(dx)) {
      pointerStartRef.current.hasScrolled = true
      const maxScroll = Math.max(0, (activeScreen.maxScrollY || 400) - DISPLAY_H)
      setScrollY(Math.max(0, Math.min(maxScroll, pointerStartRef.current.startScrollY + dy)))
    }
  }

  const handlePointerUp = (e) => {
    if (!pointerStartRef.current.isDown) return
    pointerStartRef.current.isDown = false

    if (pointerStartRef.current.hasScrolled) return

    const dx = e.clientX - pointerStartRef.current.x
    const dy = e.clientY - pointerStartRef.current.y
    const dt = Date.now() - pointerStartRef.current.time

    // Detect screen-level horizontal swipe (drag > 35px in < 600ms)
    if (Math.abs(dx) > 35 && Math.abs(dx) > Math.abs(dy) * 1.3 && dt < 600) {
      if (dx < 0) {
        // Swipe Left (finger moved left)
        const act = activeScreen?.gestures?.swipeLeft
        if (act && act.actionType === 'navigate' && act.targetScreenId) {
          const target = screens.find(s => s.id === act.targetScreenId)
          if (target) {
            setTransitionAnim('slideLeft')
            onSelectScreen(target.id)
            logEvent(`👈 Swiped Left: Navigated to "${target.name}"`)
            setTimeout(() => setTransitionAnim('none'), 350)
            return
          }
        } else if (act && act.actionType === 'alert') {
          setWatchAlert({ message: act.alertMessage || 'Swipe Left Triggered' })
          return
        }
      } else {
        // Swipe Right (finger moved right)
        const act = activeScreen?.gestures?.swipeRight
        if (act && act.actionType === 'navigate' && act.targetScreenId) {
          const target = screens.find(s => s.id === act.targetScreenId)
          if (target) {
            setTransitionAnim('slideRight')
            onSelectScreen(target.id)
            logEvent(`👉 Swiped Right: Navigated to "${target.name}"`)
            setTimeout(() => setTransitionAnim('none'), 350)
            return
          }
        } else if (act && act.actionType === 'alert') {
          setWatchAlert({ message: act.alertMessage || 'Swipe Right Triggered' })
          return
        }
      }
    }
  }

  const handleWheel = (e) => {
    if (activeScreen?.isScrollable) {
      e.preventDefault()
      const maxScroll = Math.max(0, (activeScreen.maxScrollY || 400) - DISPLAY_H)
      setScrollY(prev => Math.max(0, Math.min(maxScroll, prev + e.deltaY * 0.4)))
    }
  }

  const getScreenBgStyle = () => {
    if (isWireframe) {
      return {
        background: '#f8fafc',
        backgroundImage: `linear-gradient(rgba(100,116,139,0.12) 1px, transparent 1px), linear-gradient(90deg, rgba(100,116,139,0.12) 1px, transparent 1px)`,
        backgroundSize: `${8 * zoom}px ${8 * zoom}px`,
      }
    }
    const bgType = activeScreen?.bgType || 'color'
    if (bgType === 'stars' || (bgType === 'pattern' && activeScreen?.bgPattern === 'stars')) {
      return { backgroundColor: '#090a0f' }
    }
    if (bgType === 'gradient' && activeScreen?.bgGradient) {
      return { background: activeScreen.bgGradient }
    }
    if (bgType === 'image' && activeScreen?.bgImage) {
      return {
        backgroundImage: `url(${activeScreen.bgImage})`,
        backgroundSize: 'cover',
        backgroundPosition: 'center',
      }
    }
    if (bgType === 'pattern') {
      const base = activeScreen?.bgColor || '#0b0f19'
      const pattern = activeScreen?.bgPattern || 'grid'
      if (pattern === 'grid') {
        return {
          backgroundColor: base,
          backgroundImage: `linear-gradient(rgba(56,189,248,0.15) 1px, transparent 1px), linear-gradient(90deg, rgba(56,189,248,0.15) 1px, transparent 1px)`,
          backgroundSize: `${16 * zoom}px ${16 * zoom}px`,
        }
      }
      if (pattern === 'dots') {
        return {
          backgroundColor: base,
          backgroundImage: `radial-gradient(rgba(56,189,248,0.3) 1.5px, transparent 1.5px)`,
          backgroundSize: `${12 * zoom}px ${12 * zoom}px`,
        }
      }
      if (pattern === 'scanlines') {
        return {
          backgroundColor: base,
          backgroundImage: `repeating-linear-gradient(0deg, rgba(0,0,0,0.3) 0px, rgba(0,0,0,0.3) 2px, transparent 2px, transparent 4px)`,
        }
      }
      if (pattern === 'carbon') {
        return {
          backgroundColor: base,
          backgroundImage: `linear-gradient(45deg, rgba(0,0,0,0.4) 25%, transparent 25%), linear-gradient(-45deg, rgba(0,0,0,0.4) 25%, transparent 25%), linear-gradient(45deg, transparent 75%, rgba(0,0,0,0.4) 75%), linear-gradient(-45deg, transparent 75%, rgba(0,0,0,0.4) 75%)`,
          backgroundSize: `${10 * zoom}px ${10 * zoom}px`,
        }
      }
      if (pattern === 'hex') {
        return {
          backgroundColor: base,
          backgroundImage: `radial-gradient(circle at 50% 50%, rgba(56,189,248,0.2) 2px, transparent 2px)`,
          backgroundSize: `${20 * zoom}px ${20 * zoom}px`,
        }
      }
    }
    return { backgroundColor: activeScreen?.bgColor || '#0b0f19' }
  }

  const snapVal = (v) => gridSnap > 1 ? Math.round(v / gridSnap) * gridSnap : Math.round(v)

  const handleDragOver = (e) => {
    if (isTestMode) return
    e.preventDefault()
    e.dataTransfer.dropEffect = 'copy'
  }

  const handleDrop = (e) => {
    if (isTestMode) return
    e.preventDefault()
    const type = e.dataTransfer.getData('text/plain')
    if (!type || !UI_COMPONENTS[type]) return
    const rect = containerRef.current.getBoundingClientRect()
    const x = snapVal(Math.round((e.clientX - rect.left) / zoom))
    const y = snapVal(Math.round((e.clientY - rect.top) / zoom))
    onAddDrop(type, Math.max(0, Math.min(x, DISPLAY_W - 40)), Math.max(0, Math.min(y, DISPLAY_H - 20)))
  }

  const handleBackdropClick = (e) => {
    if (e.target === e.currentTarget && !isTestMode) {
      onSelect(null)
    }
  }

  // Execute mapped action block in Interactive Test Mode
  const handleElementActionTrigger = useCallback((el) => {
    if (!isTestMode) return

    const actions = el.actions || []
    if (actions.length === 0) {
      logEvent(`ℹ️ "${el.props?.label || el.name}" has no mapped actions (map in ⚡ Mapping tab)`)
      return
    }

    actions.forEach(act => {
      if (act.actionType === 'navigate' && act.targetScreenId) {
        const target = screens.find(s => s.id === act.targetScreenId)
        if (target) {
          onSelectScreen(target.id)
          logEvent(`🚀 Triggered: [${el.props?.label || el.name}] ➔ Navigated to "${target.name}"`)
        }
      } else if (act.actionType === 'scroll') {
        const dir = act.scrollDirection || 'down'
        const amt = act.scrollAmount || 80
        if (dir === 'down') {
          setScrollY(prev => prev + amt)
          logEvent(`📜 Triggered: [${el.props?.label || el.name}] ➔ Scrolled Down +${amt}px`)
        } else if (dir === 'up') {
          setScrollY(prev => Math.max(0, prev - amt))
          logEvent(`📜 Triggered: [${el.props?.label || el.name}] ➔ Scrolled Up -${amt}px`)
        } else if (dir === 'top') {
          setScrollY(0)
          logEvent(`🔝 Triggered: [${el.props?.label || el.name}] ➔ Scrolled to Top`)
        }
      } else if (act.actionType === 'alert') {
        const msg = act.alertMessage || 'Watch Notification'
        setWatchAlert({ message: msg })
        logEvent(`🔔 Triggered: [${el.props?.label || el.name}] ➔ Alert: "${msg}"`)
        setTimeout(() => setWatchAlert(null), 3000)
      } else if (act.actionType === 'toggle') {
        if (onUpdateElement) {
          onUpdateElement(el.id, { checked: !el.props.checked })
          logEvent(`🔄 Triggered: [${el.name}] ➔ Toggled state to ${!el.props.checked ? 'ON' : 'OFF'}`)
        }
      }
    })
  }, [isTestMode, screens, onSelectScreen, onUpdateElement, logEvent])

  return (
    <StageWrapper>
      {/* Screen Tabs Switcher Bar */}
      <ScreenTabs
        screens={screens}
        activeScreenId={activeScreenId}
        onSelectScreen={onSelectScreen}
        onAddScreen={onAddScreen}
        onDeleteScreen={onDeleteScreen}
        onDuplicateScreen={onDuplicateScreen}
        onRenameScreen={onRenameScreen}
        onOpenFlowMap={onOpenFlowMap}
        testMode={isTestMode}
      />

      <Stage>
        {/* Test Mode Banner */}
        {isTestMode && (
          <TestModeBanner>
            <BannerDot />
            <span>🎮 <strong>INTERACTIVE TEST MODE ACTIVE</strong> — Click buttons to trigger screen navigation & scrolling!</span>
          </TestModeBanner>
        )}

        <WatchOuter $testMode={isTestMode}>
          <WatchBezel $testMode={isTestMode}>
            <SideButton style={{ top: 60 }} title="Crown Knob" />
            <SideButton style={{ top: 100 }} title="Action Button" />

            <Screen
              ref={containerRef}
              style={{
                width: DISPLAY_W * zoom,
                height: DISPLAY_H * zoom,
                cursor: activeScreen?.isScrollable ? 'grab' : 'default',
                ...getScreenBgStyle(),
              }}
              onDragOver={handleDragOver}
              onDrop={handleDrop}
              onClick={handleBackdropClick}
              onPointerDown={handlePointerDown}
              onPointerMove={handlePointerMove}
              onPointerUp={handlePointerUp}
              onWheel={handleWheel}
            >
              {/* Screen Scrollable Indicator Badge */}
              {activeScreen?.isScrollable && (
                <ScrollBadgeTitle>
                  📜 SCROLLABLE ({Math.round(scrollY)}px)
                </ScrollBadgeTitle>
              )}

              {/* Animated Cosmic Starfield Background (Parallax 3-layer stars) */}
              {(activeScreen?.bgType === 'stars' || (activeScreen?.bgType === 'pattern' && activeScreen?.bgPattern === 'stars')) && (
                <StarfieldBackground />
              )}

              {/* Animated Screen Content Container for Scrolling and Transitions */}
              <ScreenContent
                $anim={transitionAnim}
                style={{
                  transform: `translateY(${-scrollY * zoom}px)`,
                  width: '100%',
                  height: '100%',
                }}
              >
                {elements.map(el => (
                  <CanvasElement
                    key={el.id}
                    el={el}
                    zoom={zoom}
                    gridSnap={gridSnap}
                    wireframe={isWireframe}
                    isTestMode={isTestMode}
                    selected={!isTestMode && el.id === selectedId}
                    onSelect={() => !isTestMode && onSelect(el.id)}
                    onMove={(nx, ny, isDone) => onMove(el.id, nx, ny, isDone)}
                    onResize={(nw, nh, isDone) => onResize(el.id, nw, nh, isDone)}
                    onActionClick={() => handleElementActionTrigger(el)}
                  />
                ))}
              </ScreenContent>

              {/* Smartwatch Alert Notification Overlay */}
              {watchAlert && (
                <WatchAlertOverlay onClick={() => setWatchAlert(null)}>
                  <AlertBox>
                    <AlertHeader>
                      <span>🔔 LUNA ALERT</span>
                      <DismissBadge>Tap to close</DismissBadge>
                    </AlertHeader>
                    <AlertMessage>{watchAlert.message}</AlertMessage>
                  </AlertBox>
                </WatchAlertOverlay>
              )}

              {/* Watch Vertical Scroll Indicator */}
              {scrollY > 0 && (
                <WatchScrollTrack>
                  <WatchScrollThumb
                    style={{
                      transform: `translateY(${Math.min(180, (scrollY / 160) * 100)}px)`
                    }}
                  />
                </WatchScrollTrack>
              )}
            </Screen>
          </WatchBezel>
        </WatchOuter>

        {/* HUD Bar */}
        <HUD>
          <HUDItem>Screen: <strong>{activeScreen?.name || 'Screen 1'}</strong></HUDItem>
          <HUDItem>Display: <strong>240 × 280</strong></HUDItem>
          <HUDItem>Zoom: <strong>{Math.round(zoom * 100)}%</strong></HUDItem>
          <HUDItem>Elements: <strong>{elements.length}</strong></HUDItem>
        </HUD>

        {/* Floating Test Mode Controller HUD */}
        {isTestMode && (
          <TestControlsFloating>
            <TestControlsHeader>
              <span style={{ fontSize: 13 }}>🎮</span>
              <TestControlsTitle>Interactive Controller</TestControlsTitle>
              <ExitTestBtn onClick={() => setMode('design')}>Exit Test Mode ✕</ExitTestBtn>
            </TestControlsHeader>

            <TestControlsRow>
              <TestBtn
                onClick={() => {
                  if (screens[0]) onSelectScreen(screens[0].id)
                }}
                title="Jump back to Screen 1"
              >
                ⏮ Back to Screen 1
              </TestBtn>

              <TestBtn
                onClick={() => setScrollY(0)}
                title="Reset canvas scroll"
              >
                📜 Reset Scroll ({scrollY}px)
              </TestBtn>

              <TestBtn
                onClick={() => onOpenFlowMap()}
                title="View UI Flow Map"
              >
                🗺️ View Flow Map
              </TestBtn>
            </TestControlsRow>

            {actionLog.length > 0 && (
              <ActionLogBox>
                <LogHeader>LAST ACTION LOG:</LogHeader>
                <LogText>{actionLog[0]}</LogText>
              </ActionLogBox>
            )}
          </TestControlsFloating>
        )}
      </Stage>
    </StageWrapper>
  )
}

function CanvasElement({ el, zoom, gridSnap, wireframe, isTestMode, selected, onSelect, onMove, onResize, onActionClick }) {
  const Comp = UI_COMPONENTS[el.type]?.component
  const p = el.props
  const hasActions = el.actions && el.actions.length > 0
  const dragRef = useRef({ dragging: false, startMouseX: 0, startMouseY: 0, startX: 0, startY: 0, lastX: 0, lastY: 0 })
  const resizeRef = useRef({ resizing: false, startMouseX: 0, startMouseY: 0, startW: 0, startH: 0, lastW: 0, lastH: 0 })

  const handleMouseDown = (e) => {
    if (isTestMode) {
      e.stopPropagation()
      onActionClick()
      return
    }

    if (e.button !== 0) return
    e.stopPropagation()
    if (e.target.tagName !== 'INPUT' && e.target.tagName !== 'TEXTAREA') {
      e.preventDefault()
    }
    onSelect()
    dragRef.current = {
      dragging: true,
      startMouseX: e.clientX,
      startMouseY: e.clientY,
      startX: p.x,
      startY: p.y,
      lastX: p.x,
      lastY: p.y
    }

    const onMove_ = (e2) => {
      if (!dragRef.current.dragging) return
      const dx = (e2.clientX - dragRef.current.startMouseX) / zoom
      const dy = (e2.clientY - dragRef.current.startMouseY) / zoom
      let nx = dragRef.current.startX + dx
      let ny = dragRef.current.startY + dy
      if (gridSnap > 1) {
        nx = Math.round(nx / gridSnap) * gridSnap
        ny = Math.round(ny / gridSnap) * gridSnap
      } else {
        nx = Math.round(nx)
        ny = Math.round(ny)
      }
      const clampedX = Math.max(0, Math.min(DISPLAY_W - (p.w || 40), nx))
      const clampedY = Math.max(0, Math.min(DISPLAY_H - (p.h || 20), ny))
      dragRef.current.lastX = clampedX
      dragRef.current.lastY = clampedY
      onMove(clampedX, clampedY, false)
    }

    const onUp = () => {
      dragRef.current.dragging = false
      window.removeEventListener('mousemove', onMove_)
      window.removeEventListener('mouseup', onUp)
      onMove(dragRef.current.lastX, dragRef.current.lastY, true)
    }

    window.addEventListener('mousemove', onMove_)
    window.addEventListener('mouseup', onUp)
  }

  const handleResizeDown = (e) => {
    if (isTestMode) return
    e.stopPropagation()
    resizeRef.current = {
      resizing: true,
      startMouseX: e.clientX,
      startMouseY: e.clientY,
      startW: p.w,
      startH: p.h,
      lastW: p.w,
      lastH: p.h,
    }

    const onResize_ = (e2) => {
      if (!resizeRef.current.resizing) return
      const dw = (e2.clientX - resizeRef.current.startMouseX) / zoom
      const dh = (e2.clientY - resizeRef.current.startMouseY) / zoom
      let nw = Math.max(20, Math.min(DISPLAY_W - p.x, Math.round(resizeRef.current.startW + dw)))
      let nh = Math.max(16, Math.min(DISPLAY_H - p.y, Math.round(resizeRef.current.startH + dh)))
      if (gridSnap > 1) {
        nw = Math.round(nw / gridSnap) * gridSnap
        nh = Math.round(nh / gridSnap) * gridSnap
      }
      resizeRef.current.lastW = nw
      resizeRef.current.lastH = nh
      onResize(nw, nh, false)
    }

    const onResizeUp = () => {
      resizeRef.current.resizing = false
      window.removeEventListener('mousemove', onResize_)
      window.removeEventListener('mouseup', onResizeUp)
      onResize(resizeRef.current.lastW, resizeRef.current.lastH, true)
    }

    window.addEventListener('mousemove', onResize_)
    window.addEventListener('mouseup', onResizeUp)
  }

  const category = UI_COMPONENTS[el.type]?.category || 'default'
  const colors = WIRE_COLORS[category] || WIRE_COLORS.default

  return (
    <ElementWrap
      $selected={selected}
      $wireframe={wireframe}
      $testMode={isTestMode}
      $hasActions={hasActions}
      style={{
        left: p.x * zoom,
        top: p.y * zoom,
        width: (p.w || 80) * zoom,
        height: (p.h || 40) * zoom,
        cursor: isTestMode ? (hasActions ? 'pointer' : 'default') : 'move',
      }}
      onMouseDown={handleMouseDown}
      title={isTestMode && hasActions ? `Click to trigger action (${el.actions.length} mapped)` : undefined}
    >
      <ScaledContent
        style={{
          width: p.w || 80,
          height: p.h || 40,
          transform: `scale(${zoom})`,
          transformOrigin: 'top left',
          pointerEvents: isTestMode ? 'none' : 'auto',
        }}
      >
        {wireframe ? (
          <WireframePlaceholder $colors={colors}>
            <WireframeHeader>
              <WireframeType>{el.name}</WireframeType>
              <WireframeDims>{p.w}×{p.h}</WireframeDims>
            </WireframeHeader>
            {p.label && <WireframeLabel>{p.label}</WireframeLabel>}
            {p.title && <WireframeLabel>{p.title}</WireframeLabel>}
          </WireframePlaceholder>
        ) : Comp ? (
          <Comp {...p} />
        ) : (
          <div style={{ color: '#fff', fontSize: 10 }}>{el.name}</div>
        )}
      </ScaledContent>

      {/* Action Indicator Badge in Design Mode */}
      {!isTestMode && hasActions && (
        <ActionIndicatorBadge title={`${el.actions.length} action(s) mapped`}>
          ⚡{el.actions.length}
        </ActionIndicatorBadge>
      )}

      {/* Resize Handle in Design Mode */}
      {!isTestMode && selected && (
        <ResizeHandle
          style={{ width: 10, height: 10 }}
          onMouseDown={handleResizeDown}
        />
      )}
    </ElementWrap>
  )
}

const StageWrapper = styled.div`
  flex: 1;
  display: flex;
  flex-direction: column;
  height: 100%;
  overflow: hidden;
`

const Stage = styled.div`
  flex: 1;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  background: var(--bg-base);
  position: relative;
  overflow: auto;
  padding: 24px;
`

const TestModeBanner = styled.div`
  position: absolute;
  top: 14px;
  background: linear-gradient(135deg, rgba(16, 185, 129, 0.18), rgba(5, 150, 105, 0.22));
  border: 1px solid rgba(16, 185, 129, 0.4);
  color: #059669;
  padding: 6px 16px;
  border-radius: 20px;
  font-size: 11px;
  font-weight: 700;
  display: flex;
  align-items: center;
  gap: 8px;
  box-shadow: 0 4px 14px rgba(16, 185, 129, 0.15);
  z-index: 50;
  animation: pulseIn 0.3s ease;
  @keyframes pulseIn {
    from { transform: translateY(-10px); opacity: 0; }
    to { transform: none; opacity: 1; }
  }
`

const BannerDot = styled.div`
  width: 8px;
  height: 8px;
  border-radius: 50%;
  background: #10b981;
  box-shadow: 0 0 8px #10b981;
  animation: blinkDot 1.2s infinite;
  @keyframes blinkDot {
    0%, 100% { opacity: 1; }
    50% { opacity: 0.3; }
  }
`

const WatchOuter = styled.div`
  position: relative;
  padding: 12px;
  transition: all 0.3s ease;
`

const WatchBezel = styled.div`
  position: relative;
  background: #1e2433;
  border-radius: 42px;
  padding: 20px 16px;
  box-shadow:
    ${p => p.$testMode
      ? '0 0 0 3px #10b981, 0 10px 40px rgba(16,185,129,0.3), inset 0 2px 6px rgba(255,255,255,0.1)'
      : '0 20px 60px rgba(0,0,0,0.45), 0 4px 12px rgba(0,0,0,0.3), inset 0 2px 6px rgba(255,255,255,0.08)'};
  display: flex;
  align-items: center;
  justify-content: center;
  transition: box-shadow 0.3s ease;
`

const SideButton = styled.div`
  position: absolute;
  right: -8px;
  width: 8px;
  height: 32px;
  background: linear-gradient(to right, #2d3748, #4a5568);
  border-radius: 0 4px 4px 0;
  box-shadow: 2px 2px 5px rgba(0,0,0,0.4);
`

const Screen = styled.div`
  border-radius: 24px;
  position: relative;
  overflow: hidden;
  box-shadow: inset 0 0 14px rgba(0,0,0,0.7);
  user-select: none;
`

const ScrollBadgeTitle = styled.div`
  position: absolute;
  top: 8px;
  right: 12px;
  background: rgba(15, 23, 42, 0.75);
  border: 1px solid rgba(56, 189, 248, 0.4);
  color: #38bdf8;
  font-size: 8px;
  font-weight: 800;
  padding: 2px 6px;
  border-radius: 10px;
  backdrop-filter: blur(4px);
  z-index: 50;
  pointer-events: none;
`

const ScreenContent = styled.div`
  position: relative;
  transition: transform 0.25s cubic-bezier(0.2, 0.8, 0.2, 1);
  ${p => p.$anim === 'slideIn' && `
    animation: slideScreen 0.3s cubic-bezier(0.2, 0.8, 0.2, 1);
    @keyframes slideScreen {
      from { transform: translateX(30px); opacity: 0.5; }
      to { transform: none; opacity: 1; }
    }
  `}
  ${p => p.$anim === 'slideLeft' && `
    animation: slideLeftAnim 0.3s cubic-bezier(0.2, 0.8, 0.2, 1);
    @keyframes slideLeftAnim {
      from { transform: translateX(60px); opacity: 0.3; }
      to { transform: none; opacity: 1; }
    }
  `}
  ${p => p.$anim === 'slideRight' && `
    animation: slideRightAnim 0.3s cubic-bezier(0.2, 0.8, 0.2, 1);
    @keyframes slideRightAnim {
      from { transform: translateX(-60px); opacity: 0.3; }
      to { transform: none; opacity: 1; }
    }
  `}
`

const ElementWrap = styled.div`
  position: absolute;
  border-radius: 4px;
  border: ${p => p.$selected ? '1.5px solid var(--accent-blue)' : '1px solid transparent'};
  box-shadow: ${p => p.$selected ? '0 0 0 1px var(--accent-blue)66' : 'none'};
  transition: border 0.12s, box-shadow 0.12s, transform 0.15s;

  ${p => p.$testMode && p.$hasActions && `
    &:hover {
      filter: brightness(1.15) drop-shadow(0 0 8px rgba(16, 185, 129, 0.6));
      transform: scale(1.02);
    }
    &:active {
      transform: scale(0.97);
    }
  `}
`

const ScaledContent = styled.div`
  position: absolute;
  top: 0;
  left: 0;
`

const ActionIndicatorBadge = styled.div`
  position: absolute;
  top: -8px;
  right: -8px;
  background: linear-gradient(135deg, #10b981, #059669);
  color: #fff;
  font-size: 8px;
  font-weight: 900;
  padding: 1px 4px;
  border-radius: 8px;
  box-shadow: 0 2px 6px rgba(0,0,0,0.3);
  z-index: 20;
  pointer-events: none;
`

const ResizeHandle = styled.div`
  position: absolute;
  bottom: -4px;
  right: -4px;
  background: var(--accent-blue);
  border: 1.5px solid #fff;
  border-radius: 2px;
  cursor: se-resize;
  z-index: 10;
  box-shadow: 0 1px 4px rgba(0,0,0,0.3);
`

const WireframePlaceholder = styled.div`
  width: 100%;
  height: 100%;
  border: 1.5px dashed ${p => p.$colors.stroke};
  background: ${p => p.$colors.fill};
  border-radius: 6px;
  padding: 4px;
  box-sizing: border-box;
  display: flex;
  flex-direction: column;
  justify-content: space-between;
  overflow: hidden;
`

const WireframeHeader = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
`

const WireframeType = styled.span`
  font-size: 8px;
  font-weight: 800;
  color: var(--text-primary);
  text-transform: uppercase;
`

const WireframeDims = styled.span`
  font-size: 7px;
  font-family: var(--font-mono);
  color: var(--text-muted);
`

const WireframeLabel = styled.div`
  font-size: 9px;
  font-weight: 700;
  color: var(--text-secondary);
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
`

const WatchAlertOverlay = styled.div`
  position: absolute;
  inset: 0;
  background: rgba(0, 0, 0, 0.6);
  backdrop-filter: blur(4px);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 100;
  padding: 16px;
  cursor: pointer;
  animation: fadeIn 0.18s ease;
  @keyframes fadeIn {
    from { opacity: 0; }
    to { opacity: 1; }
  }
`

const AlertBox = styled.div`
  background: #1e293b;
  border: 1.5px solid #10b981;
  border-radius: 14px;
  padding: 14px;
  width: 100%;
  box-shadow: 0 10px 25px rgba(0,0,0,0.5), 0 0 15px rgba(16,185,129,0.3);
  text-align: center;
`

const AlertHeader = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  font-size: 9px;
  font-weight: 800;
  color: #10b981;
  letter-spacing: 0.5px;
  margin-bottom: 8px;
`

const DismissBadge = styled.span`
  font-size: 8px;
  color: #94a3b8;
`

const AlertMessage = styled.div`
  font-size: 12px;
  font-weight: 700;
  color: #ffffff;
  line-height: 1.4;
`

const WatchScrollTrack = styled.div`
  position: absolute;
  top: 20px;
  bottom: 20px;
  right: 4px;
  width: 3px;
  border-radius: 3px;
  background: rgba(255,255,255,0.1);
  pointer-events: none;
`

const WatchScrollThumb = styled.div`
  width: 100%;
  height: 24px;
  border-radius: 3px;
  background: #10b981;
  box-shadow: 0 0 6px #10b981;
  transition: transform 0.1s ease-out;
`

const HUD = styled.div`
  display: flex;
  align-items: center;
  gap: 16px;
  background: var(--bg-raised);
  padding: 6px 16px;
  border-radius: 20px;
  box-shadow: var(--neu-button);
  margin-top: 14px;
  border: 1px solid var(--border-subtle);
`

const HUDItem = styled.span`
  font-size: 11px;
  color: var(--text-muted);
  strong {
    color: var(--text-primary);
  }
`

const TestControlsFloating = styled.div`
  position: absolute;
  bottom: 16px;
  background: var(--bg-surface);
  border: 1.5px solid rgba(16,185,129,0.4);
  border-radius: var(--radius-md);
  box-shadow: 0 10px 30px rgba(0,0,0,0.2), var(--neu-raised);
  padding: 10px 16px;
  display: flex;
  flex-direction: column;
  gap: 8px;
  z-index: 80;
  animation: slideUp 0.2s ease;
  @keyframes slideUp {
    from { transform: translateY(15px); opacity: 0; }
    to { transform: none; opacity: 1; }
  }
`

const TestControlsHeader = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 12px;
`

const TestControlsTitle = styled.span`
  font-size: 11px;
  font-weight: 800;
  color: #10b981;
  letter-spacing: 0.5px;
`

const ExitTestBtn = styled.button`
  border: none;
  background: rgba(239,68,68,0.1);
  color: #ef4444;
  font-size: 10px;
  font-weight: 700;
  padding: 2px 8px;
  border-radius: 4px;
  cursor: pointer;
  &:hover {
    background: #ef4444;
    color: #fff;
  }
`

const TestControlsRow = styled.div`
  display: flex;
  align-items: center;
  gap: 8px;
`

const TestBtn = styled.button`
  border: none;
  background: var(--bg-raised);
  padding: 5px 12px;
  border-radius: var(--radius-sm);
  font-size: 11px;
  font-weight: 700;
  color: var(--text-secondary);
  box-shadow: var(--neu-button);
  cursor: pointer;
  transition: all 0.15s;
  &:hover {
    color: #10b981;
    transform: translateY(-1px);
  }
`

const ActionLogBox = styled.div`
  background: var(--bg-inset);
  border-radius: 4px;
  padding: 4px 8px;
  display: flex;
  align-items: center;
  gap: 6px;
  box-shadow: var(--neu-inset);
`

const LogHeader = styled.span`
  font-size: 8px;
  font-weight: 800;
  color: var(--text-muted);
`

const LogText = styled.span`
  font-size: 10px;
  font-weight: 600;
  color: #10b981;
  font-family: var(--font-mono);
`
