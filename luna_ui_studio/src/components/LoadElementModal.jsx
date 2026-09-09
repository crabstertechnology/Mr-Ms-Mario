// ============================================================
// Luna Display Studio — React UI Compiler & Element Loader Modal
// Compiles pasted UIverse / React + styled-components code
// with full working animations, live preview, and studio registration.
// ============================================================
import React, { useState, useEffect, useRef } from 'react'
import styled from 'styled-components'
import { compileReactComponent, inferElementMetadata } from '../utils/reactCompiler'
import { registerCustomElement } from '../ui-elements/registry'
import {
  getSavedComponents,
  saveComponent,
  deleteComponent,
  downloadComponentFile,
  exportAllComponents,
  importComponentsFromFile,
} from '../utils/customComponentStorage'

const PRESETS = [
  {
    id: 'damith_yellow',
    name: 'Yellow Neon Flip Button',
    category: 'buttons',
    code: `import React from 'react';
import styled from 'styled-components';

const YellowNeonButton = ({ label = 'BUTTON' }) => {
  return (
    <StyledWrapper $label={label}>
      <button type="button" data-label={label}><span />{label}</button>
    </StyledWrapper>
  );
};

const StyledWrapper = styled.div\`
  display: flex;
  align-items: center;
  justify-content: center;
  width: 100%;
  height: 100%;
  background: #363636;
  overflow: hidden;

  button {
    padding: 0.9em 1.8em;
    text-transform: uppercase;
    text-decoration: none;
    letter-spacing: 4px;
    color: transparent;
    border: 3px solid #ffff00;
    font-size: 14px;
    position: relative;
    font-family: inherit;
    background: transparent;
    cursor: pointer;
    white-space: nowrap;
  }

  button::before {
    content: "\${p => p.$label || 'BUTTON'}";
    position: absolute;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background-color: #363636;
    color: #ffff00;
    display: flex;
    justify-content: center;
    align-items: center;
    transition: all 0.5s;
    font-size: 14px;
    text-transform: uppercase;
    letter-spacing: 4px;
  }

  button:hover::before {
    left: 100%;
    transform: scale(0) rotateY(360deg);
    opacity: 0;
  }

  button::after {
    content: "\${p => p.$label || 'BUTTON'}";
    position: absolute;
    top: 0;
    left: -100%;
    width: 100%;
    height: 100%;
    background-color: #363636;
    color: #ffff00;
    display: flex;
    justify-content: center;
    align-items: center;
    transition: all 0.5s;
    transform: scale(0) rotateY(0deg);
    opacity: 0;
    font-size: 14px;
    text-transform: uppercase;
    letter-spacing: 4px;
  }

  button:hover::after {
    left: 0;
    transform: scale(1) rotateY(360deg);
    opacity: 1;
  }
\`;

export default YellowNeonButton;`
  },
  {
    id: 'happy_coding',
    name: 'Happy Coding 3D Button',
    category: 'buttons',
    code: `import React from 'react';
import styled from 'styled-components';

const HappyCodingButton = ({ label = 'Happy Coding!' }) => {
  return (
    <StyledWrapper>
      <button className="button" role="button">{label}</button>
    </StyledWrapper>
  );
};

const StyledWrapper = styled.div\`
  display: flex;
  align-items: center;
  justify-content: center;
  width: 100%;
  height: 100%;

  .button {
    align-items: center;
    appearance: none;
    background-color: #EEF2FF;
    border-radius: 8px;
    border: 2px solid #536DFE;
    box-shadow: rgba(83, 109, 254, 0.2) 0 2px 4px, rgba(83, 109, 254, 0.15) 0 7px 13px -3px, #D6D6E7 0 -3px 0 inset;
    box-sizing: border-box;
    color: #536DFE;
    cursor: pointer;
    display: inline-flex;
    font-family: "JetBrains Mono", monospace;
    width: 100%;
    height: 100%;
    justify-content: center;
    padding-left: 20px;
    padding-right: 20px;
    position: relative;
    text-align: center;
    transition: box-shadow 0.15s, transform 0.15s;
    user-select: none;
    font-size: 14px;
    font-weight: 700;
  }

  .button:hover {
    box-shadow: rgba(83, 109, 254, 0.3) 0 4px 8px, rgba(83, 109, 254, 0.2) 0 7px 13px -3px, #D6D6E7 0 -3px 0 inset;
    transform: translateY(-2px);
  }

  .button:active {
    box-shadow: #D6D6E7 0 3px 7px inset;
    transform: translateY(2px);
  }
\`;

export default HappyCodingButton;`
  },
  {
    id: 'cyber_loader',
    name: 'Quantum Pulsar Loader',
    category: 'loaders',
    code: `import React from 'react';
import styled, { keyframes } from 'styled-components';

const QuantumLoader = ({ label = 'PROCESSING...' }) => {
  return (
    <Container>
      <Rings>
        <div className="ring ring1" />
        <div className="ring ring2" />
        <div className="core" />
      </Rings>
      <Label>{label}</Label>
    </Container>
  );
};

const spin = keyframes\`
  0% { transform: rotate(0deg); }
  100% { transform: rotate(360deg); }
\`;

const pulse = keyframes\`
  0%, 100% { transform: scale(0.8); opacity: 0.6; }
  50% { transform: scale(1.1); opacity: 1; }
\`;

const Container = styled.div\`
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: 8px;
  width: 100%;
  height: 100%;
  background: #0f172a;
  border-radius: 8px;
\`;

const Rings = styled.div\`
  position: relative;
  width: 44px;
  height: 44px;

  .ring {
    position: absolute;
    inset: 0;
    border-radius: 50%;
    border: 2px solid transparent;
  }

  .ring1 {
    border-top-color: #38bdf8;
    border-bottom-color: #38bdf8;
    animation: \${spin} 1s linear infinite;
  }

  .ring2 {
    border-left-color: #a855f7;
    border-right-color: #a855f7;
    animation: \${spin} 1.5s linear infinite reverse;
  }

  .core {
    position: absolute;
    inset: 14px;
    border-radius: 50%;
    background: #06b6d4;
    box-shadow: 0 0 10px #06b6d4;
    animation: \${pulse} 1s ease-in-out infinite;
  }
\`;

const Label = styled.span\`
  font-size: 9px;
  font-family: 'JetBrains Mono', monospace;
  color: #38bdf8;
  letter-spacing: 1px;
\`;

export default QuantumLoader;`
  }
]

