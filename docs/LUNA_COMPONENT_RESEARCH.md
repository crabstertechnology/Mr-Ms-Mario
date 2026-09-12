# Luna Component Research & Adaptation Map

## Executive Vision

This document establishes the architectural bridge between established modern UI component ecosystems (**shadcn/ui**, **Radix UI**, **Base UI**, **MUI**, **Mantine**, **Chakra UI**, **Aceternity UI**, **Magic UI**, **21st.dev**, **Origin UI**) and **Luna's original Kinesis / Monolith design language**.

### The Core Equation:
$$\text{Proven Interaction Patterns} + \text{Luna Monolith Design Language} = \text{Authoritative Wearable Interface}$$

### The Hard Hardware Constraints:
- **Target**: Waveshare ESP32-S3 Touch LCD 1.69" ($240 \times 280\text{px}$)
- **Input**: Single-point capacitive touch (CST816T, I2C 0x15)
- **Ergonomics**: Rapid one-thumb operation at arm's length
- **Memory & Rendering**: ST7789 display controller via 80MHz SPI with double-buffered PSRAM canvas. No DOM, no CSS engine, no GPU blurs.

---

## 1. Navigation Components

### 1.1 Tabs / Segmented Control
- **Source / Reference**: Radix UI `Tabs`, shadcn/ui `Tabs`, Base UI `Tabs`
- **What is Good**: Clear active-index modeling, accessible keyboard/focus semantics, aria attributes, sliding thumb indicator.
- **What Should Be Changed**: Desktop tabs use narrow text targets (24–28px high) with mouse hover states and thin underlines that are impossible to accurately target with a thumb.
- **Luna Version (`LunaNavigation` / `LunaSegmented`)**:
  - Full display width ($220\text{px}$) with max 2–3 segments.
  - Minimum height $42\text{px}$.
  - Tactile high-contrast background pill (`#121721`) with glowing amber/cyan selection marker.
  - Instant capacitive swipe navigation replaces clicking between tabs.

### 1.2 Breadcrumbs / Hierarchical Rails
- **Source / Reference**: shadcn/ui `Breadcrumb`, Ant Design `Breadcrumb`
- **What is Good**: Communicates deep spatial location clearly.
- **What Should Be Changed**: Tiny chevron dividers (`>`) and horizontal chains of links overflow completely on a 240px portrait screen and invite mis-taps.
- **Luna Version (`LunaHeader` / `LunaNavigationRail`)**:
  - Replaced by a 24px vertical edge rail or peripheral dot array indicating carousel position (e.g. 8 dots for the 8 canonical screens).
  - Screen title rendered prominently in 14pt bold tabular typography at top left ($X=10, Y=12$).

### 1.3 Back Navigation Trigger
- **Source / Reference**: iOS Navigation Bar, React Navigation Header
- **What is Good**: Consistent, muscle-memory location for returning up the stack.
- **What Should Be Changed**: Standard web back arrows are often $16 \times 16\text{px}$ icons nested inside tight padding.
- **Luna Version (`LunaHeader`)**:
  - Dedicated $44 \times 44\text{px}$ touch zone in top-left corner.
  - Bold chevron (`‹`) with tactile feedback state (`#1A2232`).
  - Edge-swipe right ($dx \ge 50\text{px}$) acts as universal back gesture.

### 1.4 Swipe Navigation & Gesture Carousel
- **Source / Reference**: Embla Carousel, Swiper.js, Aceternity UI Swipe Cards
- **What is Good**: Physics-based drag tracking, snap-to-slide inertia.
- **What Should Be Changed**: Web carousels rely on heavy CSS translate3d and touchmove event spam that can lock single-core microcontroller loops.
- **Luna Version (`LunaGestureSurface`)**:
  - CST816T raw hardware interrupt driven with $10\text{px}$ deadzone filter.
  - Strict classification gate: $|dx| \ge 50\text{px} \land |dx| > 1.5 \times |dy|$.
  - Debounced screen-switching state machine with instant double-buffer page flip.

---

## 2. Action Components

### 2.1 Buttons
- **Source / Reference**: shadcn/ui `Button`, Mantine `Button`, Base UI `Button`
- **What is Good**: Comprehensive state coverage (`default`, `secondary`, `destructive`, `outline`, `ghost`, `link`), clear loading spinners, disabled states.
- **What Should Be Changed**: Web buttons default to 32–36px height, rely heavily on `:hover` styles, subtle box-shadows, and micro-borders that vanish under outdoor sunlight.
- **Luna Version (`LunaButton`)**:
  - Full-width tactile slab ($220 \times 48\text{px}$) providing an enormous touch target.
  - Distinct high-contrast states:
    - `normal`: High-visibility Amber (`#FF9E3B`) or Deep Obsidian (`#121721`) with 1px border.
    - `pressed`: Instant inverse fill or cyan flash (`#38BDF8`).
    - `disabled`: Muted slate `#232D3F` with 40% contrast text `#8290A4`.
    - `loading`: Integrated 3-dot pulse indicator.
    - `success`: Emerald `#10B981` confirmation flash.
    - `destructive`: Crimson `#EF4444` tactile slab.

