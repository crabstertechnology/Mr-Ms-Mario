import React from 'react'
import styled from 'styled-components'
import LunaDevice from '../luna/LunaDevice'

export default function FigmaLunaModal({ open, onClose }) {
  if (!open) return null

  return (
    <Backdrop onClick={e => e.target === e.currentTarget && onClose()}>
      <Modal>
        <ModalHeader>
          <HeaderLeft>
            <TitleIcon>✨</TitleIcon>
            <div>
              <ModalTitle>Figma Luna OS UI/UX Flow Simulator</ModalTitle>
              <ModalSub>
                Original Figma Design Package • 8 Interactive Screens • 8 Retro Arcade Games • Hardware Tested (COM3)
              </ModalSub>
            </div>
          </HeaderLeft>

          <HeaderRight>
            <StatusBadge>
              <Dot /> ESP32-S3 Flash Ready (COM3)
            </StatusBadge>
            <CloseBtn onClick={onClose}>✕</CloseBtn>
          </HeaderRight>
        </ModalHeader>

        <ModalBody>
          <SimulatorContainer>
            <LunaDevice />
          </SimulatorContainer>
        </ModalBody>
      </Modal>
    </Backdrop>
  )
}

const Backdrop = styled.div`
  position: fixed;
  inset: 0;
  background: rgba(4, 7, 13, 0.88);
  backdrop-filter: blur(12px);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 9999;
  padding: 16px;
`

const Modal = styled.div`
  background: #0d1117;
  border: 1px solid #30363d;
  border-radius: 20px;
  width: 95vw;
  max-width: 900px;
  max-height: 94vh;
  display: flex;
  flex-direction: column;
  box-shadow: 0 25px 60px rgba(0, 0, 0, 0.7), 0 0 40px rgba(56, 189, 248, 0.15);
  overflow: hidden;
`

const ModalHeader = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 14px 20px;
  background: #161b22;
  border-bottom: 1px solid #30363d;
`

const HeaderLeft = styled.div`
  display: flex;
  align-items: center;
  gap: 12px;
`

const TitleIcon = styled.div`
  width: 38px;
  height: 38px;
  border-radius: 10px;
  background: rgba(56, 189, 248, 0.15);
  border: 1px solid rgba(56, 189, 248, 0.3);
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 18px;
`

const ModalTitle = styled.div`
  font-size: 15px;
  font-weight: 700;
  color: #e6edf3;
  letter-spacing: 0.5px;
`

const ModalSub = styled.div`
  font-size: 11px;
  color: #8b949e;
  margin-top: 1px;
`

const HeaderRight = styled.div`
  display: flex;
  align-items: center;
  gap: 12px;
`

const StatusBadge = styled.div`
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 4px 10px;
  border-radius: 20px;
  font-size: 11px;
  font-weight: 600;
  background: rgba(34, 197, 94, 0.15);
  border: 1px solid rgba(34, 197, 94, 0.3);
  color: #4ade80;
`

const Dot = styled.div`
  width: 6px;
  height: 6px;
  border-radius: 50%;
  background: #22c55e;
  box-shadow: 0 0 6px #22c55e;
`

const CloseBtn = styled.button`
  background: transparent;
  border: none;
  color: #8b949e;
  font-size: 18px;
  cursor: pointer;
  padding: 4px 8px;
  border-radius: 6px;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: all 0.2s;

  &:hover {
    color: #e6edf3;
    background: rgba(255, 255, 255, 0.08);
  }
`

const ModalBody = styled.div`
  flex: 1;
  overflow-y: auto;
  background: radial-gradient(ellipse 80% 60% at 50% 30%, #0d1829 0%, #06080f 60%, #000000 100%);
  display: flex;
  justify-content: center;
  align-items: flex-start;
  padding: 20px 0;
`

const SimulatorContainer = styled.div`
  width: 100%;
  display: flex;
  justify-content: center;
`
