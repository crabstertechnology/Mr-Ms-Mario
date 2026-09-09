import React, { useState } from 'react'
import styled from 'styled-components'
import { UI_COMPONENTS } from '../ui-elements/registry'

export default function FlowMapModal({
  open,
  onClose,
  screens,
  activeScreenId,
  onSelectScreen,
  onAddScreen,
  onUpdateElementActions,
  onEnterTestMode
}) {
  const [selectedScreenId, setSelectedScreenId] = useState(activeScreenId)

  if (!open) return null

  // Collect all mapped navigation transitions
  const transitions = []
  screens.forEach(srcScreen => {
    srcScreen.elements.forEach(el => {
      if (el.actions && el.actions.length > 0) {
        el.actions.forEach(act => {
          if (act.actionType === 'navigate' && act.targetScreenId) {
            const target = screens.find(s => s.id === act.targetScreenId)
            transitions.push({
              sourceScreen: srcScreen,
              targetScreen: target || { name: 'Unknown Screen' },
              element: el,
              action: act
            })
          }
        })
      }
    })
  })

  return (
    <Backdrop onClick={e => e.target === e.currentTarget && onClose()}>
      <Modal>
        <ModalHeader>
          <HeaderLeft>
            <TitleIcon>🗺️</TitleIcon>
            <div>
              <ModalTitle>UI Flow & Block Action Map</ModalTitle>
              <ModalSub>
                Visual screen architecture, button click transitions, and interactive mapping
              </ModalSub>
            </div>
          </HeaderLeft>

          <HeaderRight>
            <TestModeBtn onClick={() => { onClose(); onEnterTestMode(); }}>
              ▶ Test Live Interactions
            </TestModeBtn>
            <CloseBtn onClick={onClose}>✕</CloseBtn>
          </HeaderRight>
        </ModalHeader>

        {/* Transition Summary Bar */}
        <SummaryBar>
          <StatBadge>
            <span>Total Screens:</span> <strong>{screens.length}</strong>
          </StatBadge>
          <StatBadge>
            <span>Navigation Links:</span> <strong>{transitions.length}</strong>
          </StatBadge>
          <AddScreenBarBtn onClick={() => onAddScreen()}>
            ➕ Add New Screen
          </AddScreenBarBtn>
        </SummaryBar>

        {/* Screens Grid */}
        <ModalBody>
          <ScreensContainer>
            {screens.map((screen, sIdx) => {
              const isActive = screen.id === activeScreenId
              const interactiveEls = screen.elements.filter(el => el.actions && el.actions.length > 0)
              const buttons = screen.elements.filter(el => el.type.includes('btn') || el.type.includes('button'))

              return (
                <ScreenCard key={screen.id} $active={isActive}>
                  <CardHeader>
                    <ScreenTitleRow>
                      <ScreenBadge>{sIdx === 0 ? '🏠 SCREEN 1' : `📄 SCREEN ${sIdx + 1}`}</ScreenBadge>
                      {isActive && <ActiveTag>ACTIVE IN EDITOR</ActiveTag>}
                    </ScreenTitleRow>
                    <ScreenName>{screen.name}</ScreenName>
                  </CardHeader>

                  {/* Mini Screen Preview */}
                  <MiniWatchFrame>
                    <MiniScreen style={{ background: screen.bgColor || '#0b0f19' }}>
                      {screen.elements.map(el => {
                        const hasAction = el.actions && el.actions.length > 0
                        return (
                          <MiniElement
                            key={el.id}
                            style={{
                              left: `${(el.props.x / 240) * 100}%`,
                              top: `${(el.props.y / 280) * 100}%`,
                              width: `${(Math.min(el.props.w || 60, 240) / 240) * 100}%`,
                              height: `${(Math.min(el.props.h || 30, 280) / 280) * 100}%`,
                              borderColor: hasAction ? '#10b981' : 'rgba(255,255,255,0.2)',
                              backgroundColor: hasAction ? 'rgba(16,185,129,0.3)' : 'rgba(255,255,255,0.08)',
                            }}
                            title={`${el.name} (${hasAction ? 'Has Actions' : 'Static'})`}
                          >
                            <span style={{ fontSize: 7, color: '#fff', overflow: 'hidden' }}>
                              {el.props.label || el.name}
                            </span>
                          </MiniElement>
                        )
                      })}
                    </MiniScreen>
                  </MiniWatchFrame>

                  {/* Mapped Actions on this Screen */}
                  <ActionsList>
                    <ActionsListHeader>
                      <span>Interactive Links ({interactiveEls.length})</span>
                    </ActionsListHeader>

                    {interactiveEls.length === 0 ? (
                      <NoActionsNotice>
                        No action blocks mapped on this screen yet.
                        {buttons.length > 0 && (
                          <QuickMapNotice>
                            Tip: Map <strong>{buttons[0].name}</strong> to navigate!
                          </QuickMapNotice>
                        )}
                      </NoActionsNotice>
                    ) : (
                      interactiveEls.map(el => (
                        <ActionRow key={el.id}>
                          <ActionElName>
                            <span>👆</span> {el.props.label || el.name}
                          </ActionElName>
                          {el.actions.map(act => (
                            <ActionPill key={act.id}>
                              {act.actionType === 'navigate' ? (
                                <>
                                  <span style={{ color: '#60a5fa' }}>➔ GO TO:</span>{' '}
                                  <strong>{screens.find(s => s.id === act.targetScreenId)?.name || 'Next Page'}</strong>
                                  <span style={{ fontSize: 9, opacity: 0.7 }}>({act.transition || 'slide'})</span>
                                </>
                              ) : act.actionType === 'scroll' ? (
                                <>
                                  <span style={{ color: '#34d399' }}>📜 SCROLL:</span>{' '}
                                  <strong>{act.scrollDirection || 'down'} {act.scrollAmount || 80}px</strong>
                                </>
                              ) : (
                                <>
                                  <span style={{ color: '#f59e0b' }}>🔔 ALERT:</span>{' '}
                                  <em>"{act.alertMessage || 'Notice'}"</em>
                                </>
                              )}
                            </ActionPill>
                          ))}
                        </ActionRow>
                      ))
                    )}
                  </ActionsList>

                  <CardFooter>
                    <EditScreenBtn onClick={() => { onSelectScreen(screen.id); onClose(); }}>
                      ✏️ Edit in Canvas
                    </EditScreenBtn>
                  </CardFooter>
                </ScreenCard>
              )
            })}
          </ScreensContainer>
        </ModalBody>

        <ModalFooter>
          <FooterLegend>
            <LegendItem><LegendColor $color="#10b981" /> Mapped Block Action</LegendItem>
            <LegendItem><LegendColor $color="#60a5fa" /> Screen Navigation</LegendItem>
            <LegendItem><LegendColor $color="#34d399" /> Scroll Action</LegendItem>
          </FooterLegend>
          <FooterActions>
            <ActionBtn $variant="secondary" onClick={onClose}>Done</ActionBtn>
            <ActionBtn $variant="primary" onClick={() => { onClose(); onEnterTestMode(); }}>
              ▶ Enter Test Mode
            </ActionBtn>
          </FooterActions>
        </ModalFooter>
      </Modal>
    </Backdrop>
  )
}

