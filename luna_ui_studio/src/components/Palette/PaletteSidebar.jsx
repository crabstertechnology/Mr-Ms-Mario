import React, { useState, useEffect } from 'react'
import styled from 'styled-components'
import { UI_COMPONENTS, CATEGORIES, subscribeRegistry } from '../../ui-elements/registry'

export default function PaletteSidebar({ onAdd, onOpenCompileModal }) {
  const [search, setSearch] = useState('')
  const [activeCat, setActiveCat] = useState('all')
  const [components, setComponents] = useState(UI_COMPONENTS)

  useEffect(() => {
    return subscribeRegistry(updated => setComponents({ ...updated }))
  }, [])

  const filtered = Object.entries(components).filter(([key, comp]) => {
    if (activeCat !== 'all' && comp.category !== activeCat) return false
    if (search && !comp.name.toLowerCase().includes(search.toLowerCase())) return false
    return true
  })

  return (
    <Sidebar>
      <SidebarHeader>
        <HeaderTitle>UI Elements</HeaderTitle>
        <HeaderTag>Drag & Drop</HeaderTag>
      </SidebarHeader>

      <CompileBanner onClick={onOpenCompileModal}>
        <span style={{ fontSize: 13 }}>⚡</span>
        <div>
          <div style={{ fontWeight: 700, fontSize: 11 }}>Compile React UI</div>
          <div style={{ fontSize: 9, opacity: 0.8 }}>Paste UIverse & live load</div>
        </div>
        <span style={{ fontSize: 12, opacity: 0.7 }}>+</span>
      </CompileBanner>

      <SearchBox
        type="text"
        placeholder="Search elements..."
        value={search}
        onChange={e => setSearch(e.target.value)}
      />

      <CategoryTabs>
        {CATEGORIES.map(cat => (
          <CatTab
            key={cat.id}
            $active={activeCat === cat.id}
            onClick={() => setActiveCat(cat.id)}
          >
            {cat.label}
          </CatTab>
        ))}
      </CategoryTabs>

      <ElementsList>
        {filtered.map(([key, comp]) => (
          <ElementCard
            key={key}
            compKey={key}
            comp={comp}
            onAdd={onAdd}
          />
        ))}
        {filtered.length === 0 && (
          <EmptyState>No elements found for "{search}"</EmptyState>
        )}
      </ElementsList>
    </Sidebar>
  )
}

function ElementCard({ compKey, comp, onAdd }) {
  const Comp = comp.component
  const p = comp.defaultProps || {}

  const handleDragStart = (e) => {
    e.dataTransfer.setData('text/plain', compKey)
    e.dataTransfer.effectAllowed = 'copy'
  }

  const isPattern = comp.category === 'patterns'
  const compW = p.w || 200
  const compH = p.h || 60
  // Scale to fit within 210px width preview, max 70px height
  const scaleX = 210 / compW
  const scaleY = 70 / compH
  const scale = isPattern ? 1 : Math.min(scaleX, scaleY, 1)
  const previewH = isPattern ? 76 : Math.max(Math.ceil(compH * scale), 48)

  return (
    <Card draggable onDragStart={handleDragStart}>
      <CardHeader>
        <CardName title={comp.name}>{comp.name}</CardName>
        <CardBadge style={isPattern ? { background: 'rgba(124, 58, 237, 0.2)', color: '#c084fc', borderColor: 'rgba(124, 58, 237, 0.4)' } : undefined}>
          {comp.category}
        </CardBadge>
      </CardHeader>

      <PreviewBox style={{ height: previewH + 16 }}>
        <div style={{
          position: 'absolute',
          top: '50%',
          left: '50%',
          transform: isPattern ? 'translate(-50%, -50%)' : `translate(-50%, -50%) scale(${scale})`,
          transformOrigin: 'center center',
          width: isPattern ? '100%' : compW,
          height: isPattern ? '100%' : compH,
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          pointerEvents: 'none',
        }}>
          {Comp && <Comp {...p} w={isPattern ? 228 : compW} h={isPattern ? 76 : compH} />}
        </div>
      </PreviewBox>

      <CardFooter>
        <FooterHint>{isPattern ? 'Drag or + Add' : 'Drag to canvas'}</FooterHint>
        <AddBtn onClick={() => onAdd(compKey)}>+ Add</AddBtn>
      </CardFooter>
    </Card>
  )
}

// ── Styled Components ──────────────────────────────────────

const Sidebar = styled.aside`
  width: 256px;
  min-width: 256px;
  height: 100%;
  max-height: 100%;
  background: var(--bg-base);
  display: flex;
  flex-direction: column;
  border-right: 1px solid var(--border-subtle);
  box-shadow: 4px 0 16px rgba(0,0,0,0.06);
  overflow: hidden;
`

const SidebarHeader = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 14px 16px 10px;
  border-bottom: 1px solid var(--border-subtle);
  background: var(--bg-raised);
  box-shadow: var(--neu-raised);
  margin: 10px 10px 0;
  border-radius: var(--radius-md);
  flex-shrink: 0;
