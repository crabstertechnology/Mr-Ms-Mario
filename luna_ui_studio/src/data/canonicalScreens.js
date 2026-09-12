// ============================================================
// Luna UI Studio — Canonical Screens Specification
// Visual Language: Kinesis / Monolith Design System
// Target: Waveshare ESP32-S3 Touch LCD 1.69" (240x280)
// ============================================================

export const LUNA_CANONICAL_SCREENS = [
  {
    id: 'screen_home',
    name: 'Home (Glance)',
    bgColor: '#080A0F',
    bgType: 'color',
    bgGradient: 'none',
    bgPattern: 'none',
    isScrollable: false,
    maxScrollY: 280,
    gestures: {
      swipeLeft: { actionType: 'navigate', targetScreenId: 'screen_notifications', transition: 'slide-left' },
      swipeRight: { actionType: 'navigate', targetScreenId: 'screen_about', transition: 'slide-right' },
    },
    elements: [
      {
        id: 'el_home_time',
        name: 'Monolith Time Hero',
        type: 'monolith_time',
        props: {
          x: 10, y: 15, w: 220, h: 90,
          timeStr: '10:42', secondsStr: ':38', dateStr: 'WED 09 SEP',
          batteryPct: 94, statusText: 'LUNA • READY',
          bgColor: '#121721', borderColor: '#232D3F',
          textColor: '#EAEFF5', accentColor: '#38BDF8', radius: 10
        },
        actions: []
      },
      {
        id: 'el_home_glance',
        name: 'Context Glance Bar',
        type: 'glance_bar',
        props: {
          x: 10, y: 112, w: 220, h: 42,
          icon: '⚡', label: 'NEXT FOCUS', detail: 'Sprint Review @ 11:00',
          bgColor: '#121721', borderColor: '#232D3F',
          textColor: '#EAEFF5', accentColor: '#FF9E3B', radius: 8
        },
        actions: [
          {
            id: 'act_glance_to_cal',
            trigger: 'onClick',
            actionType: 'navigate',
            targetScreenId: 'screen_calendar',
            transition: 'slide-left',
            alertMessage: 'Opening Calendar...'
          }
        ]
      },
      {
        id: 'el_home_action',
        name: 'Tactile Action Button',
        type: 'tactile_button',
        props: {
          x: 10, y: 205, w: 220, h: 50,
          label: 'START FOCUS SESSION', icon: '▶',
          bgColor: '#FF9E3B', borderColor: '#FFB266',
          textColor: '#080A0F', accentColor: '#080A0F', radius: 12
        },
        actions: [
          {
            id: 'act_home_start_focus',
            trigger: 'onClick',
            actionType: 'navigate',
            targetScreenId: 'screen_focus',
            transition: 'slide-left',
            alertMessage: 'Launching Focus Chamber...'
          }
        ]
      }
    ]
  },
  {
    id: 'screen_notifications',
    name: 'Notifications',
    bgColor: '#080A0F',
    bgType: 'color',
    bgGradient: 'none',
    bgPattern: 'none',
    isScrollable: true,
    maxScrollY: 400,
    gestures: {
      swipeLeft: { actionType: 'navigate', targetScreenId: 'screen_calendar', transition: 'slide-left' },
      swipeRight: { actionType: 'navigate', targetScreenId: 'screen_home', transition: 'slide-right' },
    },
    elements: [
      {
        id: 'el_notif_1',
        name: 'Sensor Ready Notice',
        type: 'notification_block',
        props: {
          x: 10, y: 15, w: 220, h: 72,
          sender: 'SARAH CONNOR', timeStr: '4m ago',
          preview: 'Firmware calibration complete. Sensor ready.',
          unread: true,
          bgColor: '#121721', borderColor: '#232D3F',
          textColor: '#EAEFF5', subtextColor: '#8290A4',
          accentColor: '#38BDF8', radius: 10
        },
        actions: [
          {
            id: 'act_dismiss_notif1',
            trigger: 'onClick',
            actionType: 'alert',
            alertMessage: 'Notification 1 dismissed.'
          }
        ]
      },
      {
        id: 'el_notif_2',
        name: 'BLE Heartbeat Notice',
        type: 'notification_block',
        props: {
          x: 10, y: 95, w: 220, h: 72,
          sender: 'LUNA BLE MESH', timeStr: '18m ago',
          preview: 'Connected to iPhone 15 Pro • RSSI -58dBm.',
          unread: false,
          bgColor: '#121721', borderColor: '#232D3F',
          textColor: '#EAEFF5', subtextColor: '#8290A4',
          accentColor: '#10B981', radius: 10
        },
        actions: []
      },
      {
        id: 'el_notif_3',
        name: 'Battery System Notice',
        type: 'notification_block',
        props: {
          x: 10, y: 175, w: 220, h: 72,
          sender: 'POWER MANAGER', timeStr: '1h ago',
          preview: 'Battery charged to 94% • Est. 18 hrs remaining.',
          unread: false,
          bgColor: '#121721', borderColor: '#232D3F',
          textColor: '#EAEFF5', subtextColor: '#8290A4',
          accentColor: '#FF9E3B', radius: 10
        },
        actions: []
      },
      {
        id: 'el_notif_clear',
        name: 'Clear All Button',
        type: 'tactile_button',
        props: {
          x: 10, y: 255, w: 220, h: 44,
          label: 'CLEAR ALL NOTIFICATIONS', icon: '✕',
          bgColor: '#1E293B', borderColor: '#334155',
          textColor: '#EAEFF5', accentColor: '#EAEFF5', radius: 10
        },
        actions: [
          {
            id: 'act_clear_all_notifs',
            trigger: 'onClick',
            actionType: 'alert',
            alertMessage: 'All stream notifications cleared.'
          }
        ]
      }
    ]
  },
  {
    id: 'screen_calendar',
    name: 'Calendar',
    bgColor: '#080A0F',
    bgType: 'color',
    bgGradient: 'none',
    bgPattern: 'none',
    isScrollable: true,
    maxScrollY: 380,
    gestures: {
      swipeLeft: { actionType: 'navigate', targetScreenId: 'screen_games', transition: 'slide-left' },
      swipeRight: { actionType: 'navigate', targetScreenId: 'screen_notifications', transition: 'slide-right' },
    },
    elements: [
      {
        id: 'el_cal_1',
        name: 'Next Agenda Item',
        type: 'agenda_block',
        props: {
          x: 10, y: 15, w: 220, h: 84,
          countdown: 'IN 24m', timeRange: '10:30 - 11:15',
          title: 'Architecture Sync', location: 'Lab 4 / BLE Orbit',
          bgColor: '#121721', borderColor: '#232D3F',
          textColor: '#EAEFF5', subtextColor: '#8290A4',
          accentColor: '#38BDF8', radius: 10
        },
        actions: []
      },
      {
        id: 'el_cal_2',
        name: 'Afternoon Item',
        type: 'agenda_block',
        props: {
          x: 10, y: 106, w: 220, h: 84,
          countdown: 'AT 14:00', timeRange: '14:00 - 15:30',
          title: 'Hardware Touch Testing', location: 'Bench COM3 (ESP32-S3)',
          bgColor: '#121721', borderColor: '#232D3F',
          textColor: '#EAEFF5', subtextColor: '#8290A4',
          accentColor: '#10B981', radius: 10
        },
        actions: []
      },
      {
        id: 'el_cal_3',
        name: 'Evening Review',
        type: 'agenda_block',
        props: {
          x: 10, y: 198, w: 220, h: 84,
          countdown: 'AT 17:30', timeRange: '17:30 - 18:00',
          title: 'Sprint Review & Demo', location: 'Luna Core Studio',
          bgColor: '#121721', borderColor: '#232D3F',
          textColor: '#EAEFF5', subtextColor: '#8290A4',
          accentColor: '#FF9E3B', radius: 10
        },
        actions: []
      }
    ]
  },
  {
    id: 'screen_games',
    name: 'Games Launcher',
    bgColor: '#080A0F',
    bgType: 'color',
    bgGradient: 'none',
    bgPattern: 'none',
    isScrollable: false,
    maxScrollY: 280,
    gestures: {
      swipeLeft: { actionType: 'navigate', targetScreenId: 'screen_focus', transition: 'slide-left' },
      swipeRight: { actionType: 'navigate', targetScreenId: 'screen_calendar', transition: 'slide-right' },
    },
    elements: [
      {
        id: 'el_game_hero',
        name: 'Retro Runner Hero',
        type: 'game_launcher',
        props: {
          x: 10, y: 15, w: 220, h: 170,
          title: 'RETRO RUNNER', genre: 'CYBERPLATFORM • 60FPS',
          highScore: '12,480 PTS', icon: '🏃', badgeText: 'CHALLENGE ACTIVE',
          bgColor: '#121721', borderColor: '#232D3F',
          textColor: '#EAEFF5', accentColor: '#FF9E3B', radius: 12
        },
        actions: [
          {
            id: 'act_play_runner',
            trigger: 'onClick',
            actionType: 'alert',
            alertMessage: '🎮 Launching Retro Runner on hardware!'
          }
        ]
      },
      {
        id: 'el_game_launch_btn',
        name: 'Launch Game Action',
        type: 'tactile_button',
        props: {
          x: 10, y: 195, w: 220, h: 50,
          label: 'LAUNCH GAME ▶', icon: '',
          bgColor: '#FF9E3B', borderColor: '#FFB266',
          textColor: '#080A0F', accentColor: '#080A0F', radius: 12
        },
        actions: [
          {
            id: 'act_game_launch',
            trigger: 'onClick',
            actionType: 'alert',
            alertMessage: '🎮 Launching Retro Runner!'
          }
        ]
      }
    ]
  },
  {
    id: 'screen_focus',
    name: 'Focus / Pomodoro',
    bgColor: '#080A0F',
    bgType: 'color',
    bgGradient: 'none',
    bgPattern: 'none',
    isScrollable: false,
    maxScrollY: 280,
    gestures: {
      swipeLeft: { actionType: 'navigate', targetScreenId: 'screen_cards', transition: 'slide-left' },
      swipeRight: { actionType: 'navigate', targetScreenId: 'screen_games', transition: 'slide-right' },
    },
    elements: [
      {
        id: 'el_focus_chamber',
        name: 'Focus Ambient Chamber',
        type: 'focus_chamber',
        props: {
          x: 10, y: 15, w: 220, h: 175,
          timeRemaining: '24:50', modeLabel: 'DEEP WORK', sessionTag: 'SESSION 2 / 4',
          progress: 75,
          bgColor: '#121721', borderColor: '#232D3F',
          textColor: '#EAEFF5', subtextColor: '#8290A4',
          accentColor: '#FF9E3B', radius: 12
        },
        actions: [
          {
            id: 'act_focus_pause',
            trigger: 'onClick',
            actionType: 'alert',
            alertMessage: '⏸ Focus timer paused.'
          }
        ]
      },
      {
        id: 'el_focus_pause_btn',
        name: 'Focus Action Button',
        type: 'tactile_button',
        props: {
          x: 10, y: 202, w: 220, h: 48,
          label: 'PAUSE FOCUS SESSION', icon: '⏸',
          bgColor: '#1E293B', borderColor: '#334155',
          textColor: '#EAEFF5', accentColor: '#EAEFF5', radius: 12
        },
        actions: [
          {
            id: 'act_pause_btn_click',
            trigger: 'onClick',
            actionType: 'alert',
            alertMessage: 'Focus session paused.'
          }
        ]
      }
    ]
  },
  {
    id: 'screen_cards',
    name: 'Cards & Utility',
    bgColor: '#080A0F',
    bgType: 'color',
    bgGradient: 'none',
    bgPattern: 'none',
    isScrollable: true,
    maxScrollY: 340,
    gestures: {
      swipeLeft: { actionType: 'navigate', targetScreenId: 'screen_settings', transition: 'slide-left' },
      swipeRight: { actionType: 'navigate', targetScreenId: 'screen_focus', transition: 'slide-right' },
    },
    elements: [
      {
        id: 'el_qr_card',
        name: 'Luna ID QR Card',
        type: 'qr_utility_card',
        props: {
          x: 10, y: 15, w: 220, h: 155,
          title: 'LUNA ID CARD', subtitle: 'ESP32-S3 • BLE PEER', idTag: 'UID: LN-8842-X',
          bgColor: '#121721', borderColor: '#232D3F',
          textColor: '#EAEFF5', accentColor: '#38BDF8', radius: 12
        },
        actions: []
      },
      {
        id: 'el_imu_level',
        name: 'IMU Spirit Level',
        type: 'imu_level_card',
        props: {
          x: 10, y: 178, w: 220, h: 140,
          title: 'IMU SPIRIT LEVEL', pitch: '+2.4°', roll: '-0.8°', status: 'STABLE',
          bgColor: '#121721', borderColor: '#232D3F',
          textColor: '#EAEFF5', subtextColor: '#8290A4',
          accentColor: '#10B981', radius: 12
        },
        actions: []
      }
    ]
  },
  {
    id: 'screen_settings',
    name: 'Settings',
    bgColor: '#080A0F',
    bgType: 'color',
    bgGradient: 'none',
    bgPattern: 'none',
    isScrollable: true,
    maxScrollY: 360,
    gestures: {
      swipeLeft: { actionType: 'navigate', targetScreenId: 'screen_about', transition: 'slide-left' },
      swipeRight: { actionType: 'navigate', targetScreenId: 'screen_cards', transition: 'slide-right' },
    },
    elements: [
      {
        id: 'el_set_bright',
        name: 'Brightness Setting',
        type: 'setting_row',
        props: {
          x: 10, y: 15, w: 220, h: 48,
          label: 'BRIGHTNESS', valueStr: '85%', sublabel: 'Auto-dim in 30s', checked: true,
          bgColor: '#121721', borderColor: '#232D3F',
          textColor: '#EAEFF5', subtextColor: '#8290A4',
          accentColor: '#38BDF8', radius: 8
        },
        actions: [
          {
            id: 'act_toggle_bright',
            trigger: 'onClick',
            actionType: 'toggle',
            alertMessage: 'Toggled brightness'
          }
        ]
      },
      {
        id: 'el_set_ble',
        name: 'Bluetooth Setting',
        type: 'setting_row',
        props: {
          x: 10, y: 70, w: 220, h: 48,
          label: 'BLUETOOTH LE', valueStr: 'ACTIVE', sublabel: 'Advertising as Luna_Core', checked: true,
          bgColor: '#121721', borderColor: '#232D3F',
          textColor: '#EAEFF5', subtextColor: '#8290A4',
          accentColor: '#10B981', radius: 8
        },
        actions: [
          {
            id: 'act_toggle_ble',
            trigger: 'onClick',
            actionType: 'toggle',
            alertMessage: 'Toggled BLE'
          }
        ]
      },
      {
        id: 'el_set_haptic',
        name: 'Haptic Audio Setting',
        type: 'setting_row',
        props: {
          x: 10, y: 125, w: 220, h: 48,
          label: 'HAPTIC & AUDIO', valueStr: 'CHIRP', sublabel: 'Buzzer pin GPIO 42', checked: true,
          bgColor: '#121721', borderColor: '#232D3F',
          textColor: '#EAEFF5', subtextColor: '#8290A4',
          accentColor: '#FF9E3B', radius: 8
        },
        actions: [
          {
            id: 'act_toggle_audio',
            trigger: 'onClick',
            actionType: 'toggle',
            alertMessage: 'Toggled audio'
          }
        ]
      },
      {
        id: 'el_set_diag_btn',
        name: 'Device Diagnostics Button',
        type: 'tactile_button',
        props: {
          x: 10, y: 185, w: 220, h: 48,
          label: 'DEVICE DIAGNOSTICS ›', icon: '🛠',
          bgColor: '#1E293B', borderColor: '#334155',
          textColor: '#EAEFF5', accentColor: '#EAEFF5', radius: 10
        },
        actions: [
          {
            id: 'act_open_about',
            trigger: 'onClick',
            actionType: 'navigate',
            targetScreenId: 'screen_about',
            transition: 'slide-left',
            alertMessage: 'Opening Device Specs...'
          }
        ]
      }
    ]
  },
  {
    id: 'screen_about',
    name: 'About & Device',
    bgColor: '#080A0F',
    bgType: 'color',
    bgGradient: 'none',
    bgPattern: 'none',
    isScrollable: true,
    maxScrollY: 340,
    gestures: {
      swipeLeft: { actionType: 'navigate', targetScreenId: 'screen_home', transition: 'slide-left' },
      swipeRight: { actionType: 'navigate', targetScreenId: 'screen_settings', transition: 'slide-right' },
    },
    elements: [
      {
        id: 'el_device_spec',
        name: 'Hardware Specs Card',
        type: 'device_spec_card',
        props: {
          x: 10, y: 15, w: 220, h: 160,
          deviceName: 'LUNA CORE 1.69', soc: 'ESP32-S3 Dual 240MHz',
          memory: '16MB Flash - 8MB PSRAM', batteryInfo: 'VBAT 4.12V (94%)',
          status: 'CST816T OK - ST7789 80MHz',
          bgColor: '#121721', borderColor: '#232D3F',
          textColor: '#EAEFF5', subtextColor: '#8290A4',
          accentColor: '#10B981', radius: 12
        },
        actions: []
      },
      {
        id: 'el_about_lab_btn',
        name: 'Open Component Lab Action',
        type: 'tactile_button',
        props: {
          x: 10, y: 185, w: 220, h: 48,
          label: 'OPEN COMPONENT LAB ›', icon: '🧪',
          bgColor: '#1E293B', borderColor: '#334155',
          textColor: '#EAEFF5', accentColor: '#38BDF8', radius: 12
        },
        actions: [
          {
            id: 'act_open_lab',
            trigger: 'onClick',
            actionType: 'navigate',
            targetScreenId: 'screen_component_lab',
            transition: 'slide-left',
            alertMessage: 'Opening Component Lab...'
          }
        ]
      },
      {
        id: 'el_about_return_btn',
        name: 'Return Home Action',
        type: 'tactile_button',
        props: {
          x: 10, y: 242, w: 220, h: 48,
          label: 'RETURN TO HOME', icon: '⌂',
          bgColor: '#38BDF8', borderColor: '#7DD3FC',
          textColor: '#080A0F', accentColor: '#080A0F', radius: 12
        },
        actions: [
          {
            id: 'act_return_to_home',
            trigger: 'onClick',
            actionType: 'navigate',
            targetScreenId: 'screen_home',
            transition: 'slide-right',
            alertMessage: 'Returning to Home Screen...'
          }
        ]
      }
    ]
  },
  {
    id: 'screen_component_lab',
    name: 'Component Lab',
    bgColor: '#080A0F',
    bgType: 'color',
    bgGradient: 'none',
    bgPattern: 'none',
    isScrollable: true,
    maxScrollY: 760,
    gestures: {
      swipeLeft: { actionType: 'navigate', targetScreenId: 'screen_home', transition: 'slide-left' },
      swipeRight: { actionType: 'navigate', targetScreenId: 'screen_about', transition: 'slide-right' },
    },
    elements: [
      {
        id: 'el_lab_header',
        name: 'Lab Header',
        type: 'luna_header',
        props: {
          x: 0, y: 0, w: 240, h: 44,
          title: 'COMPONENT LAB'
        },
        actions: []
      },
      {
        id: 'el_lab_btn_primary',
        name: 'Primary Button',
        type: 'luna_button',
        props: {
          x: 10, y: 52, w: 220, h: 48,
          label: 'PRIMARY BUTTON', variant: 'primary', state: 'normal'
        },
        actions: [
          {
            id: 'act_lab_btn_prim',
            trigger: 'onClick',
            actionType: 'alert',
            alertMessage: 'Primary button tapped'
          }
        ]
      },
      {
        id: 'el_lab_btn_secondary',
        name: 'Secondary Button',
        type: 'luna_button',
        props: {
          x: 10, y: 108, w: 220, h: 48,
          label: 'SECONDARY OUTLINE', variant: 'secondary', state: 'normal'
        },
        actions: [
          {
            id: 'act_lab_btn_sec',
            trigger: 'onClick',
            actionType: 'alert',
            alertMessage: 'Secondary button tapped'
          }
        ]
      },
      {
        id: 'el_lab_btn_destruct',
        name: 'Destructive Button',
        type: 'luna_button',
        props: {
          x: 10, y: 164, w: 220, h: 48,
          label: 'DESTRUCTIVE SLAB', variant: 'destructive', state: 'normal'
        },
        actions: [
          {
            id: 'act_lab_btn_destruct',
            trigger: 'onClick',
            actionType: 'alert',
            alertMessage: 'Destructive action tapped'
          }
        ]
      },
      {
        id: 'el_lab_toggle',
        name: 'Binary Toggle',
        type: 'luna_toggle',
        props: {
          x: 10, y: 220, w: 220, h: 48,
          label: 'TOUCH VIBRATION', checked: true,
          activeColor: '#38BDF8', inactiveColor: '#232D3F'
        },
        actions: [
          {
            id: 'act_lab_toggle',
            trigger: 'onClick',
            actionType: 'toggle',
            alertMessage: 'Toggle changed'
          }
        ]
      },
      {
        id: 'el_lab_slider',
        name: 'Continuous Slider',
        type: 'luna_slider',
        props: {
          x: 10, y: 276, w: 220, h: 64,
          label: 'DISPLAY BRIGHTNESS', value: 75, min: 0, max: 100,
          accentColor: '#FF9E3B', unit: '%'
        },
        actions: []
      },
      {
        id: 'el_lab_progress',
        name: 'Progress Gauge',
        type: 'luna_progress',
        props: {
          x: 10, y: 348, w: 220, h: 48,
          type: 'linear', value: 68, strokeWidth: 6,
          accentColor: '#38BDF8', trackColor: '#1A2232'
        },
        actions: []
      },
      {
        id: 'el_lab_status',
        name: 'Status Badge',
        type: 'luna_status',
        props: {
          x: 10, y: 404, w: 90, h: 26,
          status: 'online', label: 'ONLINE'
        },
        actions: []
      },
      {
        id: 'el_lab_indicator',
        name: 'Status Indicator',
        type: 'luna_indicator',
        props: {
          x: 110, y: 404, w: 100, h: 30,
          status: 'warning', label: 'SYNCING'
        },
        actions: []
      },
      {
        id: 'el_lab_number',
        name: 'Tabular Telemetry Number',
        type: 'luna_number',
        props: {
          x: 10, y: 442, w: 100, h: 44,
          value: '240', unit: 'MHz', label: 'CPU FREQ',
          accentColor: '#38BDF8'
        },
        actions: []
      },
      {
        id: 'el_lab_navigation',
        name: 'Carousel Navigation Dots',
        type: 'luna_navigation',
        props: {
          x: 10, y: 494, w: 220, h: 36,
          total: 8, activeIndex: 7
        },
        actions: []
      },
      {
        id: 'el_lab_surface',
        name: 'Monolith Surface Container',
        type: 'luna_surface',
        props: {
          x: 10, y: 538, w: 220, h: 74,
          title: 'TACTILE MONOLITH', subtitle: 'Elevation level 1 container'
        },
        actions: []
      },
      {
        id: 'el_lab_return_btn',
        name: 'Return Home Action',
        type: 'luna_button',
        props: {
          x: 10, y: 622, w: 220, h: 48,
          label: 'RETURN TO HOME', variant: 'primary'
        },
        actions: [
          {
            id: 'act_lab_return_home',
            trigger: 'onClick',
            actionType: 'navigate',
            targetScreenId: 'screen_home',
            transition: 'slide-right',
            alertMessage: 'Returning Home...'
          }
        ]
      }
    ]
  }
]
