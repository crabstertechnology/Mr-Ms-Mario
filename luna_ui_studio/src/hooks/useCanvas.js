import { useState, useCallback, useRef } from 'react'
import { UI_COMPONENTS } from '../ui-elements/registry'

let _id = 1
const genId = () => `el_${Date.now().toString(36)}_${_id++}`
let _screenCounter = 2

function snap(val, grid) {
  return Math.round(val / grid) * grid
}

const INITIAL_SCREENS = [
  {
    id: 'screen_1',
    name: 'Screen 1 (Home)',
    bgColor: '#0b0f19',
    elements: []
  }
]

export function useCanvas() {
  const [screens, setScreens] = useState(INITIAL_SCREENS)
  const [activeScreenId, setActiveScreenId] = useState('screen_1')
  const [selectedId, setSelectedId] = useState(null)
  const [zoom, setZoom] = useState(1.5)
  const [gridSnap, setGridSnap] = useState(1)
  const [mode, setMode] = useState('design') // 'design' | 'wireframe' | 'test'

  const historyRef = useRef([])
  const historyIdxRef = useRef(-1)

  // Save history state across all screens
  const saveHistory = useCallback((currScreens, currActiveScreenId) => {
    const snapshot = JSON.stringify({ screens: currScreens, activeScreenId: currActiveScreenId })
    historyRef.current = historyRef.current.slice(0, historyIdxRef.current + 1)
    historyRef.current.push(snapshot)
    historyIdxRef.current = historyRef.current.length - 1
  }, [])

  // Active screen lookup
  const activeScreen = screens.find(s => s.id === activeScreenId) || screens[0] || INITIAL_SCREENS[0]
  const elements = activeScreen.elements || []

  // Add Screen
  const addScreen = useCallback((name) => {
    const newId = `screen_${Date.now().toString(36)}_${_screenCounter++}`
    const newName = name || `Screen ${screens.length + 1}`
    const newScreen = {
      id: newId,
      name: newName,
      bgColor: '#0b0f19',
      elements: []
    }
    setScreens(prev => {
      const next = [...prev, newScreen]
      saveHistory(next, newId)
      return next
    })
    setActiveScreenId(newId)
    setSelectedId(null)
    return newId
  }, [screens.length, saveHistory])

  // Delete Screen
  const deleteScreen = useCallback((screenId) => {
    let nextActive = null
    let didDelete = false

    setScreens(prev => {
      if (prev.length <= 1) return prev
      const next = prev.filter(s => s.id !== screenId)
      if (next.length === prev.length) return prev // screenId not found
      didDelete = true
      nextActive = activeScreenId === screenId ? next[0].id : activeScreenId
      saveHistory(next, nextActive)
      return next
    })

    if (didDelete && nextActive) {
      setActiveScreenId(nextActive)
      setSelectedId(null)
    }
    return didDelete
  }, [activeScreenId, saveHistory])

  // Duplicate Screen
  const duplicateScreen = useCallback((screenId) => {
    const src = screens.find(s => s.id === screenId)
    if (!src) return
    const newId = `screen_${Date.now().toString(36)}_${_screenCounter++}`
    const dupElements = src.elements.map(el => ({
      ...el,
      id: genId(),
      props: { ...el.props },
      actions: el.actions ? el.actions.map(a => ({ ...a, id: `act_${Date.now().toString(36)}_${Math.random().toString(36).substr(2, 5)}` })) : []
    }))
    const newScreen = {
      id: newId,
      name: `${src.name} (Copy)`,
      bgColor: src.bgColor || '#0b0f19',
      elements: dupElements
    }
    setScreens(prev => {
      const next = [...prev, newScreen]
      saveHistory(next, newId)
      return next
    })
    setActiveScreenId(newId)
    setSelectedId(null)
    return newId
  }, [screens, saveHistory])

  // Rename Screen
  const renameScreen = useCallback((screenId, newName) => {
    if (!newName || !newName.trim()) return
    setScreens(prev => {
      const next = prev.map(s => s.id === screenId ? { ...s, name: newName.trim() } : s)
      saveHistory(next, activeScreenId)
      return next
    })
  }, [activeScreenId, saveHistory])

  // Set Screen Background
  const setScreenBg = useCallback((screenId, color) => {
    setScreens(prev => {
      const next = prev.map(s => s.id === screenId ? { ...s, bgColor: color } : s)
      saveHistory(next, activeScreenId)
      return next
    })
  }, [activeScreenId, saveHistory])

  // Add Element to Active Screen
  const addElement = useCallback((type, x, y, targetScreenId = null) => {
    const comp = UI_COMPONENTS[type]
    if (!comp) return
    const snapTo = gridSnap > 1 ? gridSnap : 1
    const el = {
      id: genId(),
      type,
      name: comp.name,
      actions: [], // Block action mappings
      props: {
        ...comp.defaultProps,
        x: Math.round(Math.max(0, Math.min(x, 240 - (comp.defaultProps.w || 80))) / snapTo) * snapTo,
        y: Math.round(Math.max(0, Math.min(y, 280 - (comp.defaultProps.h || 40))) / snapTo) * snapTo,
      }
    }
    const scrId = targetScreenId || activeScreenId
    setScreens(prev => {
      const next = prev.map(s => {
        if (s.id === scrId) {
          return { ...s, elements: [...s.elements, el] }
        }
        return s
      })
      saveHistory(next, activeScreenId)
      return next
    })
    setSelectedId(el.id)
    return el.id
  }, [gridSnap, activeScreenId, saveHistory])

  // Update Element in Active Screen
  const updateElement = useCallback((id, propsUpdate, recordHistory = true) => {
    setScreens(prev => {
      const next = prev.map(s => {
        if (s.id === activeScreenId) {
          const updatedElements = s.elements.map(el =>
            el.id === id ? { ...el, props: { ...el.props, ...propsUpdate } } : el
          )
          return { ...s, elements: updatedElements }
        }
        return s
      })
      if (recordHistory) {
        saveHistory(next, activeScreenId)
      }
      return next
    })
  }, [activeScreenId, saveHistory])

  // Update Actions / Blocks Mapping for an Element
  const updateElementActions = useCallback((id, newActions) => {
    setScreens(prev => {
      const next = prev.map(s => {
        if (s.id === activeScreenId) {
          const updatedElements = s.elements.map(el =>
            el.id === id ? { ...el, actions: newActions } : el
          )
          return { ...s, elements: updatedElements }
        }
        return s
      })
      saveHistory(next, activeScreenId)
      return next
    })
  }, [activeScreenId, saveHistory])

  // Delete Selected Element
  const deleteSelected = useCallback(() => {
    if (!selectedId) return
    setScreens(prev => {
      const next = prev.map(s => {
        if (s.id === activeScreenId) {
          return { ...s, elements: s.elements.filter(el => el.id !== selectedId) }
        }
        return s
      })
      saveHistory(next, activeScreenId)
      return next
    })
    setSelectedId(null)
  }, [selectedId, activeScreenId, saveHistory])

  // Duplicate Selected Element
  const duplicateSelected = useCallback(() => {
    if (!selectedId) return
    setScreens(prev => {
      const next = prev.map(s => {
        if (s.id === activeScreenId) {
          const src = s.elements.find(el => el.id === selectedId)
          if (!src) return s
          const dup = {
            ...src,
            id: genId(),
            props: { ...src.props, x: src.props.x + 8, y: src.props.y + 8 },
            actions: src.actions ? src.actions.map(a => ({ ...a, id: `act_${Date.now().toString(36)}` })) : []
          }
          return { ...s, elements: [...s.elements, dup] }
        }
        return s
      })
      saveHistory(next, activeScreenId)
      return next
    })
  }, [selectedId, activeScreenId, saveHistory])

  // Clear All on current active screen
  const clearAll = useCallback(() => {
    setScreens(prev => {
      const next = prev.map(s => s.id === activeScreenId ? { ...s, elements: [] } : s)
      saveHistory(next, activeScreenId)
      return next
    })
    setSelectedId(null)
  }, [activeScreenId, saveHistory])

  // Undo / Redo
  const undo = useCallback(() => {
    if (historyIdxRef.current <= 0) return
    historyIdxRef.current -= 1
    const snapshot = JSON.parse(historyRef.current[historyIdxRef.current])
    setScreens(snapshot.screens)
    if (snapshot.activeScreenId) setActiveScreenId(snapshot.activeScreenId)
    setSelectedId(null)
  }, [])

  const redo = useCallback(() => {
    if (historyIdxRef.current >= historyRef.current.length - 1) return
    historyIdxRef.current += 1
    const snapshot = JSON.parse(historyRef.current[historyIdxRef.current])
    setScreens(snapshot.screens)
    if (snapshot.activeScreenId) setActiveScreenId(snapshot.activeScreenId)
    setSelectedId(null)
  }, [])

  // Layer order
  const bringForward = useCallback(() => {
    if (!selectedId) return
    setScreens(prev => {
      return prev.map(s => {
        if (s.id === activeScreenId) {
          const els = [...s.elements]
          const idx = els.findIndex(el => el.id === selectedId)
          if (idx < els.length - 1 && idx >= 0) {
            ;[els[idx], els[idx + 1]] = [els[idx + 1], els[idx]]
          }
          return { ...s, elements: els }
        }
        return s
      })
    })
  }, [selectedId, activeScreenId])

  const sendBackward = useCallback(() => {
    if (!selectedId) return
    setScreens(prev => {
      return prev.map(s => {
        if (s.id === activeScreenId) {
          const els = [...s.elements]
          const idx = els.findIndex(el => el.id === selectedId)
          if (idx > 0) {
            ;[els[idx], els[idx - 1]] = [els[idx - 1], els[idx]]
          }
          return { ...s, elements: els }
        }
        return s
      })
    })
  }, [selectedId, activeScreenId])

  const loadTemplate = useCallback((loadedScreens) => {
    if (Array.isArray(loadedScreens) && loadedScreens[0]?.elements !== undefined) {
      setScreens(loadedScreens)
      setActiveScreenId(loadedScreens[0]?.id || 'screen_1')
      saveHistory(loadedScreens, loadedScreens[0]?.id || 'screen_1')
    } else if (Array.isArray(loadedScreens)) {
      // Legacy format: array of elements for screen 1
      const s = [{ id: 'screen_1', name: 'Screen 1 (Home)', bgColor: '#0b0f19', elements: loadedScreens }]
      setScreens(s)
      setActiveScreenId('screen_1')
      saveHistory(s, 'screen_1')
    }
    setSelectedId(null)
  }, [saveHistory])

  const selectedElement = elements.find(el => el.id === selectedId) || null

  return {
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
  }
}
