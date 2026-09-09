export type Screen = 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10;
export type ClockStyle = 0 | 1 | 2 | 3;
export type OverlayType = "notification" | "partner" | "voip" | null;
export type CalendarView = "timeline" | "month";
export type ArcadeMode = "menu" | "playing" | "gameover";
export type PomodoroMode = "focus" | "shortBreak" | "longBreak";
export type PomodoroStatus = "idle" | "running" | "paused" | "complete";
export type AppIcon = "WA" | "SLACK" | "MAIL" | "PHONE" | "CAL";

export interface LunaNotification {
  id: number;
  app: AppIcon;
  sender: string;
  channel?: string;
  time: string;
  preview: string;
  full: string;
  isRead: boolean;
}

export interface CalendarEvent {
  id: number;
  time: string;
  hour: number;
  title: string;
  duration: string;
  type: "video" | "in-person" | "personal" | "birthday";
  completed: boolean;
  location?: string;
  isCurrent?: boolean;
}

export interface NavEvent {
  direction: "straight" | "left" | "right" | "hard-left" | "hard-right" | "u-turn" | "roundabout" | "destination";
  distance: string;
  street: string;
  eta: string;
  remaining: string;
  totalDistance: string;
}

export interface LunaSettings {
  bluetooth: boolean;
  brightness: "LOW" | "MED" | "HIGH";
  invertColors: boolean;
  clockStyle: number;
  gifSpeed: number;
  dnd: boolean;
  audioLoopback: boolean;
}

export interface GameHighScores {
  lunaRacer: number;
  lunaSpace: number;
  flappyMochy: number;
  coinCatcher: number;
  mochyJump: number;
  stacker: number;
  memoryMatrix: number;
  tiltMaze: number;
}

export interface GameInfo {
  id: number;
  key: keyof GameHighScores;
  name: string;
  desc: string;
  icon: string;
  color: string;
}

export const GAMES: GameInfo[] = [
  { id: 0, key: "lunaRacer", name: "Luna Racer", desc: "Dodge traffic at speed", icon: "🏎", color: "#F59E0B" },
  { id: 1, key: "lunaSpace", name: "Luna Space", desc: "Blast through asteroids", icon: "🚀", color: "#8B5CF6" },
  { id: 2, key: "flappyMochy", name: "Flappy Mochy", desc: "Flap through the gap", icon: "🐾", color: "#10B981" },
  { id: 3, key: "coinCatcher", name: "Coin Catcher", desc: "Catch the falling coins", icon: "🪙", color: "#F59E0B" },
  { id: 4, key: "mochyJump", name: "Mochy Jump", desc: "Jump over obstacles", icon: "🦘", color: "#38BDF8" },
  { id: 5, key: "stacker", name: "Stacker", desc: "Stack blocks perfectly", icon: "🧱", color: "#EF4444" },
  { id: 6, key: "memoryMatrix", name: "Memory Matrix", desc: "Match all the pairs", icon: "🧠", color: "#EC4899" },
  { id: 7, key: "tiltMaze", name: "Tilt Maze", desc: "Navigate to the exit", icon: "🌀", color: "#06B6D4" },
];

export interface LunaState {
  screen: Screen;
  prevScreen: Screen;
  transitionDirection: "left" | "right";
  isTransitioning: boolean;

  // Clock
  clockStyle: ClockStyle;
  currentTime: Date;

  // Notifications
  notifView: "list" | "detail";
  selectedNotifId: number | null;
  notifications: LunaNotification[];

  // Calendar
  calView: CalendarView;
  calEvents: CalendarEvent[];

  // Arcade
  arcadeMode: ArcadeMode;
  selectedGame: number;
  activeGame: number | null;
  highScores: GameHighScores;

  // Pomodoro
  pomStatus: PomodoroStatus;
  pomMode: PomodoroMode;
  pomCount: number;
  pomTimeLeft: number;

  // Navigation
  navEvent: NavEvent;

  // Spirit Level
  pitch: number;
  roll: number;
  rawPitch: number;
  rawRoll: number;
  isLeveled: boolean;
  calibrating: boolean;
  calibStep: number;
  offsetPitch: number;
  offsetRoll: number;

  // Settings
  settingsScrollIndex: number;
  editingSettingIdx: number | null;
  settings: LunaSettings;
  showResetConfirm: boolean;
  resetConfirmStep: number;

  // System
  batteryLevel: number;
  bleConnected: boolean;
  partnerOnline: boolean;
  steps: number;
  temperature: number;

  // Overlays
  overlay: OverlayType;
  overlayNotif: LunaNotification | null;
  voipDuration: number;
  voipMuted: boolean;
  partnerReturn: Screen;
}

export type LunaAction =
  | { type: "SWIPE_LEFT" }
  | { type: "SWIPE_RIGHT" }
  | { type: "TRANSITION_DONE" }
  | { type: "CYCLE_CLOCK_STYLE" }
  | { type: "TICK_TIME" }
  | { type: "OPEN_NOTIF"; id: number }
  | { type: "CLOSE_NOTIF" }
  | { type: "TOGGLE_CAL_VIEW" }
  | { type: "SELECT_GAME"; idx: number }
  | { type: "LAUNCH_GAME" }
  | { type: "EXIT_GAME"; score?: number }
  | { type: "SAVE_HIGH_SCORE"; gameKey: keyof GameHighScores; score: number }
  | { type: "POM_PLAY_PAUSE" }
  | { type: "POM_SKIP" }
  | { type: "POM_RESET" }
  | { type: "POM_TICK" }
  | { type: "POM_PHASE_COMPLETE" }
  | { type: "UPDATE_LEVEL"; pitch: number; roll: number }
  | { type: "CALIBRATE_START" }
  | { type: "CALIBRATE_TICK" }
  | { type: "CALIBRATE_DONE" }
  | { type: "SETTINGS_SCROLL"; delta: number }
  | { type: "EDIT_SETTING"; idx: number }
  | { type: "CHANGE_SETTING"; key: keyof LunaSettings; value: unknown }
  | { type: "SAVE_SETTING" }
  | { type: "SHOW_RESET_CONFIRM" }
  | { type: "CANCEL_RESET" }
  | { type: "EXECUTE_RESET" }
  | { type: "SHOW_OVERLAY"; overlay: OverlayType; notif?: LunaNotification }
  | { type: "DISMISS_OVERLAY" }
  | { type: "VOIP_TICK" }
  | { type: "VOIP_MUTE_TOGGLE" }
  | { type: "NAVIGATE_TO"; screen: Screen };