const Backdrop = styled.div`
  position: fixed;
  inset: 0;
  background: rgba(0,0,0,0.5);
  backdrop-filter: blur(6px);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 1500;
  padding: 24px;
`

const Modal = styled.div`
  background: var(--bg-surface);
  border-radius: var(--radius-lg);
  box-shadow: 0 25px 60px rgba(0,0,0,0.35), var(--neu-raised);
  border: 1px solid var(--border-subtle);
  width: 95vw;
  max-width: 1000px;
  max-height: 90vh;
  display: flex;
  flex-direction: column;
  overflow: hidden;
  animation: popIn 0.22s cubic-bezier(0.34, 1.56, 0.64, 1);
  @keyframes popIn {
    from { transform: scale(0.93) translateY(12px); opacity: 0; }
    to { transform: none; opacity: 1; }
  }
`

const ModalHeader = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 18px 24px;
  border-bottom: 1px solid var(--border-subtle);
  background: var(--bg-raised);
`

const HeaderLeft = styled.div`
  display: flex;
  align-items: center;
  gap: 12px;
`

const TitleIcon = styled.div`
  font-size: 24px;
`

const ModalTitle = styled.div`
  font-size: 16px;
  font-weight: 800;
  color: var(--text-primary);
`

const ModalSub = styled.div`
  font-size: 11px;
  color: var(--text-muted);
