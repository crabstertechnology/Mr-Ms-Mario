import { useReducer, useEffect, useRef, useCallback } from "react";
import type {
  LunaState,
  LunaAction,
  Screen,
  PomodoroMode,
  LunaNotification,
  CalendarEvent,
} from "./types";

const SCREENS: Screen[] = [3, 4, 5, 6, 7, 8, 9, 10];
const POM_DURATIONS: Record<string, number> = {
  focus: 25 * 60,
  shortBreak: 5 * 60,
  longBreak: 15 * 60,
};

const SAMPLE_NOTIFICATIONS: LunaNotification[] = [
  {
    id: 1,
    app: "WA",
    sender: "Sarah Jenkins",
    time: "10:38 AM",
    preview: "Are we still on for lunch today?",
    full: "Are we still on for lunch today? I was thinking Noodle House on Kings Ave, around 1pm if that works for you!",
    isRead: false,
  },
  {
    id: 2,
    app: "SLACK",
    sender: "#engineering",
    channel: "#engineering",
    time: "10:25 AM",
    preview: "PR #42 merged to main",
    full: "jordan: PR #42 merged to main ✅ — Auth refactor is live. Deploying to staging now. Nice work everyone.",
    isRead: false,
  },
  {
    id: 3,
    app: "MAIL",
    sender: "Stripe Support",
    time: "09:50 AM",
    preview: "Your payout of $2,840.00 has processed",
    full: "Your payout of $2,840.00 has been sent to your bank account ending in 4242. Funds typically arrive within 2 business days.",
    isRead: false,
  },
  {
    id: 4,
    app: "CAL",
    sender: "Google Calendar",
    time: "09:00 AM",
    preview: "Design Sync starts in 30 minutes",
    full: "Reminder: Design Sync starts in 30 minutes. Meeting link: meet.google.com/xyz-abc. Attendees: You, Alex, Maya, Jordan.",
    isRead: true,
  },
  {
    id: 5,
    app: "PHONE",
    sender: "Mom",
    time: "08:45 AM",
    preview: "Missed call",
    full: "You missed a call from Mom at 8:45 AM. Call back when you can!",
    isRead: true,
  },
];

const SAMPLE_EVENTS: CalendarEvent[] = [
  { id: 1, time: "09:00", hour: 9, title: "Morning Standup", duration: "15 min", type: "video", completed: true },
  { id: 2, time: "11:30", hour: 11, title: "Founder Sync", duration: "45 min", type: "video", completed: false, location: "Google Meet", isCurrent: true },
  { id: 3, time: "14:00", hour: 14, title: "Sprint Review", duration: "60 min", type: "in-person", completed: false, location: "Conf Room B" },
  { id: 4, time: "16:30", hour: 16, title: "Design Review", duration: "30 min", type: "video", completed: false, location: "Figma" },
  { id: 5, time: "18:30", hour: 18, title: "Leo's Birthday 🎂", duration: "All day", type: "birthday", completed: false },
];

const initialState: LunaState = {
  screen: 3,
  prevScreen: 3,
  transitionDirection: "left",
  isTransitioning: false,
  clockStyle: 0,
  currentTime: new Date(),
  notifView: "list",
  selectedNotifId: null,
  notifications: SAMPLE_NOTIFICATIONS,
  calView: "timeline",
  calEvents: SAMPLE_EVENTS,
  arcadeMode: "menu",
  selectedGame: 0,
  activeGame: null,
  highScores: {
    lunaRacer: 1840,
    lunaSpace: 4920,
    flappyMochy: 48,
    coinCatcher: 95,
    mochyJump: 312,
    stacker: 18,
    memoryMatrix: 42,
    tiltMaze: 87,
  },
  pomStatus: "idle",
  pomMode: "focus",
  pomCount: 0,
  pomTimeLeft: POM_DURATIONS.focus,
  navEvent: {
    direction: "right",
    distance: "150 m",
    street: "Kings Avenue",
    eta: "12:45 PM",
    remaining: "4.2 km",
    totalDistance: "5.8 km",
  },
  pitch: 0,
  roll: 0,
  rawPitch: 0,
  rawRoll: 0,
  isLeveled: true,
  calibrating: false,
  calibStep: 3,
  offsetPitch: 0,
  offsetRoll: 0,
  settingsScrollIndex: 0,
  editingSettingIdx: null,
  settings: {
    bluetooth: true,
    brightness: "HIGH",
    invertColors: false,
    clockStyle: 0,
    gifSpeed: 169,
    dnd: false,
    audioLoopback: false,
  },
  showResetConfirm: false,
  resetConfirmStep: 0,
  batteryLevel: 78,
  bleConnected: true,
  partnerOnline: true,
  steps: 6420,
  temperature: 24.5,
  overlay: null,
  overlayNotif: null,
  voipDuration: 0,
  voipMuted: false,
  partnerReturn: 3,
};