export default function LoadElementModal({ open, onClose, onAddElement }) {
  const [code, setCode] = useState(PRESETS[0].code)
  const [selectedPreset, setSelectedPreset] = useState(PRESETS[0].id)
  const [name, setName] = useState(PRESETS[0].name)
  const [category, setCategory] = useState(PRESETS[0].category)
  const [width, setWidth] = useState(160)
  const [height, setHeight] = useState(48)
  const [label, setLabel] = useState('BUTTON')
  const [compileStatus, setCompileStatus] = useState({ success: false, error: null })
  const [CompiledComp, setCompiledComp] = useState(null)
  const [savedComponents, setSavedComponents] = useState([])
  const [saveStatus, setSaveStatus] = useState(null) // 'saved' | 'error' | null
  const [showSaved, setShowSaved] = useState(false)
  const importRef = useRef(null)

  // Load saved components from localStorage on open
  useEffect(() => {
    if (open) {
      setSavedComponents(getSavedComponents())
    }
  }, [open])

  // Auto-register saved components into studio palette on load
  useEffect(() => {
    const saved = getSavedComponents()
    saved.forEach(comp => {
      const res = compileReactComponent(comp.code)
      if (!res.error && res.Component) {
        registerCustomElement(comp.id, {
          name: comp.name,
          category: comp.category || 'custom',
          description: `Saved custom component: ${comp.name}`,
          defaultProps: {
            x: 40, y: 100,
            w: comp.width || 160,
            h: comp.height || 48,
            label: comp.label || 'BUTTON',
            bgColor: '#1e293b',
            borderColor: '#3b82f6',
            textColor: '#ffffff',
          },
          component: res.Component,
        })
      }
    })
  }, [])

  // Compile code on change
  useEffect(() => {
    if (!open) return
    const res = compileReactComponent(code)
    if (res.error) {
      setCompileStatus({ success: false, error: res.error })
      setCompiledComp(null)
    } else {
      setCompileStatus({ success: true, error: null })
      setCompiledComp(() => res.Component)
      const meta = inferElementMetadata(code)
      if (meta.w) setWidth(meta.w)
      if (meta.h) setHeight(meta.h)
      if (meta.label) setLabel(meta.label)
    }
  }, [code, open])

  const handleSelectPreset = (preset) => {
    setSelectedPreset(preset.id)
    setCode(preset.code)
    setName(preset.name)
    setCategory(preset.category)
  }

  const handleLoadSaved = (saved) => {
    setSelectedPreset(saved.id)
    setCode(saved.code)
    setName(saved.name)
    setCategory(saved.category || 'custom')
    setWidth(saved.width || 160)
    setHeight(saved.height || 48)
    setLabel(saved.label || 'BUTTON')
    setShowSaved(false)
  }

  const handleDeleteSaved = (id, e) => {
    e.stopPropagation()
    deleteComponent(id)
    setSavedComponents(getSavedComponents())
  }

  const handleSaveToFile = () => {
    if (!compileStatus.success) return
    try {
      saveComponent({ name, category, code, width, height, label })
      setSavedComponents(getSavedComponents())
      setSaveStatus('saved')
      setTimeout(() => setSaveStatus(null), 2500)
    } catch {
      setSaveStatus('error')
      setTimeout(() => setSaveStatus(null), 2500)
    }
  }

  const handleDownloadFile = () => {
    downloadComponentFile(name, code)
  }

  const handleImport = async (e) => {
    const file = e.target.files?.[0]
    if (!file) return
    try {
      const count = await importComponentsFromFile(file)
      setSavedComponents(getSavedComponents())
      setSaveStatus('saved')
      setTimeout(() => setSaveStatus(null), 2500)
      alert(`✅ Imported ${count} new component(s) successfully!`)
    } catch (err) {
      alert('❌ Import failed: ' + err.message)
    }
    e.target.value = ''
  }

  const handleRegisterAndAdd = () => {
    if (!CompiledComp) return
    const key = `compiled_${Date.now()}_${name.toLowerCase().replace(/[^a-z0-9]/g, '_')}`
    const definition = {
      name,
      category,
      description: `Compiled with Live React Engine: ${name}`,
      defaultProps: {
        x: 40,
        y: 100,
        w: Number(width) || 160,
        h: Number(height) || 48,
        label,
        bgColor: '#1e293b',
        borderColor: '#3b82f6',
        textColor: '#ffffff',
      },
      component: CompiledComp,
    }

    registerCustomElement(key, definition)
    if (onAddElement) {
      onAddElement(key)
    }
    onClose()
  }

  if (!open) return null

  return (
    <Backdrop onClick={e => { if (e.target === e.currentTarget) onClose() }}>
      <Modal>
        {/* Header */}
        <ModalHeader>
          <HeaderLeft>
            <TitleIcon>⚡</TitleIcon>
            <div>
              <ModalTitle>React UI Compiler & Element Loader</ModalTitle>
              <ModalSub>
                Paste any UIverse or React + styled-components code. Compiles live with full animations into Luna Studio!
              </ModalSub>
            </div>
          </HeaderLeft>
          <CloseBtn onClick={onClose}>✕</CloseBtn>
        </ModalHeader>

        {/* Presets Bar */}
        <PresetsBar>
          <span style={{ fontSize: 11, fontWeight: 700, color: 'var(--text-secondary)' }}>Presets:</span>
          {PRESETS.map(preset => (
            <PresetBtn
              key={preset.id}
              $active={selectedPreset === preset.id}
              onClick={() => handleSelectPreset(preset)}
            >
              {preset.name}
            </PresetBtn>
          ))}
          <PresetBtn
            $active={selectedPreset === 'custom'}
            onClick={() => {
              setSelectedPreset('custom')
              setCode('')
              setName('My Custom Component')
              setCategory('custom')
            }}
          >
            ✏️ Paste New Code
          </PresetBtn>
          <SavedBtn onClick={() => setShowSaved(s => !s)} $active={showSaved}>
            📁 My Saved ({savedComponents.length})
          </SavedBtn>
          <input
            ref={importRef}
            type="file"
            accept=".json"
            style={{ display: 'none' }}
            onChange={handleImport}
          />
          <SavedBtn onClick={() => importRef.current?.click()}>
            ⬆️ Import
          </SavedBtn>
          {savedComponents.length > 0 && (
            <SavedBtn onClick={exportAllComponents}>
              ⬇️ Export All
            </SavedBtn>
          )}
        </PresetsBar>

        {/* Saved Components Dropdown */}
        {showSaved && (
          <SavedPanel>
            <SavedPanelHeader>
              <span>📁 My Saved Components ({savedComponents.length})</span>
              <CloseBtn onClick={() => setShowSaved(false)} style={{ fontSize: 12, padding: '2px 8px' }}>✕</CloseBtn>
            </SavedPanelHeader>
            {savedComponents.length === 0 ? (
              <SavedEmpty>No saved components yet. Compile a component and click "Save to Library" to save it here.</SavedEmpty>
            ) : (
              <SavedList>
                {savedComponents.map(comp => (
                  <SavedItem key={comp.id} onClick={() => handleLoadSaved(comp)}>
                    <SavedItemInfo>
                      <SavedItemName>{comp.name}</SavedItemName>
                      <SavedItemMeta>{comp.category} · {comp.width}×{comp.height}px · saved {new Date(comp.savedAt).toLocaleDateString()}</SavedItemMeta>
                    </SavedItemInfo>
                    <SavedItemActions>
                      <SavedActionBtn title="Download as .jsx" onClick={e => { e.stopPropagation(); downloadComponentFile(comp.name, comp.code) }}>⬇️</SavedActionBtn>
                      <SavedActionBtn title="Delete" $danger onClick={e => handleDeleteSaved(comp.id, e)}>🗑️</SavedActionBtn>
                    </SavedItemActions>
                  </SavedItem>
                ))}
              </SavedList>
            )}
          </SavedPanel>
        )}

        {/* Content Body */}
        <ModalBody>
          {/* Left: Code Editor */}
          <EditorCol>
            <ColHeader>
              <span>React JSX & styled-components Code</span>
              <CompilerTag $success={compileStatus.success}>
                {compileStatus.success ? '✓ React Compiler Ready' : '✕ Compilation Error'}
              </CompilerTag>
            </ColHeader>

            <CodeTextarea
              value={code}
              onChange={e => setCode(e.target.value)}
              placeholder="Paste your React component here..."
              spellCheck="false"
            />

            {compileStatus.error && (
              <ErrorBox>
                <strong>Compile Error:</strong> {compileStatus.error}
              </ErrorBox>
            )}
          </EditorCol>

          {/* Right: Live Animation Test Stage */}
          <PreviewCol>
            <ColHeader>
              <span>Live Animation & Interactive Test</span>
              <span style={{ fontSize: 10, color: 'var(--text-muted)' }}>Hover or click to test animations</span>
            </ColHeader>

            <PreviewStage>
              <TestChassis>
                <div style={{ fontSize: 9, color: '#64748b', marginBottom: 12, textTransform: 'uppercase', letterSpacing: 1 }}>
                  Display Surface (240×280 ST7789)
                </div>
                <ComponentTestContainer style={{ width: Number(width) || 160, height: Number(height) || 48 }}>
                  {CompiledComp ? (
                    <CompiledComp
                      label={label}
                      w={Number(width) || 160}
                      h={Number(height) || 48}
                      bgColor="#1e293b"
                      borderColor="#3b82f6"
                      textColor="#ffffff"
                    />
                  ) : (
                    <div style={{ color: '#ef4444', fontSize: 11, padding: 12, textAlign: 'center' }}>
                      Unable to render preview. Check syntax above.
                    </div>
                  )}
                </ComponentTestContainer>
                <div style={{ fontSize: 10, color: '#94a3b8', marginTop: 14 }}>
                  Hover mouse over component to test CSS hover & keyframe animations!
                </div>
              </TestChassis>
            </PreviewStage>

            {/* Element Properties */}
            <MetaGrid>
              <FormGroup>
                <Label>Element Name</Label>
                <Input value={name} onChange={e => setName(e.target.value)} />
              </FormGroup>
              <FormGroup>
                <Label>Category</Label>
                <Select value={category} onChange={e => setCategory(e.target.value)}>
                  <option value="buttons">Buttons</option>
                  <option value="cards">Cards</option>
                  <option value="loaders">Loaders</option>
                  <option value="toggles">Toggles</option>
                  <option value="custom">Compiled / Custom</option>
                </Select>
              </FormGroup>
              <FormGroup>
                <Label>Width (px)</Label>
                <Input type="number" value={width} onChange={e => setWidth(Number(e.target.value))} />
              </FormGroup>
              <FormGroup>
                <Label>Height (px)</Label>
                <Input type="number" value={height} onChange={e => setHeight(Number(e.target.value))} />
              </FormGroup>
              <FormGroup style={{ gridColumn: 'span 2' }}>
                <Label>Text / Label Property</Label>
                <Input value={label} onChange={e => setLabel(e.target.value)} />
              </FormGroup>
            </MetaGrid>
          </PreviewCol>
        </ModalBody>

        {/* Footer */}
        <ModalFooter>
          <div style={{ display: 'flex', alignItems: 'center', gap: 8 }}>
            <span style={{ fontSize: 11, color: 'var(--text-muted)' }}>
              Compiled with in-browser Babel + React 19 + styled-components engine.
            </span>
            {saveStatus === 'saved' && (
              <SaveBadge>✅ Saved to Library!</SaveBadge>
            )}
            {saveStatus === 'error' && (
              <SaveBadge $error>❌ Save failed</SaveBadge>
            )}
          </div>
          <FooterActions>
            <CancelBtn onClick={onClose}>Cancel</CancelBtn>
            <DownloadBtn
              disabled={!compileStatus.success}
              onClick={handleDownloadFile}
              title="Download the component code as a .jsx file"
            >
              ⬇️ Download .jsx
            </DownloadBtn>
            <SaveLibBtn
              disabled={!compileStatus.success}
              onClick={handleSaveToFile}
              title="Save this component to the persistent library (survives page refresh)"
            >
              💾 Save to Library
            </SaveLibBtn>
            <LoadBtn
              disabled={!compileStatus.success}
              onClick={handleRegisterAndAdd}
            >
              ➕ Add to Canvas
            </LoadBtn>
          </FooterActions>
        </ModalFooter>
      </Modal>
    </Backdrop>
  )
}

