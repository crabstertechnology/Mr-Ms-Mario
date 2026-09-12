import React, { useState } from 'react'
import styled from 'styled-components'

export default function ScreenInspectorPanel({
  screen,
  screens,
  onUpdateScreen,
  onAddScreen,
  onTestSwipe
}) {
  if (!screen) return null

  const [bgTab, setBgTab] = useState(screen.bgType || 'color')

  const isScrollable = screen.isScrollable || false
  const maxScrollY = screen.maxScrollY || 400
  const gestures = screen.gestures || {
    swipeLeft: { actionType: 'none', targetScreenId: null, transition: 'slide-left' },
    swipeRight: { actionType: 'none', targetScreenId: null, transition: 'slide-right' }
  }

  const setScreenProp = (prop, val) => {
    onUpdateScreen(screen.id, { [prop]: val })
  }

  const updateGesture = (gestureKey, patch) => {
    const nextGestures = {
      ...gestures,
      [gestureKey]: {
        ...(gestures[gestureKey] || { actionType: 'none', targetScreenId: null }),
        ...patch
      }
    }
    onUpdateScreen(screen.id, { gestures: nextGestures })
  }

  const handleFileUpload = (e) => {
    const file = e.target.files?.[0]
    if (!file) return
    const reader = new FileReader()
    reader.onload = (event) => {
      const dataUrl = event.target.result
      onUpdateScreen(screen.id, {
        bgType: 'image',
        bgImage: dataUrl
      })
      setBgTab('image')
    }
    reader.readAsDataURL(file)
  }

  const SOLID_PRESETS = [
    { name: 'Cyber Black', hex: '#0b0f19' },
    { name: 'Deep Slate', hex: '#0f172a' },
    { name: 'Midnight Navy', hex: '#070d1e' },
    { name: 'Arcade Violet', hex: '#0b0014' },
    { name: 'Obsidian', hex: '#18181b' },
    { name: 'Clean White', hex: '#ffffff' },
    { name: 'Cyber Emerald', hex: '#022c22' },
    { name: 'Crimson Red', hex: '#3b0712' },
  ]

  const GRADIENT_PRESETS = [
    {
      name: 'Cyber Dark',
      css: 'radial-gradient(ellipse at 50% 30%, #0d1829 0%, #06080f 60%, #000000 100%)',
    },
    {
      name: 'Neon Aurora',
      css: 'linear-gradient(180deg, #0f172a 0%, #022c22 50%, #0b0f19 100%)',
    },
    {
      name: 'Deep Ocean',
      css: 'linear-gradient(180deg, #0284c7 0%, #0369a1 40%, #082f49 100%)',
    },
    {
      name: 'Cyber Sunset',
      css: 'linear-gradient(180deg, #831843 0%, #3b0764 50%, #09090b 100%)',
    },
    {
      name: 'Midnight Purple',
      css: 'radial-gradient(ellipse at 50% 20%, #2e1065 0%, #0f051d 70%, #000000 100%)',
    },
    {
      name: 'Synthwave',
      css: 'linear-gradient(180deg, #1e1b4b 0%, #311042 50%, #050505 100%)',
    },
  ]

  const PATTERN_PRESETS = [
    { id: 'none', name: 'None (Plain)' },
    { id: 'stars', name: 'Cosmic Starfield ✨ (Parallax Animated)' },
    { id: 'grid', name: 'Cyber Grid (16px)' },
    { id: 'dots', name: 'Dot Matrix (12px)' },
    { id: 'scanlines', name: 'CRT Scanlines' },
    { id: 'carbon', name: 'Carbon Fiber' },
    { id: 'hex', name: 'Hexagon Mesh' },
  ]

  return (
    <PanelContainer>
      {/* ── Screen Identification ── */}
      <Section>
        <SectionTitle>
          <span>🖥️ Screen Settings</span>
          <ScreenBadge>240×280 Portrait</ScreenBadge>
        </SectionTitle>
        <PropRow>
          <PropLabel>Screen Name</PropLabel>
          <TextInput
            value={screen.name || ''}
            onChange={e => setScreenProp('name', e.target.value)}
            placeholder="e.g. Home Screen"
          />
        </PropRow>
      </Section>

      {/* ── 1. Full-Display Scrolling (User Requested) ── */}
      <Section>
        <SectionTitle>
          <span>📜 Full-Display Scrolling</span>
          <ToggleBadge $active={isScrollable}>
            {isScrollable ? 'ENABLED' : 'DISABLED'}
          </ToggleBadge>
        </SectionTitle>
        <HintText>
          When enabled, the entire display is scrollable vertically by dragging anywhere on the screen (no buttons required!).
        </HintText>

        <ToggleRow>
          <ToggleLabel>
            <strong>Enable Display Scroll</strong>
            <span>Allows scrolling through content taller than 280px</span>
          </ToggleLabel>
          <SwitchBtn
            $checked={isScrollable}
            onClick={() => setScreenProp('isScrollable', !isScrollable)}
          >
            <SwitchKnob $checked={isScrollable} />
          </SwitchBtn>
        </ToggleRow>

        {isScrollable && (
          <ScrollOptionsBox>
            <PropRow>
              <PropLabel>Max Scroll (px)</PropLabel>
              <NumInput
                type="number"
                min="280"
                max="1200"
                step="20"
                value={maxScrollY}
                onChange={e => setScreenProp('maxScrollY', parseInt(e.target.value) || 280)}
              />
            </PropRow>
            <PresetChips>
              {[280, 420, 560, 700].map(h => (
                <Chip
                  key={h}
                  $active={maxScrollY === h}
                  onClick={() => setScreenProp('maxScrollY', h)}
                >
                  {h}px ({Math.round((h / 280) * 10) / 10}x)
                </Chip>
              ))}
            </PresetChips>
          </ScrollOptionsBox>
        )}
      </Section>

      {/* ── 2. Screen-Level Swipe Gestures (User Requested) ── */}
      <Section>
        <SectionTitle>
          <span>👉 Screen Gestures (Swipe Navigation)</span>
        </SectionTitle>
        <HintText>
          Swipe left/right anywhere on the 1.69" display to navigate between screens.
        </HintText>

        {/* Swipe Left (←) */}
        <GestureCard>
          <GestureHeader>
            <GestureIcon>👈</GestureIcon>
            <GestureTitle>
              <strong>Swipe Left (←)</strong>
              <span>Finger drags right-to-left</span>
            </GestureTitle>
            <GestureSelect
              value={gestures.swipeLeft?.actionType || 'none'}
              onChange={e => updateGesture('swipeLeft', { actionType: e.target.value })}
            >
              <option value="none">No Action</option>
              <option value="navigate">🚀 Navigate to Screen</option>
              <option value="alert">🔔 Show Alert</option>
            </GestureSelect>
          </GestureHeader>

          {gestures.swipeLeft?.actionType === 'navigate' && (
            <GestureBody>
              <PropRow>
                <PropLabel>Target Screen</PropLabel>
                <Select
                  value={gestures.swipeLeft?.targetScreenId || ''}
                  onChange={e => updateGesture('swipeLeft', { targetScreenId: e.target.value })}
                >
                  <option value="">-- Choose Screen --</option>
                  {screens.map(s => (
                    <option key={s.id} value={s.id}>
                      {s.name} {s.id === screen.id ? '(Current)' : ''}
                    </option>
                  ))}
                </Select>
              </PropRow>
              <TestGestureBtn
                onClick={() => onTestSwipe && onTestSwipe('left', gestures.swipeLeft)}
              >
                🎮 Test Swipe Left
              </TestGestureBtn>
            </GestureBody>
          )}
        </GestureCard>

        {/* Swipe Right (→) */}
        <GestureCard>
          <GestureHeader>
            <GestureIcon>👉</GestureIcon>
            <GestureTitle>
              <strong>Swipe Right (→)</strong>
              <span>Finger drags left-to-right</span>
            </GestureTitle>
            <GestureSelect
              value={gestures.swipeRight?.actionType || 'none'}
              onChange={e => updateGesture('swipeRight', { actionType: e.target.value })}
            >
              <option value="none">No Action</option>
              <option value="navigate">🚀 Navigate to Screen</option>
              <option value="alert">🔔 Show Alert</option>
            </GestureSelect>
          </GestureHeader>

          {gestures.swipeRight?.actionType === 'navigate' && (
            <GestureBody>
              <PropRow>
                <PropLabel>Target Screen</PropLabel>
                <Select
                  value={gestures.swipeRight?.targetScreenId || ''}
                  onChange={e => updateGesture('swipeRight', { targetScreenId: e.target.value })}
                >
                  <option value="">-- Choose Screen --</option>
                  {screens.map(s => (
                    <option key={s.id} value={s.id}>
                      {s.name} {s.id === screen.id ? '(Current)' : ''}
                    </option>
                  ))}
                </Select>
              </PropRow>
              <TestGestureBtn
                onClick={() => onTestSwipe && onTestSwipe('right', gestures.swipeRight)}
              >
                🎮 Test Swipe Right
              </TestGestureBtn>
            </GestureBody>
          )}
        </GestureCard>
      </Section>

      {/* ── 3. Screen Backgrounds & Patterns (User Requested) ── */}
      <Section>
        <SectionTitle>
          <span>🎨 Background & Theme</span>
        </SectionTitle>

        <BgTypeTabs>
          {[
            { id: 'color', label: 'Color' },
            { id: 'gradient', label: 'Gradient' },
            { id: 'pattern', label: 'Pattern' },
            { id: 'image', label: '🖼️ Image' },
          ].map(t => (
            <BgTypeTab
              key={t.id}
              $active={bgTab === t.id}
              onClick={() => {
                setBgTab(t.id)
                setScreenProp('bgType', t.id)
              }}
            >
              {t.label}
            </BgTypeTab>
          ))}
        </BgTypeTabs>

        {/* Solid Color Tab */}
        {bgTab === 'color' && (
          <div>
            <PropRow>
              <PropLabel>Color (HEX)</PropLabel>
              <ColorPickerWrap>
                <ColorInput
                  type="color"
                  value={screen.bgColor?.startsWith('#') ? screen.bgColor : '#0b0f19'}
                  onChange={e => setScreenProp('bgColor', e.target.value)}
                />
                <TextInput
                  value={screen.bgColor || '#0b0f19'}
                  onChange={e => setScreenProp('bgColor', e.target.value)}
                />
              </ColorPickerWrap>
            </PropRow>

            <SwatchesGrid>
              {SOLID_PRESETS.map(c => (
                <Swatch
                  key={c.hex}
                  style={{ background: c.hex }}
                  $active={screen.bgColor === c.hex}
                  title={c.name}
                  onClick={() => setScreenProp('bgColor', c.hex)}
                />
              ))}
            </SwatchesGrid>
          </div>
        )}

        {/* Gradient Tab */}
        {bgTab === 'gradient' && (
          <GradientGrid>
            {GRADIENT_PRESETS.map(g => (
              <GradientCard
                key={g.name}
                style={{ background: g.css }}
                $active={screen.bgGradient === g.css}
                onClick={() => {
                  onUpdateScreen(screen.id, {
                    bgType: 'gradient',
                    bgGradient: g.css
                  })
                }}
              >
                <GradientLabel>{g.name}</GradientLabel>
              </GradientCard>
            ))}
          </GradientGrid>
        )}

        {/* Pattern Tab */}
        {bgTab === 'pattern' && (
          <div>
            <PropRow>
              <PropLabel>Base Color</PropLabel>
              <ColorPickerWrap>
                <ColorInput
                  type="color"
                  value={screen.bgColor || '#0b0f19'}
                  onChange={e => setScreenProp('bgColor', e.target.value)}
                />
                <TextInput
                  value={screen.bgColor || '#0b0f19'}
                  onChange={e => setScreenProp('bgColor', e.target.value)}
                />
              </ColorPickerWrap>
            </PropRow>

            <PatternList>
              {PATTERN_PRESETS.map(p => (
                <PatternItem
                  key={p.id}
                  $active={(screen.bgPattern || 'none') === p.id}
                  onClick={() => {
                    onUpdateScreen(screen.id, {
                      bgType: 'pattern',
                      bgPattern: p.id
                    })
                  }}
                >
                  <span>{p.name}</span>
                  {(screen.bgPattern || 'none') === p.id && <CheckMark>✓</CheckMark>}
                </PatternItem>
              ))}
            </PatternList>
          </div>
        )}

        {/* Custom Image / Wallpaper Tab */}
        {bgTab === 'image' && (
          <div>
            <UploadBox>
              <UploadLabel>
                <span>📁 Upload Image / Wallpaper</span>
                <input
                  type="file"
                  accept="image/png, image/jpeg, image/webp"
                  onChange={handleFileUpload}
                  style={{ display: 'none' }}
                />
              </UploadLabel>
              <UploadHint>Supports PNG, JPG, WebP (auto-scaled to 240×280)</UploadHint>
            </UploadBox>

            {screen.bgImage && (
              <ImagePreviewBox>
                <img
                  src={screen.bgImage}
                  alt="Wallpaper preview"
                  style={{ width: '100%', height: 140, objectFit: 'cover', borderRadius: 8 }}
                />
                <ClearImgBtn
                  onClick={() => onUpdateScreen(screen.id, { bgImage: '', bgType: 'color' })}
                >
                  ✕ Remove Image
                </ClearImgBtn>
              </ImagePreviewBox>
            )}
          </div>
        )}
      </Section>
    </PanelContainer>
  )
}