function nextScreen(s: Screen): Screen {
  const idx = SCREENS.indexOf(s);
  return SCREENS[(idx + 1) % SCREENS.length];
}

function prevScreen(s: Screen): Screen {
  const idx = SCREENS.indexOf(s);
  return SCREENS[(idx - 1 + SCREENS.length) % SCREENS.length];
}

function nextPomMode(mode: PomodoroMode, count: number): PomodoroMode {
  if (mode === "focus") return count > 0 && count % 4 === 0 ? "longBreak" : "shortBreak";
  return "focus";
}

function reducer(state: LunaState, action: LunaAction): LunaState {
  switch (action.type) {
    case "SWIPE_LEFT": {
      if (state.isTransitioning || state.arcadeMode === "playing") return state;
      return { ...state, prevScreen: state.screen, screen: nextScreen(state.screen), transitionDirection: "left", isTransitioning: true };
    }
    case "SWIPE_RIGHT": {
      if (state.isTransitioning || state.arcadeMode === "playing") return state;
      return { ...state, prevScreen: state.screen, screen: prevScreen(state.screen), transitionDirection: "right", isTransitioning: true };
    }
    case "TRANSITION_DONE":
      return { ...state, isTransitioning: false };
    case "NAVIGATE_TO":
      if (state.isTransitioning) return state;
      return { ...state, prevScreen: state.screen, screen: action.screen, transitionDirection: "left", isTransitioning: true };
    case "TICK_TIME":
      return { ...state, currentTime: new Date() };
    case "CYCLE_CLOCK_STYLE":
      return { ...state, clockStyle: ((state.clockStyle + 1) % 4) as 0 | 1 | 2 | 3 };
    case "OPEN_NOTIF":
      return { ...state, notifView: "detail", selectedNotifId: action.id, notifications: state.notifications.map(n => n.id === action.id ? { ...n, isRead: true } : n) };
    case "CLOSE_NOTIF":
      return { ...state, notifView: "list", selectedNotifId: null };
    case "TOGGLE_CAL_VIEW":
      return { ...state, calView: state.calView === "timeline" ? "month" : "timeline" };
    case "SELECT_GAME":
      return { ...state, selectedGame: action.idx };
    case "LAUNCH_GAME":
      return { ...state, arcadeMode: "playing", activeGame: state.selectedGame };
    case "EXIT_GAME":
      return { ...state, arcadeMode: action.score !== undefined ? "gameover" : "menu", activeGame: action.score !== undefined ? state.activeGame : null };
    case "SAVE_HIGH_SCORE": {
      const cur = state.highScores[action.gameKey];
      if (action.score <= cur) return { ...state, arcadeMode: "gameover" };
      return { ...state, highScores: { ...state.highScores, [action.gameKey]: action.score }, arcadeMode: "gameover" };
    }
    case "POM_PLAY_PAUSE":
      if (state.pomStatus === "complete") return state;
      return { ...state, pomStatus: state.pomStatus === "running" ? "paused" : "running" };
    case "POM_SKIP": {
      const newMode = nextPomMode(state.pomMode, state.pomCount);
      const newCount = state.pomMode === "focus" ? state.pomCount + 1 : state.pomCount;
      return { ...state, pomMode: newMode, pomStatus: "idle", pomTimeLeft: POM_DURATIONS[newMode], pomCount: newCount };
    }
    case "POM_RESET":
      return { ...state, pomStatus: "idle", pomTimeLeft: POM_DURATIONS[state.pomMode] };
    case "POM_TICK": {
      if (state.pomStatus !== "running") return state;
      const newTime = state.pomTimeLeft - 1;
      if (newTime <= 0) return { ...state, pomTimeLeft: 0, pomStatus: "complete" };
      return { ...state, pomTimeLeft: newTime };
    }
    case "POM_PHASE_COMPLETE": {
      const newMode = nextPomMode(state.pomMode, state.pomCount);
      const newCount = state.pomMode === "focus" ? state.pomCount + 1 : state.pomCount;
      return { ...state, pomMode: newMode, pomStatus: "idle", pomTimeLeft: POM_DURATIONS[newMode], pomCount: newCount };
    }
    case "UPDATE_LEVEL": {
      const p = action.pitch - state.offsetPitch;
      const r = action.roll - state.offsetRoll;
      return { ...state, rawPitch: action.pitch, rawRoll: action.roll, pitch: p, roll: r, isLeveled: Math.abs(p) < 0.5 && Math.abs(r) < 0.5 };
    }
    case "CALIBRATE_START":
      return { ...state, calibrating: true, calibStep: 3 };
    case "CALIBRATE_TICK":
      if (state.calibStep <= 1) return state;
      return { ...state, calibStep: state.calibStep - 1 };
    case "CALIBRATE_DONE":
      return { ...state, calibrating: false, calibStep: 3, offsetPitch: state.rawPitch, offsetRoll: state.rawRoll, pitch: 0, roll: 0, isLeveled: true };
    case "SETTINGS_SCROLL":
      return { ...state, settingsScrollIndex: Math.max(0, Math.min(7, state.settingsScrollIndex + action.delta)) };
    case "EDIT_SETTING":
      return { ...state, editingSettingIdx: action.idx };
    case "CHANGE_SETTING":
      return { ...state, settings: { ...state.settings, [action.key]: action.value } };
    case "SAVE_SETTING":
      return { ...state, editingSettingIdx: null };
    case "SHOW_RESET_CONFIRM":
      return { ...state, showResetConfirm: true };
    case "CANCEL_RESET":
      return { ...state, showResetConfirm: false, resetConfirmStep: 0 };
    case "EXECUTE_RESET":
      return { ...initialState, currentTime: new Date() };
    case "SHOW_OVERLAY":
      return { ...state, overlay: action.overlay, overlayNotif: action.notif || null, partnerReturn: state.screen };
    case "DISMISS_OVERLAY":
      return { ...state, overlay: null, overlayNotif: null, voipDuration: 0 };
    case "VOIP_TICK":
      return { ...state, voipDuration: state.voipDuration + 1 };
    case "VOIP_MUTE_TOGGLE":
      return { ...state, voipMuted: !state.voipMuted };
    default:
      return state;
  }
}

