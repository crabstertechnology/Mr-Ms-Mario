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
  const p = comp.defaultProps

  const handleDragStart = (e) => {
    e.dataTransfer.setData('text/plain', compKey)
    e.dataTransfer.effectAllowed = 'copy'
  }

  const scaleW = 220
  const scaleH = Math.min(p.h || 60, 120)
  const scale = Math.min(scaleW / (p.w || 200), 1)

  return (
    <Card draggable onDragStart={handleDragStart}>
      <CardHeader>
        <CardName>{comp.name}</CardName>
        <CardBadge>{comp.category}</CardBadge>
      </CardHeader>

      <PreviewBox style={{ height: Math.round(scaleH * scale) + 20 }}>
        <PreviewInner style={{
          transform: `scale(${scale})`,
          transformOrigin: 'top left',
          width: p.w || 200,
          height: p.h || 60,
        }}>
          <Comp {...p} />
        </PreviewInner>
      </PreviewBox>

      <CardFooter>
        <FooterHint>Click or Drag to add</FooterHint>
        <AddBtn onClick={() => onAdd(compKey)}>+ Add</AddBtn>
      </CardFooter>
    </Card>
  )
}

// ── Styled Components ──────────────────────────────────────

const Sidebar = styled.aside`
  width: 248px;
  min-width: 248px;
  height: 100%;
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
  &::placeholder { color: var(--text-muted); }
  &:focus { box-shadow: var(--neu-inset), 0 0 0 2px var(--accent-blue)44; }
`

const CategoryTabs = styled.div`
  display: flex;
  flex-wrap: wrap;
  gap: 4px;
  padding: 6px 10px;
  border-bottom: 1px solid var(--border-subtle);
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
  flex: 1;
  overflow-y: auto;
  padding: 8px;
  display: flex;
  flex-direction: column;
  gap: 8px;
`

const Card = styled.div`
  background: var(--bg-raised);
  border-radius: var(--radius-md);
  box-shadow: var(--neu-raised);
  overflow: hidden;
  cursor: grab;
  transition: transform 0.15s, box-shadow 0.15s;
  &:hover { transform: translateY(-2px); box-shadow: 8px 8px 20px var(--shadow-dark), -8px -8px 20px var(--shadow-light); }
  &:active { cursor: grabbing; transform: scale(0.98); }
`

const CardHeader = styled.div`
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 8px 10px 6px;
`

const CardName = styled.span`
  font-size: 11px;
  font-weight: 700;
  color: var(--text-primary);
`

const CardBadge = styled.span`
  font-size: 9px;
  font-weight: 700;
  padding: 2px 6px;
  border-radius: 10px;
  background: var(--accent-blue)22;
  color: var(--accent-blue);
  text-transform: uppercase;
  letter-spacing: 0.5px;
`

const PreviewBox = styled.div`
  margin: 0 8px;
  background: var(--bg-inset);
  border-radius: var(--radius-sm);
  box-shadow: var(--neu-inset);
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

const PreviewInner = styled.div`
  position: absolute;
  top: 10px;
  left: 10px;
  pointer-events: auto;
`

const CardFooter = styled.div`
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 6px 10px 8px;
`

const FooterHint = styled.span`
  font-size: 10px;
  color: var(--text-muted);
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