const PanelContainer = styled.div`
  padding: 16px;
  display: flex;
  flex-direction: column;
  gap: 18px;
`

const Section = styled.div`
  display: flex;
  flex-direction: column;
  gap: 10px;
  padding-bottom: 16px;
  border-bottom: 1px solid var(--border-subtle);
`

const SectionTitle = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  font-size: 12px;
  font-weight: 800;
  color: var(--text-primary);
  text-transform: uppercase;
  letter-spacing: 0.5px;
`

const ScreenBadge = styled.span`
  font-size: 9px;
  font-weight: 700;
  padding: 2px 6px;
  border-radius: 4px;
  background: rgba(14, 165, 233, 0.15);
  color: #38bdf8;
  border: 1px solid rgba(14, 165, 233, 0.3);
`

const ToggleBadge = styled.span`
  font-size: 9px;
  font-weight: 800;
  padding: 2px 6px;
  border-radius: 4px;
  background: ${p => p.$active ? 'rgba(34, 197, 94, 0.15)' : 'rgba(100, 116, 139, 0.15)'};
  color: ${p => p.$active ? '#4ade80' : '#94a3b8'};
  border: 1px solid ${p => p.$active ? 'rgba(34, 197, 94, 0.3)' : 'rgba(100, 116, 139, 0.2)'};
