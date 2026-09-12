// ============================================================
// Luna Display Studio — C++ / Arduino_GFX & Hardware Code Generator
// ============================================================

function hexToRGB565(hex) {
  if (!hex || typeof hex !== 'string') return '0xFFFF'
  const clean = hex.replace('#', '').substring(0, 6)
  const num = parseInt(clean, 16)
  if (isNaN(num)) return '0xFFFF'
  const r = (num >> 16) & 255
  const g = (num >> 8) & 255
  const b = num & 255
  const rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
  return '0x' + rgb565.toString(16).toUpperCase().padStart(4, '0')
}

function cleanStr(str) {
  if (str === null || str === undefined) return ''
  return String(str)
    .replace(/\\/g, '\\\\')
    .replace(/"/g, '\\"')
    .replace(/\n/g, ' ')
    .replace(/\r/g, '')
    .replace(/•/g, '-')
}

export function generateArduinoCode(elements, screenBg = '#0b0f19') {
  const bgRGB = hexToRGB565(screenBg)
  const lines = []
  lines.push('// ============================================================================')
  lines.push('// GENERATED FILE — DO NOT EDIT MANUALLY.')
  lines.push('// SOURCE: Luna UI Studio')
  lines.push('// REGENERATE FROM STUDIO.')
  lines.push('// Generated for ESP32-S3 1.69" (240x280 ST7789)')
  lines.push('// ============================================================================')
  lines.push('#include "luna_gfx_compat.h"')
  lines.push('#include "luna_ui_elements.h"')
  lines.push('')
  lines.push('extern TFT_eSPI tft;')
  lines.push('')
  lines.push('void drawGeneratedScreen() {')
  lines.push(`  tft.fillScreen(${bgRGB});`)
  lines.push('')

  elements.forEach((el, i) => {
    const p = el.props
    lines.push(`  // ── Element ${i + 1}: ${el.type} (${el.name}) ──`)
    switch (el.type) {
      // ── Kinesis / Monolith Design System Primitives ──
      case 'monolith_time': {
        const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
        const text = hexToRGB565(p.textColor || '#EAEFF5'); const accent = hexToRGB565(p.accentColor || '#38BDF8');
        lines.push(`  drawMonolithTime(${p.x}, ${p.y}, ${p.w||220}, ${p.h||90}, ${p.radius||10}, ${bg}, ${border}, "${cleanStr(p.timeStr||'10:42')}", "${cleanStr(p.secondsStr||':38')}", "${cleanStr(p.dateStr||'WED 09 SEP')}", ${p.batteryPct||94}, "${cleanStr(p.statusText||'LUNA - READY')}", ${text}, ${accent});`)
        break
      }
      case 'glance_bar': {
        const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
        const text = hexToRGB565(p.textColor || '#EAEFF5'); const accent = hexToRGB565(p.accentColor || '#FF9E3B');
        lines.push(`  drawGlanceBar(${p.x}, ${p.y}, ${p.w||220}, ${p.h||42}, ${p.radius||8}, ${bg}, ${border}, "${cleanStr(p.label||'NEXT FOCUS')}", "${cleanStr(p.detail||'')}", ${accent}, ${text});`)
        break
      }
      case 'tactile_button': {
        const bg = hexToRGB565(p.bgColor || '#FF9E3B'); const border = hexToRGB565(p.borderColor || '#FFB266');
        const text = hexToRGB565(p.textColor || '#080A0F');
        lines.push(`  drawTactileButton(${p.x}, ${p.y}, ${p.w||220}, ${p.h||48}, ${p.radius||12}, ${bg}, ${border}, "${cleanStr(p.label||'ACTION')}", ${text});`)
        break
      }
      case 'notification_block': {
        const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
        const text = hexToRGB565(p.textColor || '#EAEFF5'); const sub = hexToRGB565(p.subtextColor || '#8290A4');
        const accent = hexToRGB565(p.accentColor || '#38BDF8');
        lines.push(`  drawNotificationBlock(${p.x}, ${p.y}, ${p.w||220}, ${p.h||72}, ${p.radius||10}, ${bg}, ${border}, "${cleanStr(p.sender||'SENDER')}", "${cleanStr(p.timeStr||'now')}", "${cleanStr(p.preview||'')}", ${p.unread ? 1 : 0}, ${accent}, ${text}, ${sub});`)
        break
      }
      case 'agenda_block': {
        const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
        const text = hexToRGB565(p.textColor || '#EAEFF5'); const sub = hexToRGB565(p.subtextColor || '#8290A4');
        const accent = hexToRGB565(p.accentColor || '#38BDF8');
        lines.push(`  drawAgendaBlock(${p.x}, ${p.y}, ${p.w||220}, ${p.h||84}, ${p.radius||10}, ${bg}, ${border}, "${cleanStr(p.countdown||'IN 24m')}", "${cleanStr(p.timeRange||'')}", "${cleanStr(p.title||'Event')}", "${cleanStr(p.location||'')}", ${accent}, ${text}, ${sub});`)
        break
      }
      case 'game_launcher': {
        const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
        const text = hexToRGB565(p.textColor || '#EAEFF5'); const accent = hexToRGB565(p.accentColor || '#FF9E3B');
        lines.push(`  drawGameLauncher(${p.x}, ${p.y}, ${p.w||220}, ${p.h||170}, ${p.radius||12}, ${bg}, ${border}, "${cleanStr(p.title||'GAME')}", "${cleanStr(p.genre||'ARCADE')}", "${cleanStr(p.highScore||'0 PTS')}", ${accent}, ${text});`)
        break
      }
      case 'focus_chamber': {
        const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
        const text = hexToRGB565(p.textColor || '#EAEFF5'); const sub = hexToRGB565(p.subtextColor || '#8290A4');
        const accent = hexToRGB565(p.accentColor || '#FF9E3B');
        lines.push(`  drawFocusChamber(${p.x}, ${p.y}, ${p.w||220}, ${p.h||175}, ${p.radius||12}, ${bg}, ${border}, "${cleanStr(p.timeRemaining||'25:00')}", "${cleanStr(p.modeLabel||'FOCUS')}", "${cleanStr(p.sessionTag||'SESSION 1')}", ${p.progress||50}, ${accent}, ${text}, ${sub});`)
        break
      }
      case 'qr_utility_card': {
        const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
        const text = hexToRGB565(p.textColor || '#EAEFF5'); const accent = hexToRGB565(p.accentColor || '#38BDF8');
        lines.push(`  drawQRUtilityCard(${p.x}, ${p.y}, ${p.w||220}, ${p.h||160}, ${p.radius||12}, ${bg}, ${border}, "${cleanStr(p.title||'LUNA ID')}", "${cleanStr(p.subtitle||'')}", "${cleanStr(p.idTag||'')}", ${accent}, ${text});`)
        break
      }
      case 'imu_level_card': {
        const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
        const text = hexToRGB565(p.textColor || '#EAEFF5'); const sub = hexToRGB565(p.subtextColor || '#8290A4');
        const accent = hexToRGB565(p.accentColor || '#10B981');
        lines.push(`  drawIMULevelCard(${p.x}, ${p.y}, ${p.w||220}, ${p.h||140}, ${p.radius||12}, ${bg}, ${border}, "${cleanStr(p.title||'IMU LEVEL')}", "${cleanStr(p.pitch||'+0.0')}", "${cleanStr(p.roll||'+0.0')}", "${cleanStr(p.status||'LEVEL')}", ${accent}, ${text}, ${sub});`)
        break
      }
      case 'setting_row': {
        const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
        const text = hexToRGB565(p.textColor || '#EAEFF5'); const sub = hexToRGB565(p.subtextColor || '#8290A4');
        const accent = hexToRGB565(p.accentColor || '#38BDF8');
        lines.push(`  drawSettingRow(${p.x}, ${p.y}, ${p.w||220}, ${p.h||48}, ${p.radius||8}, ${bg}, ${border}, "${cleanStr(p.label||'SETTING')}", "${cleanStr(p.valueStr||'')}", "${cleanStr(p.sublabel||'')}", ${p.checked ? 1 : 0}, ${accent}, ${text}, ${sub});`)
        break
      }
      case 'device_spec_card': {
        const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
        const text = hexToRGB565(p.textColor || '#EAEFF5'); const sub = hexToRGB565(p.subtextColor || '#8290A4');
        const accent = hexToRGB565(p.accentColor || '#10B981');
        lines.push(`  drawDeviceSpecCard(${p.x}, ${p.y}, ${p.w||220}, ${p.h||160}, ${p.radius||12}, ${bg}, ${border}, "${cleanStr(p.deviceName||'LUNA CORE')}", "${cleanStr(p.soc||'ESP32-S3')}", "${cleanStr(p.memory||'16MB/8MB')}", "${cleanStr(p.batteryInfo||'4.12V')}", "${cleanStr(p.status||'OK')}", ${accent}, ${text}, ${sub});`)
        break
      }
      // ── Controlled Luna Component Primitives ──
      case 'luna_button': {
        const bg = hexToRGB565(p.variant === 'secondary' ? '#121721' : p.variant === 'destructive' ? '#991B1B' : '#FF9E3B');
        const border = hexToRGB565(p.variant === 'secondary' ? '#232D3F' : p.variant === 'destructive' ? '#DC2626' : '#FFB266');
        const text = hexToRGB565(p.variant === 'secondary' ? '#EAEFF5' : p.variant === 'destructive' ? '#FFFFFF' : '#080A0F');
        lines.push(`  drawLunaButton(${p.x}, ${p.y}, ${p.w||220}, ${p.h||48}, 12, ${bg}, ${border}, "${cleanStr(p.label||'BUTTON')}", ${text});`)
        break
      }
      case 'luna_icon_button': {
        const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
        const text = hexToRGB565(p.textColor || '#38BDF8');
        lines.push(`  drawLunaIconButton(${p.x}, ${p.y}, ${p.w||44}, ${p.h||44}, 10, ${bg}, ${border}, "${cleanStr(p.icon||'⚙')}", ${text});`)
        break
      }
      case 'luna_surface': {
        const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
        const text = hexToRGB565(p.textColor || '#EAEFF5'); const sub = hexToRGB565(p.subtextColor || '#8290A4');
        lines.push(`  drawLunaSurface(${p.x}, ${p.y}, ${p.w||220}, ${p.h||80}, ${p.radius||10}, ${bg}, ${border}, "${cleanStr(p.title||'Surface')}", "${cleanStr(p.subtitle||'')}", ${text}, ${sub});`)
        break
      }
      case 'luna_list_item': {
        const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
        const text = hexToRGB565(p.textColor || '#EAEFF5'); const sub = hexToRGB565(p.subtextColor || '#8290A4');
        const accent = hexToRGB565(p.accentColor || '#38BDF8');
        lines.push(`  drawLunaListItem(${p.x}, ${p.y}, ${p.w||220}, ${p.h||52}, 8, ${bg}, ${border}, "${cleanStr(p.title||'Item')}", "${cleanStr(p.subtitle||'')}", ${p.unread ? 1 : 0}, ${text}, ${sub}, ${accent});`)
        break
      }
      case 'luna_toggle': {
        const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
        const text = hexToRGB565(p.textColor || '#EAEFF5'); const active = hexToRGB565(p.activeColor || '#38BDF8');
        lines.push(`  drawLunaToggle(${p.x}, ${p.y}, ${p.w||220}, ${p.h||48}, 10, ${bg}, ${border}, "${cleanStr(p.label||'Option')}", ${p.checked ? 1 : 0}, ${active}, ${text});`)
        break
      }
      case 'luna_slider': {
        const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
        const text = hexToRGB565(p.textColor || '#EAEFF5'); const accent = hexToRGB565(p.accentColor || '#FF9E3B');
        lines.push(`  drawLunaSlider(${p.x}, ${p.y}, ${p.w||220}, ${p.h||64}, 10, ${bg}, ${border}, "${cleanStr(p.label||'SLIDER')}", ${p.value||50}, ${accent}, ${text});`)
        break
      }
      case 'luna_progress': {
        const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
        const fill = hexToRGB565(p.accentColor || '#38BDF8'); const text = hexToRGB565(p.textColor || '#EAEFF5');
        lines.push(`  drawLunaProgress(${p.x}, ${p.y}, ${p.w||220}, ${p.h||48}, 10, ${bg}, ${border}, ${p.value||50}, ${fill}, ${text});`)
        break
      }
      case 'luna_indicator': {
        const col = hexToRGB565(p.status === 'error' ? '#EF4444' : p.status === 'warning' ? '#FF9E3B' : '#10B981');
        lines.push(`  drawLunaIndicator(${p.x}, ${p.y}, ${p.w||100}, ${p.h||30}, "${cleanStr(p.label||'ONLINE')}", ${col});`)
        break
      }
      case 'luna_header': {
        const bg = hexToRGB565(p.bgColor || '#080A0F'); const border = hexToRGB565(p.borderColor || '#1A2232');
        const text = hexToRGB565(p.textColor || '#EAEFF5');
        lines.push(`  drawLunaHeader(${p.x}, ${p.y}, ${p.w||240}, ${p.h||44}, "${cleanStr(p.title||'HEADER')}", ${bg}, ${border}, ${text});`)
        break
      }
      case 'luna_navigation': {
        const active = hexToRGB565(p.activeColor || '#38BDF8'); const inactive = hexToRGB565(p.inactiveColor || '#232D3F');
        lines.push(`  drawLunaNavigation(${p.x}, ${p.y}, ${p.w||220}, ${p.h||36}, ${p.activeIndex||0}, ${p.total||8}, ${active}, ${inactive});`)
        break
      }
      case 'luna_timer': {
        const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
        const text = hexToRGB565(p.textColor || '#EAEFF5'); const accent = hexToRGB565(p.accentColor || '#FF9E3B');
        lines.push(`  drawLunaTimer(${p.x}, ${p.y}, ${p.w||220}, ${p.h||175}, 12, ${bg}, ${border}, "${cleanStr(p.timeStr||'25:00')}", "${cleanStr(p.label||'FOCUS')}", ${p.progress||50}, ${accent}, ${text});`)
        break
      }
      case 'luna_number': {
        const col = hexToRGB565(p.accentColor || '#38BDF8'); const sub = hexToRGB565(p.subtextColor || '#8290A4');
        lines.push(`  drawLunaNumber(${p.x}, ${p.y}, ${p.w||100}, ${p.h||44}, "${cleanStr(p.value||'0')}", "${cleanStr(p.unit||'')}", "${cleanStr(p.label||'')}", ${col}, ${sub});`)
        break
      }
      case 'luna_status': {
        const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
        const col = hexToRGB565(p.status === 'error' ? '#EF4444' : p.status === 'warning' ? '#FF9E3B' : '#10B981');
        lines.push(`  drawLunaStatus(${p.x}, ${p.y}, ${p.w||90}, ${p.h||26}, 6, ${bg}, ${border}, "${cleanStr(p.label||'ONLINE')}", ${col});`)
        break
      }
      // ── Legacy Elements ──
      case 'card_glass': {
        const bg = hexToRGB565(p.bgColor); const border = hexToRGB565(p.borderColor); const text = hexToRGB565(p.textColor)
        lines.push(`  drawGlassCard(${p.x}, ${p.y}, ${p.w}, ${p.h}, ${p.radius||14}, ${bg}, ${border}, "${p.title}", "${p.subtitle}", ${text});`)
        break
      }
      case 'card_stat': {
        const bg = hexToRGB565(p.bgColor); const border = hexToRGB565(p.borderColor); const accent = hexToRGB565(p.accentColor); const text = hexToRGB565(p.textColor)
        lines.push(`  drawStatCard(${p.x}, ${p.y}, ${p.w}, ${p.h}, ${p.radius||10}, ${bg}, ${border}, "${p.label}", "${p.value}", "${p.unit}", ${accent}, ${text});`)
        break
      }
      case 'card_transaction': {
        const bg = hexToRGB565(p.bgColor); const iconBg = hexToRGB565(p.iconBg); const border = hexToRGB565(p.borderColor); const text = hexToRGB565(p.textColor)
        lines.push(`  drawTransactionCard(${p.x}, ${p.y}, ${p.w}, ${p.h}, ${p.radius||12}, ${bg}, ${border}, ${iconBg}, "${p.title}", ${text});`)
        break
      }
      case 'card_retro': {
        const bg = hexToRGB565(p.bgColor); const border = hexToRGB565(p.borderColor); const text = hexToRGB565(p.textColor)
        lines.push(`  drawRetroArcadeBox(${p.x}, ${p.y}, ${p.w}, ${p.h}, ${bg}, ${border}, "${p.title}", "${p.score}", ${text});`)
        break
      }
      case 'button_neon': {
        const bg = hexToRGB565(p.bgColor); const border = hexToRGB565(p.borderColor); const text = hexToRGB565(p.textColor)
        lines.push(`  drawNeonGlowButton(${p.x}, ${p.y}, ${p.w}, ${p.h}, ${p.radius||8}, ${bg}, ${border}, "${p.label}", ${text});`)
        break
      }
      case 'button_retro': {
        const bg = hexToRGB565(p.bgColor); const border = hexToRGB565(p.borderColor); const text = hexToRGB565(p.textColor)
        lines.push(`  drawRetroButton(${p.x}, ${p.y}, ${p.w}, ${p.h}, ${bg}, ${border}, "${p.label}", ${text});`)
        break
      }
      case 'button_download': {
        const bg = hexToRGB565(p.bgColor); const text = hexToRGB565(p.textColor)
        lines.push(`  drawDownloadButton(${p.x}, ${p.y}, ${p.w}, ${p.h}, ${p.radius||18}, ${bg}, "${p.label}", ${text});`)
        break
      }
      case 'uiv_btn_damith_yellow': {
        const bg = hexToRGB565(p.bgColor); const border = hexToRGB565(p.borderColor); const text = hexToRGB565(p.textColor)
        lines.push(`  drawNeonYellowButton(${p.x}, ${p.y}, ${p.w}, ${p.h}, ${bg}, ${border}, "${p.label||'BUTTON'}", ${text});`)
        break
      }
      case 'uiv_btn_happy_coding': {
        const bg = hexToRGB565(p.bgColor); const border = hexToRGB565(p.borderColor); const text = hexToRGB565(p.textColor)
        lines.push(`  drawHappyCodingButton(${p.x}, ${p.y}, ${p.w}, ${p.h}, ${bg}, ${border}, "${p.label||'Happy Coding!'}", ${text});`)
        break
      }
      case 'toggle_switch':
      case 'toggle_neon': {
        const accent = hexToRGB565(p.accentColor); const bg = hexToRGB565(p.bgColor||'#1e293b')
        lines.push(`  drawToggleSwitch(${p.x}, ${p.y}, ${p.w}, ${p.h}, ${p.checked ? 1 : 0}, "${p.label}", ${accent}, ${bg});`)
        break
      }
      case 'loader_spinner': {
        const accent = hexToRGB565(p.accentColor); const bg = hexToRGB565(p.bgColor)
        lines.push(`  drawSpinnerLoader(${p.x}, ${p.y}, ${p.w}, ${p.h}, "${p.label}", ${accent}, ${bg});`)
        break
      }
      case 'loader_dots': {
        const accent = hexToRGB565(p.accentColor)
        lines.push(`  drawDotLoader(${p.x}, ${p.y}, ${p.w}, ${p.h}, ${accent});`)
        break
      }
      case 'loader_pulse': {
        const accent = hexToRGB565(p.accentColor)
        lines.push(`  drawPulseLoader(${p.x}, ${p.y}, ${p.w}, ${p.h}, ${accent});`)
        break
      }
      case 'checkbox_neon': {
        const accent = hexToRGB565(p.accentColor); const bg = hexToRGB565(p.bgColor); const text = hexToRGB565(p.textColor)
        lines.push(`  drawNeonCheckbox(${p.x}, ${p.y}, ${p.w}, ${p.h}, ${p.checked ? 1 : 0}, "${p.label}", ${accent}, ${bg}, ${text});`)
        break
      }
      case 'gauge_progress': {
        const fill = hexToRGB565(p.fillColor); const bg = hexToRGB565(p.bgColor); const border = hexToRGB565(p.borderColor)
        lines.push(`  drawProgressBar(${p.x}, ${p.y}, ${p.w}, ${p.h}, ${p.value||0}, "${p.label}", ${fill}, ${bg}, ${border});`)
        break
      }
      case 'digital_clock': {
        const col = hexToRGB565(p.color); const dateCol = hexToRGB565(p.dateColor)
        lines.push(`  drawDigitalClock(${p.x}, ${p.y}, "${p.timeStr}", "${p.dateStr}", ${col}, ${dateCol});`)
        break
      }
      case 'custom_label': {
        const col = hexToRGB565(p.color); const bg = hexToRGB565(p.bgColor)
        lines.push(`  drawCustomLabel(${p.x}, ${p.y}, ${p.w}, ${p.h}, "${p.text}", ${p.fontSize||12}, ${col}, ${bg});`)
        break
      }
      case 'alert_badge': {
        const bg = hexToRGB565(p.bgColor); const accent = hexToRGB565(p.accentColor); const text = hexToRGB565(p.textColor)
        lines.push(`  drawAlertBadge(${p.x}, ${p.y}, ${p.w}, ${p.h}, ${p.radius||10}, "${p.title}", "${p.message}", ${bg}, ${accent}, ${text});`)
        break
      }
      case 'text_input': {
        const bg = hexToRGB565(p.bgColor); const border = hexToRGB565(p.borderColor); const text = hexToRGB565(p.textColor)
        lines.push(`  drawTextInput(${p.x}, ${p.y}, ${p.w}, ${p.h}, ${p.radius||8}, "${p.placeholder}", ${bg}, ${border}, ${text});`)
        break
      }
      case 'pattern_stars': {
        lines.push(`  drawPatternStars(${p.x}, ${p.y}, ${p.w}, ${p.h});`)
        break
      }
      case 'pattern_cyber_grid': {
        const line = hexToRGB565(p.lineColor || '#00f2fe'); const bg = hexToRGB565(p.bgColor || '#060a12')
        lines.push(`  drawPatternCyberGrid(${p.x}, ${p.y}, ${p.w}, ${p.h}, ${p.gridSize||16}, ${line}, ${bg});`)
        break
      }
      case 'pattern_dot_matrix': {
        const dot = hexToRGB565(p.dotColor || '#38bdf8'); const bg = hexToRGB565(p.bgColor || '#080c14')
        lines.push(`  drawPatternDotMatrix(${p.x}, ${p.y}, ${p.w}, ${p.h}, ${p.spacing||12}, ${dot}, ${bg});`)
        break
      }
      case 'pattern_crt_scanlines': {
        const scan = hexToRGB565(p.scanColor || '#00ff66'); const bg = hexToRGB565(p.bgColor || '#05070d')
        lines.push(`  drawPatternScanlines(${p.x}, ${p.y}, ${p.w}, ${p.h}, ${scan}, ${bg});`)
        break
      }
      case 'pattern_carbon_fiber': {
        const bg = hexToRGB565(p.bgColor || '#111318'); const acc = hexToRGB565(p.accentColor || '#27272a')
        lines.push(`  drawPatternCarbonFiber(${p.x}, ${p.y}, ${p.w}, ${p.h}, ${bg}, ${acc});`)
        break
      }
      case 'pattern_hexagon': {
        const stroke = hexToRGB565(p.strokeColor || '#7c3aed'); const bg = hexToRGB565(p.bgColor || '#090d16')
        lines.push(`  drawPatternHexagon(${p.x}, ${p.y}, ${p.w}, ${p.h}, ${stroke}, ${bg});`)
        break
      }
      default: {
        const bg = hexToRGB565(p.bgColor || '#1e293b'); const text = hexToRGB565(p.textColor || '#ffffff')
        lines.push(`  drawGenericElement(${p.x}, ${p.y}, ${p.w||80}, ${p.h||40}, 8, ${bg}, "${el.name}", ${text});`)
      }
    }
    lines.push('')
  })

  lines.push('}')
  return lines.join('\n')
}

export function generateHelperHeader() {
  return `// ============================================================================
// GENERATED FILE — DO NOT EDIT MANUALLY.
// SOURCE: Luna UI Studio
// REGENERATE FROM STUDIO.
// ============================================================================
#pragma once
#include "luna_gfx_compat.h"
#include <math.h>

extern TFT_eSPI tft;

// ============================================================================
// ── KINESIS / MONOLITH EMBEDDED DRAWING PRIMITIVES ──
// ============================================================================

// ── 1. Monolith Time Hero ──
inline void drawMonolithTime(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* timeStr, const char* secStr, const char* dateStr, uint8_t batteryPct, const char* statusText, uint16_t textCol, uint16_t accentCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.fillCircle(x + 14, y + 14, 3, accentCol);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(accentCol, bg);
  tft.drawString(statusText, x + 22, y + 10, 1);
  char bBuf[16];
  snprintf(bBuf, sizeof(bBuf), "%d%%", batteryPct);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(0x07E0, bg);
  tft.drawString(bBuf, x + w - 12, y + 10, 1);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(timeStr, x + 12, y + 26, 6);
  int16_t tw = tft.textWidth(timeStr, 6);
  tft.setTextColor(accentCol, bg);
  tft.drawString(secStr, x + 14 + tw, y + 34, 2);
  tft.drawFastHLine(x + 12, y + h - 22, w - 24, border);
  tft.setTextColor(0x8410, bg);
  tft.drawString(dateStr, x + 12, y + h - 16, 1);
  tft.drawFastHLine(x + w - 28, y + h - 13, 16, accentCol);
}

// ── 2. Glance Bar ──
inline void drawGlanceBar(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* label, const char* detail, uint16_t accentCol, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.fillRoundRect(x + 6, y + 6, 26, h - 12, 4, 0x18C3);
  tft.drawPixel(x + 19, y + 12, accentCol);
  tft.drawLine(x + 19, y + 12, x + 15, y + 19, accentCol);
  tft.drawLine(x + 15, y + 19, x + 22, y + 19, accentCol);
  tft.drawLine(x + 22, y + 19, x + 17, y + 28, accentCol);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(accentCol, bg);
  tft.drawString(label, x + 38, y + 8, 1);
  tft.setTextColor(textCol, bg);
  tft.drawString(detail, x + 38, y + 22, 1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(0x8410, bg);
  tft.drawString(">", x + w - 10, y + 14, 2);
}

// ── 3. Tactile Action Button ──
inline void drawTactileButton(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* label, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(label, x + w / 2, y + h / 2, 2);
}

// ── 4. Notification Monolith ──
inline void drawNotificationBlock(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* sender, const char* timeStr, const char* preview, uint8_t unread, uint16_t accentCol, uint16_t textCol, uint16_t subCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  if (unread) tft.fillRoundRect(x, y, 4, h, 2, accentCol);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(sender, x + 12, y + 8, 1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(subCol, bg);
  tft.drawString(timeStr, x + w - 10, y + 8, 1);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(preview, x + 12, y + 26, 1);
  tft.setTextDatum(BR_DATUM);
  tft.setTextColor(accentCol, bg);
  tft.drawString("DISMISS >", x + w - 10, y + h - 6, 1);
}

// ── 5. Agenda Glance Item ──
inline void drawAgendaBlock(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* countdown, const char* timeRange, const char* title, const char* location, uint16_t accentCol, uint16_t textCol, uint16_t subCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.fillRoundRect(x + 10, y + 8, 54, 16, 4, 0x18C3);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(accentCol, 0x18C3);
  tft.drawString(countdown, x + 37, y + 16, 1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(subCol, bg);
  tft.drawString(timeRange, x + w - 10, y + 10, 1);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(title, x + 10, y + 32, 2);
  tft.setTextColor(subCol, bg);
  tft.drawString(location, x + 10, y + 56, 1);
}

// ── 6. Game Hero Launcher ──
inline void drawGameLauncher(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* title, const char* genre, const char* highScore, uint16_t accentCol, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.fillRoundRect(x + 12, y + 12, 36, 36, 8, 0x18C3);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(accentCol, 0x18C3);
  tft.drawString("G", x + 30, y + 30, 4);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(accentCol, bg);
  tft.drawString("ACTIVE", x + w - 12, y + 14, 1);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(title, x + 12, y + 58, 2);
  tft.setTextColor(0x8410, bg);
  tft.drawString(genre, x + 12, y + 82, 1);
  tft.setTextColor(accentCol, bg);
  tft.drawString(highScore, x + 12, y + 100, 2);
  tft.fillRoundRect(x + 12, y + h - 42, w - 24, 30, 6, accentCol);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(0x0000, accentCol);
  tft.drawString("PLAY NOW >", x + w / 2, y + h - 27, 2);
}

// ── 7. Focus / Pomodoro Chamber ──
inline void drawFocusChamber(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* timeRemaining, const char* modeLabel, const char* sessionTag, uint8_t progress, uint16_t accentCol, uint16_t textCol, uint16_t subCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(accentCol, bg);
  tft.drawString(modeLabel, x + 12, y + 10, 1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(subCol, bg);
  tft.drawString(sessionTag, x + w - 12, y + 10, 1);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(timeRemaining, x + w / 2, y + 56, 6);
  tft.drawRoundRect(x + 12, y + 92, w - 24, 8, 3, border);
  tft.fillRoundRect(x + 13, y + 93, ((w - 26) * progress) / 100, 6, 2, accentCol);
  tft.fillRoundRect(x + 12, y + h - 38, w - 24, 26, 6, 0x18C3);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(textCol, 0x18C3);
  tft.drawString("PAUSE SESSION", x + w / 2, y + h - 25, 1);
}

// ── 8. Digital ID & QR Card ──
inline void drawQRUtilityCard(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* title, const char* subtitle, const char* idTag, uint16_t accentCol, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(title, x + 12, y + 10, 1);
  tft.setTextColor(0x8410, bg);
  tft.drawString(subtitle, x + 12, y + 24, 1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(accentCol, bg);
  tft.drawString(idTag, x + w - 12, y + 10, 1);
  int qX = x + w / 2 - 32;
  int qY = y + 42;
  tft.fillRect(qX - 4, qY - 4, 72, 72, 0xFFFF);
  const uint8_t qrMatrix[8] = { 0xE7, 0x95, 0x99, 0xE7, 0x2A, 0xEB, 0x8C, 0xE7 };
  for (int rIdx = 0; rIdx < 8; rIdx++) {
    for (int cIdx = 0; cIdx < 8; cIdx++) {
      if ((qrMatrix[rIdx] >> (7 - cIdx)) & 1) {
        tft.fillRect(qX + cIdx * 8, qY + rIdx * 8, 8, 8, 0x0000);
      }
    }
  }
  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(0x8410, bg);
  tft.drawString("SCAN TO PAIR LUNA", x + w / 2, y + h - 8, 1);
}

// ── 9. IMU 6-Axis Spirit Level ──
inline void drawIMULevelCard(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* title, const char* pitch, const char* roll, const char* status, uint16_t accentCol, uint16_t textCol, uint16_t subCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(title, x + 12, y + 10, 1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(accentCol, bg);
  tft.drawString(status, x + w - 12, y + 10, 1);
  int cX = x + w / 2;
  int cY = y + 62;
  tft.drawCircle(cX, cY, 24, border);
  tft.drawCircle(cX, cY, 12, border);
  tft.fillCircle(cX + 3, cY - 2, 5, accentCol);
  tft.drawFastHLine(x + 12, y + h - 26, w - 24, border);
  tft.setTextDatum(BL_DATUM);
  tft.setTextColor(subCol, bg);
  tft.drawString("PITCH:", x + 16, y + h - 8, 1);
  tft.setTextColor(textCol, bg);
  tft.drawString(pitch, x + 60, y + h - 8, 1);
  tft.setTextDatum(BR_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(roll, x + w - 16, y + h - 8, 1);
  tft.setTextColor(subCol, bg);
  tft.drawString("ROLL:", x + w - 60, y + h - 8, 1);
}

// ── 10. Tactile Setting Row ──
inline void drawSettingRow(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* label, const char* valueStr, const char* sublabel, uint8_t checked, uint16_t accentCol, uint16_t textCol, uint16_t subCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(label, x + 12, y + 8, 1);
  if (sublabel && strlen(sublabel) > 0) {
    tft.setTextColor(subCol, bg);
    tft.drawString(sublabel, x + 12, y + 26, 1);
  }
  if (valueStr && strlen(valueStr) > 0) {
    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(accentCol, bg);
    tft.drawString(valueStr, x + w - 44, y + 16, 1);
  }
  int sX = x + w - 38;
  int sY = y + h / 2 - 8;
  tft.fillRoundRect(sX, sY, 28, 16, 8, checked ? accentCol : 0x2945);
  tft.fillCircle(checked ? sX + 20 : sX + 8, sY + 8, 6, 0xFFFF);
}

// ── 11. Device Specifications Readout ──
inline void drawDeviceSpecCard(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* devName, const char* soc, const char* mem, const char* power, const char* buses, uint16_t accentCol, uint16_t textCol, uint16_t subCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(devName, x + 12, y + 10, 2);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(accentCol, bg);
  tft.drawString("VERIFIED", x + w - 12, y + 12, 1);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(subCol, bg);
  tft.drawString("SoC", x + 12, y + 36, 1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(soc, x + w - 12, y + 36, 1);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(subCol, bg);
  tft.drawString("Memory", x + 12, y + 54, 1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(mem, x + w - 12, y + 54, 1);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(subCol, bg);
  tft.drawString("Power", x + 12, y + 72, 1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(power, x + w - 12, y + 72, 1);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(subCol, bg);
  tft.drawString("Buses", x + 12, y + 90, 1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(accentCol, bg);
  tft.drawString(buses, x + w - 12, y + 90, 1);
  tft.drawFastHLine(x + 12, y + h - 22, w - 24, border);
  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(subCol, bg);
  tft.drawString("LUNA OS v2.5 • ESP32-S3", x + w / 2, y + h - 6, 1);
}

// ============================================================================
// ── CONTROLLED LUNA COMPONENT PRIMITIVES EMBEDDED DRAWING ROUTINES ──
// ============================================================================

inline void drawLunaButton(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* label, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(label, x + w / 2, y + h / 2, 2);
}

inline void drawLunaIconButton(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* iconStr, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(iconStr, x + w / 2, y + h / 2, 2);
}

inline void drawLunaSurface(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* title, const char* sub, uint16_t textCol, uint16_t subCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(title, x + 12, y + 10, 2);
  tft.setTextColor(subCol, bg);
  tft.drawString(sub, x + 12, y + 32, 1);
}

inline void drawLunaListItem(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* title, const char* sub, uint8_t unread, uint16_t textCol, uint16_t subCol, uint16_t accentCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  if (unread) tft.fillRoundRect(x, y, 4, h, 2, accentCol);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(title, x + 12, y + 8, 2);
  tft.setTextColor(subCol, bg);
  tft.drawString(sub, x + 12, y + 26, 1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(subCol, bg);
  tft.drawString(">", x + w - 10, y + 14, 2);
}

inline void drawLunaToggle(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* label, uint8_t checked, uint16_t activeCol, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(label, x + 12, y + h / 2, 2);
  int sX = x + w - 44;
  int sY = y + h / 2 - 9;
  tft.fillRoundRect(sX, sY, 32, 18, 9, checked ? activeCol : 0x2167);
  tft.fillCircle(checked ? sX + 23 : sX + 9, sY + 9, 7, 0xFFFF);
}

inline void drawLunaSlider(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* label, int16_t val, uint16_t accentCol, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(0x8410, bg);
  tft.drawString(label, x + 12, y + 8, 1);
  char buf[16];
  snprintf(buf, sizeof(buf), "%d%%", val);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(accentCol, bg);
  tft.drawString(buf, x + w - 12, y + 8, 1);
  int tX = x + 12;
  int tY = y + 30;
  int tW = w - 24;
  tft.fillRoundRect(tX, tY, tW, 8, 4, 0x18C3);
  int fillW = constrain((val * tW) / 100, 0, tW);
  tft.fillRoundRect(tX, tY, fillW, 8, 4, accentCol);
  tft.fillCircle(tX + fillW, tY + 4, 8, 0xFFFF);
  tft.drawCircle(tX + fillW, tY + 4, 8, accentCol);
}

inline void drawLunaProgress(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, int16_t val, uint16_t fillCol, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  int tX = x + 12;
  int tY = y + h / 2 - 4;
  int tW = w - 24;
  tft.fillRoundRect(tX, tY, tW, 8, 4, 0x18C3);
  int fillW = constrain((val * tW) / 100, 0, tW);
  tft.fillRoundRect(tX, tY, fillW, 8, 4, fillCol);
}

inline void drawLunaIndicator(int16_t x, int16_t y, int16_t w, int16_t h, const char* label, uint16_t col) {
  tft.fillRoundRect(x, y, w, h, h / 2, 0x10A4);
  tft.drawRoundRect(x, y, w, h, h / 2, col);
  tft.fillCircle(x + 10, y + h / 2, 3, col);
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(col, 0x10A4);
  tft.drawString(label, x + 18, y + h / 2, 1);
}

inline void drawLunaHeader(int16_t x, int16_t y, int16_t w, int16_t h, const char* title, uint16_t bg, uint16_t border, uint16_t textCol) {
  tft.fillRect(x, y, w, h, bg);
  tft.drawFastHLine(x, y + h - 1, w, border);
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(title, x + 14, y + h / 2, 2);
}

inline void drawLunaNavigation(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t activeIdx, uint8_t total, uint16_t activeCol, uint16_t inactiveCol) {
  int totalW = total * 12;
  int startX = x + (w - totalW) / 2;
  int cY = y + h / 2;
  for (uint8_t i = 0; i < total; i++) {
    if (i == activeIdx) {
      tft.fillRoundRect(startX + i * 12, cY - 2, 10, 4, 2, activeCol);
    } else {
      tft.fillCircle(startX + i * 12 + 2, cY, 2, inactiveCol);
    }
  }
}

inline void drawLunaTimer(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* timeStr, const char* label, int16_t progress, uint16_t accentCol, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(accentCol, bg);
  tft.drawString(label, x + 14, y + 10, 1);
  int cX = x + w / 2;
  int cY = y + 78;
  tft.drawCircle(cX, cY, 44, 0x18C3);
  tft.drawCircle(cX, cY, 45, 0x18C3);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(timeStr, cX, cY, 4);
  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(0x15D0, bg);
  tft.drawString("ACTIVE", cX, y + h - 10, 1);
}

inline void drawLunaNumber(int16_t x, int16_t y, int16_t w, int16_t h, const char* valStr, const char* unitStr, const char* labelStr, uint16_t col, uint16_t subCol) {
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(subCol, 0x0841);
  tft.drawString(labelStr, x, y, 1);
  tft.setTextColor(col, 0x0841);
  tft.drawString(valStr, x, y + 14, 4);
  int tw = tft.textWidth(valStr, 4);
  tft.setTextColor(subCol, 0x0841);
  tft.drawString(unitStr, x + tw + 4, y + 20, 1);
}

inline void drawLunaStatus(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* label, uint16_t col) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.fillCircle(x + 8, y + h / 2, 2, col);
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(col, bg);
  tft.drawString(label, x + 15, y + h / 2, 1);
}

// ── Legacy Elements ──
// ── Glass Card ──
inline void drawGlassCard(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* title, const char* sub, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(title, x + 10, y + 10, 2);
  tft.setTextColor(0x7BEF, bg);
  tft.drawString(sub, x + 10, y + 32, 1);
}

// ── Stat Card ──
inline void drawStatCard(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* label, const char* value, const char* unit, uint16_t accent, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(accent, bg);
  tft.drawString(label, x + 8, y + 8, 1);
  tft.setTextColor(textCol, bg);
  tft.drawString(value, x + 8, y + 20, 4);
  tft.setTextColor(accent, bg);
  tft.drawString(unit, x + 8 + tft.textWidth(value, 4), y + 26, 1);
}

// ── Transaction Card ──
inline void drawTransactionCard(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, uint16_t iconBg, const char* title, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.fillRoundRect(x + 8, y + 8, 32, h - 16, 6, iconBg);
  tft.setTextColor(0xFFFF, iconBg);
  tft.drawString("$", x + 18, y + 16, 2);
  tft.setTextColor(textCol, bg);
  tft.drawString(title, x + 48, y + 18, 2);
  tft.setTextColor(0x7BEF, bg);
  tft.drawString(">", x + w - 16, y + 16, 2);
}

// ── Retro Arcade Box ──
inline void drawRetroArcadeBox(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t bg, uint16_t border, const char* title, const char* score, uint16_t textCol) {
  tft.fillRect(x, y, w, h, bg);
  tft.drawRect(x, y, w, h, border);
  tft.drawRect(x + 3, y + 3, w - 6, h - 6, border);
  tft.setTextColor(border, bg);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(title, x + 8, y + 8, 1);
  tft.setTextColor(textCol, bg);
  tft.drawString(score, x + 8, y + 22, 2);
}

// ── Neon Glow Button ──
inline void drawNeonGlowButton(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, uint16_t border, const char* label, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.drawRoundRect(x + 1, y + 1, w - 2, h - 2, r, border);
  tft.setTextColor(textCol, bg);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(label, x + w / 2, y + h / 2, 2);
}

// ── Retro Button ──
inline void drawRetroButton(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t bg, uint16_t border, const char* label, uint16_t textCol) {
  tft.fillRect(x, y, w, h, bg);
  tft.drawRect(x, y, w, h, border);
  tft.fillRect(x + w, y + 4, 4, h - 4, border);
  tft.fillRect(x + 4, y + h, w - 4, 4, border);
  tft.setTextColor(textCol, bg);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(label, x + w / 2, y + h / 2, 1);
}

// ── Download Button ──
inline void drawDownloadButton(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, const char* label, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.setTextColor(textCol, bg);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("v", x + 18, y + h / 2, 2);
  tft.drawString(label, x + w / 2 + 6, y + h / 2, 2);
}

// ── Neon Yellow Button (Damithkumara) ──
inline void drawNeonYellowButton(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t bg, uint16_t border, const char* label, uint16_t textCol) {
  tft.fillRect(x, y, w, h, bg);
  tft.drawRect(x, y, w, h, border);
  tft.drawRect(x + 1, y + 1, w - 2, h - 2, border);
  tft.setTextColor(textCol, bg);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(label, x + w / 2, y + h / 2, 2);
}

// ── Happy Coding Button (UIverse Neumorphic) ──
inline void drawHappyCodingButton(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t bg, uint16_t border, const char* label, uint16_t textCol) {
  // Beveled neumorphic rounded button with 3D inset bottom lip (#D6D6E7 -> 0xD6B9)
  tft.fillRoundRect(x, y, w, h, 8, bg);
  tft.drawRoundRect(x, y, w, h, 8, border);
  tft.drawRoundRect(x + 1, y + 1, w - 2, h - 2, 7, border);
  tft.fillRoundRect(x + 2, y + h - 5, w - 4, 3, 2, 0xD6B9);
  tft.setTextColor(textCol, bg);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(label, x + w / 2, y + (h - 3) / 2, 2);
}

// ── Toggle Switch ──
inline void drawToggleSwitch(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t on, const char* label, uint16_t accent, uint16_t bg) {
  uint16_t trackBg = on ? accent : 0x4208;
  tft.fillRoundRect(x, y + (h - 14) / 2, 36, 14, 7, trackBg);
  int16_t knobX = on ? x + 36 - 14 : x + 2;
  tft.fillCircle(knobX + 6, y + h / 2, 6, 0xFFFF);
  tft.setTextColor(on ? accent : 0x7BEF, bg);
  tft.setTextDatum(ML_DATUM);
  tft.drawString(label, x + 42, y + h / 2, 2);
}

// ── Spinner Loader ──
inline void drawSpinnerLoader(int16_t x, int16_t y, int16_t w, int16_t h, const char* label, uint16_t accent, uint16_t bg) {
  int16_t cx = x + w / 2, cy = y + h / 2 - 8, r = 14;
  tft.drawCircle(cx, cy, r, 0x4208);
  tft.drawCircle(cx, cy, r, accent);
  tft.fillCircle(cx, cy - r, 4, accent);
  tft.setTextColor(accent, bg);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(label, cx, y + h - 10, 1);
}

// ── Dot Loader ──
inline void drawDotLoader(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t accent) {
  int16_t cx = x + w / 2, cy = y + h / 2;
  for (int i = 0; i < 3; i++) tft.fillCircle(cx - 16 + i * 16, cy, 5, accent);
}

// ── Pulse Loader ──
inline void drawPulseLoader(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t accent) {
  int16_t cx = x + w / 2, cy = y + h / 2;
  tft.drawCircle(cx, cy, 18, accent);
  tft.drawCircle(cx, cy, 12, accent);
  tft.fillCircle(cx, cy, 6, accent);
}

// ── Neon Checkbox ──
inline void drawNeonCheckbox(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t checked, const char* label, uint16_t accent, uint16_t bg, uint16_t textCol) {
  uint16_t boxBg = checked ? 0x07FF : bg;
  tft.fillRoundRect(x, y + (h - 16) / 2, 16, 16, 3, boxBg);
  tft.drawRoundRect(x, y + (h - 16) / 2, 16, 16, 3, checked ? accent : 0x4208);
  if (checked) { tft.setTextColor(bg, boxBg); tft.setTextDatum(MC_DATUM); tft.drawString("v", x + 8, y + h / 2, 1); }
  tft.setTextColor(textCol, 0x0000);
  tft.setTextDatum(ML_DATUM);
  tft.drawString(label, x + 22, y + h / 2, 2);
}

// ── Progress Bar ──
inline void drawProgressBar(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t val, const char* label, uint16_t fill, uint16_t bg, uint16_t border) {
  tft.setTextColor(fill, 0x0000);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(label, x, y, 1);
  tft.fillRoundRect(x, y + 14, w, h - 14, 4, bg);
  tft.drawRoundRect(x, y + 14, w, h - 14, 4, border);
  int16_t fw = (int32_t)(w - 4) * val / 100;
  if (fw > 0) tft.fillRoundRect(x + 2, y + 16, fw, h - 18, 3, fill);
}

// ── Digital Clock ──
inline void drawDigitalClock(int16_t x, int16_t y, const char* timeStr, const char* dateStr, uint16_t col, uint16_t dateCol) {
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(col);
  tft.drawString(timeStr, x + 100, y + 26, 6);
  tft.setTextColor(dateCol);
  tft.drawString(dateStr, x + 100, y + 58, 2);
}

// ── Custom Label ──
inline void drawCustomLabel(int16_t x, int16_t y, int16_t w, int16_t h, const char* text, uint8_t fontSize, uint16_t col, uint16_t bg) {
  tft.fillRoundRect(x, y, w, h, 6, bg);
  tft.setTextColor(col, bg);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(text, x + w / 2, y + h / 2, fontSize > 14 ? 2 : 1);
}

// ── Alert Badge ──
inline void drawAlertBadge(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, const char* title, const char* msg, uint16_t bg, uint16_t accent, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, accent);
  tft.fillCircle(x + 16, y + h / 2, 4, accent);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(textCol, bg);
  tft.drawString(title, x + 28, y + 8, 2);
  tft.setTextColor(accent, bg);
  tft.drawString(msg, x + 28, y + 26, 1);
}

// ── Text Input ──
inline void drawTextInput(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, const char* placeholder, uint16_t bg, uint16_t border, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextColor(textCol, bg);
  tft.setTextDatum(ML_DATUM);
  tft.drawString(placeholder, x + 10, y + h / 2, 1);
}

// ── Generic Element fallback ──
inline void drawGenericElement(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t bg, const char* label, uint16_t textCol) {
  tft.fillRoundRect(x, y, w, h, r, bg);
  tft.drawRoundRect(x, y, w, h, r, textCol);
  tft.setTextColor(textCol, bg);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(label, x + w / 2, y + h / 2, 1);
}

// ── Pattern Elements (User Requested) ──
inline void drawPatternStars(int16_t x, int16_t y, int16_t w, int16_t h) {
  tft.fillRect(x, y, w, h, 0x0842);
  const uint16_t sX[] = { 15, 42, 68, 92, 115, 140, 185, 210, 230, 30, 75, 120, 160, 200, 50, 85, 130, 175, 220, 10 };
  const uint16_t sY[] = { 20, 15, 45, 30, 55, 25, 40, 18, 50, 80, 95, 70, 85, 90, 130, 145, 120, 135, 150, 175 };
  for (int i = 0; i < 20; i++) {
    int px = x + (sX[i] % (w > 0 ? w : 240));
    int py = y + (sY[i] % (h > 0 ? h : 280));
    tft.drawPixel(px, py, 0xFFFF);
  }
}

inline void drawPatternCyberGrid(int16_t x, int16_t y, int16_t w, int16_t h, int16_t sz, uint16_t lineCol, uint16_t bgCol) {
  tft.fillRect(x, y, w, h, bgCol);
  if (sz < 8) sz = 16;
  for (int gx = x; gx < x + w; gx += sz) tft.drawFastVLine(gx, y, h, lineCol);
  for (int gy = y; gy < y + h; gy += sz) tft.drawFastHLine(x, gy, w, lineCol);
}

inline void drawPatternDotMatrix(int16_t x, int16_t y, int16_t w, int16_t h, int16_t sp, uint16_t dotCol, uint16_t bgCol) {
  tft.fillRect(x, y, w, h, bgCol);
  if (sp < 6) sp = 12;
  for (int gx = x + sp/2; gx < x + w; gx += sp) {
    for (int gy = y + sp/2; gy < y + h; gy += sp) {
      tft.drawPixel(gx, gy, dotCol);
    }
  }
}

inline void drawPatternScanlines(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t scanCol, uint16_t bgCol) {
  tft.fillRect(x, y, w, h, bgCol);
  for (int gy = y; gy < y + h; gy += 4) tft.drawFastHLine(x, gy, w, scanCol);
}

inline void drawPatternCarbonFiber(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t bgCol, uint16_t accCol) {
  tft.fillRect(x, y, w, h, bgCol);
  for (int gy = y; gy < y + h; gy += 8) {
    for (int gx = x + (gy % 16 == 0 ? 0 : 4); gx < x + w; gx += 8) {
      tft.fillRect(gx, gy, 3, 3, accCol);
    }
  }
}

inline void drawPatternHexagon(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t strokeCol, uint16_t bgCol) {
  tft.fillRect(x, y, w, h, bgCol);
  for (int gy = y; gy < y + h; gy += 24) tft.drawFastHLine(x, gy, w, strokeCol);
  for (int gx = x; gx < x + w; gx += 24) tft.drawFastVLine(gx, y, h, strokeCol);
}
`
}

// ============================================================
// Multi-Screen C++ Code Generator with Touch Navigation
// ============================================================
export function generateMultiScreenArduinoCode(screens, activeScreenId = 'screen_1') {
  const lines = []
  lines.push('// ============================================================================')
  lines.push('// GENERATED FILE — DO NOT EDIT MANUALLY.')
  lines.push('// SOURCE: Luna UI Studio')
  lines.push('// REGENERATE FROM STUDIO.')
  lines.push('// Multi-Screen Interactive UI for ESP32-S3 1.69" (240x280 ST7789)')
  lines.push('// ============================================================================')
  lines.push('#include "luna_gfx_compat.h"')
  lines.push('#include "luna_ui_elements.h"')
  lines.push('')
  lines.push('extern TFT_eSPI tft;')
  lines.push('')

  // Screen Enum
  lines.push('// ── Available Screens ──')
  lines.push('enum LunaScreen {')
  screens.forEach((s, idx) => {
    const sName = s.name.toUpperCase().replace(/[^A-Z0-9]/g, '_')
    lines.push(`  SCREEN_${sName || idx}${idx < screens.length - 1 ? ',' : ''}`)
  })
  lines.push('};')
  lines.push('')
  const firstScreenName = screens[0]?.name.toUpperCase().replace(/[^A-Z0-9]/g, '_') || '0'
  lines.push(`LunaScreen currentScreen = SCREEN_${firstScreenName};`)
  lines.push('int currentScrollY = 0;')
  lines.push('unsigned long alertDismissMs = 0;')
  lines.push('')

  // Draw Functions for each screen
  screens.forEach((screen, sIdx) => {
    const sName = screen.name.toUpperCase().replace(/[^A-Z0-9]/g, '_')
    const bgRGB = hexToRGB565(screen.bgColor || '#0b0f19')
    lines.push(`// ── Draw Function: ${screen.name} ──`)
    lines.push(`void drawScreen_${sName}() {`)
    lines.push(`  tft.fillScreen(${bgRGB});`)

    // Background Pattern Support
    if (screen.bgType === 'pattern' || screen.bgType === 'stars') {
      if (screen.bgPattern === 'stars' || screen.bgType === 'stars') {
        lines.push('  // Background Pattern: Cosmic Starfield ✨')
        lines.push('  const uint16_t sX[] = { 15, 42, 68, 92, 115, 140, 185, 210, 230, 30, 75, 120, 160, 200, 50, 85, 130, 175, 220, 10, 60, 105, 150, 195, 225, 35, 80, 125, 165, 205, 25, 70, 110, 155, 190, 215, 45, 95, 135, 180 };')
        lines.push('  const uint16_t sY[] = { 20, 15, 45, 30, 55, 25, 40, 18, 50, 80, 95, 70, 85, 90, 130, 145, 120, 135, 150, 175, 190, 165, 180, 195, 220, 235, 210, 225, 240, 260, 250, 275, 245, 265, 270, 100, 115, 155, 170, 210 };')
        lines.push('  const uint8_t sSz[] = { 1, 2, 1, 3, 1, 2, 1, 2, 1, 2, 1, 3, 1, 2, 1, 2, 3, 1, 2, 1, 2, 1, 3, 1, 2, 1, 2, 1, 3, 1, 2, 1, 2, 1, 3, 1, 2, 1, 2, 1 };')
        lines.push('  for (int i = 0; i < 40; i++) {')
        lines.push('    if (sSz[i] == 1) tft.drawPixel(sX[i], sY[i], 0xFFFF);')
        lines.push('    else if (sSz[i] == 2) tft.fillRect(sX[i], sY[i], 2, 2, 0xFFFF);')
        lines.push('    else { tft.fillCircle(sX[i], sY[i], 1, 0xCE7F); tft.drawPixel(sX[i], sY[i], 0xFFFF); }')
        lines.push('  }')
      } else if (screen.bgPattern === 'grid') {
        lines.push('  // Background Pattern: Cyber Grid')
        lines.push('  for (int x = 0; x < 240; x += 20) tft.drawFastVLine(x, 0, 280, 0x18C3);')
        lines.push('  for (int y = 0; y < 280; y += 20) tft.drawFastHLine(0, y, 240, 0x18C3);')
      } else if (screen.bgPattern === 'dots') {
        lines.push('  // Background Pattern: Micro Dots')
        lines.push('  for (int x = 10; x < 240; x += 20) {')
        lines.push('    for (int y = 10; y < 280; y += 20) tft.drawPixel(x, y, 0x39E7);')
        lines.push('  }')
      } else if (screen.bgPattern === 'scanlines') {
        lines.push('  // Background Pattern: CRT Scanlines')
        lines.push('  for (int y = 0; y < 280; y += 4) tft.drawFastHLine(0, y, 240, 0x10A2);')
      } else if (screen.bgPattern === 'carbon') {
        lines.push('  // Background Pattern: Carbon Weave')
        lines.push('  for (int y = 0; y < 280; y += 8) {')
        lines.push('    for (int x = (y % 16 == 0 ? 0 : 4); x < 240; x += 8) {')
        lines.push('      tft.fillRect(x, y, 3, 3, 0x2124);')
        lines.push('    }')
        lines.push('  }')
      } else if (screen.bgPattern === 'hex') {
        lines.push('  // Background Pattern: Tech Hex Grid')
        lines.push('  for (int y = 0; y < 280; y += 30) tft.drawFastHLine(0, y, 240, 0x03E0);')
        lines.push('  for (int x = 0; x < 240; x += 30) tft.drawFastVLine(x, 0, 280, 0x03E0);')
      }
    }
    lines.push('')

    // Render Elements with currentScrollY applied
    screen.elements.forEach((el, i) => {
      const p = el.props
      lines.push(`  // [${el.type}] ${p.label || el.name}`)
      switch (el.type) {
        // ── Kinesis / Monolith Design System Primitives ──
        case 'monolith_time': {
          const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
          const text = hexToRGB565(p.textColor || '#EAEFF5'); const accent = hexToRGB565(p.accentColor || '#38BDF8');
          lines.push(`  drawMonolithTime(${p.x}, ${p.y} - currentScrollY, ${p.w||220}, ${p.h||90}, ${p.radius||10}, ${bg}, ${border}, "${cleanStr(p.timeStr||'10:42')}", "${cleanStr(p.secondsStr||':38')}", "${cleanStr(p.dateStr||'WED 09 SEP')}", ${p.batteryPct||94}, "${cleanStr(p.statusText||'LUNA - READY')}", ${text}, ${accent});`)
          break
        }
        case 'glance_bar': {
          const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
          const text = hexToRGB565(p.textColor || '#EAEFF5'); const accent = hexToRGB565(p.accentColor || '#FF9E3B');
          lines.push(`  drawGlanceBar(${p.x}, ${p.y} - currentScrollY, ${p.w||220}, ${p.h||42}, ${p.radius||8}, ${bg}, ${border}, "${cleanStr(p.label||'NEXT FOCUS')}", "${cleanStr(p.detail||'')}", ${accent}, ${text});`)
          break
        }
        case 'tactile_button': {
          const bg = hexToRGB565(p.bgColor || '#FF9E3B'); const border = hexToRGB565(p.borderColor || '#FFB266');
          const text = hexToRGB565(p.textColor || '#080A0F');
          lines.push(`  drawTactileButton(${p.x}, ${p.y} - currentScrollY, ${p.w||220}, ${p.h||48}, ${p.radius||12}, ${bg}, ${border}, "${cleanStr(p.label||'ACTION')}", ${text});`)
          break
        }
        case 'notification_block': {
          const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
          const text = hexToRGB565(p.textColor || '#EAEFF5'); const sub = hexToRGB565(p.subtextColor || '#8290A4');
          const accent = hexToRGB565(p.accentColor || '#38BDF8');
          lines.push(`  drawNotificationBlock(${p.x}, ${p.y} - currentScrollY, ${p.w||220}, ${p.h||72}, ${p.radius||10}, ${bg}, ${border}, "${cleanStr(p.sender||'SENDER')}", "${cleanStr(p.timeStr||'now')}", "${cleanStr(p.preview||'')}", ${p.unread ? 1 : 0}, ${accent}, ${text}, ${sub});`)
          break
        }
        case 'agenda_block': {
          const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
          const text = hexToRGB565(p.textColor || '#EAEFF5'); const sub = hexToRGB565(p.subtextColor || '#8290A4');
          const accent = hexToRGB565(p.accentColor || '#38BDF8');
          lines.push(`  drawAgendaBlock(${p.x}, ${p.y} - currentScrollY, ${p.w||220}, ${p.h||84}, ${p.radius||10}, ${bg}, ${border}, "${cleanStr(p.countdown||'IN 24m')}", "${cleanStr(p.timeRange||'')}", "${cleanStr(p.title||'Event')}", "${cleanStr(p.location||'')}", ${accent}, ${text}, ${sub});`)
          break
        }
        case 'game_launcher': {
          const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
          const text = hexToRGB565(p.textColor || '#EAEFF5'); const accent = hexToRGB565(p.accentColor || '#FF9E3B');
          lines.push(`  drawGameLauncher(${p.x}, ${p.y} - currentScrollY, ${p.w||220}, ${p.h||170}, ${p.radius||12}, ${bg}, ${border}, "${cleanStr(p.title||'GAME')}", "${cleanStr(p.genre||'ARCADE')}", "${cleanStr(p.highScore||'0 PTS')}", ${accent}, ${text});`)
          break
        }
        case 'focus_chamber': {
          const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
          const text = hexToRGB565(p.textColor || '#EAEFF5'); const sub = hexToRGB565(p.subtextColor || '#8290A4');
          const accent = hexToRGB565(p.accentColor || '#FF9E3B');
          lines.push(`  drawFocusChamber(${p.x}, ${p.y} - currentScrollY, ${p.w||220}, ${p.h||175}, ${p.radius||12}, ${bg}, ${border}, "${cleanStr(p.timeRemaining||'25:00')}", "${cleanStr(p.modeLabel||'FOCUS')}", "${cleanStr(p.sessionTag||'SESSION 1')}", ${p.progress||50}, ${accent}, ${text}, ${sub});`)
          break
        }
        case 'qr_utility_card': {
          const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
          const text = hexToRGB565(p.textColor || '#EAEFF5'); const accent = hexToRGB565(p.accentColor || '#38BDF8');
          lines.push(`  drawQRUtilityCard(${p.x}, ${p.y} - currentScrollY, ${p.w||220}, ${p.h||160}, ${p.radius||12}, ${bg}, ${border}, "${cleanStr(p.title||'LUNA ID')}", "${cleanStr(p.subtitle||'')}", "${cleanStr(p.idTag||'')}", ${accent}, ${text});`)
          break
        }
        case 'imu_level_card': {
          const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
          const text = hexToRGB565(p.textColor || '#EAEFF5'); const sub = hexToRGB565(p.subtextColor || '#8290A4');
          const accent = hexToRGB565(p.accentColor || '#10B981');
          lines.push(`  drawIMULevelCard(${p.x}, ${p.y} - currentScrollY, ${p.w||220}, ${p.h||140}, ${p.radius||12}, ${bg}, ${border}, "${cleanStr(p.title||'IMU LEVEL')}", "${cleanStr(p.pitch||'+0.0')}", "${cleanStr(p.roll||'+0.0')}", "${cleanStr(p.status||'LEVEL')}", ${accent}, ${text}, ${sub});`)
          break
        }
        case 'setting_row': {
          const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
          const text = hexToRGB565(p.textColor || '#EAEFF5'); const sub = hexToRGB565(p.subtextColor || '#8290A4');
          const accent = hexToRGB565(p.accentColor || '#38BDF8');
          lines.push(`  drawSettingRow(${p.x}, ${p.y} - currentScrollY, ${p.w||220}, ${p.h||48}, ${p.radius||8}, ${bg}, ${border}, "${cleanStr(p.label||'SETTING')}", "${cleanStr(p.valueStr||'')}", "${cleanStr(p.sublabel||'')}", ${p.checked ? 1 : 0}, ${accent}, ${text}, ${sub});`)
          break
        }
        case 'device_spec_card': {
          const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
          const text = hexToRGB565(p.textColor || '#EAEFF5'); const sub = hexToRGB565(p.subtextColor || '#8290A4');
          const accent = hexToRGB565(p.accentColor || '#10B981');
          lines.push(`  drawDeviceSpecCard(${p.x}, ${p.y} - currentScrollY, ${p.w||220}, ${p.h||160}, ${p.radius||12}, ${bg}, ${border}, "${cleanStr(p.deviceName||'LUNA CORE')}", "${cleanStr(p.soc||'ESP32-S3')}", "${cleanStr(p.memory||'16MB/8MB')}", "${cleanStr(p.batteryInfo||'4.12V')}", "${cleanStr(p.status||'OK')}", ${accent}, ${text}, ${sub});`)
          break
        }
        // ── Controlled Luna Component Primitives ──
        case 'luna_button': {
          const bg = hexToRGB565(p.variant === 'secondary' ? '#121721' : p.variant === 'destructive' ? '#991B1B' : '#FF9E3B');
          const border = hexToRGB565(p.variant === 'secondary' ? '#232D3F' : p.variant === 'destructive' ? '#DC2626' : '#FFB266');
          const text = hexToRGB565(p.variant === 'secondary' ? '#EAEFF5' : p.variant === 'destructive' ? '#FFFFFF' : '#080A0F');
          lines.push(`  drawLunaButton(${p.x}, ${p.y} - currentScrollY, ${p.w||220}, ${p.h||48}, 12, ${bg}, ${border}, "${cleanStr(p.label||'BUTTON')}", ${text});`)
          break
        }
        case 'luna_icon_button': {
          const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
          const text = hexToRGB565(p.textColor || '#38BDF8');
          lines.push(`  drawLunaIconButton(${p.x}, ${p.y} - currentScrollY, ${p.w||44}, ${p.h||44}, 10, ${bg}, ${border}, "${cleanStr(p.icon||'⚙')}", ${text});`)
          break
        }
        case 'luna_surface': {
          const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
          const text = hexToRGB565(p.textColor || '#EAEFF5'); const sub = hexToRGB565(p.subtextColor || '#8290A4');
          lines.push(`  drawLunaSurface(${p.x}, ${p.y} - currentScrollY, ${p.w||220}, ${p.h||80}, ${p.radius||10}, ${bg}, ${border}, "${cleanStr(p.title||'Surface')}", "${cleanStr(p.subtitle||'')}", ${text}, ${sub});`)
          break
        }
        case 'luna_list_item': {
          const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
          const text = hexToRGB565(p.textColor || '#EAEFF5'); const sub = hexToRGB565(p.subtextColor || '#8290A4');
          const accent = hexToRGB565(p.accentColor || '#38BDF8');
          lines.push(`  drawLunaListItem(${p.x}, ${p.y} - currentScrollY, ${p.w||220}, ${p.h||52}, 8, ${bg}, ${border}, "${cleanStr(p.title||'Item')}", "${cleanStr(p.subtitle||'')}", ${p.unread ? 1 : 0}, ${text}, ${sub}, ${accent});`)
          break
        }
        case 'luna_toggle': {
          const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
          const text = hexToRGB565(p.textColor || '#EAEFF5'); const active = hexToRGB565(p.activeColor || '#38BDF8');
          lines.push(`  drawLunaToggle(${p.x}, ${p.y} - currentScrollY, ${p.w||220}, ${p.h||48}, 10, ${bg}, ${border}, "${cleanStr(p.label||'Option')}", ${p.checked ? 1 : 0}, ${active}, ${text});`)
          break
        }
        case 'luna_slider': {
          const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
          const text = hexToRGB565(p.textColor || '#EAEFF5'); const accent = hexToRGB565(p.accentColor || '#FF9E3B');
          lines.push(`  drawLunaSlider(${p.x}, ${p.y} - currentScrollY, ${p.w||220}, ${p.h||64}, 10, ${bg}, ${border}, "${cleanStr(p.label||'SLIDER')}", ${p.value||50}, ${accent}, ${text});`)
          break
        }
        case 'luna_progress': {
          const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
          const fill = hexToRGB565(p.accentColor || '#38BDF8'); const text = hexToRGB565(p.textColor || '#EAEFF5');
          lines.push(`  drawLunaProgress(${p.x}, ${p.y} - currentScrollY, ${p.w||220}, ${p.h||48}, 10, ${bg}, ${border}, ${p.value||50}, ${fill}, ${text});`)
          break
        }
        case 'luna_indicator': {
          const col = hexToRGB565(p.status === 'error' ? '#EF4444' : p.status === 'warning' ? '#FF9E3B' : '#10B981');
          lines.push(`  drawLunaIndicator(${p.x}, ${p.y} - currentScrollY, ${p.w||100}, ${p.h||30}, "${cleanStr(p.label||'ONLINE')}", ${col});`)
          break
        }
        case 'luna_header': {
          const bg = hexToRGB565(p.bgColor || '#080A0F'); const border = hexToRGB565(p.borderColor || '#1A2232');
          const text = hexToRGB565(p.textColor || '#EAEFF5');
          lines.push(`  drawLunaHeader(${p.x}, ${p.y} - currentScrollY, ${p.w||240}, ${p.h||44}, "${cleanStr(p.title||'HEADER')}", ${bg}, ${border}, ${text});`)
          break
        }
        case 'luna_navigation': {
          const active = hexToRGB565(p.activeColor || '#38BDF8'); const inactive = hexToRGB565(p.inactiveColor || '#232D3F');
          lines.push(`  drawLunaNavigation(${p.x}, ${p.y} - currentScrollY, ${p.w||220}, ${p.h||36}, ${p.activeIndex||0}, ${p.total||8}, ${active}, ${inactive});`)
          break
        }
        case 'luna_timer': {
          const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
          const text = hexToRGB565(p.textColor || '#EAEFF5'); const accent = hexToRGB565(p.accentColor || '#FF9E3B');
          lines.push(`  drawLunaTimer(${p.x}, ${p.y} - currentScrollY, ${p.w||220}, ${p.h||175}, 12, ${bg}, ${border}, "${cleanStr(p.timeStr||'25:00')}", "${cleanStr(p.label||'FOCUS')}", ${p.progress||50}, ${accent}, ${text});`)
          break
        }
        case 'luna_number': {
          const col = hexToRGB565(p.accentColor || '#38BDF8'); const sub = hexToRGB565(p.subtextColor || '#8290A4');
          lines.push(`  drawLunaNumber(${p.x}, ${p.y} - currentScrollY, ${p.w||100}, ${p.h||44}, "${cleanStr(p.value||'0')}", "${cleanStr(p.unit||'')}", "${cleanStr(p.label||'')}", ${col}, ${sub});`)
          break
        }
        case 'luna_status': {
          const bg = hexToRGB565(p.bgColor || '#121721'); const border = hexToRGB565(p.borderColor || '#232D3F');
          const col = hexToRGB565(p.status === 'error' ? '#EF4444' : p.status === 'warning' ? '#FF9E3B' : '#10B981');
          lines.push(`  drawLunaStatus(${p.x}, ${p.y} - currentScrollY, ${p.w||90}, ${p.h||26}, 6, ${bg}, ${border}, "${cleanStr(p.label||'ONLINE')}", ${col});`)
          break
        }
        // ── Legacy Elements ──
        case 'card_glass': {
          const bg = hexToRGB565(p.bgColor); const border = hexToRGB565(p.borderColor); const text = hexToRGB565(p.textColor)
          lines.push(`  drawGlassCard(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, ${p.radius||14}, ${bg}, ${border}, "${p.title||'Card'}", "${p.subtitle||''}", ${text});`)
          break
        }
        case 'card_stat': {
          const bg = hexToRGB565(p.bgColor); const border = hexToRGB565(p.borderColor); const accent = hexToRGB565(p.accentColor); const text = hexToRGB565(p.textColor)
          lines.push(`  drawStatCard(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, ${p.radius||10}, ${bg}, ${border}, "${p.label||'STAT'}", "${p.value||'98'}", "${p.unit||'%'}", ${accent}, ${text});`)
          break
        }
        case 'card_transaction': {
          const bg = hexToRGB565(p.bgColor); const iconBg = hexToRGB565(p.iconBg); const border = hexToRGB565(p.borderColor); const text = hexToRGB565(p.textColor)
          lines.push(`  drawTransactionCard(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, ${p.radius||12}, ${bg}, ${border}, ${iconBg}, "${p.title||'Tx'}", ${text});`)
          break
        }
        case 'card_retro': {
          const bg = hexToRGB565(p.bgColor); const border = hexToRGB565(p.borderColor); const text = hexToRGB565(p.textColor)
          lines.push(`  drawRetroArcadeBox(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, ${bg}, ${border}, "${p.title||'ARCADE'}", "${p.score||'0'}", ${text});`)
          break
        }
        case 'button_neon': {
          const bg = hexToRGB565(p.bgColor); const border = hexToRGB565(p.borderColor); const text = hexToRGB565(p.textColor)
          lines.push(`  drawNeonGlowButton(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, ${p.radius||8}, ${bg}, ${border}, "${p.label||'Button'}", ${text});`)
          break
        }
        case 'button_retro': {
          const bg = hexToRGB565(p.bgColor); const border = hexToRGB565(p.borderColor); const text = hexToRGB565(p.textColor)
          lines.push(`  drawRetroButton(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, ${bg}, ${border}, "${p.label||'PRESS'}", ${text});`)
          break
        }
        case 'button_download': {
          const bg = hexToRGB565(p.bgColor); const text = hexToRGB565(p.textColor)
          lines.push(`  drawDownloadButton(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, ${p.radius||18}, ${bg}, "${p.label||'Download'}", ${text});`)
          break
        }
        case 'uiv_btn_damith_yellow': {
          const bg = hexToRGB565(p.bgColor); const border = hexToRGB565(p.borderColor); const text = hexToRGB565(p.textColor)
          lines.push(`  drawNeonYellowButton(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, ${bg}, ${border}, "${p.label||'BUTTON'}", ${text});`)
          break
        }
        case 'uiv_btn_happy_coding': {
          const bg = hexToRGB565(p.bgColor); const border = hexToRGB565(p.borderColor); const text = hexToRGB565(p.textColor)
          lines.push(`  drawHappyCodingButton(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, ${bg}, ${border}, "${p.label||'Happy Coding!'}", ${text});`)
          break
        }
        case 'toggle_switch':
        case 'toggle_neon': {
          const accent = hexToRGB565(p.accentColor); const bg = hexToRGB565(p.bgColor||'#1e293b')
          lines.push(`  drawToggleSwitch(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, ${p.checked ? 1 : 0}, "${p.label||''}", ${accent}, ${bg});`)
          break
        }
        case 'loader_spinner': {
          const accent = hexToRGB565(p.accentColor); const bg = hexToRGB565(p.bgColor)
          lines.push(`  drawSpinnerLoader(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, "${p.label||''}", ${accent}, ${bg});`)
          break
        }
        case 'loader_dots': {
          const accent = hexToRGB565(p.accentColor)
          lines.push(`  drawDotLoader(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, ${accent});`)
          break
        }
        case 'loader_pulse': {
          const accent = hexToRGB565(p.accentColor)
          lines.push(`  drawPulseLoader(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, ${accent});`)
          break
        }
        case 'checkbox_neon': {
          const accent = hexToRGB565(p.accentColor); const bg = hexToRGB565(p.bgColor); const text = hexToRGB565(p.textColor)
          lines.push(`  drawNeonCheckbox(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, ${p.checked ? 1 : 0}, "${p.label||''}", ${accent}, ${bg}, ${text});`)
          break
        }
        case 'gauge_progress': {
          const fill = hexToRGB565(p.fillColor); const bg = hexToRGB565(p.bgColor); const border = hexToRGB565(p.borderColor)
          lines.push(`  drawProgressBar(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, ${p.value||0}, "${p.label||''}", ${fill}, ${bg}, ${border});`)
          break
        }
        case 'digital_clock': {
          const col = hexToRGB565(p.color); const dateCol = hexToRGB565(p.dateColor)
          lines.push(`  drawDigitalClock(${p.x}, ${p.y} - currentScrollY, "${p.timeStr||'12:45'}", "${p.dateStr||'WED, 09 SEP'}", ${col}, ${dateCol});`)
          break
        }
        case 'custom_label': {
          const col = hexToRGB565(p.color); const bg = hexToRGB565(p.bgColor)
          lines.push(`  drawCustomLabel(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, "${p.text||''}", ${p.fontSize||12}, ${col}, ${bg});`)
          break
        }
        case 'alert_badge': {
          const bg = hexToRGB565(p.bgColor); const accent = hexToRGB565(p.accentColor); const text = hexToRGB565(p.textColor)
          lines.push(`  drawAlertBadge(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, ${p.radius||10}, "${p.title||''}", "${p.message||''}", ${bg}, ${accent}, ${text});`)
          break
        }
        case 'text_input': {
          const bg = hexToRGB565(p.bgColor); const border = hexToRGB565(p.borderColor); const text = hexToRGB565(p.textColor)
          lines.push(`  drawTextInput(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, ${p.radius||8}, "${p.placeholder||''}", ${bg}, ${border}, ${text});`)
          break
        }
        case 'pattern_stars': {
          lines.push(`  drawPatternStars(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h});`)
          break
        }
        case 'pattern_cyber_grid': {
          const line = hexToRGB565(p.lineColor || '#00f2fe'); const bg = hexToRGB565(p.bgColor || '#060a12')
          lines.push(`  drawPatternCyberGrid(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, ${p.gridSize||16}, ${line}, ${bg});`)
          break
        }
        case 'pattern_dot_matrix': {
          const dot = hexToRGB565(p.dotColor || '#38bdf8'); const bg = hexToRGB565(p.bgColor || '#080c14')
          lines.push(`  drawPatternDotMatrix(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, ${p.spacing||12}, ${dot}, ${bg});`)
          break
        }
        case 'pattern_crt_scanlines': {
          const scan = hexToRGB565(p.scanColor || '#00ff66'); const bg = hexToRGB565(p.bgColor || '#05070d')
          lines.push(`  drawPatternScanlines(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, ${scan}, ${bg});`)
          break
        }
        case 'pattern_carbon_fiber': {
          const bg = hexToRGB565(p.bgColor || '#111318'); const acc = hexToRGB565(p.accentColor || '#27272a')
          lines.push(`  drawPatternCarbonFiber(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, ${bg}, ${acc});`)
          break
        }
        case 'pattern_hexagon': {
          const stroke = hexToRGB565(p.strokeColor || '#7c3aed'); const bg = hexToRGB565(p.bgColor || '#090d16')
          lines.push(`  drawPatternHexagon(${p.x}, ${p.y} - currentScrollY, ${p.w}, ${p.h}, ${stroke}, ${bg});`)
          break
        }
        default: {
          const bg = hexToRGB565(p.bgColor || '#1e293b'); const text = hexToRGB565(p.textColor || '#ffffff')
          lines.push(`  drawGenericElement(${p.x}, ${p.y} - currentScrollY, ${p.w||80}, ${p.h||40}, 8, ${bg}, "${el.name}", ${text});`)
        }
      }
    })

    // Scrollbar indicator if screen is scrollable
    if (screen.isScrollable) {
      const maxScroll = Math.max(1, (screen.maxScrollY || 560) - 280)
      lines.push('  // Display Scrollbar Indicator')
      lines.push(`  const int maxScrollH_${sName} = ${maxScroll};`)
      lines.push(`  int thumbH = max(24, (280 * 280) / ${screen.maxScrollY || 560});`)
      lines.push(`  int thumbY = (currentScrollY * (280 - thumbH)) / maxScrollH_${sName};`)
      lines.push('  tft.drawFastVLine(238, 0, 280, 0x18C3);')
      lines.push('  tft.fillRoundRect(236, thumbY, 3, thumbH, 1, 0x07FF);')
    }

    lines.push('}')
    lines.push('')
  })

  // Screen Switcher
  lines.push('// ── Main Screen Refresh Dispatcher ──')
  lines.push('void drawCurrentScreen() {')
  lines.push('  Serial.printf("[Screen] Rendering Screen ID: %d\\n", currentScreen);')
  lines.push('  switch(currentScreen) {')
  screens.forEach((s) => {
    const sName = s.name.toUpperCase().replace(/[^A-Z0-9]/g, '_')
    lines.push(`    case SCREEN_${sName}: drawScreen_${sName}(); break;`)
  })
  lines.push('  }')
  lines.push('  tft.flush(); // Push complete frame buffer to display at 80MHz')
  lines.push('}')
  lines.push('')

  // Touch & Action Router
  lines.push('// ── Touch & Button Interaction Handler ──')
  lines.push('void handleScreenTouch(int touchX, int touchY) {')
  screens.forEach((s) => {
    const sName = s.name.toUpperCase().replace(/[^A-Z0-9]/g, '_')
    const interactiveEls = s.elements.filter(el => el.actions && el.actions.length > 0)
    if (interactiveEls.length > 0) {
      lines.push(`  if (currentScreen == SCREEN_${sName}) {`)
      interactiveEls.forEach(el => {
        const p = el.props
        const act = el.actions[0]
        lines.push(`    // Check touch on "${p.label || el.name}" (${p.x}, ${p.y}, ${p.w}, ${p.h}) with scroll offset`)
        lines.push(`    if (touchX >= ${p.x} && touchX <= ${p.x + (p.w||80)} && touchY >= (${p.y} - currentScrollY) && touchY <= (${p.y} - currentScrollY + ${p.h||40})) {`)
        if (act.actionType === 'navigate' && act.targetScreenId) {
          const target = screens.find(t => t.id === act.targetScreenId)
          if (target) {
            const targetName = target.name.toUpperCase().replace(/[^A-Z0-9]/g, '_')
            lines.push(`      // Action: Navigate to "${target.name}"`)
            lines.push(`      currentScreen = SCREEN_${targetName};`)
            lines.push(`      currentScrollY = 0;`)
            lines.push(`      drawCurrentScreen();`)
            lines.push(`      return;`)
          }
        } else if (act.actionType === 'scroll') {
          lines.push(`      // Action: Button Scroll ${act.scrollDirection || 'down'}`)
          lines.push(`      currentScrollY += ${act.scrollAmount || 80};`)
          lines.push(`      drawCurrentScreen();`)
          lines.push(`      return;`)
        } else if (act.actionType === 'alert') {
          lines.push(`      // Action: Show Alert Notification`)
          lines.push(`      drawAlertBadge(20, 20, 200, 50, 8, "LUNA ALERT", "${act.alertMessage || 'Notice'}", 0x18E3, 0x07E0, 0xFFFF);`)
          lines.push(`      alertDismissMs = millis() + 1500;`)
          lines.push(`      tft.flush();`)
          lines.push(`      return;`)
        } else if (act.actionType === 'toggle') {
          lines.push(`      // Action: Toggle Setting`)
          lines.push(`      drawAlertBadge(20, 20, 200, 50, 8, "SETTING UPDATED", "${act.alertMessage || 'Toggled'}", 0x18E3, 0x10B981, 0xFFFF);`)
          lines.push(`      alertDismissMs = millis() + 1500;`)
          lines.push(`      tft.flush();`)
          lines.push(`      return;`)
        }
        lines.push(`    }`)
      })
      lines.push(`  }`)
    }
  })
  lines.push('}')
  lines.push('')

  // Screen Gesture Navigation Router (Swipe Left / Swipe Right)
  lines.push('// ── Screen-Level Gesture Swipe Handler (Left / Right) ──')
  lines.push('void handleScreenSwipe(uint8_t direction) {')
  lines.push('  // direction: 1 = Swipe Left, 2 = Swipe Right')
  screens.forEach((s) => {
    const sName = s.name.toUpperCase().replace(/[^A-Z0-9]/g, '_')
    const getTgtId = (g) => typeof g === 'string' ? g : (g?.targetScreenId || g?.target || null)
    const swLeftTarget = s.gestures?.swipeLeft ? screens.find(t => t.id === getTgtId(s.gestures.swipeLeft)) : null
    const swRightTarget = s.gestures?.swipeRight ? screens.find(t => t.id === getTgtId(s.gestures.swipeRight)) : null
    if (swLeftTarget || swRightTarget) {
      lines.push(`  if (currentScreen == SCREEN_${sName}) {`)
      if (swLeftTarget) {
        const tgtName = swLeftTarget.name.toUpperCase().replace(/[^A-Z0-9]/g, '_')
        lines.push(`    if (direction == 1) { // Swipe Left -> "${swLeftTarget.name}"`)
        lines.push(`      currentScreen = SCREEN_${tgtName};`)
        lines.push(`      currentScrollY = 0;`)
        lines.push(`      drawCurrentScreen();`)
        lines.push(`      return;`)
        lines.push(`    }`)
      }
      if (swRightTarget) {
        const tgtName = swRightTarget.name.toUpperCase().replace(/[^A-Z0-9]/g, '_')
        lines.push(`    if (direction == 2) { // Swipe Right -> "${swRightTarget.name}"`)
        lines.push(`      currentScreen = SCREEN_${tgtName};`)
        lines.push(`      currentScrollY = 0;`)
        lines.push(`      drawCurrentScreen();`)
        lines.push(`      return;`)
        lines.push(`    }`)
      }
      lines.push(`  }`)
    }
  })
  lines.push('}')
  lines.push('')

  // Full-Display Vertical Scroll Handler
  lines.push('// ── Full-Display Touch Drag Vertical Scrolling Handler ──')
  lines.push('void handleScreenScroll(int deltaY) {')
  screens.forEach((s) => {
    const sName = s.name.toUpperCase().replace(/[^A-Z0-9]/g, '_')
    if (s.isScrollable) {
      const maxScroll = Math.max(1, (s.maxScrollY || 560) - 280)
      lines.push(`  if (currentScreen == SCREEN_${sName}) {`)
      lines.push(`    int newScroll = constrain(currentScrollY + deltaY, 0, ${maxScroll});`)
      lines.push(`    if (newScroll != currentScrollY) {`)
      lines.push(`      currentScrollY = newScroll;`)
      lines.push(`      drawCurrentScreen();`)
      lines.push(`    }`)
      lines.push(`    return;`)
      lines.push(`  }`)
    }
  })
  lines.push('}')
  lines.push('')

  return lines.join('\n')
}

// ============================================================
// Hardware Launcher Generator (Waveshare ESP32-S3 Touch LCD 1.69)
// ============================================================
export function generateLauncherIno() {
  return `// ============================================================================
// GENERATED FILE — DO NOT EDIT MANUALLY.
// SOURCE: Luna UI Studio
// REGENERATE FROM STUDIO.
// Hardware Launcher for Waveshare ESP32-S3-Touch-LCD-1.69 (240x280)
// ============================================================================
#include <Arduino.h>
#include <Wire.h>
#include "Arduino_GFX_Library.h"
#include "luna_gfx_compat.h"

// Hardware Pin Definitions for Waveshare ESP32-S3-Touch-LCD-1.69
#define LCD_DC      4    // ST7789 DC
#define LCD_CS      5    // ST7789 CS
#define LCD_SCL     6    // ST7789 SCLK
#define LCD_SDA     7    // ST7789 MOSI
#define LCD_RST     8    // ST7789 RST
#define LCD_BL      15   // Backlight
#define POWER_HOLD  41   // Power hold (MUST be held HIGH for board power)
#define TOUCH_SDA   11   // I2C SDA
#define TOUCH_SCL   10   // I2C SCL
#define TOUCH_INT   14   // CST816T Interrupt
#define TOUCH_RST   13   // CST816T Reset
#define CST816T_ADDR 0x15

#define LCD_WIDTH   240
#define LCD_HEIGHT  280

// Official Waveshare Arduino_GFX ST7789 display bus with exact 20-pixel panel offset
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCL, LCD_SDA);
Arduino_GFX *output_gfx = new Arduino_ST7789(bus, LCD_RST, 0 /* rotation */, true /* IPS */,
                                              LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 20);
// High-performance Double-Buffered Canvas in PSRAM/RAM for 60FPS tearing-free UI
Arduino_Canvas *canvas = new Arduino_Canvas(LCD_WIDTH, LCD_HEIGHT, output_gfx);

// Global wrapper instance required by Luna UI Studio generated code
LunaGFXWrapper tft;

// Forward declarations from Luna_MultiScreen_App.ino
extern void drawCurrentScreen();
extern void handleScreenTouch(int touchX, int touchY);
extern void handleScreenSwipe(uint8_t direction);
extern void handleScreenScroll(int deltaY);

// Touch tracking
static volatile bool touchInterruptOccurred = false;
static void IRAM_ATTR touchISR() {
  touchInterruptOccurred = true;
}

void initTouch() {
  Serial.println("[Touch] Resetting CST816T...");
  pinMode(TOUCH_RST, OUTPUT);
  digitalWrite(TOUCH_RST, LOW);
  delay(10);
  digitalWrite(TOUCH_RST, HIGH);
  delay(50);

  Serial.println("[Touch] Starting I2C Wire on SDA=11, SCL=10...");
  Wire.begin(TOUCH_SDA, TOUCH_SCL, 400000);
  Wire.setTimeOut(50);

  pinMode(TOUCH_INT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TOUCH_INT), touchISR, FALLING);
  touchInterruptOccurred = false;
  Serial.println("[Touch] CST816T ready.");
}

bool readTouchPoint(int &touchX, int &touchY, uint8_t &gesture) {
  gesture = 0;
  if (!touchInterruptOccurred && digitalRead(TOUCH_INT) == HIGH) {
    return false;
  }
  touchInterruptOccurred = false;

  Wire.beginTransmission(CST816T_ADDR);
  Wire.write(0x01);
  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  if (Wire.requestFrom((uint16_t)CST816T_ADDR, (uint8_t)6) < 6) {
    return false;
  }

  gesture           = Wire.read();
  uint8_t fingerNum = Wire.read();
  uint8_t xH        = Wire.read();
  uint8_t xL        = Wire.read();
  uint8_t yH        = Wire.read();
  uint8_t yL        = Wire.read();

  if (fingerNum == 0) {
    return false;
  }

  int rawX = ((xH & 0x0F) << 8) | xL;
  int rawY = ((yH & 0x0F) << 8) | yL;

  touchX = constrain(rawX, 0, 239);
  touchY = constrain(rawY, 0, 279);

  return true;
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("==========================================");
  Serial.println("  Luna UI Studio - ESP32-S3 1.69 App");
  Serial.println("  Double-Buffered 60FPS UI Active");
  Serial.println("==========================================");

  // 1. Maintain hardware power hold (GPIO 41)
  Serial.println("[Boot] 1. Enabling Power Hold (GPIO 41)...");
  pinMode(POWER_HOLD, OUTPUT);
  digitalWrite(POWER_HOLD, HIGH);

  // 2. Hardware LCD Reset (GPIO 8)
  Serial.println("[Boot] 2. Hardware Reset LCD (GPIO 8)...");
  pinMode(LCD_DC, OUTPUT);
  pinMode(LCD_RST, OUTPUT);
  digitalWrite(LCD_RST, HIGH);
  delay(50);
  digitalWrite(LCD_RST, LOW);
  delay(100);
  digitalWrite(LCD_RST, HIGH);
  delay(150);

  // 3. Initialize Backlight (GPIO 15)
  Serial.println("[Boot] 3. Enabling Backlight (GPIO 15)...");
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);

  // 4. Initialize ST7789 Display at 80MHz SPI and Double-Buffered Canvas
  Serial.println("[Boot] 4. Initializing Arduino_GFX ST7789 (80MHz SPI)...");
  if (!output_gfx->begin(80000000)) {
    Serial.println("[Boot] ERROR: output_gfx->begin() failed!");
  } else {
    Serial.println("[Boot] output_gfx->begin(80MHz) succeeded!");
  }

  Serial.println("[Boot] Initializing Double-Buffered Canvas in RAM/PSRAM...");
  if (!canvas->begin()) {
    Serial.println("[Boot] Canvas begin failed, falling back to direct display");
    tft.attach(output_gfx);
  } else {
    Serial.println("[Boot] Canvas ready! Ultra-smooth 60FPS enabled.");
    tft.attach(canvas);
  }

  // 5. Initialize Capacitive Touch
  Serial.println("[Boot] 5. Initializing Touch...");
  initTouch();

  // 6. Draw initial screen
  Serial.println("[Boot] 6. Rendering Luna UI Studio screen...");
  drawCurrentScreen();
  Serial.println("[Boot] Screen rendered successfully! Entering loop.");
}

static int touchStartX = 0;
static int touchStartY = 0;
static int lastTouchY = 0;
static int lastTouchX = 0;
static bool isTouching = false;
static bool isDragging = false;
static unsigned long touchStartTime = 0;
static uint8_t detectedGesture = 0;

void loop() {
  int tx = 0, ty = 0;
  uint8_t g = 0;
  bool touchActive = readTouchPoint(tx, ty, g);

  if (touchActive) {
    unsigned long now = millis();
    if (g != 0) {
      detectedGesture = g;
    }

    if (!isTouching) {
      // Touch Down
      isTouching = true;
      isDragging = false;
      touchStartX = tx;
      touchStartY = ty;
      lastTouchY = ty;
      lastTouchX = tx;
      touchStartTime = now;
      detectedGesture = g;
    } else {
      // Finger is dragging / moving
      int diffY = lastTouchY - ty; // Drag up -> scroll down
      int totalDistX = abs(tx - touchStartX);
      int totalDistY = abs(ty - touchStartY);

      if (!isDragging && (totalDistY > 12 || totalDistX > 15)) {
        isDragging = true;
      }

      // Vertical display scroll when dragging anywhere on screen
      if (isDragging && abs(diffY) >= 4) {
        handleScreenScroll(diffY);
        lastTouchY = ty;
      }
      lastTouchX = tx;
    }
  } else {
    // Touch Released
    if (isTouching) {
      unsigned long duration = millis() - touchStartTime;
      int dx = lastTouchX - touchStartX;
      int dy = lastTouchY - touchStartY;

      // 1. Gesture or horizontal swipe check
      if (detectedGesture == 0x03 || dx < -35) {
        Serial.printf("[Gesture] Swipe Left (dx=%d)\\n", dx);
        handleScreenSwipe(1); // Left
      } else if (detectedGesture == 0x04 || dx > 35) {
        Serial.printf("[Gesture] Swipe Right (dx=%d)\\n", dx);
        handleScreenSwipe(2); // Right
      } else if (!isDragging && abs(dx) < 15 && abs(dy) < 15 && duration < 500) {
        // Clean single tap / button click
        Serial.printf("[Touch] Click at X=%d, Y=%d\\n", touchStartX, touchStartY);
        handleScreenTouch(touchStartX, touchStartY);
      }

      isTouching = false;
      isDragging = false;
      detectedGesture = 0;
    }
  }

  delay(12);
}
`
}

// ============================================================
// Compatibility Header (luna_gfx_compat.h)
// ============================================================
export function generateCompatHeader() {
  return `// ============================================================================
// GENERATED FILE — DO NOT EDIT MANUALLY.
// SOURCE: Luna UI Studio
// REGENERATE FROM STUDIO.
// ============================================================================
#pragma once
#include <Arduino.h>
#include "Arduino_GFX_Library.h"

// Text Datum constants matching TFT_eSPI
#define TL_DATUM 0
#define TC_DATUM 1
#define TR_DATUM 2
#define ML_DATUM 3
#define MC_DATUM 4
#define MR_DATUM 5
#define BL_DATUM 6
#define BC_DATUM 7
#define BR_DATUM 8

class LunaGFXWrapper {
public:
  Arduino_GFX *gfx = nullptr;
  uint8_t textDatum = TL_DATUM;
  uint16_t currentTextColor = 0xFFFF;
  uint16_t currentBgColor = 0x0000;
  bool hasBg = false;

  void attach(Arduino_GFX *g) { gfx = g; }

  void fillScreen(uint16_t color) {
    if (gfx) gfx->fillScreen(color);
  }

  void flush() {
    if (gfx) gfx->flush();
  }

  void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    if (gfx) gfx->fillRoundRect(x, y, w, h, r, color);
  }

  void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    if (gfx) gfx->drawRoundRect(x, y, w, h, r, color);
  }

  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (gfx) gfx->fillRect(x, y, w, h, color);
  }

  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (gfx) gfx->drawRect(x, y, w, h, color);
  }

  void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    if (gfx) gfx->fillCircle(x, y, r, color);
  }

  void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    if (gfx) gfx->drawCircle(x, y, r, color);
  }

  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    if (gfx) gfx->drawFastVLine(x, y, h, color);
  }

  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (gfx) gfx->drawFastHLine(x, y, w, color);
  }

  void drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (gfx) gfx->drawPixel(x, y, color);
  }

  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    if (gfx) gfx->drawLine(x0, y0, x1, y1, color);
  }

  void setTextDatum(uint8_t d) {
    textDatum = d;
  }

  void setTextColor(uint16_t c) {
    currentTextColor = c;
    hasBg = false;
    if (gfx) gfx->setTextColor(c);
  }

  void setTextColor(uint16_t c, uint16_t bg) {
    currentTextColor = c;
    currentBgColor = bg;
    hasBg = true;
    if (gfx) gfx->setTextColor(c, bg);
  }

  uint8_t getScale(uint8_t font) {
    if (font <= 1) return 1;
    if (font == 2) return 2;
    if (font == 4) return 3;
    if (font >= 6) return 4;
    return 1;
  }

  int16_t textWidth(const char* str, uint8_t font = 1) {
    if (!str) return 0;
    uint8_t s = getScale(font);
    return strlen(str) * 6 * s;
  }

  void drawString(const char* str, int16_t x, int16_t y, uint8_t font = 1) {
    if (!gfx || !str) return;
    uint8_t s = getScale(font);
    gfx->setTextSize(s);
    if (hasBg) {
      gfx->setTextColor(currentTextColor, currentBgColor);
    } else {
      gfx->setTextColor(currentTextColor);
    }

    int16_t tw = strlen(str) * 6 * s;
    int16_t th = 8 * s;
    int16_t cx = x;
    int16_t cy = y;

    switch (textDatum) {
      case TL_DATUM: break;
      case TC_DATUM: cx = x - tw / 2; break;
      case TR_DATUM: cx = x - tw; break;
      case ML_DATUM: cy = y - th / 2; break;
      case MC_DATUM: cx = x - tw / 2; cy = y - th / 2; break;
      case MR_DATUM: cx = x - tw; cy = y - th / 2; break;
      case BL_DATUM: cy = y - th; break;
      case BC_DATUM: cx = x - tw / 2; cy = y - th; break;
      case BR_DATUM: cx = x - tw; cy = y - th; break;
      default: break;
    }
    gfx->setCursor(cx, cy);
    gfx->print(str);
  }
};

// Aliased so Luna UI Studio exported code compiles transparently
typedef LunaGFXWrapper TFT_eSPI;
`
}

// ============================================================
// Custom 16MB Partition Table (partitions.csv)
// ============================================================
export function generatePartitionsCsv() {
  return `# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     0x9000,  0x5000,
otadata,  data, ota,     0xe000,  0x2000,
app0,     app,  ota_0,   0x10000, 0x640000,
app1,     app,  ota_1,   0x650000,0x640000,
spiffs,   data, spiffs,  0xc90000,0x360000,
coredump, data, coredump,0xFF0000,0x10000,
`
}

// ============================================================
// Flash Guide & CLI Command (README.md)
// ============================================================
export function generateFlashReadme() {
  return `# Waveshare ESP32-S3-Touch-LCD-1.69 Flash Guide

## 1. Required Arduino Library
In Arduino IDE Library Manager, install:
- **GFX Library for Arduino** (by Moon On Our Nation)

## 2. Arduino IDE Board Settings
- **Board**: \`ESP32S3 Dev Module\`
- **Flash Size**: \`16MB (128Mb)\` ⚠️ *(CRITICAL: Must select 16MB or bootloader panics!)*
- **Partition Scheme**: \`Custom\` (reads \`partitions.csv\` automatically)
- **PSRAM**: \`OPI PSRAM\`
- **USB CDC On Boot**: \`Enabled\`
- **Flash Mode**: \`QIO 80MHz\`

## 3. Fast 1-Line CLI Flash
\`\`\`bash
# Compile:
arduino-cli compile --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PartitionScheme=custom,PSRAM=opi .

# Upload to COM port (e.g. COM3):
arduino-cli upload -p COM3 --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PartitionScheme=custom,PSRAM=opi .
\`\`\`
`
}

export { hexToRGB565 }