`

const HeaderRight = styled.div`
  display: flex;
  align-items: center;
  gap: 10px;
`

const TestModeBtn = styled.button`
  border: none;
  background: linear-gradient(135deg, #10b981, #059669);
  color: #fff;
  font-size: 12px;
  font-weight: 800;
  padding: 6px 14px;
  border-radius: var(--radius-sm);
  cursor: pointer;
  box-shadow: 0 4px 12px rgba(16,185,129,0.3);
  transition: all 0.15s;
  &:hover {
    transform: translateY(-1px);
    box-shadow: 0 6px 16px rgba(16,185,129,0.4);
  }
`

const CloseBtn = styled.button`
  width: 32px;
  height: 32px;
  border-radius: var(--radius-sm);
  border: none;
  background: var(--bg-raised);
  color: var(--text-muted);
  cursor: pointer;
  font-size: 14px;
  box-shadow: var(--neu-button);
  &:hover {
    color: var(--text-primary);
  }
`

const SummaryBar = styled.div`
  display: flex;
  align-items: center;
  gap: 16px;
  padding: 10px 24px;
  background: var(--bg-inset);
  border-bottom: 1px solid var(--border-subtle);
`

const StatBadge = styled.div`
  font-size: 11px;
  color: var(--text-secondary);
  strong {
    color: var(--text-primary);
  }
`

const AddScreenBarBtn = styled.button`
  margin-left: auto;
  border: 1px dashed var(--accent-blue);
  background: rgba(37,99,235,0.08);
  color: var(--accent-blue);
  font-size: 11px;
  font-weight: 700;
  padding: 4px 12px;
  border-radius: var(--radius-sm);
  cursor: pointer;
  transition: all 0.15s;
  &:hover {
    background: rgba(37,99,235,0.18);
  }
`

const ModalBody = styled.div`
  flex: 1;
  overflow-y: auto;
  padding: 24px;
`

const ScreensContainer = styled.div`
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
  gap: 20px;
`

const ScreenCard = styled.div`
  background: var(--bg-raised);
  border-radius: var(--radius-md);
  border: 1.5px solid ${p => p.$active ? 'var(--accent-blue)' : 'var(--border-subtle)'};
  box-shadow: var(--neu-button);
  padding: 14px;
  display: flex;
  flex-direction: column;
  gap: 12px;
  position: relative;
  transition: all 0.2s;
  &:hover {
    box-shadow: 0 8px 24px rgba(0,0,0,0.12);
  }
`

const CardHeader = styled.div`
  display: flex;
  flex-direction: column;
  gap: 2px;
`

const ScreenTitleRow = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
`

const ScreenBadge = styled.span`
  font-size: 9px;
  font-weight: 800;
  color: var(--accent-blue);
  letter-spacing: 0.5px;
`

const ActiveTag = styled.span`
  font-size: 8px;
  font-weight: 800;
  padding: 2px 6px;
  border-radius: 4px;
  background: rgba(37,99,235,0.15);
  color: var(--accent-blue);
`

const ScreenName = styled.div`
  font-size: 14px;
  font-weight: 800;
  color: var(--text-primary);
`

const MiniWatchFrame = styled.div`
  width: 100%;
  height: 140px;
  background: #1e2433;
  border-radius: 12px;
  padding: 6px;
  box-shadow: inset 0 2px 6px rgba(0,0,0,0.4);
  display: flex;
  align-items: center;
  justify-content: center;
`

const MiniScreen = styled.div`
  width: 100px;
  height: 120px;
  border-radius: 8px;
  position: relative;
  overflow: hidden;
  box-shadow: 0 0 10px rgba(0,0,0,0.5);
`

const MiniElement = styled.div`
  position: absolute;
  border: 1px solid rgba(255,255,255,0.3);
  border-radius: 2px;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 1px;
  box-sizing: border-box;
`

const ActionsList = styled.div`
  display: flex;
  flex-direction: column;
  gap: 8px;
  background: var(--bg-inset);
  padding: 10px;
  border-radius: var(--radius-sm);
  box-shadow: var(--neu-inset);
  min-height: 80px;
`

const ActionsListHeader = styled.div`
  font-size: 10px;
  font-weight: 800;
  color: var(--text-muted);
  letter-spacing: 0.5px;
`

const NoActionsNotice = styled.div`
  font-size: 10px;
  color: var(--text-muted);
  text-align: center;
  padding: 10px 0;
`

const QuickMapNotice = styled.div`
  margin-top: 4px;
  color: var(--accent-blue);
`

const ActionRow = styled.div`
  display: flex;
  flex-direction: column;
  gap: 4px;
`

const ActionElName = styled.div`
  font-size: 10px;
  font-weight: 700;
  color: var(--text-primary);
  display: flex;
  align-items: center;
  gap: 4px;
`

const ActionPill = styled.div`
  background: var(--bg-surface);
  border: 1px solid var(--border-subtle);
  border-radius: 4px;
  padding: 3px 6px;
  font-size: 10px;
  color: var(--text-secondary);
  display: flex;
  align-items: center;
  gap: 6px;
  margin-left: 12px;
`

const CardFooter = styled.div`
  display: flex;
  align-items: center;
  justify-content: flex-end;
`

const EditScreenBtn = styled.button`
  border: none;
  background: var(--bg-raised);
  color: var(--text-secondary);
  font-size: 10px;
  font-weight: 700;
  padding: 4px 10px;
  border-radius: 4px;
  box-shadow: var(--neu-button);
  cursor: pointer;
  transition: all 0.15s;
  &:hover {
    color: var(--accent-blue);
    transform: translateY(-1px);
  }
`

const ModalFooter = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 14px 24px;
  border-top: 1px solid var(--border-subtle);
  background: var(--bg-raised);
`

const FooterLegend = styled.div`
  display: flex;
  align-items: center;
  gap: 14px;
`

const LegendItem = styled.div`
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: 11px;
  color: var(--text-muted);
`

const LegendColor = styled.span`
  width: 8px;
  height: 8px;
  border-radius: 2px;
  background: ${p => p.$color};
`

const FooterActions = styled.div`
  display: flex;
  align-items: center;
  gap: 10px;
`

const ActionBtn = styled.button`
  padding: 7px 18px;
  border-radius: var(--radius-sm);
  font-size: 12px;
  font-weight: 800;
  cursor: pointer;
  transition: all 0.18s;
  border: ${p => p.$variant === 'primary' ? 'none' : '1px solid var(--border-subtle)'};
  background: ${p => p.$variant === 'primary' ? 'linear-gradient(135deg, var(--accent-blue), #7c3aed)' : 'var(--bg-raised)'};
  color: ${p => p.$variant === 'primary' ? '#fff' : 'var(--text-secondary)'};
  box-shadow: ${p => p.$variant === 'primary' ? '0 4px 14px rgba(37,99,235,0.35)' : 'var(--neu-button)'};
  &:hover {
    transform: translateY(-1px);
  }
`