`

const HintText = styled.div`
  font-size: 11px;
  color: var(--text-muted);
  line-height: 1.4;
`

const PropRow = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 8px;
`

const PropLabel = styled.span`
  font-size: 11px;
  font-weight: 600;
  color: var(--text-secondary);
`

const TextInput = styled.input`
  flex: 1;
  max-width: 150px;
  padding: 6px 10px;
  border-radius: var(--radius-sm);
  border: 1px solid var(--border-subtle);
  background: var(--bg-inset);
  color: var(--text-primary);
  font-size: 11px;
  outline: none;
  &:focus { border-color: var(--accent-blue); }
`

const NumInput = styled.input`
  width: 90px;
  padding: 6px 10px;
  border-radius: var(--radius-sm);
  border: 1px solid var(--border-subtle);
  background: var(--bg-inset);
  color: var(--text-primary);
  font-size: 11px;
  text-align: right;
  outline: none;
  &:focus { border-color: var(--accent-blue); }
`

const ToggleRow = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 10px 12px;
  background: var(--bg-surface);
  border-radius: var(--radius-md);
  border: 1px solid var(--border-subtle);
`

const ToggleLabel = styled.div`
  display: flex;
  flex-direction: column;
  gap: 2px;
  strong { font-size: 12px; color: var(--text-primary); }
  span { font-size: 10px; color: var(--text-muted); }
