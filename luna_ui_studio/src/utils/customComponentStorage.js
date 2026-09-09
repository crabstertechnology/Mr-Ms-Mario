// ============================================================
// Luna Display Studio — Custom Component Storage
// Persists pasted React components to localStorage so they
// survive page refreshes. Also supports file download.
// ============================================================

const STORAGE_KEY = 'luna_studio_custom_components'

/**
 * Get all saved custom components from localStorage
 * @returns {Array<{id, name, category, code, width, height, label, savedAt}>}
 */
export function getSavedComponents() {
  try {
    const raw = localStorage.getItem(STORAGE_KEY)
    return raw ? JSON.parse(raw) : []
  } catch {
    return []
  }
}

/**
 * Save a custom component to localStorage
 * @param {Object} component - Component metadata and code
 * @returns {string} The generated ID
 */
export function saveComponent({ name, category, code, width, height, label }) {
  const id = `custom_${Date.now()}_${name.toLowerCase().replace(/[^a-z0-9]/g, '_')}`
  const components = getSavedComponents()
  components.push({
    id,
    name,
    category,
    code,
    width: width || 160,
    height: height || 48,
    label: label || 'BUTTON',
    savedAt: new Date().toISOString(),
  })
  localStorage.setItem(STORAGE_KEY, JSON.stringify(components))
  return id
}

/**
 * Delete a saved component by ID
 * @param {string} id
 */
export function deleteComponent(id) {
  const components = getSavedComponents().filter(c => c.id !== id)
  localStorage.setItem(STORAGE_KEY, JSON.stringify(components))
}

/**
 * Update an existing saved component
 * @param {string} id
 * @param {Object} updates
 */
export function updateComponent(id, updates) {
  const components = getSavedComponents().map(c =>
    c.id === id ? { ...c, ...updates } : c
  )
  localStorage.setItem(STORAGE_KEY, JSON.stringify(components))
}

/**
 * Download the component code as a .jsx file
 * @param {string} name - Component name (used for filename)
 * @param {string} code - JSX source code
 */
export function downloadComponentFile(name, code) {
  const sanitized = name.replace(/[^a-zA-Z0-9]/g, '')
  const filename = `${sanitized || 'CustomComponent'}.jsx`

  const header = `// ============================================================
// Luna Display Studio — Saved Custom Component
// Component: ${name}
// Saved: ${new Date().toLocaleString()}
// Place this file in: src/ui-elements/elements/custom/
// Then import and register it in: src/ui-elements/registry.jsx
// ============================================================\n\n`

  const blob = new Blob([header + code], { type: 'text/javascript' })
  const url = URL.createObjectURL(blob)
  const a = document.createElement('a')
  a.href = url
  a.download = filename
  document.body.appendChild(a)
  a.click()
  document.body.removeChild(a)
  URL.revokeObjectURL(url)
}

/**
 * Export all saved components as a zip-like JSON bundle
 * (single JSON file containing all components)
 */
export function exportAllComponents() {
  const components = getSavedComponents()
  const blob = new Blob([JSON.stringify(components, null, 2)], { type: 'application/json' })
  const url = URL.createObjectURL(blob)
  const a = document.createElement('a')
  a.href = url
  a.download = `luna_custom_components_${Date.now()}.json`
  document.body.appendChild(a)
  a.click()
  document.body.removeChild(a)
  URL.revokeObjectURL(url)
}

/**
 * Import components from a JSON bundle file
 * @param {File} file
 * @returns {Promise<number>} Number of components imported
 */
export async function importComponentsFromFile(file) {
  return new Promise((resolve, reject) => {
    const reader = new FileReader()
    reader.onload = (e) => {
      try {
        const imported = JSON.parse(e.target.result)
        const existing = getSavedComponents()
        const existingIds = new Set(existing.map(c => c.id))
        let count = 0
        const merged = [...existing]
        for (const comp of imported) {
          if (!existingIds.has(comp.id)) {
            merged.push(comp)
            count++
          }
        }
        localStorage.setItem(STORAGE_KEY, JSON.stringify(merged))
        resolve(count)
      } catch {
        reject(new Error('Invalid component bundle file'))
      }
    }
    reader.onerror = () => reject(new Error('Failed to read file'))
    reader.readAsText(file)
  })
}