`

const HeaderTitle = styled.span`
  font-size: 13px;
  font-weight: 800;
  color: var(--text-primary);
  letter-spacing: 0.5px;
  text-transform: uppercase;
`

const HeaderTag = styled.span`
  font-size: 10px;
  color: var(--accent-blue);
  font-family: var(--font-mono);
  font-weight: 600;
`

const SearchBox = styled.input`
  margin: 10px 10px 6px;
  padding: 8px 12px;
  border-radius: var(--radius-sm);
  border: none;
  background: var(--bg-inset);
  box-shadow: var(--neu-inset);
  font-size: 12px;
  color: var(--text-primary);
  outline: none;
  flex-shrink: 0;
  &::placeholder { color: var(--text-muted); }
  &:focus { box-shadow: var(--neu-inset), 0 0 0 2px var(--accent-blue)44; }
`

const CategoryTabs = styled.div`
  display: flex;
  flex-wrap: wrap;
  gap: 4px;
  padding: 6px 10px;
  border-bottom: 1px solid var(--border-subtle);
  flex-shrink: 0;
`

const CatTab = styled.button`
  padding: 4px 10px;
  border-radius: 20px;
  border: none;
  font-size: 11px;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.2s;
  background: ${p => p.$active ? 'var(--accent-blue)' : 'var(--bg-raised)'};
  color: ${p => p.$active ? '#fff' : 'var(--text-secondary)'};
  box-shadow: ${p => p.$active ? '0 2px 8px rgba(37,99,235,0.35)' : 'var(--neu-button)'};
  &:hover { background: ${p => p.$active ? 'var(--accent-blue)' : 'var(--bg-surface)'}; }
`

const ElementsList = styled.div`
  flex: 1 1 0%;
  min-height: 0;
  overflow-y: auto;
  overflow-x: hidden;
  padding: 10px 10px 24px;
  display: flex;
  flex-direction: column;
  gap: 12px;
`

const Card = styled.div`
  flex-shrink: 0;
  min-height: fit-content;
  background: var(--bg-raised);
  border: 1px solid rgba(0, 0, 0, 0.05);
  border-radius: var(--radius-md);
  box-shadow: var(--neu-raised);
  overflow: hidden;
  cursor: grab;
  transition: transform 0.15s, box-shadow 0.15s;
  &:hover {
    transform: translateY(-2px);
    box-shadow: 6px 6px 16px var(--shadow-dark), -6px -6px 16px var(--shadow-light);
  }
  &:active { cursor: grabbing; transform: scale(0.98); }
`

const CardHeader = styled.div`
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 8px 10px;
`

const CardName = styled.span`
  font-size: 11.5px;
  font-weight: 700;
  color: var(--text-primary);
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
  max-width: 145px;
`

const CardBadge = styled.span`
  font-size: 9px;
  font-weight: 700;
  padding: 2px 7px;
  border-radius: 10px;
  background: rgba(37, 99, 235, 0.12);
  color: var(--accent-blue);
  text-transform: uppercase;
  letter-spacing: 0.5px;
  flex-shrink: 0;
`

const PreviewBox = styled.div`
  margin: 0 8px;
  background: #0b0f19;
  border: 1px solid rgba(255, 255, 255, 0.08);
  border-radius: var(--radius-sm);
  box-shadow: inset 0 2px 8px rgba(0, 0, 0, 0.5);
  overflow: hidden;
  position: relative;
`

const CompileBanner = styled.div`
  margin: 8px 10px 0;
  padding: 8px 12px;
  background: linear-gradient(135deg, rgba(37,99,235,0.1), rgba(124,58,237,0.12));
  border: 1.5px dashed var(--accent-blue);
  border-radius: var(--radius-md);
  display: flex;
  align-items: center;
  gap: 8px;
  cursor: pointer;
  color: var(--accent-blue);
  flex-shrink: 0;
  transition: all 0.2s;
  &:hover {
    background: linear-gradient(135deg, rgba(37,99,235,0.18), rgba(124,58,237,0.2));
    transform: translateY(-1px);
    box-shadow: 0 4px 12px rgba(37,99,235,0.15);
  }
  &:active {
    transform: translateY(0);
  }
`

const CardFooter = styled.div`
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 6px 10px 8px;
  border-top: 1px solid rgba(0, 0, 0, 0.04);
`

const FooterHint = styled.span`
  font-size: 10px;
  color: var(--text-muted);
  font-weight: 500;
`

const AddBtn = styled.button`
  padding: 4px 10px;
  border-radius: 6px;
  border: none;
  background: var(--accent-blue);
  color: #fff;
  font-size: 11px;
  font-weight: 700;
  cursor: pointer;
  box-shadow: 0 2px 6px rgba(37,99,235,0.3);
  transition: all 0.15s;
  &:hover { background: #1d4ed8; transform: scale(1.05); }
`

const EmptyState = styled.div`
  text-align: center;
  padding: 30px 10px;
  color: var(--text-muted);
  font-size: 12px;
`
