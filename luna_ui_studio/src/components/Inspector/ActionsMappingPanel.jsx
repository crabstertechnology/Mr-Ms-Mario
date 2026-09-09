import React, { useState } from 'react'
import styled from 'styled-components'

export default function ActionsMappingPanel({
  element,
  screens,
  activeScreenId,
  onUpdateActions,
  onAddScreen,
  onTestTrigger
}) {
  if (!element) {
    return (
      <EmptyState>
        <EmptyIcon>⚡</EmptyIcon>
        <EmptyTitle>No Element Selected</EmptyTitle>
        <EmptyText>
          Click any element on the canvas (like a Button, Card, or Toggle) to map interactive action blocks.
        </EmptyText>
        <HintBox>
          <strong>Easy Mapping:</strong> You can assign actions like:
          <ul>
            <li>👆 <strong>Button Click</strong> ➔ 🚀 Take to another Screen</li>
            <li>👆 <strong>Button Click</strong> ➔ 📜 Scroll down the page</li>
            <li>👆 <strong>Button Click</strong> ➔ 🔔 Show Watch Alert</li>
          </ul>
        </HintBox>
      </EmptyState>
    )
  }

  const actions = element.actions || []

  const handleAddAction = (presetType = 'navigate') => {
    let newAction
    if (presetType === 'navigate') {
      // Find other screen, or first screen
      let targetScreen = screens.find(s => s.id !== activeScreenId)
      let targetScreenId = targetScreen ? targetScreen.id : activeScreenId
      
      // If only 1 screen exists, automatically prompt or create Screen 2
      if (screens.length <= 1) {
        targetScreenId = onAddScreen('Screen 2 (Page 2)')
      }

      newAction = {
        id: `act_${Date.now().toString(36)}`,
        trigger: 'onClick',
        actionType: 'navigate',
        targetScreenId: targetScreenId,
        transition: 'slide-left',
        alertMessage: 'Navigating to page...',
        scrollAmount: 80,
      }
    } else if (presetType === 'scroll') {
      newAction = {
        id: `act_${Date.now().toString(36)}`,
        trigger: 'onClick',
        actionType: 'scroll',
        targetScreenId: activeScreenId,
        transition: 'smooth',
        scrollAmount: 80,
        scrollDirection: 'down',
      }
    } else if (presetType === 'alert') {
      newAction = {
        id: `act_${Date.now().toString(36)}`,
        trigger: 'onClick',
        actionType: 'alert',
        alertMessage: `${element.props.label || element.name} Clicked!`,
      }
    } else {
      newAction = {
        id: `act_${Date.now().toString(36)}`,
        trigger: 'onClick',
        actionType: 'toggle',
      }
    }

    onUpdateActions(element.id, [...actions, newAction])
  }

  const handleUpdateAction = (actionId, updates) => {
    const updated = actions.map(a => a.id === actionId ? { ...a, ...updates } : a)
    onUpdateActions(element.id, updated)
  }

  const handleDeleteAction = (actionId) => {
    const updated = actions.filter(a => a.id !== actionId)
    onUpdateActions(element.id, updated)
  }

  return (
    <PanelScroll>
      <HeaderSection>
        <ElementBadge>
          <span style={{ fontSize: 13 }}>⚡</span>
          <div>
            <ElName>{element.name}</ElName>
            <ElSub>ID: {element.id.substring(0, 10)}...</ElSub>
          </div>
        </ElementBadge>
        <HeaderHint>
          Assign easy block mapping functions to make this element interactive!
        </HeaderHint>
      </HeaderSection>

      {/* Preset Quick Actions */}
      <QuickPresetsSection>
        <SectionTitle>⚡ Quick 1-Click Mapping</SectionTitle>
        <PresetGrid>
          <PresetBtn onClick={() => handleAddAction('navigate')} title="Map to navigate to another screen">
            🚀 Go to Page 2
          </PresetBtn>
          <PresetBtn onClick={() => handleAddAction('scroll')} title="Map to scroll canvas down">
            📜 Scroll Down
          </PresetBtn>
          <PresetBtn onClick={() => handleAddAction('alert')} title="Map to show a watch alert popup">
            🔔 Watch Alert
          </PresetBtn>
        </PresetGrid>
      </QuickPresetsSection>

      {/* Action Blocks List */}
      <BlocksSection>
        <SectionTitle>
          <span>Blocks Mapping ({actions.length})</span>
          {actions.length > 0 && (
            <AddSmallBtn onClick={() => handleAddAction('navigate')}>
              + Add Block
            </AddSmallBtn>
          )}
        </SectionTitle>

        {actions.length === 0 ? (
          <NoBlocksBox>
            <NoBlocksIcon>🧩</NoBlocksIcon>
            <NoBlocksText>No block mappings assigned yet.</NoBlocksText>
            <AddBlockMainBtn onClick={() => handleAddAction('navigate')}>
              ➕ Add Action Block
            </AddBlockMainBtn>
          </NoBlocksBox>
        ) : (
          actions.map((act, index) => (
            <BlockCard key={act.id}>
              {/* TRIGGER BLOCK */}
              <TriggerBlock>
                <BlockHeader>
                  <BlockTag $type="trigger">WHEN (EVENT)</BlockTag>
                  <ActionControls>
                    <TestTriggerBtn
                      title="Test this block now"
                      onClick={() => onTestTrigger && onTestTrigger(act, element)}
                    >
                      ▶ Test
                    </TestTriggerBtn>
                    <DeleteBlockBtn
                      title="Remove block"
                      onClick={() => handleDeleteAction(act.id)}
                    >
                      ✕
                    </DeleteBlockBtn>
                  </ActionControls>
                </BlockHeader>
                <BlockFieldRow>
                  <FieldIcon>👆</FieldIcon>
                  <BlockSelect
                    value={act.trigger || 'onClick'}
                    onChange={e => handleUpdateAction(act.id, { trigger: e.target.value })}
                  >
                    <option value="onClick">On Click / Tap</option>
                    <option value="onDoubleClick">On Double Click</option>
                    <option value="onLongPress">On Long Press</option>
                    <option value="onScrollDown">On Swipe / Scroll Down</option>
                    <option value="onScrollUp">On Swipe / Scroll Up</option>
                  </BlockSelect>
                </BlockFieldRow>
              </TriggerBlock>

              {/* CONNECTOR */}
              <ConnectorWire>
                <WireLine />
                <WireBadge>THEN DO ⬇</WireBadge>
                <WireLine />
              </ConnectorWire>

              {/* ACTION BLOCK */}
              <ActionBlock>
                <BlockTag $type="action">DO (ACTION)</BlockTag>
                <BlockFieldRow style={{ marginTop: 6 }}>
                  <FieldIcon>⚙️</FieldIcon>
                  <BlockSelect
                    value={act.actionType || 'navigate'}
                    onChange={e => handleUpdateAction(act.id, { actionType: e.target.value })}
                  >
                    <option value="navigate">🚀 Navigate to Screen / Page</option>
                    <option value="scroll">📜 Scroll Canvas Content</option>
                    <option value="alert">🔔 Show Watch Notification Alert</option>
                    <option value="toggle">🔄 Toggle Switch State</option>
                  </BlockSelect>
                </BlockFieldRow>

                {/* SUB-PROPERTIES: NAVIGATE */}
                {act.actionType === 'navigate' && (
                  <SubPropsBox>
                    <SubPropRow>
                      <SubLabel>Target Screen:</SubLabel>
                      <BlockSelect
                        value={act.targetScreenId}
                        onChange={e => handleUpdateAction(act.id, { targetScreenId: e.target.value })}
                        style={{ flex: 1 }}
                      >
                        {screens.map(s => (
                          <option key={s.id} value={s.id}>
                            {s.id === activeScreenId ? `Current: ${s.name}` : `➔ ${s.name}`}
                          </option>
                        ))}
                      </BlockSelect>
                    </SubPropRow>

                    <SubPropRow>
                      <SubLabel>Transition:</SubLabel>
                      <BlockSelect
                        value={act.transition || 'slide-left'}
                        onChange={e => handleUpdateAction(act.id, { transition: e.target.value })}
                        style={{ flex: 1 }}
                      >
                        <option value="slide-left">Slide Left ◀</option>
                        <option value="slide-right">Slide Right ▶</option>
                        <option value="fade">Smooth Fade ✨</option>
                        <option value="instant">Instant ⚡</option>
                      </BlockSelect>
                    </SubPropRow>

                    {screens.length <= 1 && (
                      <NewScreenQuickBtn
                        onClick={() => {
                          const newId = onAddScreen('Screen 2 (Page 2)')
                          handleUpdateAction(act.id, { targetScreenId: newId })
                        }}
                      >
                        ➕ Create & Link "Screen 2" Now
                      </NewScreenQuickBtn>
                    )}
                  </SubPropsBox>
                )}

                {/* SUB-PROPERTIES: SCROLL */}
                {act.actionType === 'scroll' && (
                  <SubPropsBox>
                    <SubPropRow>
                      <SubLabel>Direction:</SubLabel>
                      <BlockSelect
                        value={act.scrollDirection || 'down'}
                        onChange={e => handleUpdateAction(act.id, { scrollDirection: e.target.value })}
                        style={{ flex: 1 }}
                      >
                        <option value="down">Scroll Down ⬇</option>
                        <option value="up">Scroll Up ⬆</option>
                        <option value="top">Scroll to Top 🔝</option>
                      </BlockSelect>
                    </SubPropRow>
                    <SubPropRow>
                      <SubLabel>Distance:</SubLabel>
                      <BlockInput
                        type="number"
                        value={act.scrollAmount || 80}
                        onChange={e => handleUpdateAction(act.id, { scrollAmount: parseInt(e.target.value) || 80 })}
                        style={{ width: 70 }}
                      />
                      <span style={{ fontSize: 10, color: 'var(--text-muted)' }}>px</span>
                    </SubPropRow>
                  </SubPropsBox>
                )}

                {/* SUB-PROPERTIES: ALERT */}
                {act.actionType === 'alert' && (
                  <SubPropsBox>
                    <SubPropRow>
                      <SubLabel>Alert Text:</SubLabel>
                      <BlockInput
                        type="text"
                        value={act.alertMessage || ''}
                        placeholder="Notification text..."
                        onChange={e => handleUpdateAction(act.id, { alertMessage: e.target.value })}
                        style={{ flex: 1 }}
                      />
                    </SubPropRow>
                  </SubPropsBox>
                )}
              </ActionBlock>
            </BlockCard>
          ))
        )}
      </BlocksSection>
    </PanelScroll>
  )
}

