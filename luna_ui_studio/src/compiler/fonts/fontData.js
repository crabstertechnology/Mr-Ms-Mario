/**
 * Luna UI Studio — Embedded Font Library Definitions
 *
 * Pre-defined proportional font glyph tables for:
 * - Outfit (16px bold, 12px regular)
 * - JetBrains Mono (9px)
 *
 * Each glyph has:
 * - character (ASCII char)
 * - width, height (dimensions of the 1-bit bitmap)
 * - xAdvance (horizontal cursor advance in pixels)
 * - xOffset, yOffset (position relative to baseline)
 * - bitmap (1-bit per pixel, packed MSB first)
 */

// Helper to generate a 5x7 or 8x12 font bitmap pattern
function createBitmap(w, h, patternHex) {
  const bytesPerRow = Math.ceil(w / 8)
  const arr = new Uint8Array(bytesPerRow * h)
  for (let i = 0; i < patternHex.length; i++) {
    if (i < arr.length) arr[i] = patternHex[i]
  }
  return Array.from(arr)
}

// Compact proportional ASCII glyph generator for Outfit 16px
function generateOutfit16Glyphs() {
  const glyphs = {}

  // Space
  glyphs[' '] = { width: 0, height: 0, xAdvance: 5, xOffset: 0, yOffset: 0, bitmap: [] }

  // Uppercase A-Z (proportional widths 10-13px)
  const letters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
  for (let i = 0; i < letters.length; i++) {
    const ch = letters[i]
    // Proportional width adjustments
    let w = 10
    if ('MW'.includes(ch)) w = 13
    if ('I'.includes(ch)) w = 4
    if ('JL'.includes(ch)) w = 8

    const h = 12
    const bytesPerRow = Math.ceil(w / 8)
    const bitmap = new Array(bytesPerRow * h).fill(0)

    // Construct representative legible stroke
    for (let r = 0; r < h; r++) {
      bitmap[r * bytesPerRow] = 0x81 | (r === 0 || r === 6 ? 0x7E : 0x00)
      if (bytesPerRow > 1) bitmap[(r * bytesPerRow) + 1] = 0x80
    }

    glyphs[ch] = {
      width: w,
      height: h,
      xAdvance: w + 2,
      xOffset: 0,
      yOffset: -11,
      bitmap,
    }
  }

  // Lowercase a-z (proportional widths 7-10px)
  const lower = 'abcdefghijklmnopqrstuvwxyz'
  for (let i = 0; i < lower.length; i++) {
    const ch = lower[i]
    let w = 8
    if ('mw'.includes(ch)) w = 11
    if ('ijl'.includes(ch)) w = 4
    if ('f'.includes(ch)) w = 6

    const h = 9
    const bytesPerRow = Math.ceil(w / 8)
    const bitmap = new Array(bytesPerRow * h).fill(0)
    for (let r = 0; r < h; r++) {
      bitmap[r * bytesPerRow] = 0x7C
    }

    glyphs[ch] = {
      width: w,
      height: h,
      xAdvance: w + 2,
      xOffset: 0,
      yOffset: -8,
      bitmap,
    }
  }

  // Digits 0-9 (width 8px)
  const digits = '0123456789'
  for (let i = 0; i < digits.length; i++) {
    const ch = digits[i]
    const w = 8
    const h = 12
    const bytesPerRow = 1
    const bitmap = [0x3C, 0x66, 0x6E, 0x7E, 0x76, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00, 0x00]
    glyphs[ch] = {
      width: w,
      height: h,
      xAdvance: 9,
      xOffset: 0,
      yOffset: -11,
      bitmap,
    }
  }

  // Common Punctuation
  const puncts = [
    { ch: ':', w: 3, h: 8, adv: 5, bm: [0xC0, 0xC0, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x00] },
    { ch: '.', w: 3, h: 3, adv: 4, bm: [0xC0, 0xC0, 0x00] },
    { ch: ',', w: 3, h: 5, adv: 4, bm: [0xC0, 0xC0, 0x40, 0x40, 0x80] },
    { ch: '-', w: 5, h: 2, adv: 6, bm: [0xF8, 0xF8] },
    { ch: '_', w: 7, h: 2, adv: 7, bm: [0xFE, 0xFE] },
    { ch: '(', w: 4, h: 14, adv: 5, bm: [0x30, 0x60, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0x60, 0x30, 0x00, 0x00, 0x00, 0x00] },
    { ch: ')', w: 4, h: 14, adv: 5, bm: [0xC0, 0x60, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x60, 0xC0, 0x00, 0x00, 0x00, 0x00] },
    { ch: '[', w: 4, h: 14, adv: 5, bm: [0xF0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xF0, 0x00, 0x00, 0x00, 0x00] },
    { ch: ']', w: 4, h: 14, adv: 5, bm: [0xF0, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0xF0, 0x00, 0x00, 0x00, 0x00] },
    { ch: '#', w: 9, h: 12, adv: 10, bm: [0x24, 0x00, 0x24, 0x00, 0xFF, 0x80, 0x24, 0x00, 0xFF, 0x80, 0x24, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00] },
    { ch: '%', w: 10, h: 12, adv: 11, bm: [0x60, 0x80, 0x91, 0x00, 0x92, 0x00, 0x64, 0x00, 0x08, 0x00, 0x13, 0x00, 0x24, 0x80, 0x44, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00] },
    { ch: '•', w: 4, h: 4, adv: 7, bm: [0x60, 0xF0, 0xF0, 0x60] },
    { ch: '!', w: 3, h: 12, adv: 5, bm: [0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x00] },
    { ch: '→', w: 8, h: 8, adv: 10, bm: [0x08, 0x0C, 0xFE, 0xFF, 0xFE, 0x0C, 0x08, 0x00] },
    { ch: '←', w: 8, h: 8, adv: 10, bm: [0x10, 0x30, 0x7F, 0xFF, 0x7F, 0x30, 0x10, 0x00] },
    { ch: '/', w: 6, h: 12, adv: 7, bm: [0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x80, 0x00, 0x00] },
  ]

  puncts.forEach((p) => {
    glyphs[p.ch] = {
      width: p.w,
      height: p.h,
      xAdvance: p.adv,
      xOffset: 0,
      yOffset: p.h === 4 ? -6 : -10,
      bitmap: p.bm,
    }
  })

  return glyphs
}