### 2.2 Icon Buttons
- **Source / Reference**: Radix UI `IconButton`, Origin UI `Button with Icon`
- **What is Good**: Space-efficient interaction for secondary triggers.
- **What Should Be Changed**: Often rendered as $24 \times 24\text{px}$ squares on web dashboards, leading to high touch error rates on wearables.
- **Luna Version (`LunaIconButton`)**:
  - Minimum bounding box $44 \times 44\text{px}$ (active visual icon $20\text{px}$ centered with $12\text{px}$ hit padding).
  - High-contrast geometric background with $10\text{px}$ radius.

### 2.3 Floating Action Button (FAB) / Contextual Triggers
- **Source / Reference**: Material UI `FAB`, Aceternity UI `Hover Border Gradient`
- **What is Good**: Prominent primary action floating above scrolling feeds.
- **What Should Be Changed**: Web FABs obscure lower list items and consume valuable viewport real estate on a 280px tall display.
- **Luna Version (`LunaFloatingAction`)**:
  - Integrated directly into the bottom viewport bar ($Y=215, H=50\text{px}$) with automatic list padding so the bottom-most item is never occluded.

### 2.4 Confirmation & Destructive Action Dialogs
- **Source / Reference**: Radix UI `AlertDialog`, shadcn/ui `AlertDialog`
- **What is Good**: Accessible focus lock, modal backdrop, distinct cancel vs confirm hierarchy.
- **What Should Be Changed**: Desktop modals use complex flexbox overlays with tiny "X" close buttons in the top-right corner.
- **Luna Version (`LunaDialog`)**:
  - Full-screen takeover surface ($240 \times 280\text{px}$) with 80% darkened obsidian backdrop.
  - Two massive vertical slabs: Primary Action ($48\text{px}$ high, top) and Cancel Action ($48\text{px}$ high, bottom). Zero ambiguous escape vectors.

---

## 3. Data Display Components

### 3.1 Cards / Monolith Surfaces
- **Source / Reference**: shadcn/ui `Card`, Origin UI `Card`, 21st.dev `Bento Grid`
- **What is Good**: Grouping related information with structured headers, content, and footers.
- **What Should Be Changed**: Web cards use multiple competing borders, light drop shadows, thin divider lines, and dense padding that wastes screen area.
- **Luna Version (`LunaSurface`)**:
  - Monolithic obsidian block (`#121721`, border `#232D3F`, radius $10\text{px}$).
  - Maximum glanceability: primary metric/status occupies the upper 60%, supporting secondary context in the lower 40%.
  - High contrast under outdoor reflection.

### 3.2 Lists & List Items
- **Source / Reference**: Mantine `List`, MUI `ListItem`
- **What is Good**: Clean tabular scanning, leading icons, trailing action disclosures.
- **What Should Be Changed**: Web list items are often 28–32px tall with fine text and hair-line dividers that blur during wrist movement.
- **Luna Version (`LunaList` & `LunaListItem`)**:
  - Minimum item height $52\text{px}$.
  - Leading status indicator strip (4px width) indicating state (unread, warning, active).
  - Clear trailing chevron (`›`) indicating navigation drill-down.
  - Integrated drag scroll with physical scrollbar thumb indicator on right display edge.

### 3.3 Status Indicators & Badges
- **Source / Reference**: Radix UI `Badge`, Base UI `StatusIndicator`, Magic UI `Shimmer Button`
- **What is Good**: Semantic color coding (success, warning, error, info).
- **What Should Be Changed**: Web badges use 10px fonts and 2px padding, becoming unreadable on a wearable display.
- **Luna Version (`LunaStatus` / `LunaIndicator`)**:
  - High-visibility pill ($H=22\text{px}$) or 6px solid circular beacon.
  - Tabular bold monospace font (`11pt`) with semantic colors: Cyan `#38BDF8`, Amber `#FF9E3B`, Emerald `#10B981`, Rose `#F43F5E`.

