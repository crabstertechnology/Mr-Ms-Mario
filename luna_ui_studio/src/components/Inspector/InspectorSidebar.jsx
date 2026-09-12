import React, { useState, useEffect } from 'react'
import styled from 'styled-components'
import { hexToRGB565 } from '../../ui-elements/code-generator'
import ActionsMappingPanel from './ActionsMappingPanel'
import ScreenInspectorPanel from './ScreenInspectorPanel'

export default function InspectorSidebar({
  element,
  onUpdate,
  elements,
  selectedId,
  onSelect,
  onBringForward,
  onSendBackward,
  onDelete,
  screens = [],
  activeScreenId = 'screen_1',
  activeScreen,
  onUpdateScreen,
  onUpdateActions,
  onAddScreen,
  onTestTrigger,
  onTestSwipe
}) {
  const [activeTab, setActiveTab] = useState('screen')

  // When an element is selected, automatically show props tab; when deselected, show screen tab
  useEffect(() => {
    if (element) {
      setActiveTab('props')
    } else {
      setActiveTab('screen')
    }
  }, [element?.id])

  const hasActions = element?.actions && element.actions.length > 0
  const curScreen = activeScreen || screens.find(s => s.id === activeScreenId) || screens[0]

  return (
    <Inspector>
      <Tabs>
        <Tab $active={activeTab === 'screen'} onClick={() => setActiveTab('screen')}>
          🖥️ Screen
        </Tab>
        <Tab $active={activeTab === 'props'} onClick={() => setActiveTab('props')}>
          Properties
        </Tab>
        <Tab $active={activeTab === 'layers'} onClick={() => setActiveTab('layers')}>
          Layers
        </Tab>
        <Tab $active={activeTab === 'mapping'} onClick={() => setActiveTab('mapping')}>
          ⚡ Mapping {hasActions ? `(${element.actions.length})` : ''}
        </Tab>
      </Tabs>

      <TabContent>
        {activeTab === 'screen' ? (
          <ScreenInspectorPanel
            screen={curScreen}
            screens={screens}
            onUpdateScreen={onUpdateScreen}
            onAddScreen={onAddScreen}
            onTestSwipe={onTestSwipe}
          />
        ) : activeTab === 'props' ? (
          element ? (
            <PropertiesPanel
              element={element}
              onUpdate={onUpdate}
              curScreen={curScreen}
              onUpdateScreen={onUpdateScreen}
            />
          ) : (
            <ScreenInspectorPanel
              screen={curScreen}
              screens={screens}
              onUpdateScreen={onUpdateScreen}
              onAddScreen={onAddScreen}
              onTestSwipe={onTestSwipe}
            />
          )
        ) : activeTab === 'layers' ? (
          <LayersPanel
            elements={elements}
            selectedId={selectedId}
            onSelect={onSelect}
            onBringForward={onBringForward}
            onSendBackward={onSendBackward}
            onDelete={onDelete}
          />
        ) : (
          <ActionsMappingPanel
            element={element}
            screens={screens}
            activeScreenId={activeScreenId}
            onUpdateActions={onUpdateActions}
            onAddScreen={onAddScreen}
            onTestTrigger={onTestTrigger}
          />
        )}
      </TabContent>
    </Inspector>
  )
}

