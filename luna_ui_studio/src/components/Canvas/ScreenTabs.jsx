import React, { useState } from 'react'
import styled from 'styled-components'

export default function ScreenTabs({
  screens,
  activeScreenId,
  onSelectScreen,
  onAddScreen,
  onDeleteScreen,
  onDuplicateScreen,
  onRenameScreen,
  onOpenFlowMap,
  testMode
}) {
  const [editingId, setEditingId] = useState(null)
  const [tempName, setTempName] = useState('')

  const startRename = (screen) => {
    setEditingId(screen.id)
    setTempName(screen.name)
  }

  const saveRename = (screenId) => {
    if (tempName.trim()) {
      onRenameScreen(screenId, tempName.trim())
    }
    setEditingId(null)
  }

  return (
    <TabsBar>
      <TabsList>
        <BarLabel>SCREENS:</BarLabel>
        {screens.map((s, idx) => {
          const isActive = s.id === activeScreenId
          const isEditing = editingId === s.id
          const hasLinks = s.elements.some(el => el.actions && el.actions.length > 0)

          return (
            <TabItem
              key={s.id}
              $active={isActive}
              $testMode={testMode}
              onClick={() => !isEditing && onSelectScreen(s.id)}
            >
              <TabIcon>{idx === 0 ? '🏠' : '📄'}</TabIcon>
              {isEditing ? (
                <RenameInput
                  value={tempName}
                  autoFocus
                  onChange={e => setTempName(e.target.value)}
                  onBlur={() => saveRename(s.id)}
                  onKeyDown={e => {
                    if (e.key === 'Enter') saveRename(s.id)
                    if (e.key === 'Escape') setEditingId(null)
                  }}
                  onClick={e => e.stopPropagation()}
                />
              ) : (
                <TabName onDoubleClick={() => startRename(s)}>
                  {s.name}
                  {hasLinks && <FlowBadge title="Contains mapped block actions">⚡</FlowBadge>}
                </TabName>
              )}

              <ElCount>({s.elements.length})</ElCount>

              <TabActions>
                <ActionBtn
                  title="Rename Screen"
                  onClick={(e) => { e.stopPropagation(); startRename(s) }}
                >
                  ✏️
                </ActionBtn>
                <ActionBtn
                  title="Duplicate Screen"
                  onClick={(e) => { e.stopPropagation(); onDuplicateScreen(s.id) }}
                >
                  ⧉
                </ActionBtn>
                {screens.length > 1 && (
                  <ActionBtn
                    title={`Delete "${s.name}"`}
                    $danger
                    onClick={(e) => {
                      e.stopPropagation()
                      onDeleteScreen(s.id)
                    }}
                  >
                    ✕
                  </ActionBtn>
                )}
              </TabActions>
            </TabItem>
          )
        })}

        <AddScreenBtn onClick={() => onAddScreen()} title="Add New Screen / Page">
          ➕ Add Screen
        </AddScreenBtn>
      </TabsList>

      <RightActions>
        <FlowMapBtn onClick={onOpenFlowMap} title="View Visual UI Navigation & Block Map">
          🗺️ UI Flow & Blocks Map
        </FlowMapBtn>
      </RightActions>
    </TabsBar>
  )
}

const TabsBar = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 8px 16px;
  background: var(--bg-surface);
  border-bottom: 1px solid var(--border-subtle);
  gap: 12px;
  box-shadow: inset 0 -1px 0 rgba(0,0,0,0.03);
`

const TabsList = styled.div`
  display: flex;
  align-items: center;
  gap: 8px;
  overflow-x: auto;
  padding-bottom: 2px;
  &::-webkit-scrollbar {
    height: 4px;
  }
  &::-webkit-scrollbar-thumb {
    background: rgba(0,0,0,0.1);
    border-radius: 4px;
  }
`

const BarLabel = styled.span`
  font-size: 10px;
  font-weight: 800;
  color: var(--text-muted);
  letter-spacing: 0.8px;
  padding-right: 4px;