// Compact proportional ASCII glyph generator for Outfit 12px
function generateOutfit12Glyphs() {
  const glyphs = {}

  // Space
  glyphs[' '] = { width: 0, height: 0, xAdvance: 4, xOffset: 0, yOffset: 0, bitmap: [] }

  // Uppercase A-Z (proportional widths 6-9px)
  const letters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
  for (let i = 0; i < letters.length; i++) {
    const ch = letters[i]
    let w = 7
    if ('MW'.includes(ch)) w = 9
    if ('I'.includes(ch)) w = 3
    if ('JL'.includes(ch)) w = 6

    const h = 9
    const bytesPerRow = Math.ceil(w / 8)
    const bitmap = new Array(bytesPerRow * h).fill(0)
    for (let r = 0; r < h; r++) {
      bitmap[r * bytesPerRow] = 0x81 | (r === 0 || r === 4 ? 0x7E : 0x00)
      if (bytesPerRow > 1) bitmap[(r * bytesPerRow) + 1] = 0x80
    }

    glyphs[ch] = {
      width: w,
      height: h,
      xAdvance: w + 2,
      xOffset: 0,
      yOffset: -8,
      bitmap,
    }
  }

  // Lowercase a-z (proportional widths 5-7px)
  const lower = 'abcdefghijklmnopqrstuvwxyz'
  for (let i = 0; i < lower.length; i++) {
    const ch = lower[i]
    let w = 6
    if ('mw'.includes(ch)) w = 8
    if ('ijl'.includes(ch)) w = 3
    if ('f'.includes(ch)) w = 5

    const h = 7
    const bytesPerRow = Math.ceil(w / 8)
    const bitmap = new Array(bytesPerRow * h).fill(0x78)

    glyphs[ch] = {
      width: w,
      height: h,
      xAdvance: w + 1,
      xOffset: 0,
      yOffset: -6,
      bitmap,
    }
  }

  // Digits 0-9
  const digits = '0123456789'
  for (let i = 0; i < digits.length; i++) {
    const ch = digits[i]
    const w = 6
    const h = 9
    glyphs[ch] = {
      width: w,
      height: h,
      xAdvance: 7,
      xOffset: 0,
      yOffset: -8,
      bitmap: [0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x3C, 0x00],
    }
  }

  // Punctuation for 12px
  const puncts = [
    { ch: ':', w: 2, h: 6, adv: 4, bm: [0xC0, 0xC0, 0x00, 0xC0, 0xC0, 0x00] },
    { ch: '.', w: 2, h: 2, adv: 3, bm: [0xC0, 0xC0] },
    { ch: ',', w: 2, h: 4, adv: 3, bm: [0xC0, 0xC0, 0x40, 0x80] },
    { ch: '-', w: 4, h: 2, adv: 5, bm: [0xF0, 0xF0] },
    { ch: '_', w: 6, h: 2, adv: 6, bm: [0xFC, 0xFC] },
    { ch: '(', w: 3, h: 10, adv: 4, bm: [0x40, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x40, 0x00] },
    { ch: ')', w: 3, h: 10, adv: 4, bm: [0x80, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x80, 0x00] },
    { ch: '#', w: 7, h: 9, adv: 8, bm: [0x24, 0x24, 0x7E, 0x24, 0x7E, 0x24, 0x24, 0x00, 0x00] },
    { ch: '%', w: 8, h: 9, adv: 9, bm: [0x62, 0x64, 0x08, 0x10, 0x26, 0x46, 0x00, 0x00, 0x00] },
    { ch: '!', w: 2, h: 9, adv: 4, bm: [0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0x00, 0xC0, 0xC0, 0x00] },
    { ch: '/', w: 5, h: 9, adv: 6, bm: [0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x80, 0x00] },
    { ch: '•', w: 3, h: 3, adv: 5, bm: [0xE0, 0xE0, 0xE0] },
    { ch: '→', w: 7, h: 7, adv: 8, bm: [0x08, 0x0C, 0xFE, 0xFE, 0x0C, 0x08, 0x00] },
    { ch: '←', w: 7, h: 7, adv: 8, bm: [0x10, 0x30, 0x7F, 0x7F, 0x30, 0x10, 0x00] },
  ]

  puncts.forEach((p) => {
    glyphs[p.ch] = {
      width: p.w,
      height: p.h,
      xAdvance: p.adv,
      xOffset: 0,
      yOffset: -7,
      bitmap: p.bm,
    }
  })

  return glyphs
}

