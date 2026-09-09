import React, { useState } from 'react'
import styled from 'styled-components'

export default function TopBar({
  zoom,
  setZoom,
  gridSnap,
  setGridSnap,
  mode = 'design',
  setMode,
  onUndo,
  onRedo,
  onDup,
  onDel,
  onClear,
  onExport,
  onOpenCompile,
  onOpenFlowMap,
  onOpenFigma
}) {
  const [serialConnected, setSerialConnected] = useState(false)

  return (
    <Bar>
      <Brand>
        <BrandLogo>🌙</BrandLogo>
        <div>
          <BrandName>LUNA DISPLAY STUDIO</BrandName>
          <BrandSub>ESP32-S3 • 1.69" (240×280 ST7789)</BrandSub>
        </div>
        <BrandBadge>UI MAKER v3.0</BrandBadge>
      </Brand>

      <ToolbarCenter>
        <ToolGroup>
          <ToolLabel>🔍</ToolLabel>
          <ToolSelect value={zoom} onChange={e => setZoom(parseFloat(e.target.value))}>
            <option value={1.0}>100%</option>
            <option value={1.25}>125%</option>
            <option value={1.5}>150%</option>
            <option value={2.0}>200%</option>
          </ToolSelect>
        </ToolGroup>

        <Divider />

        <ToolGroup>
          <ToolLabel>⊞</ToolLabel>
          <ToolSelect value={gridSnap} onChange={e => setGridSnap(parseInt(e.target.value))}>
            <option value={1}>Free (Snap Off)</option>
            <option value={2}>Snap: 2px</option>
            <option value={4}>Snap: 4px</option>
            <option value={8}>Snap: 8px</option>
          </ToolSelect>
        </ToolGroup>

        <Divider />

        <ToolGroup>
          <ModeToggle
            $active={mode === 'design'}
            onClick={() => setMode('design')}
            title="Design Mode"
          >
            🎨 Design
          </ModeToggle>
          <ModeToggle
            $active={mode === 'wireframe'}
            onClick={() => setMode('wireframe')}
            title="Wireframe Mode"
          >
            ⬜ Wireframe
          </ModeToggle>
          <TestModeToggle
            $active={mode === 'test'}
            onClick={() => setMode(mode === 'test' ? 'design' : 'test')}
            title="Interactive Test Mode: Click elements to test screen switching, scroll & alerts"
          >
            ▶ Test Interactive
          </TestModeToggle>
        </ToolGroup>

        <Divider />

        <ToolGroup>
          <ToolBtn onClick={onUndo} title="Undo (Ctrl+Z)">↶</ToolBtn>
          <ToolBtn onClick={onRedo} title="Redo">↷</ToolBtn>
          <ToolBtn onClick={onDup} title="Duplicate">⧉ Dup</ToolBtn>
          <ToolBtn onClick={onDel} title="Delete Selected">✕ Del</ToolBtn>
          <ToolBtn onClick={onClear} title="Clear All" $danger>🗑</ToolBtn>
        </ToolGroup>
      </ToolbarCenter>

      <RightActions>
        <FigmaBtn onClick={onOpenFigma} title="Launch Figma Luna OS Interactive Device Simulator">
          ✨ Figma Luna Flow
        </FigmaBtn>
        <FlowBtn onClick={onOpenFlowMap} title="View Visual UI Navigation & Block Map">
          🗺️ Flow Map
        </FlowBtn>
        <CompileBtn onClick={onOpenCompile} title="Compile & Load React UI Element (Babel Live Engine)">
          ⚡ Compile React UI
        </CompileBtn>
        <SerialBtn $connected={serialConnected} onClick={() => setSerialConnected(c => !c)}>
          ⚡ {serialConnected ? 'Connected' : 'Connect COM3'}
        </SerialBtn>
        <ExportBtn onClick={onExport}>
          &lt;/&gt; Export C++ Code
        </ExportBtn>
      </RightActions>
    </Bar>
  )
}