### 3.4 Progress Indicators (Linear & Ring)
- **Source / Reference**: Radix UI `Progress`, Chakra UI `CircularProgress`
- **What is Good**: Deterministic value mapping ($0.0 \to 1.0$), smooth transition interpolation.
- **What Should Be Changed**: Web circular progress relies on complex SVG `strokeDasharray` and CSS keyframes that lack hardware acceleration on microcontrollers.
- **Luna Version (`LunaProgress`)**:
  - Linear track: 6px thick high-contrast rail (`#1A2232`) with solid color fill.
  - Ring track: Optimized 8-way symmetry integer circle rasterizer on ESP32-S3 canvas.
  - Tabular percentage readout centered in font size 16pt bold.

### 3.5 Tabular Numeric Counters
- **Source / Reference**: Magic UI `NumberTicker`, Aceternity UI `Text Generate`
- **What is Good**: Dynamic rolling transitions draw attention to changed telemetry values.
- **What Should Be Changed**: JavaScript animation loops with proportional fonts cause layout shifting and CPU stutter.
- **Luna Version (`LunaNumber`)**:
  - Strictly monospaced tabular numerals prevent layout jitter during updates.
  - Single-blitted bounding box prevents full-screen re-renders on the MCU.

---

## 4. Input Components

### 4.1 Toggles & Switches
- **Source / Reference**: Radix UI `Switch`, shadcn/ui `Switch`
- **What is Good**: Instant binary state representation, smooth sliding thumb.
- **What Should Be Changed**: Web switches are tiny ($36 \times 20\text{px}$), requiring precise cursor clicks.
- **Luna Version (`LunaToggle`)**:
  - Massive wearable track: $50 \times 28\text{px}$ with a $22 \times 22\text{px}$ thumb.
  - When integrated into a setting row, the entire $220 \times 48\text{px}$ row is clickable to toggle the switch.
  - Unambiguous active fill (Cyan `#38BDF8` vs Inactive Muted `#232D3F`).

### 4.2 Sliders & Scrubber Controls
- **Source / Reference**: Radix UI `Slider`, Base UI `Slider`
- **What is Good**: Direct manipulation of continuous ranges, accessible step increments.
- **What Should Be Changed**: Web sliders have thin 2px tracks and small 12px thumbs that are occluded by a human thumb during dragging.
- **Luna Version (`LunaSlider`)**:
  - Track height: 10px with stepped tactile notches.
  - Thumb width: $28\times 28\text{px}$ with high-contrast amber indicator.
  - Live numeric bubble rendered *above* the thumb ($Y - 24\text{px}$) so the user's thumb never blocks the current value.

### 4.3 Pickers & Selectors
- **Source / Reference**: Radix UI `Select`, Mantine `SegmentedControl`
- **What is Good**: Constrained choice selection without freeform text input.
- **What Should Be Changed**: Web dropdowns spawn popovers that extend offscreen and require tiny scrollbars.
- **Luna Version (`LunaSelector`)**:
  - Full-width stepped carousel or stacked 44px option blocks with clear radio beacons.

---

## 5. Feedback Components

### 5.1 Toasts & Transient Alerts
- **Source / Reference**: Sonner, Radix UI `Toast`, shadcn/ui `Toast`
- **What is Good**: Non-blocking contextual feedback, auto-dismiss timers, swipe-to-dismiss.
- **What Should Be Changed**: Web toasts appear in corners (top-right / bottom-right), which are outside the central focal point on a small wearable.
- **Luna Version (`LunaToast`)**:
  - Centered floating pill ($200 \times 40\text{px}$) hovering in the upper third of the display ($Y=20$).
  - High-contrast emerald or cyan accent bar with 2-second auto-dismiss and instant tap-to-dismiss.

### 5.2 Notification Streams
- **Source / Reference**: iOS Notification Center, Material You Notifications
- **What is Good**: Grouping by application/source, expandable details, actionable buttons.
- **What Should Be Changed**: Cluttered multi-line metadata, tiny profile avatars, and complex swipe-action reveal menus that misfire on capacitive touch.
- **Luna Version (`LunaNotificationBlock`)**:
  - Monolithic card ($220 \times 72\text{px}$) with sender tag, timestamp, clean 2-line preview.
  - 4px vertical unread indicator strip.
  - Single-tap triggers full-screen view or dismiss action.

### 5.3 Loading & Skeleton States
- **Source / Reference**: shadcn/ui `Skeleton`, Mantine `Loader`
- **What is Good**: Reduces perceived latency, indicates layout stability before data arrives.
- **What Should Be Changed**: CSS shimmering gradients require continuous full-screen alpha blending, which saturates microcontroller SPI buses.
- **Luna Version (`LunaLoader`)**:
  - 3-dot harmonic pulse or high-contrast circular arc spinner rendered in solid 16-bit color. Zero SPI bus saturation.

