import React, { useState, useEffect, useCallback, useRef } from 'react'
import styled from 'styled-components'
import TopBar from './components/TopBar'
import PaletteSidebar from './components/Palette/PaletteSidebar'
import CanvasStage from './components/Canvas/CanvasStage'
import InspectorSidebar from './components/Inspector/InspectorSidebar'
import ExportModal from './components/ExportModal'
import LoadElementModal from './components/LoadElementModal'
import FlowMapModal from './components/FlowMapModal'
import FigmaLunaModal from './components/FigmaLunaModal'
import { useCanvas } from './hooks/useCanvas'

function App() {
  const {
    screens,
    activeScreenId,
    setActiveScreenId,
    activeScreen,
    addScreen,
    deleteScreen,
    duplicateScreen,
    renameScreen,
    setScreenBg,

    elements,
    selectedId,
    selectedElement,
    zoom,
    setZoom,
    gridSnap,
    setGridSnap,
    mode,
    setMode,

    addElement,
    updateElement,
    updateElementActions,
    deleteSelected,
    duplicateSelected,
    clearAll,
    undo,
    redo,
    bringForward,
    sendBackward,
    setSelectedId,
    loadTemplate,
  } = useCanvas()

  const [exportOpen, setExportOpen] = useState(false)
  const [compileOpen, setCompileOpen] = useState(false)
  const [flowMapOpen, setFlowMapOpen] = useState(false)
  const [figmaOpen, setFigmaOpen] = useState(false)
  const [toast, setToast] = useState(null)
  const initializedRef = useRef(false)

  const showToast = useCallback((msg) => {
    setToast(msg)
    setTimeout(() => setToast(null), 2500)
  }, [])

  // Keyboard shortcuts
  useEffect(() => {
    const handler = (e) => {
      if (e.target.tagName === 'INPUT' || e.target.tagName === 'TEXTAREA') return
      if (e.key === 'Delete' || e.key === 'Backspace') { deleteSelected(); showToast('Element deleted') }
      if (e.ctrlKey && e.key === 'z') undo()
      if (e.ctrlKey && e.key === 'd') { e.preventDefault(); duplicateSelected() }
      if (e.key === 'w' || e.key === 'W') setMode(m => m === 'wireframe' ? 'design' : 'wireframe')
      if (e.key === 't' || e.key === 'T') setMode(m => m === 'test' ? 'design' : 'test')
    }
    window.addEventListener('keydown', handler)
    return () => window.removeEventListener('keydown', handler)
  }, [deleteSelected, duplicateSelected, undo, setMode, showToast])

  // Load default multi-screen project with interactive block mapping once on mount
  useEffect(() => {
    if (initializedRef.current) return
    initializedRef.current = true

    // Only initialize if screen 1 has no elements yet
    if (screens.length === 1 && screens[0].elements.length === 0) {
      // 1. Setup Screen 1 (Home)
      const clockId = addElement('digital_clock', 20, 20, 'screen_1')
      const cardId = addElement('card_glass', 20, 95, 'screen_1')
      const btn1Id = addElement('uiv_btn_happy_coding', 20, 205, 'screen_1')

      // 2. Setup Screen 2 (Page 2)
      const screen2Id = addScreen('Screen 2 (Page 2)')
      const statId = addElement('card_stat', 20, 25, screen2Id)
      const retroId = addElement('card_retro', 20, 110, screen2Id)
      const btn2Id = addElement('uiv_btn_damith_yellow', 20, 205, screen2Id)

      // 3. Map Button 1 on Screen 1 -> Navigates to Screen 2
      if (btn1Id) {
        updateElementActions(btn1Id, [
          {
            id: 'act_nav_to_s2',
            trigger: 'onClick',
            actionType: 'navigate',
            targetScreenId: screen2Id,
            transition: 'slide-left',
            alertMessage: 'Taking you to Page 2...',
          }
        ])
      }

      // 4. Map Button 2 on Screen 2 -> Navigates back to Screen 1
      if (btn2Id) {
        updateElementActions(btn2Id, [
          {
            id: 'act_nav_to_s1',
            trigger: 'onClick',
            actionType: 'navigate',
            targetScreenId: 'screen_1',
            transition: 'slide-right',
            alertMessage: 'Returning to Home...',
          }
        ])
      }

      // Switch back to Screen 1 as active initial view
      setActiveScreenId('screen_1')
      setSelectedId(null)
    }
  }, []) // eslint-disable-line

  const handleAdd = (type) => {
    addElement(type, 40, 80)
    showToast(`✓ Added "${type}" to ${activeScreen?.name || 'canvas'}`)
  }

  const handleMove = (id, nx, ny, isDone = false) => {
    updateElement(id, { x: nx, y: ny }, isDone)
  }

  const handleResize = (id, nw, nh, isDone = false) => {
    updateElement(id, { w: nw, h: nh }, isDone)
  }

  const handleClear = () => {
    if (window.confirm(`Clear all elements from "${activeScreen?.name || 'active screen'}"?`)) {
      clearAll()
      showToast('Screen cleared')
    }
  }

  const handleTestTrigger = (act, el) => {
    if (act.actionType === 'navigate' && act.targetScreenId) {
      setActiveScreenId(act.targetScreenId)
      const target = screens.find(s => s.id === act.targetScreenId)
      showToast(`🚀 Tested: Navigated to "${target?.name || 'target screen'}"`)
    } else if (act.actionType === 'scroll') {
      showToast(`📜 Tested: Scroll action (${act.scrollDirection || 'down'} ${act.scrollAmount || 80}px)`)
    } else if (act.actionType === 'alert') {
      showToast(`🔔 Tested Alert: "${act.alertMessage || 'Notice'}"`)
    } else if (act.actionType === 'toggle') {
      updateElement(el.id, { checked: !el.props.checked })
      showToast(`🔄 Tested: Toggled switch`)
    }
  }

  return (
    <AppShell>
      <TopBar
        zoom={zoom}
        setZoom={setZoom}
        gridSnap={gridSnap}
        setGridSnap={setGridSnap}
        mode={mode}
        setMode={setMode}
        onUndo={undo}
        onRedo={redo}
        onDup={duplicateSelected}
        onDel={deleteSelected}
        onClear={handleClear}
        onExport={() => setExportOpen(true)}
        onOpenCompile={() => setCompileOpen(true)}
        onOpenFlowMap={() => setFlowMapOpen(true)}
        onOpenFigma={() => setFigmaOpen(true)}
      />

      <Workspace>
        <PaletteSidebar
          onAdd={handleAdd}
          onOpenCompileModal={() => setCompileOpen(true)}
        />

        <CanvasStage
          screens={screens}
          activeScreenId={activeScreenId}
          elements={elements}
          selectedId={selectedId}
          zoom={zoom}
          gridSnap={gridSnap}
          mode={mode}
          setMode={setMode}
          onSelectScreen={setActiveScreenId}
          onAddScreen={(name) => {
            const newId = addScreen(name)
            showToast(`✓ Created "${name || 'New Screen'}"`)
            return newId
          }}
          onDeleteScreen={(id) => {
            deleteScreen(id)
            showToast(`Deleted screen`)
          }}
          onDuplicateScreen={(id) => {
            duplicateScreen(id)
            showToast(`✓ Duplicated screen`)
          }}
          onRenameScreen={(id, newName) => {
            renameScreen(id, newName)
            showToast(`✓ Renamed screen to "${newName}"`)
          }}
          onOpenFlowMap={() => setFlowMapOpen(true)}
          onSelect={setSelectedId}
          onAddDrop={(type, x, y) => { addElement(type, x, y); showToast(`✓ Dropped "${type}"`) }}
          onMove={handleMove}
          onResize={handleResize}
          onUpdateElement={updateElement}
        />

        <InspectorSidebar
          element={selectedElement}
          onUpdate={updateElement}
          elements={elements}
          selectedId={selectedId}
          onSelect={setSelectedId}
          onBringForward={bringForward}
          onSendBackward={sendBackward}
          onDelete={deleteSelected}
          screens={screens}
          activeScreenId={activeScreenId}
          onUpdateActions={updateElementActions}
          onAddScreen={addScreen}
          onTestTrigger={handleTestTrigger}
        />
      </Workspace>

      {/* Multi-Screen C++ Firmware Export Modal */}
      <ExportModal
        open={exportOpen}
        screens={screens}
        activeScreenId={activeScreenId}
        elements={elements}
        onClose={() => setExportOpen(false)}
      />

      {/* Live React Compiler Modal */}
      <LoadElementModal
        open={compileOpen}
        onClose={() => setCompileOpen(false)}
        onAddElement={(type) => {
          handleAdd(type)
          showToast(`⚡ Compiled and added "${type}" to canvas`)
        }}
      />

      {/* Visual UI Navigation Flow & Blocks Map Modal */}
      <FlowMapModal
        open={flowMapOpen}
        onClose={() => setFlowMapOpen(false)}
        screens={screens}
        activeScreenId={activeScreenId}
        onSelectScreen={setActiveScreenId}
        onAddScreen={addScreen}
        onDeleteScreen={(id) => {
          const success = deleteScreen(id)
          if (success) {
            showToast('✓ Deleted screen')
          } else {
            showToast('⚠️ Cannot delete the only remaining screen')
          }
        }}
        onUpdateElementActions={updateElementActions}
        onEnterTestMode={() => {
          setMode('test')
          showToast('🎮 Interactive Test Mode Enabled! Click buttons to test.')
        }}
      />

      {/* Figma Luna OS Flow Simulator Modal */}
      <FigmaLunaModal
        open={figmaOpen}
        onClose={() => setFigmaOpen(false)}
      />

      {toast && <Toast>{toast}</Toast>}
    </AppShell>
  )
}

const AppShell = styled.div`
  display: flex;
  flex-direction: column;
  height: 100vh;
  width: 100vw;
  overflow: hidden;
`

const Workspace = styled.main`
  flex: 1;
  display: flex;
  overflow: hidden;
`

const Toast = styled.div`
  position: fixed;
  bottom: 24px;
  right: 24px;
  background: var(--bg-raised);
  border: 1px solid var(--accent-blue)44;
  color: var(--text-primary);
  padding: 10px 18px;
  border-radius: var(--radius-md);
  font-size: 12px;
  font-weight: 600;
  z-index: 2000;
  box-shadow: var(--neu-raised), 0 4px 20px rgba(37,99,235,0.2);
  animation: slideInToast 0.2s ease;
  @keyframes slideInToast {
    from { transform: translateY(10px); opacity: 0; }
    to { transform: none; opacity: 1; }
  }
`

export default App
