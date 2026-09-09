import { useRef, useEffect, useCallback, useState } from "react";
import type { Screen } from "./types";
import { useLunaState } from "./useLunaState";

import ClockScreen from "./screens/ClockScreen";
import NotificationsScreen from "./screens/NotificationsScreen";
import CalendarScreen from "./screens/CalendarScreen";
import ArcadeScreen from "./screens/ArcadeScreen";
import PomodoroScreen from "./screens/PomodoroScreen";
import NavigationScreen from "./screens/NavigationScreen";
import SpiritLevelScreen from "./screens/SpiritLevelScreen";
import SettingsScreen from "./screens/SettingsScreen";
import GlobalOverlays from "./overlays/GlobalOverlays";

const SCREEN_W = 280;
const SCREEN_H = 336;

const SCREENS: Screen[] = [3, 4, 5, 6, 7, 8, 9, 10];
const SCREEN_LABELS: Record<Screen, { label: string; icon: string }> = {
  3:  { label: "CLOCK",    icon: "🕐" },
  4:  { label: "MSGS",     icon: "🔔" },
  5:  { label: "CALENDAR", icon: "📅" },
  6:  { label: "ARCADE",   icon: "🎮" },
  7:  { label: "FOCUS",    icon: "⏱" },
  8:  { label: "NAV",      icon: "🧭" },
  9:  { label: "LEVEL",    icon: "📐" },
  10: { label: "SYSTEM",   icon: "⚙️" },
};

function ScreenComponent({
  screen,
  state,
  dispatch,
}: {
  screen: Screen;
  state: ReturnType<typeof useLunaState>["state"];
  dispatch: ReturnType<typeof useLunaState>["dispatch"];
}) {
  switch (screen) {
    case 3:  return <ClockScreen state={state} dispatch={dispatch} />;
    case 4:  return <NotificationsScreen state={state} dispatch={dispatch} />;
    case 5:  return <CalendarScreen state={state} dispatch={dispatch} />;
    case 6:  return <ArcadeScreen state={state} dispatch={dispatch} />;
    case 7:  return <PomodoroScreen state={state} dispatch={dispatch} />;
    case 8:  return <NavigationScreen state={state} dispatch={dispatch} />;
    case 9:  return <SpiritLevelScreen state={state} dispatch={dispatch} />;
    case 10: return <SettingsScreen state={state} dispatch={dispatch} />;
    default: return <ClockScreen state={state} dispatch={dispatch} />;
  }
}

interface TState {
  active: boolean;
  direction: "left" | "right";
  prev: Screen;
}

