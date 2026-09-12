/**
 * Luna UI Studio — Pure-JS PNG / BMP Image Decoder
 *
 * Uses Node.js built-in 'node:zlib' with zero external npm dependencies.
 * Converts PNG and BMP images into raw RGB888 / RGBA8888 pixel buffers and RGB565 arrays.
 */

import zlib from 'node:zlib'

function paethPredictor(a, b, c) {
  const p = a + b - c
  const pa = Math.abs(p - a)
  const pb = Math.abs(p - b)
  const pc = Math.abs(p - c)
  if (pa <= pb && pa <= pc) return a
  if (pb <= pc) return b
  return c
}

/**
 * Decodes a PNG buffer into raw RGBA pixels.
 * @param {Buffer} buffer
 * @returns {{ width: number, height: number, data: Uint8Array }}
 */
export function decodePng(buffer) {
  // Check PNG signature
  if (buffer.length < 8 ||
      buffer[0] !== 0x89 || buffer[1] !== 0x50 || buffer[2] !== 0x4e || buffer[3] !== 0x47 ||
      buffer[4] !== 0x0d || buffer[5] !== 0x0a || buffer[6] !== 0x1a || buffer[7] !== 0x0a) {
    throw new Error('Invalid PNG signature.')
  }

  let offset = 8
  let width = 0
  let height = 0
  let bitDepth = 8
  let colorType = 6 // 6 = RGBA, 2 = RGB
  const idatChunks = []

  while (offset < buffer.length) {
    const chunkLength = buffer.readUInt32BE(offset)
    const chunkType = buffer.toString('ascii', offset + 4, offset + 8)
    const chunkData = buffer.subarray(offset + 8, offset + 8 + chunkLength)
    offset += 12 + chunkLength

    if (chunkType === 'IHDR') {
      width = chunkData.readUInt32BE(0)
      height = chunkData.readUInt32BE(4)
      bitDepth = chunkData[8]
      colorType = chunkData[9]
    } else if (chunkType === 'IDAT') {
      idatChunks.push(chunkData)
    } else if (chunkType === 'IEND') {
      break
    }
  }

  if (width <= 0 || height <= 0) {
    throw new Error('Invalid image dimensions in IHDR chunk.')
  }

  const compressed = Buffer.concat(idatChunks)
  const uncompressed = zlib.inflateSync(compressed)

  const bytesPerPixel = (colorType === 6) ? 4 : (colorType === 2) ? 3 : 4
  const scanlineLength = 1 + (width * bytesPerPixel)
  const outRgba = new Uint8Array(width * height * 4)

  const prevRow = new Uint8Array(width * bytesPerPixel)
  const currentRow = new Uint8Array(width * bytesPerPixel)

  for (let y = 0; y < height; y++) {
    const filterType = uncompressed[y * scanlineLength]
    const rowOffset = (y * scanlineLength) + 1

    for (let i = 0; i < width * bytesPerPixel; i++) {
      const raw = uncompressed[rowOffset + i]
      const a = (i >= bytesPerPixel) ? currentRow[i - bytesPerPixel] : 0
      const b = prevRow[i]
      const c = (i >= bytesPerPixel) ? prevRow[i - bytesPerPixel] : 0

      let val = 0
      switch (filterType) {
        case 0: val = raw; break;
        case 1: val = (raw + a) & 0xff; break;
        case 2: val = (raw + b) & 0xff; break;
        case 3: val = (raw + Math.floor((a + b) / 2)) & 0xff; break;
        case 4: val = (raw + paethPredictor(a, b, c)) & 0xff; break;
        default: val = raw; break;
      }
      currentRow[i] = val
    }

    // Write to RGBA buffer
    for (let x = 0; x < width; x++) {
      const outIdx = ((y * width) + x) * 4
      if (bytesPerPixel === 4) {
        const inIdx = x * 4
        outRgba[outIdx]     = currentRow[inIdx]     // R
        outRgba[outIdx + 1] = currentRow[inIdx + 1] // G
        outRgba[outIdx + 2] = currentRow[inIdx + 2] // B
        outRgba[outIdx + 3] = currentRow[inIdx + 3] // A
      } else {
        const inIdx = x * 3
        outRgba[outIdx]     = currentRow[inIdx]     // R
        outRgba[outIdx + 1] = currentRow[inIdx + 1] // G
        outRgba[outIdx + 2] = currentRow[inIdx + 2] // B
        outRgba[outIdx + 3] = 255                   // A
      }
    }

    prevRow.set(currentRow)
  }

  return { width, height, data: outRgba }
}

/**
 * Decodes a BMP buffer into raw RGBA pixels.
 * @param {Buffer} buffer
 * @returns {{ width: number, height: number, data: Uint8Array }}
 */
export function decodeBmp(buffer) {
  if (buffer.length < 54 || buffer[0] !== 0x42 || buffer[1] !== 0x4d) {
    throw new Error('Invalid BMP header signature.')
  }

  const pixelOffset = buffer.readUInt32LE(10)
  const width = buffer.readInt32LE(18)
  const rawHeight = buffer.readInt32LE(22)
  const isTopDown = rawHeight < 0
  const height = Math.abs(rawHeight)
  const bpp = buffer.readUInt16LE(28)
  const compression = buffer.readUInt32LE(30)

  if (compression !== 0) {
    throw new Error(
      `Unsupported BMP compression mode ${compression}. Only uncompressed BI_RGB (0) is supported.`
    )
  }

  if (bpp !== 24 && bpp !== 32) {
    throw new Error(
      `Unsupported BMP bit depth: ${bpp} bpp. Only 24bpp and 32bpp uncompressed BMPs are supported.`
    )
  }

  const outRgba = new Uint8Array(width * height * 4)
  const rowSize = Math.floor((bpp * width + 31) / 32) * 4

  for (let y = 0; y < height; y++) {
    const srcY = isTopDown ? y : (height - 1 - y)
    const rowOffset = pixelOffset + (srcY * rowSize)

    for (let x = 0; x < width; x++) {
      const outIdx = ((y * width) + x) * 4
      if (bpp === 24) {
        const inIdx = rowOffset + (x * 3)
        outRgba[outIdx]     = buffer[inIdx + 2] // R
        outRgba[outIdx + 1] = buffer[inIdx + 1] // G
        outRgba[outIdx + 2] = buffer[inIdx]     // B
        outRgba[outIdx + 3] = 255
      } else if (bpp === 32) {
        const inIdx = rowOffset + (x * 4)
        outRgba[outIdx]     = buffer[inIdx + 2]
        outRgba[outIdx + 1] = buffer[inIdx + 1]
        outRgba[outIdx + 2] = buffer[inIdx]
        outRgba[outIdx + 3] = buffer[inIdx + 3]
      }
    }
  }

  return { width, height, data: outRgba }
}

/**
 * Automatically decodes an image buffer (PNG or BMP).
 * @param {Buffer} buffer
 * @returns {{ width: number, height: number, data: Uint8Array }}
 */
export function decodeImage(buffer) {
  if (buffer[0] === 0x89 && buffer[1] === 0x50) {
    return decodePng(buffer)
  }
  if (buffer[0] === 0x42 && buffer[1] === 0x4d) {
    return decodeBmp(buffer)
  }
  throw new Error('Unsupported image format. Must be PNG or BMP.')
}
