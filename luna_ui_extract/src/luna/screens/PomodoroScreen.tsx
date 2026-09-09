import type React from "react";
import type { LunaState, LunaAction } from "../types";
import StatusBar from "../components/StatusBar";
import LunaMascot from "../components/LunaMascot";
import ProgressRing from "../components/ProgressRing";

interface Props { state: LunaState; dispatch: React.Dispatch<LunaAction> }

const PHASE = {
  focus:      { label: "FOCUS",       totalSec: 25 * 60, color: "#38BDF8",  mascot: "focus"  as const, bg: "rgba(56,189,248,0.08)"  },
  shortBreak: { label: "SHORT BREAK", totalSec: 5  * 60, color: "#4ade80",  mascot: "break"  as const, bg: "rgba(74,222,128,0.08)"  },
  longBreak:  { label: "LONG BREAK",  totalSec: 15 * 60, color: "#8B5CF6",  mascot: "sleep"  as const, bg: "rgba(139,92,246,0.08)"  },
};

function fmt(s: number) {
  const m = Math.floor(s / 60);
  return `${m.toString().padStart(2, "0")}:${(s % 60).toString().padStart(2, "0")}`;
}

function CompleteState({ state, dispatch }: Props) {
  const isFromFocus = state.pomMode === "focus";
  const nextMeta = PHASE[isFromFocus ? "shortBreak" : "focus"];
  return (
    <div className="flex-1 flex flex-col items-center justify-center gap-3 px-4">
      {/* Pulsing complete circle */}
      <div className="relative flex items-center justify-center">
        <div className="absolute w-24 h-24 rounded-full animate-ripple" style={{ border: `1px solid ${nextMeta.color}40` }} />
        <div className="w-16 h-16 rounded-full flex items-center justify-center" style={{ background: nextMeta.bg, border: `1px solid ${nextMeta.color}40` }}>
          <span className="text-[28px]">{isFromFocus ? "😮‍💨" : "🎯"}</span>
        </div>
      </div>

      <div className="flex flex-col items-center gap-0.5">
        <div className="font-orbitron text-[9px] font-black tracking-[0.25em] text-white/40">TIME'S UP</div>
        <div className="font-orbitron text-[13px] font-black tracking-wider" style={{ color: nextMeta.color }}>
          {isFromFocus ? "TAKE A BREAK" : "FOCUS TIME"}
        </div>
      </div>

      <LunaMascot mood={nextMeta.mascot} size="md" />

      {isFromFocus && (
        <div className="flex gap-3 font-mono-jb text-[7px] text-white/30">
          <span>💧 HYDRATE</span>
          <span>🧘 RELAX</span>
          <span>🚶 STRETCH</span>
        </div>
      )}

      <div className="font-mono-jb text-[8px]" style={{ color: nextMeta.color + "80" }}>
        NEXT: {nextMeta.label} #{isFromFocus ? state.pomCount + 1 : state.pomCount}
      </div>

      <button
        onClick={() => dispatch({ type: "POM_PHASE_COMPLETE" })}
        className="interactive-tap mt-1 px-8 py-2.5 rounded-full font-orbitron text-[9px] font-bold text-black"
        style={{ background: nextMeta.color, boxShadow: `0 0 24px ${nextMeta.color}60` }}
      >
        CONTINUE →
      </button>
    </div>
  );
}