export default function LunaDevice() {
  const { state, dispatch } = useLunaState();
  const dragRef = useRef<{ startX: number; startY: number; active: boolean }>({ startX: 0, startY: 0, active: false });
  const [transition, setTransition] = useState<TState | null>(null);

  useEffect(() => {
    if (state.isTransitioning && !transition) {
      setTransition({ active: true, direction: state.transitionDirection, prev: state.prevScreen });
      const id = setTimeout(() => {
        setTransition(null);
        dispatch({ type: "TRANSITION_DONE" });
      }, 300);
      return () => clearTimeout(id);
    }
  }, [state.isTransitioning, state.transitionDirection, state.prevScreen, dispatch, transition]);

  const onPointerDown = useCallback((e: React.PointerEvent) => {
    dragRef.current = { startX: e.clientX, startY: e.clientY, active: true };
  }, []);

  const onPointerUp = useCallback((e: React.PointerEvent) => {
    if (!dragRef.current.active) return;
    dragRef.current.active = false;
    const dx = e.clientX - dragRef.current.startX;
    const dy = e.clientY - dragRef.current.startY;
    if (Math.abs(dx) > 32 && Math.abs(dx) > Math.abs(dy) * 1.4) {
      dispatch({ type: dx < 0 ? "SWIPE_LEFT" : "SWIPE_RIGHT" });
    }
  }, [dispatch]);

  return (
    <div
      className="flex flex-col items-center justify-start min-h-screen py-10 gap-8 overflow-auto"
      style={{
        background: "radial-gradient(ellipse 80% 60% at 50% 30%, #0d1829 0%, #06080f 60%, #000000 100%)",
        minWidth: 360,
      }}
    >
      {/* Product header */}
      <div className="flex flex-col items-center gap-1">
        <div className="font-orbitron text-[11px] font-black tracking-[0.4em] text-white/20">LUNA OS</div>
        <div className="font-mono-jb text-[8px] text-sky-400/40 tracking-[0.2em]">DESKTOP COMPANION · v1.0</div>
      </div>

      {/* Device wrapper — outer chrome ring */}
      <div className="relative flex items-center" style={{ filter: "drop-shadow(0 24px 60px rgba(0,0,0,0.7)) drop-shadow(0 0 40px rgba(56,189,248,0.08))" }}>

        {/* Left side buttons */}
        <div className="absolute left-0 -translate-x-full pr-0 flex flex-col gap-2 items-end" style={{ right: "auto" }}>
          <div className="w-1 h-8 rounded-l-sm" style={{ background: "linear-gradient(180deg,#1e2235,#141728,#1e2235)", boxShadow: "inset 1px 0 0 rgba(255,255,255,0.05)" }} />
          <div className="w-1 h-5 rounded-l-sm" style={{ background: "linear-gradient(180deg,#1e2235,#141728,#1e2235)", boxShadow: "inset 1px 0 0 rgba(255,255,255,0.05)" }} />
        </div>

        {/* Main device body */}
        <div
          style={{
            background: "linear-gradient(145deg, #1c2035 0%, #0e1120 40%, #141828 70%, #1c2035 100%)",
            borderRadius: 36,
            padding: "18px 14px 22px",
            boxShadow: [
              "0 0 0 1px rgba(255,255,255,0.07)",
              "0 0 0 1px rgba(0,0,0,0.8)",
              "inset 0 1px 0 rgba(255,255,255,0.12)",
              "inset 0 -1px 0 rgba(0,0,0,0.5)",
              "inset 1px 0 0 rgba(255,255,255,0.04)",
              "inset -1px 0 0 rgba(0,0,0,0.4)",
            ].join(", "),
            position: "relative",
          }}
        >
          {/* Speaker grill — top */}
          <div className="flex justify-center gap-1 mb-3">
            {[...Array(6)].map((_, i) => (
              <div key={i} className="w-0.5 h-1 rounded-full" style={{ background: "rgba(255,255,255,0.08)" }} />
            ))}
          </div>

          {/* Screen bezel */}
          <div
            style={{
              borderRadius: 20,
              padding: 3,
              background: "linear-gradient(160deg, rgba(255,255,255,0.06) 0%, rgba(255,255,255,0.01) 50%, rgba(0,0,0,0.2) 100%)",
              boxShadow: "inset 0 0 0 1px rgba(255,255,255,0.05), 0 2px 8px rgba(0,0,0,0.5)",
            }}
          >
            {/* Screen */}
            <div
              style={{
                width: SCREEN_W,
                height: SCREEN_H,
                borderRadius: 17,
                overflow: "hidden",
                position: "relative",
                background: "#000",
                boxShadow: "inset 0 0 0 1px rgba(0,0,0,0.8), 0 0 30px rgba(56,189,248,0.06)",
                cursor: "grab",
              }}
              onPointerDown={onPointerDown}
              onPointerUp={onPointerUp}
              onPointerLeave={e => { if (dragRef.current.active) onPointerUp(e); }}
            >
              {/* Transition layer */}
              {transition ? (
                <>
                  <div
                    className="absolute inset-0"
                    style={{
                      animation: `${transition.direction === "left" ? "slide-left-out" : "slide-right-out"} 0.3s cubic-bezier(0.22,1,0.36,1) forwards`,
                    }}
                  >
                    <ScreenComponent screen={transition.prev} state={state} dispatch={dispatch} />
                  </div>
                  <div
                    className="absolute inset-0"
                    style={{
                      animation: `${transition.direction === "left" ? "slide-left-in" : "slide-right-in"} 0.3s cubic-bezier(0.22,1,0.36,1) forwards`,
                    }}
                  >
                    <ScreenComponent screen={state.screen} state={state} dispatch={dispatch} />
                  </div>
                </>
              ) : (
                <div className="absolute inset-0">
                  <ScreenComponent screen={state.screen} state={state} dispatch={dispatch} />
                </div>
              )}

              {/* Global overlays */}
              <GlobalOverlays state={state} dispatch={dispatch} />

              {/* Screen reflective glare */}
              <div
                className="absolute inset-0 pointer-events-none"
                style={{
                  borderRadius: 17,
                  background: "linear-gradient(135deg, rgba(255,255,255,0.03) 0%, transparent 40%)",
                }}
              />
              {/* Screen edge depth */}
              <div
                className="absolute inset-0 pointer-events-none"
                style={{
                  borderRadius: 17,
                  boxShadow: "inset 0 0 0 1px rgba(255,255,255,0.04), inset 0 1px 0 rgba(255,255,255,0.06)",
                }}
              />
            </div>
          </div>

          {/* Bottom area */}
          <div className="flex justify-center mt-3 gap-2">
            {/* Navigation dots */}
            {SCREENS.map(s => (
              <button
                key={s}
                onClick={() => dispatch({ type: "NAVIGATE_TO", screen: s })}
                className="interactive-tap transition-all duration-200"
                style={{
                  width: s === state.screen ? 18 : 5,
                  height: 5,
                  borderRadius: 3,
                  background: s === state.screen ? "#38BDF8" : "rgba(255,255,255,0.15)",
                  boxShadow: s === state.screen ? "0 0 8px #38BDF8" : "none",
                  border: "none",
                  cursor: "pointer",
                  padding: 0,
                }}
              />
            ))}
          </div>
        </div>

        {/* Right side crown + button */}
        <div className="absolute right-0 translate-x-full pl-0 flex flex-col gap-3 items-start" style={{ left: "auto" }}>
          {/* Crown button — primary */}
          <button
            onClick={() => dispatch({ type: "SWIPE_LEFT" })}
            title="Next screen"
            className="interactive-tap"
            style={{
              width: 6,
              height: 40,
              borderRadius: "0 4px 4px 0",
              background: "linear-gradient(180deg,#252840,#161929,#252840)",
              boxShadow: "inset -1px 0 0 rgba(255,255,255,0.06), 1px 0 0 rgba(0,0,0,0.4)",
              border: "none",
              cursor: "pointer",
            }}
          />
          {/* Secondary button */}
          <button
            onClick={() => dispatch({ type: "SWIPE_RIGHT" })}
            title="Previous screen"
            className="interactive-tap"
            style={{
              width: 5,
              height: 22,
              borderRadius: "0 3px 3px 0",
              background: "linear-gradient(180deg,#252840,#161929,#252840)",
              boxShadow: "inset -1px 0 0 rgba(255,255,255,0.04), 1px 0 0 rgba(0,0,0,0.3)",
              border: "none",
              cursor: "pointer",
            }}
          />
        </div>
      </div>

      {/* Screen info + gesture hints */}
      <div className="flex flex-col items-center gap-3">
        {/* Current screen label */}
        <div className="flex items-center gap-2">
          <span className="text-[10px]">{SCREEN_LABELS[state.screen].icon}</span>
          <div>
            <div className="font-orbitron text-[9px] font-bold text-sky-400 tracking-[0.25em]">
              {SCREEN_LABELS[state.screen].label}
            </div>
            <div className="font-mono-jb text-[7px] text-white/20 text-center">SCREEN {state.screen}</div>
          </div>
        </div>

        {/* Controls */}
        <div className="flex items-center gap-3 flex-wrap justify-center">
          <div className="flex items-center gap-1.5">
            <kbd className="font-mono-jb text-[7px] text-white/30 px-1.5 py-0.5 rounded border border-white/10 bg-white/5">←</kbd>
            <kbd className="font-mono-jb text-[7px] text-white/30 px-1.5 py-0.5 rounded border border-white/10 bg-white/5">→</kbd>
            <span className="font-mono-jb text-[7px] text-white/20">NAVIGATE</span>
          </div>
          <span className="text-white/10 font-mono-jb text-[7px]">·</span>
          <button
            onClick={() => dispatch({ type: "SHOW_OVERLAY", overlay: "partner" })}
            className="interactive-tap font-mono-jb text-[7px] px-2 py-1 rounded border"
            style={{ color: "rgba(244,114,182,0.7)", borderColor: "rgba(244,114,182,0.2)", background: "rgba(244,114,182,0.05)" }}
          >
            ♥ PARTNER
          </button>
          <button
            onClick={() => dispatch({ type: "SHOW_OVERLAY", overlay: "voip" })}
            className="interactive-tap font-mono-jb text-[7px] px-2 py-1 rounded border"
            style={{ color: "rgba(56,189,248,0.7)", borderColor: "rgba(56,189,248,0.2)", background: "rgba(56,189,248,0.05)" }}
          >
            📞 CALL
          </button>
        </div>
      </div>

      {/* Bottom ambient */}
      <div className="font-mono-jb text-[7px] text-white/10 tracking-[0.3em] text-center pb-4">
        SWIPE DEVICE · CLICK BUTTONS · PRESS ARROW KEYS
      </div>
    </div>
  );
}