function PropertiesPanel({ element, onUpdate, curScreen, onUpdateScreen }) {
  if (!element) return (
    <EmptyState>
      <EmptyIcon>✦</EmptyIcon>
      <p>Select an element on the canvas to edit its properties.</p>
    </EmptyState>
  )

  const p = element.props
  const set = (key, val) => onUpdate(element.id, { [key]: val })
  const isPattern = element.type?.startsWith('pattern_')

  const colorKeys = Object.keys(p).filter(k => typeof p[k] === 'string' && (p[k].startsWith('#') || p[k].startsWith('rgba')))
  const textKeys = Object.keys(p).filter(k =>
    typeof p[k] === 'string' && !p[k].startsWith('#') && !p[k].startsWith('rgba')
  )
  const numberKeys = Object.keys(p).filter(k =>
    typeof p[k] === 'number' && k !== 'x' && k !== 'y' && k !== 'w' && k !== 'h'
  )

  return (
    <PanelScroll>
      <Section>
        <SectionTitle>Element Info</SectionTitle>
        <PropRow>
          <PropLabel>Type</PropLabel>
          <TypeBadge>{element.name}</TypeBadge>
        </PropRow>
      </Section>

      {isPattern && onUpdateScreen && curScreen && (
        <Section>
          <SectionTitle>Pattern Preset Action</SectionTitle>
          <PatternActionBtn
            onClick={() => {
              const pKey = element.type.replace('pattern_', '')
              const mapped = pKey === 'stars' ? 'stars'
                : pKey === 'cyber_grid' ? 'grid'
                : pKey === 'dot_matrix' ? 'dots'
                : pKey === 'crt_scanlines' ? 'scanlines'
                : pKey === 'carbon_fiber' ? 'carbon'
                : pKey === 'hexagon' ? 'hex'
                : 'stars'
              onUpdateScreen(curScreen.id, { bgType: 'pattern', bgPattern: mapped })
            }}
          >
            🌌 Set as Screen Background
          </PatternActionBtn>
          <HintSubtext>Applies this pattern directly as the full-screen display background.</HintSubtext>
        </Section>
      )}

      <Section>
        <SectionTitle>Geometry (pixels)</SectionTitle>
        <GeomGrid>
          {['x', 'y', 'w', 'h'].map(k => (
            <FieldBox key={k}>
              <FieldPrefix>{k.toUpperCase()}</FieldPrefix>
              <NumInput
                type="number"
                value={p[k] ?? 0}
                onChange={e => set(k, parseInt(e.target.value) || 0)}
              />
            </FieldBox>
          ))}
        </GeomGrid>
      </Section>

      {textKeys.length > 0 && (
        <Section>
          <SectionTitle>Content & Text</SectionTitle>
          {textKeys.map(k => (
            <PropRow key={k}>
              <PropLabel>{k}</PropLabel>
              <TextInput
                value={p[k]}
                onChange={e => set(k, e.target.value)}
              />
            </PropRow>
          ))}
          {typeof p.checked === 'boolean' && (
            <PropRow>
              <PropLabel>Checked</PropLabel>
              <input type="checkbox" checked={p.checked} onChange={e => set('checked', e.target.checked)} style={{ cursor: 'pointer' }} />
            </PropRow>
          )}
        </Section>
      )}

      {numberKeys.length > 0 && (
        <Section>
          <SectionTitle>Values</SectionTitle>
          {numberKeys.map(k => (
            <PropRow key={k}>
              <PropLabel>{k}</PropLabel>
              <NumInput
                type="number"
                value={p[k]}
                style={{ width: 80 }}
                onChange={e => set(k, parseInt(e.target.value) || 0)}
              />
            </PropRow>
          ))}
        </Section>
      )}

      {colorKeys.length > 0 && (
        <Section>
          <SectionTitle>Colors & RGB565</SectionTitle>
          {colorKeys.map(k => {
            const hex = p[k].startsWith('#') ? p[k] : '#00f2fe'
            const rgb565 = hexToRGB565(hex)
            return (
              <PropRow key={k} style={{ marginBottom: 8 }}>
                <PropLabel>{k}</PropLabel>
                <ColorRow>
                  <ColorSwatch style={{ background: hex }}>
                    <ColorInput
                      type="color"
                      value={hex}
                      onChange={e => set(k, e.target.value)}
                    />
                  </ColorSwatch>
                  <ColorHex>{hex}</ColorHex>
                  <RGB565Badge>{rgb565}</RGB565Badge>
                </ColorRow>
              </PropRow>
            )
          })}

          <SwatchesTitle>Preset Neon Palettes:</SwatchesTitle>
          <SwatchGrid>
            {['#00f2fe', '#4facfe', '#7928ca', '#ec4899', '#f59e0b', '#10b981', '#ef4444', '#ffffff', '#0f172a', '#ffff00'].map(c => (
              <Swatch key={c} style={{ background: c }} onClick={() => {
                if (colorKeys[0]) set(colorKeys[0], c)
              }} title={c} />
            ))}
          </SwatchGrid>
        </Section>
      )}
    </PanelScroll>
  )
}

