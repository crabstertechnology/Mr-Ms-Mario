/**
 * Luna UI Studio — Embedded Image Asset Compiler
 *
 * Compiles PNG and BMP images into static 16-bit RGB565 C++ arrays for LunaRenderer.
 * Implements cryptographic deduplication to ensure identical images are stored only once in flash.
 */

import crypto from 'node:crypto'
import { decodeImage } from './pngDecoder.js'

/**
 * Converts standard 8-bit RGB channels to 16-bit RGB565.
 * @param {number} r 0..255
 * @param {number} g 0..255
 * @param {number} b 0..255
 * @returns {number} 16-bit integer (0..65535)
 */
export function rgb888ToRgb565(r, g, b) {
  return ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3)
}

/**
 * Converts an RGBA buffer into a Uint16Array of RGB565 pixels.
 * @param {Uint8Array} rgba
 * @param {number} width
 * @param {number} height
 * @returns {Uint16Array}
 */
export function rgbaToRgb565Array(rgba, width, height) {
  const pixelCount = width * height
  const out565 = new Uint16Array(pixelCount)

  for (let i = 0; i < pixelCount; i++) {
    const idx = i * 4
    const r = rgba[idx]
    const g = rgba[idx + 1]
    const b = rgba[idx + 2]
    const a = rgba[idx + 3]

    if (a < 255) {
      const px = i % width
      const py = Math.floor(i / width)
      throw new Error(
        `Unsupported PNG transparency: pixel (${px}, ${py}) has alpha ${a}. ` +
        `Embedded RGB565 hardware does not support per-pixel alpha transparency. ` +
        `Please use fully opaque images (alpha=255).`
      )
    }

    out565[i] = rgb888ToRgb565(r, g, b)
  }

  return out565
}

/**
 * Compiles a list of image assets with automatic deduplication.
 *
 * @param {Array<{ id: string, buffer?: Buffer, width?: number, height?: number, pixels565?: Uint16Array }>} assetList
 * @returns {{ header: string, source: string, assetCount: number, uniqueCount: number, assets: Array }}
 */
export function compileAssetsToCpp(assetList = []) {
  const uniqueAssetsByHash = new Map()
  const processedAssets = []

  assetList.forEach((item) => {
    let width = item.width
    let height = item.height
    let pixels565 = item.pixels565

    if (item.buffer && !pixels565) {
      const decoded = decodeImage(item.buffer)
      width = decoded.width
      height = decoded.height
      pixels565 = rgbaToRgb565Array(decoded.data, width, height)
    }

    if (!pixels565 || !width || !height) {
      throw new Error(`Asset "${item.id}" missing valid pixel data.`)
    }

    // Hash pixel data + dimensions for deduplication.
    // Width and height are included so two images with identical pixel data but different
    // dimensions are not incorrectly aliased to the same asset entry.
    const dimPrefix = Buffer.from(`${width}x${height}:`)
    const hash = crypto.createHash('sha256')
      .update(dimPrefix)
      .update(Buffer.from(pixels565.buffer))
      .digest('hex')
    const safeSymbol = `asset_${hash.substring(0, 12)}`

    if (!uniqueAssetsByHash.has(hash)) {
      uniqueAssetsByHash.set(hash, {
        symbol: safeSymbol,
        width,
        height,
        pixels565,
        byteSize: pixels565.length * 2,
        hash,
      })
    }

    const uniqueEntry = uniqueAssetsByHash.get(hash)
    processedAssets.push({
      id: item.id,
      symbol: uniqueEntry.symbol,
      width,
      height,
      byteSize: uniqueEntry.byteSize,
    })
  })

  // 1. Generate Header
  const headerLines = [
    '// ============================================================================',
    '// GENERATED FILE — DO NOT EDIT MANUALLY.',
    '// SOURCE: Luna UI Studio Asset Compiler',
    '// REGENERATE FROM STUDIO.',
    '// ============================================================================',
    '#ifndef GENERATED_ASSETS_H',
    '#define GENERATED_ASSETS_H',
    '',
    '#include "LunaTypes.h"',
    '',
    '#ifdef __cplusplus',
    'extern "C" {',
    '#endif',
    '',
    'extern const LunaImageAsset LUNA_COMPILED_ASSETS[];',
    `extern const uint8_t LUNA_COMPILED_ASSET_COUNT;`,
    '',
    'const LunaImageAsset* findAssetById(const char* id);',
    '',
    '#ifdef __cplusplus',
    '}',
    '#endif',
    '',
    '#endif // GENERATED_ASSETS_H',
    '',
  ]

  // 2. Generate Source
  const sourceLines = [
    '// ============================================================================',
    '// GENERATED FILE — DO NOT EDIT MANUALLY.',
    '// SOURCE: Luna UI Studio Asset Compiler',
    '// REGENERATE FROM STUDIO.',
    '// ============================================================================',
    '#include "GeneratedAssets.h"',
    '#include <string.h>',
    '#include <pgmspace.h>',
    '',
  ]

  // Emit unique pixel arrays
  uniqueAssetsByHash.forEach((entry) => {
    sourceLines.push(`// Unique Asset Buffer: ${entry.symbol} (${entry.width}x${entry.height}, ${entry.byteSize} bytes)`)
    sourceLines.push(`static const uint16_t PROGMEM ${entry.symbol}_pixels[${entry.pixels565.length}] = {`)

    // Format in rows of 16 values
    for (let i = 0; i < entry.pixels565.length; i += 16) {
      const slice = Array.from(entry.pixels565.slice(i, i + 16))
      const hexVals = slice.map((val) => `0x${val.toString(16).toUpperCase().padStart(4, '0')}`).join(',')
      sourceLines.push(`  ${hexVals},`)
    }

    sourceLines.push('};')
    sourceLines.push('')
  })

  // Emit Asset Table
  sourceLines.push(`const LunaImageAsset LUNA_COMPILED_ASSETS[${processedAssets.length}] = {`)
  processedAssets.forEach((asset) => {
    sourceLines.push('  {')
    sourceLines.push(`    "${asset.id}",`)
    sourceLines.push(`    ${asset.width},`)
    sourceLines.push(`    ${asset.height},`)
    sourceLines.push(`    ${asset.symbol}_pixels,`)
    sourceLines.push(`    ${asset.byteSize}`)
    sourceLines.push('  },')
  })
  sourceLines.push('};')
  sourceLines.push('')
  sourceLines.push(`const uint8_t LUNA_COMPILED_ASSET_COUNT = ${processedAssets.length};`)
  sourceLines.push('')

  // Lookup function
  sourceLines.push('const LunaImageAsset* findAssetById(const char* id) {')
  sourceLines.push('  if (!id) return nullptr;')
  sourceLines.push('  for (uint8_t i = 0; i < LUNA_COMPILED_ASSET_COUNT; i++) {')
  sourceLines.push('    if (strcmp(LUNA_COMPILED_ASSETS[i].id, id) == 0) {')
  sourceLines.push('      return &LUNA_COMPILED_ASSETS[i];')
  sourceLines.push('    }')
  sourceLines.push('  }')
  sourceLines.push('  return nullptr;')
  sourceLines.push('}')
  sourceLines.push('')

  return {
    header: headerLines.join('\n'),
    source: sourceLines.join('\n'),
    assetCount: processedAssets.length,
    uniqueCount: uniqueAssetsByHash.size,
    assets: processedAssets,
  }
}
