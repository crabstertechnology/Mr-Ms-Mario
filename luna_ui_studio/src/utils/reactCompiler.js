// ============================================================
// Luna Display Studio — Live React UI Compiler Engine
// Uses @babel/standalone to compile JSX + styled-components
// into living, animating React components at runtime.
// ============================================================
import * as Babel from '@babel/standalone'
import React, { useState, useEffect, useRef, useMemo, useCallback } from 'react'
import * as jsxRuntime from 'react/jsx-runtime'
import styled, { keyframes, css } from 'styled-components'

// Setup styled-components compatibility wrapper
const styledBase = typeof styled === 'function' ? styled : (styled.default || styled)
const styledWrapper = Object.assign(styledBase, {
  default: styledBase,
  keyframes,
  css,
})

/**
 * Compiles a React component source code string (with JSX and styled-components)
 * into an executable React component function.
 *
 * @param {string} sourceCode - Raw JSX / React / styled-components code
 * @returns {{ Component: React.ComponentType, name: string, error: null } | { Component: null, name: string, error: string }}
 */
export function compileReactComponent(sourceCode) {
  if (!sourceCode || typeof sourceCode !== 'string' || !sourceCode.trim()) {
    return { Component: null, name: 'EmptyComponent', error: 'No source code provided' }
  }

  try {
    // 1. Transform code using Babel with React classic runtime & CommonJS modules
    const transformed = Babel.transform(sourceCode, {
      presets: [
        ['react', { runtime: 'classic' }],
        ['env', { modules: 'commonjs', targets: { esmodules: true } }]
      ],
      filename: 'CompiledUIElement.jsx',
    })

    // 2. Prepare isolated module sandbox with React & styled-components
    const customRequire = (moduleName) => {
      if (moduleName === 'react') {
        return {
          default: React,
          ...React,
          useState,
          useEffect,
          useRef,
          useMemo,
          useCallback,
        }
      }
      if (moduleName === 'react/jsx-runtime') {
        return jsxRuntime
      }
      if (moduleName === 'styled-components') {
        return styledWrapper
      }
      throw new Error(`External import "${moduleName}" is not available in Luna Studio sandbox. Please use standard React and styled-components.`)
    }

    const exportsObj = {}
    const moduleObj = { exports: exportsObj }

    // 3. Execute compiled code within sandbox
    const runner = new Function(
      'require',
      'exports',
      'module',
      'React',
      'styled',
      'keyframes',
      'css',
      transformed.code
    )

    runner(
      customRequire,
      exportsObj,
      moduleObj,
      React,
      styledWrapper,
      keyframes,
      css
    )

    // 4. Retrieve exported component
    const ExportedComponent = moduleObj.exports.default || moduleObj.exports
    if (typeof ExportedComponent !== 'function' && typeof ExportedComponent !== 'object') {
      return {
        Component: null,
        name: 'InvalidComponent',
        error: 'The code did not export a valid React component. Ensure you have `export default MyComponent;`',
      }
    }

    // Try to detect component name
    let detectedName = ExportedComponent.name || ExportedComponent.displayName || 'CustomCompiledElement'
    if (detectedName === 'default' || !detectedName) {
      // Regex match function/const name
      const match = sourceCode.match(/(?:const|function|class)\s+([A-Za-z0-9_]+)/)
      if (match && match[1]) {
        detectedName = match[1]
      } else {
        detectedName = 'CustomUIElement'
      }
    }

    return {
      Component: ExportedComponent,
      name: detectedName,
      error: null,
    }
  } catch (err) {
    console.error('React Compiler Error:', err)
    return {
      Component: null,
      name: 'CompileError',
      error: err.message || String(err),
    }
  }
}

/**
 * Heuristic to extract UI element properties (width, height, default label, colors)
 * from styled-components CSS or component props.
 */
export function inferElementMetadata(sourceCode) {
  const meta = {
    w: 160,
    h: 48,
    label: 'BUTTON',
    bgColor: '#1e293b',
    borderColor: '#3b82f6',
    textColor: '#ffffff',
  }

  if (!sourceCode) return meta

  // Extract label
  const labelMatch = sourceCode.match(/label\s*=\s*['"]([^'"]+)['"]/) ||
                     sourceCode.match(/role="button"[^>]*>([^<]+)<\/button>/) ||
                     sourceCode.match(/<button[^>]*>([^<]+)<\/button>/)
  if (labelMatch && labelMatch[1] && !labelMatch[1].includes('{')) {
    meta.label = labelMatch[1].trim()
  }

  // Extract width & height heuristics
  const wMatch = sourceCode.match(/width:\s*([0-9]+)px/)
  if (wMatch && wMatch[1]) meta.w = parseInt(wMatch[1], 10)

  const hMatch = sourceCode.match(/height:\s*([0-9]+)px/)
  if (hMatch && hMatch[1]) meta.h = parseInt(hMatch[1], 10)

  // Extract colors
  const hexColors = sourceCode.match(/#[0-9a-fA-F]{3,6}\b/g)
  if (hexColors && hexColors.length > 0) {
    meta.borderColor = hexColors[0]
    if (hexColors.length > 1) meta.bgColor = hexColors[1]
    if (hexColors.length > 2) meta.textColor = hexColors[2]
  }

  return meta
}