const PanelScroll = styled.div`
  flex: 1;
  overflow-y: auto;
  padding: 14px;
  display: flex;
  flex-direction: column;
  gap: 14px;
`

const EmptyState = styled.div`
  padding: 24px 16px;
  text-align: center;
  color: var(--text-muted);
`

const EmptyIcon = styled.div`
  font-size: 32px;
  color: #8b5cf6;
  margin-bottom: 10px;
`

const EmptyTitle = styled.div`
  font-size: 14px;
  font-weight: 800;
  color: var(--text-primary);
  margin-bottom: 6px;
`

const EmptyText = styled.p`
  font-size: 12px;
  line-height: 1.5;
  margin-bottom: 16px;
`

const HintBox = styled.div`
  text-align: left;
  background: var(--bg-inset);
  padding: 12px;
  border-radius: var(--radius-sm);
  font-size: 11px;
  line-height: 1.5;
  color: var(--text-secondary);
  box-shadow: var(--neu-inset);
  ul {
    margin: 8px 0 0 16px;
    padding: 0;
  }
  li {
    margin-bottom: 4px;
  }
`

const HeaderSection = styled.div`
  display: flex;
  flex-direction: column;
  gap: 8px;
`

const ElementBadge = styled.div`
  display: flex;
  align-items: center;
  gap: 8px;
  background: var(--bg-raised);
  padding: 8px 12px;
  border-radius: var(--radius-sm);
  border-left: 3px solid #8b5cf6;
  box-shadow: var(--neu-button);
`