// ── Styled Components ──────────────────────────────────────

const Backdrop = styled.div`
  position: fixed;
  inset: 0;
  background: rgba(15, 23, 42, 0.7);
  backdrop-filter: blur(8px);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 9999;
  padding: 24px;
`

const Modal = styled.div`
  width: 100%;
  max-width: 1040px;
  height: 88vh;
  background: var(--bg-base);
  border-radius: var(--radius-lg);
  box-shadow: 0 20px 60px rgba(0,0,0,0.35);
  border: 1px solid var(--border-subtle);
  display: flex;
  flex-direction: column;
  overflow: hidden;
  animation: modalScale 0.2s ease-out;
  @keyframes modalScale {
    from { opacity: 0; transform: scale(0.97); }
    to { opacity: 1; transform: scale(1); }
  }
`

const ModalHeader = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 16px 24px;
  border-bottom: 1px solid var(--border-subtle);
  background: var(--bg-raised);
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
  background: linear-gradient(135deg, #2563eb, #7c3aed);
  color: #fff;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 18px;
  box-shadow: 0 4px 12px rgba(37,99,235,0.3);
`

const ModalTitle = styled.h2`
  margin: 0;
  font-size: 16px;
  font-weight: 800;
  color: var(--text-primary);
`

const ModalSub = styled.p`
  margin: 2px 0 0;
  font-size: 11px;
  color: var(--text-secondary);
`

const CloseBtn = styled.button`
  background: none;
  border: none;
  font-size: 18px;
  color: var(--text-muted);
  cursor: pointer;
  padding: 4px 8px;
  border-radius: 6px;
  transition: all 0.15s;
  &:hover { background: var(--border-subtle); color: var(--text-primary); }
`

const PresetsBar = styled.div`
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 10px 24px;
  background: var(--bg-base);
  border-bottom: 1px solid var(--border-subtle);
  overflow-x: auto;
`

const PresetBtn = styled.button`
  padding: 5px 12px;
  border-radius: 20px;
  font-size: 11px;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.15s;
  white-space: nowrap;
  border: 1px solid ${p => p.$active ? 'var(--accent-blue)' : 'var(--border-subtle)'};
  background: ${p => p.$active ? 'var(--accent-blue)' : 'var(--bg-raised)'};
  color: ${p => p.$active ? '#ffffff' : 'var(--text-primary)'};
  box-shadow: ${p => p.$active ? '0 2px 8px rgba(37,99,235,0.3)' : 'var(--neu-button)'};
  &:hover {
    background: ${p => p.$active ? 'var(--accent-blue)' : 'var(--bg-surface)'};
  }
`

const ModalBody = styled.div`
  flex: 1;
  display: grid;
  grid-template-columns: 1.15fr 0.85fr;
  overflow: hidden;
`

const EditorCol = styled.div`
  display: flex;
  flex-direction: column;
  border-right: 1px solid var(--border-subtle);
  background: #0f172a;
  overflow: hidden;
`

const ColHeader = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 10px 16px;
  background: rgba(15, 23, 42, 0.9);
  border-bottom: 1px solid rgba(255,255,255,0.08);
  font-size: 11px;
  font-weight: 700;
  color: #94a3b8;
`

const CompilerTag = styled.span`
  font-size: 10px;
  font-weight: 700;
  padding: 3px 8px;
  border-radius: 12px;
  background: ${p => p.$success ? 'rgba(16, 185, 129, 0.2)' : 'rgba(239, 68, 68, 0.2)'};
  color: ${p => p.$success ? '#34d399' : '#f87171'};
  border: 1px solid ${p => p.$success ? '#059669' : '#dc2626'};
`

const CodeTextarea = styled.textarea`
  flex: 1;
  background: transparent;
  color: #e2e8f0;
  font-family: 'JetBrains Mono', monospace;
  font-size: 12px;
  line-height: 1.5;
  padding: 16px;
  border: none;
  resize: none;
  outline: none;
  white-space: pre;
  overflow: auto;
  tab-size: 2;
  &::selection {
    background: #2563eb;
    color: #fff;
  }
`

const ErrorBox = styled.div`
  padding: 10px 16px;
  background: rgba(239, 68, 68, 0.15);
  border-top: 1px solid #ef4444;
  color: #fca5a5;
  font-size: 11px;
  font-family: 'JetBrains Mono', monospace;
  max-height: 100px;
  overflow-y: auto;
`

const PreviewCol = styled.div`
  display: flex;
  flex-direction: column;
  background: var(--bg-base);
  overflow-y: auto;
`

const PreviewStage = styled.div`
  padding: 20px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: radial-gradient(var(--shadow-dark) 1px, transparent 1px);
  background-size: 16px 16px;
  border-bottom: 1px solid var(--border-subtle);
`

const TestChassis = styled.div`
  background: #0b0f19;
  border-radius: 16px;
  padding: 24px;
  box-shadow: 0 10px 30px rgba(0,0,0,0.3), inset 0 0 16px rgba(0,0,0,0.5);
  border: 2px solid #1e293b;
  display: flex;
  flex-direction: column;
  align-items: center;
  min-width: 260px;
`

const ComponentTestContainer = styled.div`
  display: flex;
  align-items: center;
  justify-content: center;
  position: relative;
  pointer-events: auto;
  cursor: pointer;
  transition: transform 0.1s;
`

const MetaGrid = styled.div`
  padding: 16px 20px;
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 12px;
`

const FormGroup = styled.div`
  display: flex;
  flex-direction: column;
  gap: 4px;
`

const Label = styled.label`
  font-size: 11px;
  font-weight: 700;
  color: var(--text-secondary);
`

const Input = styled.input`
  padding: 7px 10px;
  border-radius: var(--radius-sm);
  border: 1px solid var(--border-subtle);
  background: var(--bg-surface);
  color: var(--text-primary);
  font-size: 12px;
  outline: none;
  transition: border-color 0.15s;
  &:focus { border-color: var(--accent-blue); }
`

const Select = styled.select`
  padding: 7px 10px;
  border-radius: var(--radius-sm);
  border: 1px solid var(--border-subtle);
  background: var(--bg-surface);
  color: var(--text-primary);
  font-size: 12px;
  outline: none;
  cursor: pointer;
  &:focus { border-color: var(--accent-blue); }
`

const ModalFooter = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 14px 24px;
  border-top: 1px solid var(--border-subtle);
  background: var(--bg-raised);
`

const FooterActions = styled.div`
  display: flex;
  gap: 10px;
`

const CancelBtn = styled.button`
  padding: 8px 16px;
  border-radius: var(--radius-sm);
  border: 1px solid var(--border-subtle);
  background: var(--bg-surface);
  color: var(--text-secondary);
  font-size: 12px;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.15s;
  &:hover { background: var(--bg-inset); color: var(--text-primary); }
`

const LoadBtn = styled.button`
  padding: 8px 18px;
  border-radius: var(--radius-sm);
  border: none;
  background: var(--accent-blue);
  color: #fff;
  font-size: 12px;
  font-weight: 700;
  cursor: pointer;
  box-shadow: 0 4px 14px rgba(37,99,235,0.35);
  transition: all 0.15s;
  &:disabled {
    opacity: 0.5;
    cursor: not-allowed;
    box-shadow: none;
  }
  &:not(:disabled):hover {
    background: #1d4ed8;
    transform: translateY(-1px);
    box-shadow: 0 6px 18px rgba(37,99,235,0.45);
  }
`

const SavedBtn = styled.button`
  padding: 5px 12px;
  border-radius: var(--radius-sm);
  border: 1px solid ${p => p.$active ? 'var(--accent-blue)' : 'var(--border-subtle)'};
  background: ${p => p.$active ? 'rgba(37,99,235,0.12)' : 'var(--bg-surface)'};
  color: ${p => p.$active ? 'var(--accent-blue)' : 'var(--text-secondary)'};
  font-size: 11px;
  font-weight: 600;
  cursor: pointer;
  white-space: nowrap;
  transition: all 0.15s;
  &:hover { border-color: var(--accent-blue); color: var(--accent-blue); }
`

const SavedPanel = styled.div`
  background: var(--bg-raised);
  border-bottom: 1px solid var(--border-subtle);
  max-height: 220px;
  overflow-y: auto;
  animation: slideDown 0.15s ease-out;
  @keyframes slideDown {
    from { opacity: 0; transform: translateY(-8px); }
    to { opacity: 1; transform: translateY(0); }
  }
`

const SavedPanelHeader = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 8px 16px;
  border-bottom: 1px solid var(--border-subtle);
  font-size: 12px;
  font-weight: 700;
  color: var(--text-primary);
  background: var(--bg-inset);
`

const SavedEmpty = styled.div`
  padding: 20px 16px;
  font-size: 12px;
  color: var(--text-muted);
  text-align: center;
  font-style: italic;
`

const SavedList = styled.div`
  display: flex;
  flex-direction: column;
`

const SavedItem = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 8px 16px;
  cursor: pointer;
  border-bottom: 1px solid var(--border-subtle);
  transition: background 0.1s;
  &:hover { background: var(--bg-surface); }
  &:last-child { border-bottom: none; }
`

const SavedItemInfo = styled.div`
  display: flex;
  flex-direction: column;
  gap: 2px;
`

const SavedItemName = styled.div`
  font-size: 12px;
  font-weight: 600;
  color: var(--text-primary);
`

const SavedItemMeta = styled.div`
  font-size: 10px;
  color: var(--text-muted);
`

const SavedItemActions = styled.div`
  display: flex;
  gap: 4px;
`

const SavedActionBtn = styled.button`
  background: none;
  border: 1px solid ${p => p.$danger ? 'rgba(239,68,68,0.3)' : 'var(--border-subtle)'};
  border-radius: 5px;
  padding: 3px 7px;
  font-size: 12px;
  cursor: pointer;
  transition: all 0.12s;
  &:hover {
    background: ${p => p.$danger ? 'rgba(239,68,68,0.12)' : 'var(--bg-surface)'};
    border-color: ${p => p.$danger ? '#ef4444' : 'var(--accent-blue)'};
  }
`

const SaveBadge = styled.span`
  font-size: 11px;
  font-weight: 700;
  padding: 3px 10px;
  border-radius: 100px;
  background: ${p => p.$error ? 'rgba(239,68,68,0.15)' : 'rgba(34,197,94,0.15)'};
  color: ${p => p.$error ? '#ef4444' : '#22c55e'};
  border: 1px solid ${p => p.$error ? 'rgba(239,68,68,0.3)' : 'rgba(34,197,94,0.3)'};
  animation: fadeIn 0.2s ease;
  @keyframes fadeIn { from { opacity: 0; transform: scale(0.9); } to { opacity: 1; transform: scale(1); } }
`

const DownloadBtn = styled.button`
  padding: 8px 14px;
  border-radius: var(--radius-sm);
  border: 1px solid var(--border-subtle);
  background: var(--bg-surface);
  color: var(--text-secondary);
  font-size: 12px;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.15s;
  &:disabled { opacity: 0.4; cursor: not-allowed; }
  &:not(:disabled):hover { border-color: #38bdf8; color: #38bdf8; background: rgba(56,189,248,0.08); }
`

const SaveLibBtn = styled.button`
  padding: 8px 14px;
  border-radius: var(--radius-sm);
  border: 1px solid rgba(34,197,94,0.4);
  background: rgba(34,197,94,0.1);
  color: #22c55e;
  font-size: 12px;
  font-weight: 700;
  cursor: pointer;
  transition: all 0.15s;
  &:disabled { opacity: 0.4; cursor: not-allowed; }
  &:not(:disabled):hover {
    background: rgba(34,197,94,0.18);
    border-color: #22c55e;
    transform: translateY(-1px);
    box-shadow: 0 4px 12px rgba(34,197,94,0.25);
  }
`