function LayersPanel({ elements, selectedId, onSelect, onBringForward, onSendBackward, onDelete }) {
  if (elements.length === 0) return (
    <EmptyState><EmptyIcon>⊕</EmptyIcon><p>No layers yet. Add elements from the palette.</p></EmptyState>
  )
  return (
    <PanelScroll>
      {[...elements].reverse().map(el => (
        <LayerItem key={el.id} $active={el.id === selectedId} onClick={() => onSelect(el.id)}>
          <LayerLeft>
            <span style={{ color: 'var(--accent-blue)', fontSize: 11 }}>✦</span>
            <span style={{ fontSize: 12, fontWeight: 600 }}>{el.name}</span>
          </LayerLeft>
          <LayerActions onClick={e => e.stopPropagation()}>
            <LayerBtn onClick={onBringForward} title="Bring Forward">▲</LayerBtn>
            <LayerBtn onClick={onSendBackward} title="Send Backward">▼</LayerBtn>
            <LayerBtn onClick={() => { onSelect(el.id); onDelete(); }} title="Delete" style={{ color: '#ef4444' }}>✕</LayerBtn>
          </LayerActions>
        </LayerItem>
      ))}
    </PanelScroll>
  )
}

// ── Styled Components ──────────────────────────────────────

const Inspector = styled.aside`
  width: 240px;
  min-width: 240px;
  height: 100%;
  display: flex;
  flex-direction: column;
  border-left: 1px solid var(--border-subtle);
  background: var(--bg-base);
  box-shadow: -4px 0 16px rgba(0,0,0,0.06);
`

const Tabs = styled.div`
  display: flex;
  border-bottom: 1px solid var(--border-subtle);
  background: var(--bg-raised);
  padding: 8px 8px 0;
  gap: 4px;
`

const Tab = styled.button`
  flex: 1;
  padding: 8px 4px;
  border: none;
  border-radius: var(--radius-sm) var(--radius-sm) 0 0;
  font-size: 12px;
  font-weight: 700;
  cursor: pointer;
  transition: all 0.15s;
  background: ${p => p.$active ? 'var(--bg-base)' : 'transparent'};
  color: ${p => p.$active ? 'var(--accent-blue)' : 'var(--text-muted)'};
  box-shadow: ${p => p.$active ? 'var(--neu-raised)' : 'none'};
`

const TabContent = styled.div`
  flex: 1;
  overflow: hidden;
  display: flex;
  flex-direction: column;
`

const PanelScroll = styled.div`
  flex: 1;
  overflow-y: auto;
  padding: 10px;
  display: flex;
  flex-direction: column;
  gap: 8px;
`

const Section = styled.div`
  background: var(--bg-raised);
  border-radius: var(--radius-md);
  padding: 10px;
  box-shadow: var(--neu-raised);
`

const SectionTitle = styled.div`
  font-size: 10px;
  font-weight: 800;
  text-transform: uppercase;
  letter-spacing: 0.8px;
  color: var(--text-muted);
  margin-bottom: 8px;
`

const PropRow = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 6px;
  margin-bottom: 4px;
`

const PropLabel = styled.span`
  font-size: 11px;
  color: var(--text-secondary);
  flex-shrink: 0;
  min-width: 60px;
`

const TypeBadge = styled.span`
  font-size: 10px;
  font-family: var(--font-mono);
  color: var(--accent-blue);
  background: var(--accent-blue)15;
  padding: 2px 6px;
  border-radius: 4px;
`

const GeomGrid = styled.div`
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 6px;
`

const FieldBox = styled.div`
  display: flex;
  align-items: center;
  background: var(--bg-inset);
  border-radius: var(--radius-sm);
  box-shadow: var(--neu-inset);
  overflow: hidden;
`

const FieldPrefix = styled.span`
  padding: 0 6px;
  font-size: 10px;
  font-weight: 800;
  color: var(--accent-blue);
  font-family: var(--font-mono);
  border-right: 1px solid var(--border-subtle);