export function useLunaState() {
  const [state, dispatch] = useReducer(reducer, initialState);
  const stateRef = useRef(state);
  stateRef.current = state;

  // Clock tick
  useEffect(() => {
    const id = setInterval(() => dispatch({ type: "TICK_TIME" }), 1000);
    return () => clearInterval(id);
  }, []);

  // Pomodoro ticker
  useEffect(() => {
    if (state.pomStatus !== "running") return;
    const id = setInterval(() => dispatch({ type: "POM_TICK" }), 1000);
    return () => clearInterval(id);
  }, [state.pomStatus]);

  // VoIP ticker
  useEffect(() => {
    if (state.overlay !== "voip") return;
    const id = setInterval(() => dispatch({ type: "VOIP_TICK" }), 1000);
    return () => clearInterval(id);
  }, [state.overlay]);

  // Simulate incoming notification after 8s
  const notifSent = useRef(false);
  useEffect(() => {
    if (notifSent.current) return;
    const id = setTimeout(() => {
      notifSent.current = true;
      dispatch({
        type: "SHOW_OVERLAY",
        overlay: "notification",
        notif: {
          id: 99,
          app: "WA",
          sender: "Alex Chen",
          time: "now",
          preview: "Hey! The build just broke 😅",
          full: "Hey! The build just broke 😅 Can you take a look at the logs? It's in the auth module again.",
          isRead: false,
        },
      });
    }, 8000);
    return () => clearTimeout(id);
  }, []);

  // Calibration timer
  useEffect(() => {
    if (!state.calibrating) return;
    const id = setInterval(() => {
      if (stateRef.current.calibStep <= 1) {
        dispatch({ type: "CALIBRATE_DONE" });
      } else {
        dispatch({ type: "CALIBRATE_TICK" });
      }
    }, 900);
    return () => clearInterval(id);
  }, [state.calibrating]);

  // Keyboard navigation
  const handleKeyDown = useCallback((e: KeyboardEvent) => {
    const s = stateRef.current;
    if (s.overlay) {
      if (e.key === "Escape") dispatch({ type: "DISMISS_OVERLAY" });
      return;
    }
    if (e.key === "ArrowLeft") dispatch({ type: "SWIPE_LEFT" });
    if (e.key === "ArrowRight") dispatch({ type: "SWIPE_RIGHT" });
  }, []);

  useEffect(() => {
    window.addEventListener("keydown", handleKeyDown);
    return () => window.removeEventListener("keydown", handleKeyDown);
  }, [handleKeyDown]);

  return { state, dispatch };
}
