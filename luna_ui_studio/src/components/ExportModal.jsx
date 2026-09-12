import React, { useState } from 'react'
import styled from 'styled-components'
import JSZip from 'jszip'
import {
  generateArduinoCode,
  generateMultiScreenArduinoCode,
  generateHelperHeader,
  generateLauncherIno,
  generateCompatHeader,
  generatePartitionsCsv,
  generateFlashReadme
} from '../ui-elements/code-generator'
import { legacyToCanonical, serializeCanonicalProject } from '../ui-core/index.js'

export default function ExportModal({ open, screens = [], activeScreenId = 'screen_1', elements = [], onClose }) {
  const [activeTab, setActiveTab] = useState(screens.length > 1 ? 'multi' : 'launcher')
  const [copied, setCopied] = useState(false)
  const [zipping, setZipping] = useState(false)

  if (!open) return null

  const getCodeForTab = (tab) => {
    switch (tab) {
      case 'launcher':
        return generateLauncherIno()
      case 'multi':
        return generateMultiScreenArduinoCode(screens, activeScreenId)
      case 'header':
        return generateHelperHeader()
      case 'compat':
        return generateCompatHeader()
      case 'partitions':
        return generatePartitionsCsv()
      case 'readme':
        return generateFlashReadme()
      case 'arduino':
        return generateArduinoCode(elements)
      case 'canonical':
        return serializeCanonicalProject(legacyToCanonical(screens, activeScreenId))
      case 'json':
      default:
        return JSON.stringify({ screens, exportedAt: new Date().toISOString() }, null, 2)
    }
  }

  const code = getCodeForTab(activeTab)

  const handleCopy = () => {
    navigator.clipboard.writeText(code).then(() => {
      setCopied(true)
      setTimeout(() => setCopied(false), 2000)
    })
  }

  const handleDownloadSingle = () => {
    const fnameMap = {
      launcher: 'Luna_169_Hardware_Launcher.ino',
      multi: 'Luna_MultiScreen_App.ino',
      header: 'luna_ui_elements.h',
      compat: 'luna_gfx_compat.h',
      partitions: 'partitions.csv',
      readme: 'README.md',
      arduino: 'drawGeneratedScreen.ino',
      canonical: 'canonical_project.json',
      json: 'luna_screens_project.json'
    }
    const fname = fnameMap[activeTab] || 'code.txt'
    const blob = new Blob([code], { type: 'text/plain' })
    const a = document.createElement('a')
    a.href = URL.createObjectURL(blob)
    a.download = fname
    a.click()
  }

  const handleDownloadZip = async () => {
    setZipping(true)
    try {
      const zip = new JSZip()
      const folder = zip.folder('Luna_169_Firmware')

      folder.file('Luna_169_Firmware.ino', generateLauncherIno())
      folder.file('Luna_MultiScreen_App.ino', generateMultiScreenArduinoCode(screens, activeScreenId))
      folder.file('luna_ui_elements.h', generateHelperHeader())
      folder.file('luna_gfx_compat.h', generateCompatHeader())
      folder.file('partitions.csv', generatePartitionsCsv())
      folder.file('README.md', generateFlashReadme())
      folder.file('canonical_project.json', serializeCanonicalProject(legacyToCanonical(screens, activeScreenId)))
      folder.file('luna_screens_project.json', JSON.stringify({ screens, exportedAt: new Date().toISOString() }, null, 2))

      const content = await zip.generateAsync({ type: 'blob' })
      const a = document.createElement('a')
      a.href = URL.createObjectURL(content)
      a.download = 'Luna_169_Firmware_Package.zip'
      a.click()
    } catch (err) {
      console.error('Failed to generate ZIP', err)
      alert('Failed to generate ZIP package.')
    } finally {
      setZipping(false)
    }
  }

  return (
    <Backdrop onClick={e => e.target === e.currentTarget && onClose()}>
      <Modal>
        <ModalHeader>
          <ModalTitle>
            <TitleIcon>&lt;/&gt;</TitleIcon>
            <div>
              <div style={{ display: 'flex', alignItems: 'center', gap: 8 }}>
                <span style={{ fontSize: 16, fontWeight: 800 }}>Export Firmware Package</span>
                <VerifiedBadge>✓ Tested on Hardware (COM3)</VerifiedBadge>
              </div>
              <div style={{ fontSize: 11, color: 'var(--text-muted)', fontWeight: 400 }}>
                Waveshare ESP32-S3 1.69" (240×280 ST7789 • CST816T • Arduino_GFX • 16MB Flash)
              </div>
            </div>
          </ModalTitle>
          <CloseBtn onClick={onClose}>✕</CloseBtn>
        </ModalHeader>

        <ModalTabs>
          {[
            { id: 'multi', label: `Multi-Screen UI (${screens.length} Screens)` },
            { id: 'launcher', label: 'Launcher (.ino)' },
            { id: 'header', label: 'luna_ui_elements.h' },
            { id: 'compat', label: 'luna_gfx_compat.h' },
            { id: 'partitions', label: 'partitions.csv' },
            { id: 'readme', label: 'Flash Guide' },
            { id: 'canonical', label: '✨ Canonical Schema (UIProject)' },
            { id: 'json', label: 'Project JSON' },
          ].map(t => (
            <ModalTab key={t.id} $active={activeTab === t.id} onClick={() => setActiveTab(t.id)}>
              {t.label}
            </ModalTab>
          ))}
        </ModalTabs>

        <CodeArea value={code} readOnly spellCheck={false} />

        <ModalFooter>
          <FooterNote>
            Includes <strong>GPIO 41 Power Hold</strong>, <strong>Backlight</strong>, <strong>CST816T Touch</strong>, & <strong>Arduino_GFX</strong>
          </FooterNote>
          <FooterActions>
            <ActionBtn $variant="zip" onClick={handleDownloadZip} disabled={zipping}>
              {zipping ? '⏳ Packing...' : '📦 1-Click ZIP Package'}
            </ActionBtn>
            <ActionBtn $variant="secondary" onClick={handleDownloadSingle}>
              💾 Download Tab
            </ActionBtn>
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
  background: rgba(0,0,0,0.45);
  backdrop-filter: blur(8px);
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
  box-shadow: 0 32px 80px rgba(0,0,0,0.3), var(--neu-raised);
  width: 780px;
  max-width: 95vw;
  max-height: 88vh;
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
  padding: 16px 20px;
  border-bottom: 1px solid var(--border-subtle);
  background: var(--bg-surface);
`

const ModalTitle = styled.div`
  display: flex;
  align-items: center;
  gap: 12px;
  color: var(--text-primary);
`

const VerifiedBadge = styled.span`
  background: rgba(34, 197, 94, 0.15);
  border: 1px solid rgba(34, 197, 94, 0.35);
  color: #4ade80;
  font-size: 10px;
  font-weight: 700;
  padding: 2px 8px;
  border-radius: 12px;
  letter-spacing: 0.3px;
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
  overflow-x: auto;
`

const ModalTab = styled.button`
  padding: 8px 12px;
  border: none;
  border-radius: var(--radius-sm) var(--radius-sm) 0 0;
  font-size: 11px;
  font-weight: 700;
  cursor: pointer;
  white-space: nowrap;
  background: ${p => p.$active ? 'var(--bg-raised)' : 'transparent'};
  color: ${p => p.$active ? 'var(--accent-blue)' : 'var(--text-muted)'};
  box-shadow: ${p => p.$active ? 'var(--neu-raised)' : 'none'};
`

const CodeArea = styled.textarea`
  flex: 1;
  padding: 16px;
  font-family: var(--font-mono);
  font-size: 11px;
  background: #0d1117;
  color: #e6edf3;
  border: none;
  outline: none;
  resize: none;
  line-height: 1.6;
  min-height: 320px;
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
  padding: 8px 16px;
  border-radius: var(--radius-sm);
  border: none;
  font-size: 12px;
  font-weight: 700;
  cursor: pointer;
  transition: all 0.15s;
  background: ${p => {
    if (p.$variant === 'zip') return 'linear-gradient(135deg, #10b981, #059669)'
    if (p.$variant === 'primary') return 'var(--accent-blue)'
    return 'var(--bg-raised)'
  }};
  color: ${p => (p.$variant === 'primary' || p.$variant === 'zip') ? '#fff' : 'var(--text-secondary)'};
  box-shadow: ${p => {
    if (p.$variant === 'zip') return '0 4px 14px rgba(16, 185, 129, 0.35)'
    if (p.$variant === 'primary') return '0 4px 12px rgba(37,99,235,0.3)'
    return 'var(--neu-button)'
  }};
  &:hover { transform: translateY(-1px); filter: brightness(1.08); }
  &:disabled { opacity: 0.6; cursor: not-allowed; transform: none; }
`
