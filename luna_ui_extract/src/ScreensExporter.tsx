import React from "react";
import ClockScreen from "./luna/screens/ClockScreen";
import NotificationsScreen from "./luna/screens/NotificationsScreen";
import CalendarScreen from "./luna/screens/CalendarScreen";
import ArcadeScreen from "./luna/screens/ArcadeScreen";
import PomodoroScreen from "./luna/screens/PomodoroScreen";
import NavigationScreen from "./luna/screens/NavigationScreen";
import SpiritLevelScreen from "./luna/screens/SpiritLevelScreen";
import SettingsScreen from "./luna/screens/SettingsScreen";
import type { LunaState } from "./luna/types";

const mockBaseState: LunaState = {
  screen: 3,
  prevScreen: 3,
  transitionDirection: "left",
  isTransitioning: false,
  clockStyle: 0,
  currentTime: new Date(2026, 8, 9, 12, 45, 30),
  notifView: "list",
  selectedNotifId: null,
  notifications: [
    { id: 1, app: "WA", sender: "Sarah Jenkins", time: "10:38 AM", preview: "Are we still on for lunch today?", full: "Lunch today at Noodle House?", isRead: false },
    { id: 2, app: "SLACK", sender: "#engineering", channel: "#engineering", time: "10:25 AM", preview: "PR #42 merged to main", full: "Auth refactor is live.", isRead: false },
    { id: 3, app: "MAIL", sender: "Stripe Support", time: "09:50 AM", preview: "Your payout of $2,840.00 processed", full: "Funds sent to bank account.", isRead: false },
    { id: 4, app: "CAL", sender: "Google Calendar", time: "09:00 AM", preview: "Design Sync in 30 minutes", full: "Meeting link: meet.google.com", isRead: true },
  ],
  calView: "timeline",
  calEvents: [
    { id: 1, time: "09:00", hour: 9, title: "Morning Standup", duration: "15 min", type: "video", completed: true },
    { id: 2, time: "11:30", hour: 11, title: "Founder Sync", duration: "45 min", type: "video", completed: false, location: "Google Meet", isCurrent: true },
    { id: 3, time: "14:00", hour: 14, title: "Sprint Review", duration: "60 min", type: "in-person", completed: false, location: "Conf Room B" },
    { id: 4, time: "16:30", hour: 16, title: "Design Review", duration: "30 min", type: "video", completed: false, location: "Figma" },
  ],
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
  pomStatus: "running",
  pomMode: "focus",
  pomCount: 2,
  pomTimeLeft: 18 * 60 + 25,
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
  batteryLevel: 85,
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

const dummyDispatch = () => {};

export default function ScreensExporter() {
  const clockMinimal = { ...mockBaseState, clockStyle: 0 as const };
  const clockRetro = { ...mockBaseState, clockStyle: 1 as const };
  const clockCyber = { ...mockBaseState, clockStyle: 2 as const };
  const clockAnalog = { ...mockBaseState, clockStyle: 3 as const };

  const boxStyle: React.CSSProperties = {
    width: 240,
    height: 280,
    overflow: "hidden",
    position: "relative",
    background: "#000000",
    display: "inline-block",
    margin: 10,
    boxShadow: "0 0 10px rgba(0,0,0,0.5)",
  };

  return (
    <div style={{ background: "#111", padding: 20, display: "flex", flexWrap: "wrap" }}>
      <div id="screen-clock-minimal" style={boxStyle}>
        <ClockScreen state={clockMinimal} dispatch={dummyDispatch} />
      </div>

      <div id="screen-clock-retro" style={boxStyle}>
        <ClockScreen state={clockRetro} dispatch={dummyDispatch} />
      </div>

      <div id="screen-clock-cyber" style={boxStyle}>
        <ClockScreen state={clockCyber} dispatch={dummyDispatch} />
      </div>

      <div id="screen-clock-analog" style={boxStyle}>
        <ClockScreen state={clockAnalog} dispatch={dummyDispatch} />
      </div>

      <div id="screen-notifications" style={boxStyle}>
        <NotificationsScreen state={mockBaseState} dispatch={dummyDispatch} />
      </div>

      <div id="screen-calendar" style={boxStyle}>
        <CalendarScreen state={mockBaseState} dispatch={dummyDispatch} />
      </div>

      <div id="screen-arcade" style={boxStyle}>
        <ArcadeScreen state={mockBaseState} dispatch={dummyDispatch} />
      </div>

      <div id="screen-pomodoro" style={boxStyle}>
        <PomodoroScreen state={mockBaseState} dispatch={dummyDispatch} />
      </div>

      <div id="screen-navigation" style={boxStyle}>
        <NavigationScreen state={mockBaseState} dispatch={dummyDispatch} />
      </div>

      <div id="screen-level" style={boxStyle}>
        <SpiritLevelScreen state={mockBaseState} dispatch={dummyDispatch} />
      </div>

      <div id="screen-settings" style={boxStyle}>
        <SettingsScreen state={mockBaseState} dispatch={dummyDispatch} />
      </div>

      {/* Clock Minimal with Blanked Time Area (for dynamic live drawing) */}
      <div id="screen-clock-minimal-blank" style={boxStyle}>
        <div style={{ position: "relative", width: "100%", height: "100%" }}>
          <ClockScreen state={clockMinimal} dispatch={dummyDispatch} />
          {/* Black box covering the static 12:45 :30 time digits */}
          <div style={{ position: "absolute", left: 35, top: 48, width: 170, height: 48, background: "#000000" }} />
        </div>
      </div>

      {/* Orbitron 58px Large Digits Sprite Sheet (0-9 and colon) */}
      <div id="screen-orbitron-digits-58" style={{ width: 440, height: 70, background: "#000", padding: "10px 14px", display: "inline-block", margin: 10 }}>
        <div style={{ display: "flex", gap: 8, alignItems: "center" }}>
          {["0","1","2","3","4","5","6","7","8","9",":"].map(ch => (
            <div key={ch} id={`orbitron-g-${ch === ":" ? "colon" : ch}`} style={{ width: ch === ":" ? 18 : 34, height: 50, display: "flex", alignItems: "center", justifyContent: "center", background: "#000" }}>
              <span className="font-orbitron font-black text-white leading-none" style={{ fontSize: 58, letterSpacing: "-2px", textShadow: "0 0 40px rgba(56,189,248,0.25)" }}>
                {ch}
              </span>
            </div>
          ))}
        </div>
      </div>

      {/* Orbitron 18px Sky-Blue Seconds Digits Sprite Sheet */}
      <div id="screen-orbitron-sec-18" style={{ width: 300, height: 40, background: "#000", padding: "6px 10px", display: "inline-block", margin: 10 }}>
        <div style={{ display: "flex", gap: 6, alignItems: "center" }}>
          {["0","1","2","3","4","5","6","7","8","9",":"].map(ch => (
            <div key={ch} id={`orbitron-s-${ch === ":" ? "colon" : ch}`} style={{ width: ch === ":" ? 10 : 16, height: 26, display: "flex", alignItems: "center", justifyContent: "center", background: "#000" }}>
              <span className="font-orbitron font-bold text-sky-400 leading-none text-[18px]">
                {ch}
              </span>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
}