`

const SwitchBtn = styled.div`
  width: 44px;
  height: 24px;
  border-radius: 12px;
  background: ${p => p.$checked ? 'var(--accent-blue)' : 'var(--bg-inset)'};
  border: 1.5px solid ${p => p.$checked ? 'var(--accent-blue)' : 'var(--border-subtle)'};
  position: relative;
  cursor: pointer;
  transition: all 0.2s;
  flex-shrink: 0;
`

const SwitchKnob = styled.div`
  width: 16px;
  height: 16px;
  border-radius: 50%;
  background: #fff;
  position: absolute;
  top: 2.5px;
  left: ${p => p.$checked ? '22px' : '3px'};
  transition: left 0.2s;
  box-shadow: 0 1px 4px rgba(0,0,0,0.3);
`

const ScrollOptionsBox = styled.div`
  display: flex;
  flex-direction: column;
  gap: 10px;
  padding: 10px 12px;
  background: var(--bg-inset);
  border-radius: var(--radius-sm);
  border: 1px dashed var(--border-subtle);
`

const PresetChips = styled.div`
  display: flex;
  gap: 6px;
`

const Chip = styled.button`
  flex: 1;
  padding: 4px 6px;
  border-radius: 4px;
  border: 1px solid ${p => p.$active ? 'var(--accent-blue)' : 'var(--border-subtle)'};
  background: ${p => p.$active ? 'var(--accent-blue)22' : 'var(--bg-surface)'};
  color: ${p => p.$active ? 'var(--accent-blue)' : 'var(--text-secondary)'};
  font-size: 10px;
  font-weight: 700;
  cursor: pointer;
