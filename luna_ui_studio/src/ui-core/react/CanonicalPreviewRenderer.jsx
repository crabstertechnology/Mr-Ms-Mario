/**
 * Luna UI Studio — Canonical Preview Renderer (React)
 *
 * Implements the semantic rendering contract directly from Canonical UIProject / UINode.
 * Completely independent of legacy element definitions, legacy props, or UIverse HTML snippets.
 *
 * Serves as the web-side reference renderer for visual parity testing against the ESP32 embedded renderer.
 */

import React, { useState } from 'react'
import styled, { keyframes } from 'styled-components'

const spinAnimation = keyframes`
  0% { transform: rotate(0deg); }
  100% { transform: rotate(360deg); }
`

const PreviewViewport = styled.div`
  position: relative;
  width: ${(props) => props.$width || 240}px;
  height: ${(props) => props.$height || 280}px;
  overflow: hidden;
  background-color: ${(props) => props.$bgColor || '#060a12'};
  user-select: none;
  font-family: 'Outfit', sans-serif;
  box-shadow: 0 12px 40px rgba(0, 0, 0, 0.5);
  border-radius: 12px;
  border: 1px solid rgba(255, 255, 255, 0.1);
`

const StarDot = styled.div`
  position: absolute;
  width: 2px;
  height: 2px;
  border-radius: 50%;
  background: ${(props) => props.$color || '#fff'};
  left: ${(props) => props.$x}px;
  top: ${(props) => props.$y}px;
`

const SpinnerContainer = styled.div`
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  width: 100%;
  height: 100%;
`

const SpinnerRing = styled.div`
  width: ${(props) => props.$size || 30}px;
  height: ${(props) => props.$size || 30}px;
  border-radius: 50%;
  border: 3px solid ${(props) => props.$bgColor || '#1e293b'};
  border-top-color: ${(props) => props.$accentColor || '#00f2fe'};
  animation: ${spinAnimation} ${(props) => props.$durationMs || 1200}ms linear infinite;
`

/**
 * Renders a single canonical UINode.
 */