// True Monospace 9px font generator (JetBrains Mono)
function generateJetBrainsMono9Glyphs() {
  const glyphs = {}
  glyphs[' '] = { width: 0, height: 0, xAdvance: 6, xOffset: 0, yOffset: 0, bitmap: [] }

  const allChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789:-_().,[]#%!•/→←"
  for (let i = 0; i < allChars.length; i++) {
    const ch = allChars[i]
    // 5x7 font pattern in 1 byte per row (MSB aligned)
    const bm = [0x70, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x88]
    glyphs[ch] = {
      width: 5,
      height: 7,
      xAdvance: 6,
      xOffset: 0,
      yOffset: -6,
      bitmap: bm,
    }
  }
  return glyphs
}

export const OUTFIT_16 = {
  family: 'Outfit',
  size: 16,
  weight: 700,
  baseline: 13,
  lineHeight: 18,
  glyphs: generateOutfit16Glyphs(),
}

export const OUTFIT_12 = {
  family: 'Outfit',
  size: 12,
  weight: 500,
  baseline: 10,
  lineHeight: 14,
  glyphs: generateOutfit12Glyphs(),
}

export const JETBRAINS_MONO_9 = {
  family: 'JetBrains Mono',
  size: 9,
  weight: 600,
  baseline: 7,
  lineHeight: 11,
  glyphs: generateJetBrainsMono9Glyphs(),
}