`

const GestureCard = styled.div`
  display: flex;
  flex-direction: column;
  gap: 8px;
  padding: 10px 12px;
  background: var(--bg-surface);
  border-radius: var(--radius-md);
  border: 1px solid var(--border-subtle);
`

const GestureHeader = styled.div`
  display: flex;
  align-items: center;
  gap: 8px;
`

const GestureIcon = styled.span`
  font-size: 16px;
`

const GestureTitle = styled.div`
  flex: 1;
  display: flex;
  flex-direction: column;
  strong { font-size: 11px; color: var(--text-primary); }
  span { font-size: 9px; color: var(--text-muted); }
`

const GestureSelect = styled.select`
  padding: 4px 8px;
  border-radius: 4px;
  background: var(--bg-inset);
  border: 1px solid var(--border-subtle);
  color: var(--text-primary);
  font-size: 10px;
  font-weight: 600;
`

const GestureBody = styled.div`
  display: flex;
  flex-direction: column;
  gap: 8px;
  padding-top: 6px;
  border-top: 1px dashed var(--border-subtle);
`

const Select = styled.select`
  flex: 1;
  max-width: 140px;
  padding: 4px 8px;
  border-radius: 4px;
  background: var(--bg-inset);
  border: 1px solid var(--border-subtle);
  color: var(--text-primary);
  font-size: 10px;
`

const TestGestureBtn = styled.button`
  padding: 6px;
  border-radius: 4px;
  border: none;
  background: rgba(56, 189, 248, 0.15);
  color: #38bdf8;
  font-size: 10px;
  font-weight: 700;
  cursor: pointer;
  &:hover { background: rgba(56, 189, 248, 0.25); }