export default function PomodoroScreen({ state, dispatch }: Props) {
  const meta = PHASE[state.pomMode];
  const progress = 1 - state.pomTimeLeft / meta.totalSec;
  const isRunning = state.pomStatus === "running";
  const isComplete = state.pomStatus === "complete";
  const isIdle = state.pomStatus === "idle";

  // Session dots (4 sessions per cycle)
  const filledDots = state.pomCount % 4;

  return (
    <div className="flex flex-col h-full bg-black font-dm">
      <StatusBar state={state} label="POMODORO" />

      {isComplete ? (
        <CompleteState state={state} dispatch={dispatch} />
      ) : (
        <div className="flex-1 flex flex-col">
          {/* Phase selector */}
          <div className="flex gap-1 px-3 pt-2">
            {(["focus", "shortBreak", "longBreak"] as const).map(mode => {
              const m = PHASE[mode];
              const active = state.pomMode === mode;
              return (
                <button
                  key={mode}
                  className="flex-1 py-1 rounded font-mono-jb text-[6px] interactive-tap"
                  style={{
                    background: active ? m.color + "18" : "transparent",
                    border: `1px solid ${active ? m.color + "50" : "rgba(255,255,255,0.05)"}`,
                    color: active ? m.color : "rgba(255,255,255,0.25)",
                  }}
                  onClick={() => !isRunning && dispatch({ type: "POM_SKIP" })}
                >
                  {mode === "focus" ? "FOCUS" : mode === "shortBreak" ? "5 MIN" : "15 MIN"}
                </button>
              );
            })}
          </div>

          {/* Center: ring + time */}
          <div className="flex-1 flex flex-col items-center justify-center gap-2 px-3">
            <ProgressRing
              progress={progress}
              size={140}
              strokeWidth={7}
              color={meta.color}
              glow
            >
              <div className="flex flex-col items-center gap-1">
                <span
                  className="font-orbitron text-[10px] font-semibold tracking-[0.2em]"
                  style={{ color: meta.color + "90" }}
                >
                  {meta.label}
                </span>
                <span
                  className="font-orbitron font-black leading-none text-white"
                  style={{ fontSize: 30, textShadow: isRunning ? `0 0 16px ${meta.color}50` : "none" }}
                >
                  {fmt(state.pomTimeLeft)}
                </span>
                {/* Session dots */}
                <div className="flex gap-1.5 mt-0.5">
                  {[0, 1, 2, 3].map(i => (
                    <div
                      key={i}
                      className="rounded-full"
                      style={{
                        width: 6,
                        height: 6,
                        background: i < filledDots ? meta.color : "rgba(255,255,255,0.1)",
                        boxShadow: i < filledDots ? `0 0 4px ${meta.color}` : "none",
                      }}
                    />
                  ))}
                </div>
              </div>
            </ProgressRing>

            {/* Mascot */}
            <LunaMascot mood={meta.mascot} size="sm" animate={isRunning} />
          </div>

          {/* Controls */}
          <div className="px-4 pb-3 space-y-2.5">
            {/* Play/Pause */}
            <div className="flex justify-center">
              <button
                onClick={() => dispatch({ type: "POM_PLAY_PAUSE" })}
                className="interactive-tap w-14 h-14 rounded-full flex items-center justify-center text-[22px] font-black text-black"
                style={{
                  background: `radial-gradient(circle at 35% 35%, ${meta.color}dd, ${meta.color})`,
                  boxShadow: `0 0 24px ${meta.color}50, 0 4px 12px rgba(0,0,0,0.4)`,
                }}
              >
                {isRunning ? "⏸" : "▶"}
              </button>
            </div>

            {/* Secondary controls */}
            <div className="flex justify-center gap-3">
              <button
                onClick={() => dispatch({ type: "POM_RESET" })}
                className="interactive-tap font-mono-jb text-[8px] px-4 py-1.5 rounded-lg border text-white/30 border-white/10"
              >
                RESET
              </button>
              <button
                onClick={() => dispatch({ type: "POM_SKIP" })}
                className="interactive-tap font-mono-jb text-[8px] px-4 py-1.5 rounded-lg border"
                style={{ color: meta.color + "80", borderColor: meta.color + "30" }}
              >
                SKIP ⏭
              </button>
            </div>

            <div className="font-mono-jb text-[6px] text-white/15 text-center">
              TAP · {isRunning ? "PAUSE" : isIdle ? "START" : "RESUME"} · DBL TAP · SKIP
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