const ElName = styled.div`
  font-size: 12px;
  font-weight: 800;
  color: var(--text-primary);
`

const ElSub = styled.div`
  font-size: 10px;
  color: var(--text-muted);
  font-family: var(--font-mono);
`

const HeaderHint = styled.div`
  font-size: 11px;
  color: var(--text-muted);
  line-height: 1.4;
`

const QuickPresetsSection = styled.div`
  display: flex;
  flex-direction: column;
  gap: 6px;
`

const SectionTitle = styled.div`
  font-size: 11px;
  font-weight: 800;
  color: var(--text-secondary);
  display: flex;
  align-items: center;
  justify-content: space-between;
  letter-spacing: 0.5px;
`

const PresetGrid = styled.div`
  display: grid;
  grid-template-columns: 1fr 1fr 1fr;
  gap: 6px;
`

const PresetBtn = styled.button`
  border: none;
  background: var(--bg-raised);
  padding: 7px 6px;
  border-radius: var(--radius-sm);
  font-size: 10px;
  font-weight: 700;
  color: var(--text-primary);
  box-shadow: var(--neu-button);
  cursor: pointer;
  transition: all 0.15s;
  text-align: center;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;

  &:hover {
    transform: translateY(-1px);
    background: rgba(99, 102, 241, 0.1);
    color: #6366f1;
  }
`

const BlocksSection = styled.div`
  display: flex;
  flex-direction: column;
  gap: 12px;
`

const AddSmallBtn = styled.button`
  border: none;
  background: transparent;
  color: var(--accent-blue);
  font-size: 11px;
  font-weight: 800;
  cursor: pointer;
  &:hover {
    text-decoration: underline;
  }
`

const NoBlocksBox = styled.div`
  background: var(--bg-inset);
  padding: 20px 12px;
  border-radius: var(--radius-sm);
  text-align: center;
  box-shadow: var(--neu-inset);
`