export function CanonicalNodeRenderer({ node, onEvent }) {
  const [isPressed, setIsPressed] = useState(false)

  const layout = node.layout || { x: 0, y: 0, width: 80, height: 40 }
  const style = node.style || {}
  const typography = node.typography || {}
  const props = node.properties || {}
  const events = node.events || []
  const anims = node.animations || []

  const pressedStyle = node.states?.pressed?.style || {}
  const activeBg = isPressed && pressedStyle.backgroundColor ? pressedStyle.backgroundColor : style.backgroundColor

  const handleClick = () => {
    const clickEvt = events.find((e) => e.trigger === 'onClick' || !e.trigger)
    if (clickEvt && onEvent) {
      onEvent(clickEvt, node)
    }
  }

  // Common node wrapper positioning
  const nodeWrapperStyle = {
    position: 'absolute',
    left: `${layout.x}px`,
    top: `${layout.y}px`,
    width: `${layout.width}px`,
    height: `${layout.height}px`,
    boxSizing: 'border-box',
    display: 'flex',
    flexDirection: 'column',
    justifyContent: 'center',
    backgroundColor: activeBg || 'transparent',
    borderColor: style.borderColor || 'transparent',
    borderWidth: style.borderWidth ? `${style.borderWidth}px` : 0,
    borderStyle: style.borderWidth ? 'solid' : 'none',
    borderRadius: style.borderRadius ? `${style.borderRadius}px` : 0,
    opacity: style.opacity !== undefined ? style.opacity : 1,
    cursor: events.length > 0 ? 'pointer' : 'default',
    transition: 'transform 0.1s, background-color 0.15s',
    transform: isPressed ? 'scale(0.97)' : 'none',
  }

  // 1. Text Node
  if (node.type === 'custom_label' || node.type === 'digital_clock') {
    const textVal = props.text || props.timeStr || ''
    const dateVal = props.dateStr
    const textAlign = typography.align || 'center'
    const fontFamily = typography.fontFamily === 'JetBrains Mono' ? 'JetBrains Mono, monospace' : 'Outfit, sans-serif'
    return (
      <div style={nodeWrapperStyle}>
        <div
          style={{
            fontFamily,
            fontSize: `${typography.fontSize || 14}px`,
            fontWeight: typography.fontWeight || 700,
            color: typography.color || '#fff',
            textAlign,
            width: '100%',
          }}
        >
          {textVal}
        </div>
        {dateVal && (
          <div
            style={{
              fontFamily,
              fontSize: '11px',
              color: props.dateColor || '#94a3b8',
              textAlign,
              marginTop: '4px',
            }}
          >
            {dateVal}
          </div>
        )}
      </div>
    )
  }

  // 2. Card Node
  if (node.type?.startsWith('card_')) {
    return (
      <div
        style={{
          ...nodeWrapperStyle,
          padding: '10px 14px',
          alignItems: 'flex-start',
        }}
      >
        {props.title && (
          <div
            style={{
              fontSize: `${typography.fontSize || 13}px`,
              fontWeight: typography.fontWeight || 700,
              color: typography.color || '#fff',
              marginBottom: '3px',
            }}
          >
            {props.title}
          </div>
        )}
        {props.subtitle && (
          <div
            style={{
              fontSize: '10px',
              color: props.subtextColor || '#94a3b8',
            }}
          >
            {props.subtitle}
          </div>
        )}
      </div>
    )
  }

  // 3. Button Node
  if (node.type?.includes('btn') || node.type?.includes('button')) {
    const label = props.label || props.text || 'Button'
    return (
      <div
        style={{
          ...nodeWrapperStyle,
          alignItems: 'center',
        }}
        onClick={handleClick}
        onMouseDown={() => setIsPressed(true)}
        onMouseUp={() => setIsPressed(false)}
        onMouseLeave={() => setIsPressed(false)}
      >
        <span
          style={{
            fontSize: `${typography.fontSize || 13}px`,
            fontWeight: typography.fontWeight || 700,
            color: typography.color || '#fff',
            textAlign: 'center',
          }}
        >
          {label}
        </span>
      </div>
    )
  }

  // 4. Spinner Node
  if (node.type === 'loader_spinner') {
    const anim = anims.find((a) => a.property === 'rotation') || {}
    const duration = anim.durationMs || 1200
    return (
      <div style={nodeWrapperStyle}>
        <SpinnerContainer>
          <SpinnerRing
            $size={props.size || 30}
            $bgColor={style.backgroundColor || '#1e293b'}
            $accentColor={style.accentColor || '#00f2fe'}
            $durationMs={duration}
          />
          {props.label && (
            <span
              style={{
                fontSize: `${typography.fontSize || 9}px`,
                color: typography.color || style.accentColor || '#00f2fe',
                marginTop: '6px',
                letterSpacing: '1px',
              }}
            >
              {props.label}
            </span>
          )}
        </SpinnerContainer>
      </div>
    )
  }

  // 5. Image Node
  if (node.type === 'image' || props.assetId) {
    const assetId = props.assetId || node.id
    const assetSrc = props.src || props.assetSrc
    return (
      <div
        style={{
          ...nodeWrapperStyle,
          alignItems: 'center',
          justifyContent: 'center',
          overflow: 'hidden',
        }}
      >
        {assetSrc ? (
          <img
            src={assetSrc}
            alt={assetId}
            style={{ width: '100%', height: '100%', objectFit: 'contain' }}
          />
        ) : (
          <div
            style={{
              width: '100%',
              height: '100%',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              background: style.backgroundColor || 'linear-gradient(135deg, #00f2fe22, #4facfe44)',
              border: `${style.borderWidth || 1}px solid ${style.borderColor || '#00f2fe'}`,
              borderRadius: `${style.borderRadius || 6}px`,
              color: '#38bdf8',
              fontSize: '10px',
              fontFamily: 'JetBrains Mono, monospace',
            }}
          >
            [{assetId}]
          </div>
        )}
      </div>
    )
  }

  // 6. Default Rectangle Container
  return <div style={nodeWrapperStyle} />
}

/**
 * Primary Canonical Preview Renderer Component.
 */
export default function CanonicalPreviewRenderer({ project, activeScreenId, onNavigate }) {
  if (!project || !Array.isArray(project.screens) || project.screens.length === 0) {
    return <div>No canonical screens to preview</div>
  }

  const screenId = activeScreenId || project.activeScreenId || project.screens[0].id
  const screen = project.screens.find((s) => s.id === screenId) || project.screens[0]

  const device = project.device || { width: 240, height: 280 }
  const bg = screen.background || { type: 'color', color: '#060a12' }
  const isStars = bg.pattern === 'stars'

  const stars = [
    { x: 24, y: 30, color: '#5AEB' },
    { x: 190, y: 45, color: '#FFFFFF' },
    { x: 80, y: 70, color: '#8410' },
    { x: 220, y: 110, color: '#6B6D' },
    { x: 35, y: 140, color: '#9CD3' },
    { x: 160, y: 160, color: '#4208' },
    { x: 95, y: 210, color: '#FFFFFF' },
    { x: 210, y: 230, color: '#5AEB' },
    { x: 50, y: 260, color: '#8410' },
    { x: 140, y: 275, color: '#6B6D' },
  ]

  const handleNodeEvent = (evt) => {
    if (evt.action?.type === 'navigate' && evt.action?.targetScreenId) {
      if (onNavigate) {
        onNavigate(evt.action.targetScreenId)
      }
    }
  }

  return (
    <PreviewViewport $width={device.width} $height={device.height} $bgColor={bg.color}>
      {isStars && stars.map((s, idx) => <StarDot key={idx} $x={s.x} $y={s.y} $color={s.color} />)}
      {(screen.children || []).map((node) => (
        <CanonicalNodeRenderer key={node.id} node={node} onEvent={handleNodeEvent} />
      ))}
    </PreviewViewport>
  )
}