`

const BgTypeTabs = styled.div`
  display: flex;
  gap: 4px;
  background: var(--bg-inset);
  padding: 3px;
  border-radius: var(--radius-sm);
`

const BgTypeTab = styled.button`
  flex: 1;
  padding: 5px;
  border: none;
  border-radius: 4px;
  font-size: 10px;
  font-weight: 700;
  cursor: pointer;
  background: ${p => p.$active ? 'var(--bg-raised)' : 'transparent'};
  color: ${p => p.$active ? 'var(--accent-blue)' : 'var(--text-muted)'};
  box-shadow: ${p => p.$active ? 'var(--neu-raised)' : 'none'};
`

const ColorPickerWrap = styled.div`
  display: flex;
  align-items: center;
  gap: 6px;
`

const ColorInput = styled.input`
  width: 24px;
  height: 24px;
  padding: 0;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  background: none;
`

const SwatchesGrid = styled.div`
  display: grid;
  grid-template-columns: repeat(8, 1fr);
  gap: 4px;
  margin-top: 8px;
`

const Swatch = styled.div`
  height: 22px;
  border-radius: 4px;
  cursor: pointer;
  border: 1.5px solid ${p => p.$active ? '#fff' : 'rgba(255,255,255,0.1)'};
  box-shadow: ${p => p.$active ? '0 0 8px rgba(255,255,255,0.4)' : 'none'};
  transition: transform 0.1s;
  &:hover { transform: scale(1.1); }
`

const GradientGrid = styled.div`
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 8px;
`

const GradientCard = styled.div`
  height: 48px;
  border-radius: 6px;
  display: flex;
  align-items: flex-end;
  padding: 6px 8px;
  cursor: pointer;
  border: 1.5px solid ${p => p.$active ? '#38bdf8' : 'rgba(255,255,255,0.1)'};
  box-shadow: ${p => p.$active ? '0 0 10px rgba(56, 189, 248, 0.4)' : 'none'};
  transition: transform 0.1s;
  &:hover { transform: scale(1.02); }
`

const GradientLabel = styled.span`
  font-size: 9px;
  font-weight: 700;
  color: #fff;
  text-shadow: 0 1px 3px rgba(0,0,0,0.8);
`

const PatternList = styled.div`
  display: flex;
  flex-direction: column;
  gap: 4px;
  margin-top: 8px;
`

const PatternItem = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 6px 10px;
  border-radius: 4px;
  background: ${p => p.$active ? 'rgba(56, 189, 248, 0.12)' : 'var(--bg-inset)'};
  border: 1px solid ${p => p.$active ? '#38bdf8' : 'var(--border-subtle)'};
  color: ${p => p.$active ? '#38bdf8' : 'var(--text-secondary)'};
  font-size: 11px;
  font-weight: 600;
  cursor: pointer;
`

const CheckMark = styled.span`
  font-weight: 900;
`

const UploadBox = styled.div`
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 16px;
  background: var(--bg-inset);
  border-radius: var(--radius-md);
  border: 1.5px dashed var(--border-subtle);
  gap: 6px;
  text-align: center;
`

const UploadLabel = styled.label`
  padding: 8px 14px;
  border-radius: 6px;
  background: var(--accent-blue);
  color: #fff;
  font-size: 11px;
  font-weight: 700;
  cursor: pointer;
  transition: all 0.15s;
  &:hover { filter: brightness(1.1); }
`

const UploadHint = styled.span`
  font-size: 9px;
  color: var(--text-muted);
`

const ImagePreviewBox = styled.div`
  margin-top: 10px;
  position: relative;
`

const ClearImgBtn = styled.button`
  position: absolute;
  top: 8px;
  right: 8px;
  background: rgba(0,0,0,0.7);
  color: #ef4444;
  border: 1px solid rgba(239, 68, 68, 0.5);
  font-size: 10px;
  font-weight: 700;
  padding: 3px 8px;
  border-radius: 4px;
  cursor: pointer;
  &:hover { background: #ef4444; color: #fff; }
`