const NoBlocksIcon = styled.div`
  font-size: 24px;
  margin-bottom: 6px;
`

const NoBlocksText = styled.div`
  font-size: 11px;
  color: var(--text-muted);
  margin-bottom: 12px;
`

const AddBlockMainBtn = styled.button`
  border: none;
  background: linear-gradient(135deg, #6366f1, #8b5cf6);
  color: #fff;
  padding: 6px 14px;
  border-radius: var(--radius-sm);
  font-size: 11px;
  font-weight: 800;
  cursor: pointer;
  box-shadow: 0 4px 12px rgba(99,102,241,0.3);
  transition: all 0.15s;

  &:hover {
    transform: translateY(-1px);
    box-shadow: 0 6px 16px rgba(99,102,241,0.4);
  }
`

const BlockCard = styled.div`
  display: flex;
  flex-direction: column;
  background: var(--bg-raised);
  border-radius: var(--radius-md);
  box-shadow: var(--neu-button);
  border: 1px solid var(--border-subtle);
  overflow: hidden;
`

const TriggerBlock = styled.div`
  background: linear-gradient(135deg, rgba(245, 158, 11, 0.08), rgba(217, 119, 6, 0.03));
  border-bottom: 1px solid rgba(245, 158, 11, 0.15);
  padding: 10px;
`

const ActionBlock = styled.div`
  background: linear-gradient(135deg, rgba(16, 185, 129, 0.08), rgba(5, 150, 105, 0.03));
  padding: 10px;
`

const BlockHeader = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: 6px;
`

const BlockTag = styled.span`
  font-size: 9px;
  font-weight: 900;
  letter-spacing: 0.5px;
  padding: 2px 6px;
  border-radius: 4px;
  background: ${p => p.$type === 'trigger' ? '#f59e0b' : '#10b981'};
  color: #ffffff;
`

const ActionControls = styled.div`
  display: flex;
  align-items: center;
  gap: 4px;
`

const TestTriggerBtn = styled.button`
  border: none;
  background: rgba(16, 185, 129, 0.15);
  color: #10b981;
  font-size: 10px;
  font-weight: 800;
  padding: 2px 8px;
  border-radius: 4px;
  cursor: pointer;
  transition: all 0.15s;
  &:hover {
    background: #10b981;
    color: #fff;
  }
`

const DeleteBlockBtn = styled.button`
  border: none;
  background: transparent;
  color: var(--text-muted);
  font-size: 11px;
  padding: 2px 5px;
  cursor: pointer;
  border-radius: 4px;
  &:hover {
    background: rgba(239, 68, 68, 0.15);
    color: #ef4444;
  }
`

const BlockFieldRow = styled.div`
  display: flex;
  align-items: center;
  gap: 6px;
`

const FieldIcon = styled.span`
  font-size: 13px;
`

const BlockSelect = styled.select`
  width: 100%;
  padding: 5px 8px;
  background: var(--bg-surface);
  border: 1px solid var(--border-subtle);
  border-radius: 6px;
  font-size: 11px;
  font-weight: 600;
  color: var(--text-primary);
  outline: none;
  cursor: pointer;
`

const BlockInput = styled.input`
  padding: 5px 8px;
  background: var(--bg-surface);
  border: 1px solid var(--border-subtle);
  border-radius: 6px;
  font-size: 11px;
  font-weight: 600;
  color: var(--text-primary);
  outline: none;
`

const ConnectorWire = styled.div`
  display: flex;
  align-items: center;
  justify-content: center;
  background: var(--bg-inset);
  padding: 3px 8px;
  gap: 6px;
`

const WireLine = styled.div`
  flex: 1;
  height: 1px;
  background: var(--border-subtle);
`

const WireBadge = styled.span`
  font-size: 8px;
  font-weight: 800;
  color: var(--text-muted);
  letter-spacing: 0.5px;
`

const SubPropsBox = styled.div`
  margin-top: 8px;
  background: rgba(0,0,0,0.02);
  border-radius: 6px;
  padding: 6px;
  display: flex;
  flex-direction: column;
  gap: 6px;
`

const SubPropRow = styled.div`
  display: flex;
  align-items: center;
  gap: 8px;
`

const SubLabel = styled.span`
  font-size: 10px;
  font-weight: 700;
  color: var(--text-secondary);
  width: 75px;
  flex-shrink: 0;
`

const NewScreenQuickBtn = styled.button`
  border: 1px dashed var(--accent-blue);
  background: rgba(37,99,235,0.06);
  color: var(--accent-blue);
  padding: 5px;
  border-radius: 4px;
  font-size: 10px;
  font-weight: 700;
  cursor: pointer;
  transition: all 0.15s;
  &:hover {
    background: rgba(37,99,235,0.15);
  }
`