`

const NumInput = styled.input`
  border: none;
  background: transparent;
  padding: 5px 6px;
  font-size: 12px;
  font-family: var(--font-mono);
  color: var(--text-primary);
  width: 100%;
  outline: none;
  &::-webkit-inner-spin-button { opacity: 0.4; }
`

const TextInput = styled.input`
  border: none;
  background: var(--bg-inset);
  box-shadow: var(--neu-inset);
  border-radius: 6px;
  padding: 5px 8px;
  font-size: 11px;
  color: var(--text-primary);
  flex: 1;
  outline: none;
  &:focus { box-shadow: var(--neu-inset), 0 0 0 2px var(--accent-blue)33; }
`

const ColorRow = styled.div`
  display: flex;
  align-items: center;
  gap: 6px;
`

const ColorSwatch = styled.div`
  width: 22px;
  height: 22px;
  border-radius: 6px;
  border: 2px solid var(--border-subtle);
  position: relative;
  overflow: hidden;
  cursor: pointer;
  flex-shrink: 0;
`

const ColorInput = styled.input`
  position: absolute;
  inset: -4px;
  opacity: 0;
  cursor: pointer;
  width: 140%;
  height: 140%;
`

const ColorHex = styled.span`
  font-size: 10px;
  font-family: var(--font-mono);
  color: var(--text-secondary);
`

const RGB565Badge = styled.span`
  font-size: 10px;
  font-family: var(--font-mono);
  color: var(--accent-blue);
  background: var(--accent-blue)12;
  padding: 1px 5px;
  border-radius: 4px;
`

const SwatchesTitle = styled.div`
  font-size: 10px;
  color: var(--text-muted);
  margin: 6px 0 4px;
`

const SwatchGrid = styled.div`
  display: flex;
  flex-wrap: wrap;
  gap: 4px;
`

const Swatch = styled.div`
  width: 18px;
  height: 18px;
  border-radius: 50%;
  border: 2px solid var(--border-subtle);
  cursor: pointer;
  transition: transform 0.15s;
  &:hover { transform: scale(1.2); }
`

const EmptyState = styled.div`
  flex: 1;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: 8px;
  color: var(--text-muted);
  font-size: 12px;
  text-align: center;
  padding: 20px;
`

const EmptyIcon = styled.div`
  font-size: 28px;
  opacity: 0.4;
`

const LayerItem = styled.div`
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 8px 10px;
  border-radius: var(--radius-sm);
  cursor: pointer;
  background: ${p => p.$active ? 'var(--accent-blue)15' : 'var(--bg-raised)'};
  border: 1px solid ${p => p.$active ? 'var(--accent-blue)44' : 'transparent'};
  box-shadow: ${p => p.$active ? 'none' : 'var(--neu-raised)'};
  &:hover { background: var(--bg-surface); }
`

const LayerLeft = styled.div`
  display: flex;
  align-items: center;
  gap: 6px;
`

const LayerActions = styled.div`
  display: flex;
  gap: 2px;
`

const LayerBtn = styled.button`
  width: 22px;
  height: 22px;
  border: none;
  border-radius: 4px;
  background: var(--bg-inset);
  color: var(--text-secondary);
  font-size: 10px;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  &:hover { background: var(--shadow-dark); }
`

const PatternActionBtn = styled.button`
  width: 100%;
  padding: 8px 12px;
  background: linear-gradient(135deg, #7c3aed, #4f46e5);
  border: 1px solid rgba(255, 255, 255, 0.2);
  border-radius: var(--radius-sm);
  color: #ffffff;
  font-size: 11px;
  font-weight: 700;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 6px;
  box-shadow: 0 4px 12px rgba(124, 58, 237, 0.35);
  transition: all 0.2s ease;
  &:hover {
    transform: translateY(-1px);
    box-shadow: 0 6px 16px rgba(124, 58, 237, 0.5);
  }
  &:active {
    transform: translateY(0);
  }
`

const HintSubtext = styled.div`
  font-size: 10px;
  color: var(--text-tertiary);
  margin-top: 6px;
  line-height: 1.3;
`