const Bar = styled.header`
  height: 56px;
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0 16px;
  background: var(--bg-raised);
  border-bottom: 1px solid var(--border-subtle);
  box-shadow: 0 2px 12px rgba(0,0,0,0.08), var(--neu-raised);
  z-index: 100;
  gap: 12px;
  flex-shrink: 0;
`

const Brand = styled.div`
  display: flex;
  align-items: center;
  gap: 10px;
  flex-shrink: 0;
`

const BrandLogo = styled.div`
  width: 36px;
  height: 36px;
  border-radius: var(--radius-sm);
  background: linear-gradient(135deg, #1e3a5f, #2563eb);
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 18px;
  box-shadow: var(--neu-button);
`

const BrandName = styled.div`
  font-size: 13px;
  font-weight: 900;
  color: var(--text-primary);
  letter-spacing: 1px;
`

const BrandSub = styled.div`
  font-size: 9px;
  color: var(--text-muted);
  font-family: var(--font-mono);
`

const BrandBadge = styled.span`
  padding: 3px 8px;
  background: var(--accent-blue)18;
  color: var(--accent-blue);
  border-radius: 20px;
  font-size: 9px;
  font-weight: 800;
  letter-spacing: 0.5px;
  border: 1px solid var(--accent-blue)33;
`

const ToolbarCenter = styled.div`
  display: flex;
  align-items: center;
  gap: 4px;
  flex: 1;
  justify-content: center;
`

const ToolGroup = styled.div`
  display: flex;
  align-items: center;
  gap: 2px;
  background: var(--bg-inset);
  border-radius: var(--radius-sm);
  padding: 4px 6px;
  box-shadow: var(--neu-inset);
`

const ToolLabel = styled.span`
  font-size: 12px;
  color: var(--text-muted);
  padding: 0 2px;
`

const ToolSelect = styled.select`
  border: none;
  background: transparent;
  font-size: 12px;
  font-weight: 600;
  color: var(--text-primary);
  font-family: var(--font-ui);
  cursor: pointer;
  outline: none;
`

const ToolBtn = styled.button`
  padding: 4px 10px;
  border: none;
  border-radius: 6px;
  font-size: 12px;
  font-weight: 600;
  cursor: pointer;
  background: var(--bg-raised);
  color: ${p => p.$danger ? 'var(--accent-red)' : 'var(--text-secondary)'};
  box-shadow: var(--neu-button);
  transition: all 0.15s;
  &:hover { transform: translateY(-1px); box-shadow: 5px 5px 12px var(--shadow-dark), -5px -5px 12px var(--shadow-light); }
  &:active { box-shadow: var(--neu-pressed); transform: none; }
`

const ModeToggle = styled.button`
  padding: 4px 10px;
  border: none;
  border-radius: 6px;
  font-size: 11px;
  font-weight: 700;
  cursor: pointer;
  transition: all 0.18s;
  background: ${p => p.$active ? 'linear-gradient(135deg, #2563eb, #3b82f6)' : 'var(--bg-raised)'};
  color: ${p => p.$active ? '#fff' : 'var(--text-muted)'};
  box-shadow: ${p => p.$active
    ? '0 2px 10px rgba(37,99,235,0.4), inset 0 1px 0 rgba(255,255,255,0.15)'
    : 'var(--neu-button)'};
  &:hover { transform: translateY(-1px); }
  &:active { transform: none; }
`

const TestModeToggle = styled.button`
  padding: 4px 12px;
  border: none;
  border-radius: 6px;
  font-size: 11px;
  font-weight: 800;
  cursor: pointer;
  transition: all 0.18s;
  background: ${p => p.$active
    ? 'linear-gradient(135deg, #10b981, #059669)'
    : 'rgba(16, 185, 129, 0.08)'};
  color: ${p => p.$active ? '#fff' : '#059669'};
  border: 1px solid ${p => p.$active ? 'transparent' : 'rgba(16, 185, 129, 0.3)'};
  box-shadow: ${p => p.$active
    ? '0 2px 12px rgba(16,185,129,0.45), inset 0 1px 0 rgba(255,255,255,0.2)'
    : 'var(--neu-button)'};
  &:hover {
    transform: translateY(-1px);
    background: ${p => p.$active ? 'linear-gradient(135deg, #10b981, #059669)' : 'rgba(16, 185, 129, 0.16)'};
  }
  &:active { transform: none; }
`

