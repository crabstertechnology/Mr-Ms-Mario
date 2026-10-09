import React from 'react';
import styled from 'styled-components';
import HideSeekApp from '../luna/hideseek/HideSeekApp';

export default function HideSeekModal({ open, onClose }) {
  if (!open) return null;

  return (
    <Backdrop onClick={(e) => e.target === e.currentTarget && onClose()}>
      <ModalContainer>
        <ModalHeader>
          <HeaderLeft>
            <TitleIcon>🐱</TitleIcon>
            <div>
              <ModalTitle>Luna OS · Hide &amp; Seek Experience</ModalTitle>
              <ModalSub>
                Exact 1:1 Smartwatch UI • Title Screen • Interactive Dial Setup • 3D Room Seek • Sound Effects
              </ModalSub>
            </div>
          </HeaderLeft>

          <HeaderRight>
            <StatusBadge>
              <Dot /> 240×280 LCD Profile
            </StatusBadge>
            <CloseBtn onClick={onClose}>✕</CloseBtn>
          </HeaderRight>
        </ModalHeader>

        <ModalBody>
          <HideSeekApp />
        </ModalBody>
      </ModalContainer>
    </Backdrop>
  );
}

const Backdrop = styled.div`
  position: fixed;
  inset: 0;
  background: rgba(8, 3, 16, 0.88);
  backdrop-filter: blur(16px);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 9999;
  padding: 16px;
`;

const ModalContainer = styled.div`
  background: #100620;
  border: 1px solid rgba(168, 85, 247, 0.35);
  border-radius: 24px;
  width: 95vw;
  max-width: 960px;
  max-height: 94vh;
  display: flex;
  flex-direction: column;
  box-shadow: 0 25px 60px rgba(0, 0, 0, 0.8), 0 0 50px rgba(168, 85, 247, 0.2);
  overflow: hidden;
`;

const ModalHeader = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 14px 22px;
  background: #17092E;
  border-bottom: 1px solid rgba(168, 85, 247, 0.25);
`;

const HeaderLeft = styled.div`
  display: flex;
  align-items: center;
  gap: 12px;
`;

const TitleIcon = styled.div`
  width: 40px;
  height: 40px;
  border-radius: 12px;
  background: rgba(168, 85, 247, 0.2);
  border: 1px solid rgba(168, 85, 247, 0.4);
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 20px;
`;

const ModalTitle = styled.div`
  font-size: 15px;
  font-weight: 800;
  color: #F3E8FF;
  letter-spacing: 0.5px;
  font-family: 'Outfit', sans-serif;
`;

const ModalSub = styled.div`
  font-size: 11px;
  color: #C084FC;
  margin-top: 1px;
`;

const HeaderRight = styled.div`
  display: flex;
  align-items: center;
  gap: 12px;
`;

const StatusBadge = styled.div`
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 4px 12px;
  border-radius: 20px;
  font-size: 11px;
  font-weight: 700;
  background: rgba(168, 85, 247, 0.2);
  border: 1px solid rgba(168, 85, 247, 0.4);
  color: #E9D5FF;
`;

const Dot = styled.div`
  width: 7px;
  height: 7px;
  border-radius: 50%;
  background: #A855F7;
  box-shadow: 0 0 8px #A855F7;
`;

const CloseBtn = styled.button`
  background: transparent;
  border: none;
  color: #C084FC;
  font-size: 18px;
  cursor: pointer;
  padding: 4px 10px;
  border-radius: 8px;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: all 0.2s;

  &:hover {
    color: #FFFFFF;
    background: rgba(255, 255, 255, 0.1);
  }
`;

const ModalBody = styled.div`
  flex: 1;
  overflow-y: auto;
  background: radial-gradient(ellipse 80% 60% at 50% 30%, #17092E 0%, #0A0216 70%, #040108 100%);
  display: flex;
  justify-content: center;
  align-items: flex-start;
  padding: 20px 0;
`;