### 5.4 Empty States
- **Source / Reference**: Ant Design `Empty`, Chakra UI `EmptyState`
- **What is Good**: Clear illustration, helpful explanation, call-to-action button.
- **What Should Be Changed**: Huge illustrations with tiny descriptive text that force scrolling on 280px screens.
- **Luna Version (`LunaEmptyState`)**:
  - Compact geometric symbol ($32\text{px}$), single-line authoritative text ("NO NOTIFICATIONS"), and primary return action.

---

## 6. Motion & Interaction States

### 6.1 State Matrix Standard
Every Luna component must deterministically implement and expose these 8 states:
1. **NORMAL**: High-contrast, clean resting hierarchy.
2. **PRESSED**: Immediate visual feedback ($\le 16\text{ms}$ response) via inverted luminance or border flash.
3. **DISABLED**: 40% contrast reduction, touch interaction rejected.
4. **ACTIVE / CHECKED**: Accent color engagement (Cyan `#38BDF8` or Amber `#FF9E3B`).
5. **SELECTED**: Distinct geometric focus border or highlight fill.
6. **LOADING**: Integrated pulse or spinner, input locked.
7. **SUCCESS**: Emerald confirmation flash (`#10B981`) with brief haptic trigger.
8. **ERROR**: Crimson indicator (`#EF4444`) with high-contrast error message.

### 6.2 Wearable Motion Translation
- **Directional Navigation**: Horizontal transitions communicate spatial carousel movement (slide left / slide right).
- **Vertical Feed Scroll**: Direct 1:1 finger tracking with bounded inertial stop and visible scroll thumb indicator.
- **No Gimmicks**: No physics bounces, no parallax shifts, no continuous background particle loops that waste MCU battery.

---

## 7. Adaptation Summary Matrix

| Component Primitive | External Reference | Key Borrowed Pattern | Wearable Adaptation (240x280) | Minimum Touch Target |
| :--- | :--- | :--- | :--- | :--- |
| `LunaButton` | shadcn/ui `Button` | 6 semantic states, loading slot | $220 \times 48\text{px}$ tactile monolithic slab | $220 \times 48\text{px}$ |
| `LunaIconButton` | Radix UI `IconButton` | Focus ring, accessible action | $44 \times 44\text{px}$ bounding box with 20px icon | $44 \times 44\text{px}$ |
| `LunaSurface` | Origin UI `Card` | Hierarchical zones, elevation | Monolith `#121721`, `#232D3F` border, solid fill | Full card area |
| `LunaList` / `Item` | Mantine `List` | Tabular row scanning | $52\text{px}$ item height with trailing action chevron | $220 \times 52\text{px}$ |
| `LunaToggle` | Radix UI `Switch` | Thumb displacement, binary role | $50 \times 28\text{px}$ track, whole row clickable | $220 \times 48\text{px}$ |
| `LunaSlider` | Base UI `Slider` | Continuous scrub, step notches | 10px track, thumb bubble placed *above* finger | $220 \times 40\text{px}$ |
| `LunaProgress` | Chakra UI `Progress` | Deterministic $0 \to 1$ mapping | Solid 6px rail, integer circle rasterizer | Non-interactive (Glance) |
| `LunaIndicator` | Base UI `Status` | Semantic status encoding | 6px solid beacon / $22\text{px}$ tabular badge | Non-interactive (Glance) |
| `LunaDialog` | Radix `AlertDialog` | Modal action isolation | Fullscreen $240 \times 280\text{px}$ takeover, dual slabs | $220 \times 48\text{px}$ each |
| `LunaToast` | Sonner `Toast` | Transient auto-dismiss pill | Centered $200 \times 40\text{px}$ upper pill, tap dismiss | $200 \times 40\text{px}$ |
| `LunaHeader` | iOS Navigation Bar | Persistent screen context & back | $44\text{px}$ touch zone, bold 14pt screen title | $44 \times 44\text{px}$ |
| `LunaNavigation` | Radix `Tabs` | Carousel rail & index beacons | 8-dot edge indicator or segmented pill | $220 \times 40\text{px}$ |
| `LunaTimer` | Custom / Stopwatch | Tabular figures, radial ring | Large 34pt bold numbers, active session ring | $220 \times 175\text{px}$ |
| `LunaNumber` | Magic UI `NumberTicker` | Dynamic telemetry update | Monospaced tabular blit, zero layout shift | Non-interactive (Glance) |
| `LunaStatus` | shadcn/ui `Badge` | High-contrast status pill | $22\text{px}$ height, 11pt bold uppercase | Non-interactive (Glance) |
| `LunaGestureSurface`| Swiper / CST816T | Deadzone, debounce, classify | Hardware interrupt filter, $|dx|\ge 50\land |dx|>1.5|dy|$ | Full Screen |