const FlowBtn = styled.button`
  padding: 7px 14px;
  border-radius: var(--radius-sm);
  border: 1px solid rgba(139, 92, 246, 0.35);
  background: linear-gradient(135deg, rgba(139, 92, 246, 0.08), rgba(99, 102, 241, 0.1));
  color: #7c3aed;
  font-size: 12px;
  font-weight: 800;
  cursor: pointer;
  box-shadow: var(--neu-button);
  transition: all 0.2s;
  display: flex;
  align-items: center;
  gap: 6px;
  &:hover {
    background: linear-gradient(135deg, rgba(139, 92, 246, 0.18), rgba(99, 102, 241, 0.22));
    transform: translateY(-1px);
    box-shadow: 0 4px 12px rgba(139, 92, 246, 0.25);
  }
`

const Divider = styled.div`
  width: 1px;
  height: 24px;
  background: var(--border-subtle);
  margin: 0 4px;
`

const RightActions = styled.div`
  display: flex;
  align-items: center;
  gap: 8px;
  flex-shrink: 0;
`

const SerialBtn = styled.button`
  padding: 7px 14px;
  border-radius: var(--radius-sm);
  border: 1px solid ${p => p.$connected ? 'var(--accent-green)' : 'var(--border-subtle)'};
  background: ${p => p.$connected ? 'var(--accent-green)18' : 'var(--bg-raised)'};
  color: ${p => p.$connected ? 'var(--accent-green)' : 'var(--text-secondary)'};
  font-size: 12px;
  font-weight: 700;
  cursor: pointer;
  box-shadow: var(--neu-button);
  transition: all 0.2s;
  &:hover { transform: translateY(-1px); }
`

const CompileBtn = styled.button`
  padding: 7px 14px;
  border-radius: var(--radius-sm);
  border: 1.5px solid #6366f1;
  background: rgba(99, 102, 241, 0.08);
  color: #4f46e5;
  font-size: 12px;
  font-weight: 700;
  cursor: pointer;
  box-shadow: var(--neu-button);
  transition: all 0.2s;
  display: flex;
  align-items: center;
  gap: 6px;
  &:hover {
    background: rgba(99, 102, 241, 0.16);
    transform: translateY(-1px);
    box-shadow: 0 4px 12px rgba(99, 102, 241, 0.25);
  }
`

const ExportBtn = styled.button`
  padding: 8px 18px;
  border-radius: var(--radius-sm);
  border: none;
  background: linear-gradient(135deg, var(--accent-blue), #7c3aed);
  color: #fff;
  font-size: 12px;
  font-weight: 800;
  cursor: pointer;
  box-shadow: 0 4px 14px rgba(37,99,235,0.35);
  transition: all 0.2s;
  &:hover { transform: translateY(-2px); box-shadow: 0 8px 20px rgba(37,99,235,0.45); }
  &:active { transform: none; }
`

const FigmaBtn = styled.button`
  padding: 7px 14px;
  border-radius: var(--radius-sm);
  border: 1.5px solid rgba(56, 189, 248, 0.4);
  background: linear-gradient(135deg, rgba(56, 189, 248, 0.15), rgba(147, 51, 234, 0.15));
  color: #38bdf8;
  font-size: 12px;
  font-weight: 700;
  cursor: pointer;
  box-shadow: 0 0 12px rgba(56, 189, 248, 0.15);
  transition: all 0.2s;
  display: flex;
  align-items: center;
  gap: 6px;
  &:hover {
    background: linear-gradient(135deg, rgba(56, 189, 248, 0.25), rgba(147, 51, 234, 0.25));
    border-color: #38bdf8;
    color: #fff;
    transform: translateY(-1px);
    box-shadow: 0 0 18px rgba(56, 189, 248, 0.35);
  }
`
