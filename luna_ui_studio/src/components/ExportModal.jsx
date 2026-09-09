import React, { useState } from 'react'
import styled from 'styled-components'
import { generateArduinoCode, generateMultiScreenArduinoCode, generateHelperHeader } from '../ui-elements/code-generator'

export default function ExportModal({ open, screens = [], activeScreenId = 'screen_1', elements = [], onClose }) {
  const [activeTab, setActiveTab] = useState(screens.length > 1 ? 'multi' : 'arduino')
  const [copied, setCopied] = useState(false)

  if (!open) return null

  const code = activeTab === 'multi'
    ? generateMultiScreenArduinoCode(screens, activeScreenId)
    : activeTab === 'arduino'
    ? generateArduinoCode(elements)
    : activeTab === 'header'
    ? generateHelperHeader()
    : JSON.stringify({ screens, exportedAt: new Date().toISOString() }, null, 2)

  const handleCopy = () => {
    navigator.clipboard.writeText(code).then(() => {
      setCopied(true)
      setTimeout(() => setCopied(false), 2000)
    })
  }

  const handleDownload = () => {
    const fname = activeTab === 'multi' ? 'Luna_MultiScreen_App.ino'
      : activeTab === 'arduino' ? 'drawGeneratedScreen.ino'
      : activeTab === 'header' ? 'luna_ui_elements.h' : 'luna_screens_project.json'
    const blob = new Blob([code], { type: 'text/plain' })
    const a = document.createElement('a')
    a.href = URL.createObjectURL(blob)
    a.download = fname
    a.click()
  }

  return (
    <Backdrop onClick={e => e.target === e.currentTarget && onClose()}>
      <Modal>
        <ModalHeader>
          <ModalTitle>
            <TitleIcon>&lt;/&gt;</TitleIcon>
            <div>
              <div style={{ fontSize: 16, fontWeight: 800 }}>Export Firmware Code</div>
              <div style={{ fontSize: 11, color: 'var(--text-muted)', fontWeight: 400 }}>ESP32-S3 • TFT_eSPI • 240×280 ST7789</div>
            </div>
          </ModalTitle>
          <CloseBtn onClick={onClose}>✕</CloseBtn>
        </ModalHeader>

        <ModalTabs>
          {[
            { id: 'multi', label: `Multi-Screen Firmware (${screens.length} Screens)` },
            { id: 'arduino', label: 'Current Screen Only' },
            { id: 'header', label: 'luna_ui_elements.h' },
            { id: 'json', label: 'Project JSON (Screens & Mappings)' },
          ].map(t => (
            <ModalTab key={t.id} $active={activeTab === t.id} onClick={() => setActiveTab(t.id)}>
              {t.label}
            </ModalTab>
          ))}
        </ModalTabs>

        <CodeArea value={code} readOnly spellCheck={false} />

        <ModalFooter>
          <FooterNote>Paste into your <strong>1.69 Luna Firmware.ino</strong></FooterNote>
          <FooterActions>
            <ActionBtn $variant="secondary" onClick={handleDownload}>💾 Download</ActionBtn>
            <ActionBtn $variant="primary" onClick={handleCopy}>
              {copied ? '✓ Copied!' : '📋 Copy Code'}
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
  background: rgba(0,0,0,0.35);
  backdrop-filter: blur(6px);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 1000;
  animation: fadeIn 0.15s ease;
  @keyframes fadeIn { from { opacity: 0; } to { opacity: 1; } }
`

const Modal = styled.div`
  background: var(--bg-raised);
  border-radius: var(--radius-xl);
  box-shadow: 0 32px 80px rgba(0,0,0,0.2), var(--neu-raised);
  width: 700px;
  max-width: 95vw;
  max-height: 85vh;
  display: flex;
  flex-direction: column;
  overflow: hidden;
  animation: slideUp 0.2s ease;
  @keyframes slideUp { from { transform: translateY(20px); opacity: 0; } to { transform: none; opacity: 1; } }
`

const ModalHeader = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 18px 20px;
  border-bottom: 1px solid var(--border-subtle);
  background: var(--bg-surface);
`

const ModalTitle = styled.div`
  display: flex;
  align-items: center;
  gap: 12px;
  color: var(--text-primary);
`

const TitleIcon = styled.div`
  width: 36px;
  height: 36px;
  border-radius: var(--radius-sm);
  background: var(--accent-blue);
  color: #fff;
  display: flex;
  align-items: center;
  justify-content: center;
  font-weight: 800;
  font-size: 14px;
  box-shadow: 0 4px 12px rgba(37,99,235,0.3);
`

const CloseBtn = styled.button`
  width: 30px;
  height: 30px;
  border-radius: 50%;
  border: none;
  background: var(--bg-inset);
  box-shadow: var(--neu-button);
  color: var(--text-muted);
  font-size: 14px;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  &:hover { color: var(--accent-red); background: #ef444420; }
`

const ModalTabs = styled.div`
  display: flex;
  padding: 10px 20px 0;
  gap: 4px;
  border-bottom: 1px solid var(--border-subtle);
  background: var(--bg-surface);
`

const ModalTab = styled.button`
  padding: 8px 14px;
  border: none;
  border-radius: var(--radius-sm) var(--radius-sm) 0 0;
  font-size: 12px;
  font-weight: 700;
  cursor: pointer;
  background: ${p => p.$active ? 'var(--bg-raised)' : 'transparent'};
  color: ${p => p.$active ? 'var(--accent-blue)' : 'var(--text-muted)'};
  box-shadow: ${p => p.$active ? 'var(--neu-raised)' : 'none'};
`

const CodeArea = styled.textarea`
  flex: 1;
  padding: 16px;
  font-family: var(--font-mono);
  font-size: 12px;
  background: #0d1117;
  color: #e6edf3;
  border: none;
  outline: none;
  resize: none;
  line-height: 1.6;
  min-height: 300px;
`

const ModalFooter = styled.div`
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 12px 20px;
  border-top: 1px solid var(--border-subtle);
  background: var(--bg-surface);
`

const FooterNote = styled.div`
  font-size: 11px;
  color: var(--text-muted);
  strong { color: var(--text-secondary); }
`

const FooterActions = styled.div`
  display: flex;
  gap: 8px;
`

const ActionBtn = styled.button`
  padding: 8px 18px;
  border-radius: var(--radius-sm);
  border: none;
  font-size: 12px;
  font-weight: 700;
  cursor: pointer;
  transition: all 0.15s;
  background: ${p => p.$variant === 'primary' ? 'var(--accent-blue)' : 'var(--bg-raised)'};
  color: ${p => p.$variant === 'primary' ? '#fff' : 'var(--text-secondary)'};
  box-shadow: ${p => p.$variant === 'primary' ? '0 4px 12px rgba(37,99,235,0.3)' : 'var(--neu-button)'};
  &:hover { transform: translateY(-1px); filter: brightness(1.05); }
`