`

const TabItem = styled.div`
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 5px 10px;
  border-radius: var(--radius-sm);
  font-size: 11px;
  font-weight: 700;
  cursor: pointer;
  transition: all 0.18s;
  background: ${p => p.$active
    ? (p.$testMode ? 'linear-gradient(135deg, #10b981, #059669)' : 'linear-gradient(135deg, #2563eb, #3b82f6)')
    : 'var(--bg-raised)'};
  color: ${p => p.$active ? '#ffffff' : 'var(--text-secondary)'};
  box-shadow: ${p => p.$active
    ? (p.$testMode ? '0 2px 10px rgba(16,185,129,0.35)' : '0 2px 10px rgba(37,99,235,0.35)')
    : 'var(--neu-button)'};
  border: 1px solid ${p => p.$active ? 'transparent' : 'var(--border-subtle)'};

  &:hover {
    transform: translateY(-1px);
    color: ${p => p.$active ? '#ffffff' : 'var(--text-primary)'};
  }
`

const TabIcon = styled.span`
  font-size: 12px;
`

const TabName = styled.span`
  user-select: none;
  display: flex;
  align-items: center;
  gap: 4px;
`

const FlowBadge = styled.span`
  font-size: 10px;
  color: #facc15;
  filter: drop-shadow(0 0 4px rgba(250,204,21,0.5));
`

const ElCount = styled.span`
  font-size: 9px;
  opacity: 0.75;
  font-family: var(--font-mono);
`

const TabActions = styled.div`
  display: flex;
  align-items: center;
  gap: 3px;
  margin-left: 2px;
`

const ActionBtn = styled.button`
  border: none;
  background: transparent;
  padding: 2px 5px;
  font-size: 11px;
  cursor: pointer;
  border-radius: 4px;
  opacity: 0.7;
  color: inherit;
  transition: all 0.15s;

  &:hover {
    opacity: 1;
    background: ${p => p.$danger ? '#ef4444' : 'rgba(0,0,0,0.15)'};
    color: ${p => p.$danger ? '#ffffff' : 'inherit'};
    transform: scale(1.15);
  }
  &:active {
    transform: scale(0.95);
  }
`

const RenameInput = styled.input`
  font-size: 11px;
  font-weight: 700;
  border: none;
  background: rgba(255,255,255,0.2);
  color: inherit;
  padding: 1px 4px;
  border-radius: 3px;
  outline: none;
  width: 90px;
`

const AddScreenBtn = styled.button`
  display: flex;
  align-items: center;
  gap: 4px;
  padding: 5px 12px;
  border-radius: var(--radius-sm);
  border: 1px dashed var(--accent-blue)66;
  background: rgba(37,99,235,0.06);
  color: var(--accent-blue);
  font-size: 11px;
  font-weight: 700;
  cursor: pointer;
  transition: all 0.2s;

  &:hover {
    background: rgba(37,99,235,0.15);
    border-color: var(--accent-blue);
    transform: translateY(-1px);
  }
`

const RightActions = styled.div`
  display: flex;
  align-items: center;
  gap: 8px;
  flex-shrink: 0;
`

const FlowMapBtn = styled.button`
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 5px 12px;
  border-radius: var(--radius-sm);
  border: 1px solid rgba(139, 92, 246, 0.4);
  background: linear-gradient(135deg, rgba(139, 92, 246, 0.1), rgba(99, 102, 241, 0.12));
  color: #7c3aed;
  font-size: 11px;
  font-weight: 800;
  cursor: pointer;
  box-shadow: var(--neu-button);
  transition: all 0.2s;

  &:hover {
    background: linear-gradient(135deg, rgba(139, 92, 246, 0.2), rgba(99, 102, 241, 0.25));
    transform: translateY(-1px);
    box-shadow: 0 4px 12px rgba(139, 92, 246, 0.2);
  }
`
