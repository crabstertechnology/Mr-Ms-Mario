import type React from "react";
import { useEffect, useRef } from "react";
import type { LunaState, LunaAction, AppIcon } from "../types";
import LunaMascot from "../components/LunaMascot";

interface Props { state: LunaState; dispatch: React.Dispatch<LunaAction> }

const APP_COLORS: Record<AppIcon, string> = {
  WA: "#25D366",
  SLACK: "#611f69",
  MAIL: "#38BDF8",
  PHONE: "#4ade80",
  CAL: "#F472B6",
};

function NotificationOverlay({ state, dispatch }: Props) {
  const notif = state.overlayNotif;
  if (!notif) return null;
  const color = APP_COLORS[notif.app];

  useEffect(() => {
    const id = setTimeout(() => dispatch({ type: "DISMISS_OVERLAY" }), 5000);
    return () => clearTimeout(id);
  }, [dispatch]);

  return (
    <div
      className="animate-overlay-in cursor-pointer interactive-tap"
      style={{
        position: "absolute",
        top: 0,
        left: 0,
        right: 0,
        zIndex: 50,
        padding: "8px",
      }}
      onClick={() => dispatch({ type: "DISMISS_OVERLAY" })}
    >
      <div
        className="rounded-xl px-3 py-2.5 flex items-center gap-2"
        style={{
          background: "rgba(10,12,24,0.95)",
          border: `1px solid ${color}40`,
          boxShadow: `0 0 20px ${color}20, 0 8px 32px rgba(0,0,0,0.6)`,
          backdropFilter: "blur(8px)",
        }}
      >
        {/* App dot */}
        <div className="w-6 h-6 rounded-lg flex items-center justify-center font-mono-jb font-bold text-[7px] flex-shrink-0"
          style={{ background: color + "20", border: `1px solid ${color}40`, color }}>
          {notif.app}
        </div>
        <div className="flex-1 min-w-0">
          <div className="flex items-center justify-between">
            <span className="font-dm text-[9px] font-semibold text-white">{notif.sender}</span>
            <span className="font-mono-jb text-[7px] text-white/30">{notif.time}</span>
          </div>
          <p className="font-dm text-[8px] text-white/60 leading-tight truncate">{notif.preview}</p>
        </div>
        <div className="w-1.5 h-1.5 rounded-full flex-shrink-0" style={{ background: color, boxShadow: `0 0 4px ${color}` }} />
      </div>
    </div>
  );
}

function PartnerEventOverlay({ state, dispatch }: Props) {
  const hearts = ["♥", "♥", "♥", "♥", "♥"];

  useEffect(() => {
    const id = setTimeout(() => dispatch({ type: "DISMISS_OVERLAY" }), 6000);
    return () => clearTimeout(id);
  }, [dispatch]);

  return (
    <div
      className="absolute inset-0 z-50 flex flex-col items-center justify-center gap-3 cursor-pointer"
      style={{ background: "rgba(0,0,0,0.92)", backdropFilter: "blur(6px)" }}
      onClick={() => dispatch({ type: "DISMISS_OVERLAY" })}
    >
      {/* Floating hearts */}
      <div className="flex gap-1">
        {hearts.map((h, i) => (
          <span
            key={i}
            className="text-[#F472B6] text-[14px]"
            style={{
              animation: `hearts-float ${1 + i * 0.2}s ease-out ${i * 0.1}s infinite`,
              filter: "drop-shadow(0 0 6px #F472B6)",
            }}
          >
            {h}
          </span>
        ))}
      </div>

      <div className="font-orbitron text-[10px] font-bold text-white/60 tracking-[0.15em]">SARAH SENT LOVE</div>

      <LunaMascot mood="love" size="lg" />

      <div className="font-dm text-[9px] text-white/40 italic">"Thinking of you!"</div>

      <div className="flex gap-1 mt-1">
        {hearts.map((h, i) => (
          <span
            key={i}
            className="text-[#F472B6] text-[10px]"
            style={{
              animation: `hearts-float ${0.8 + i * 0.15}s ease-out ${0.5 + i * 0.1}s infinite`,
              filter: "drop-shadow(0 0 4px #F472B6)",
              opacity: 0.6,
            }}
          >
            {h}
          </span>
        ))}
      </div>

      <div className="font-mono-jb text-[7px] text-white/20 mt-2">TAP TO DISMISS</div>
    </div>
  );
}

function VoipOverlay({ state, dispatch }: Props) {
  const dur = state.voipDuration;
  const min = Math.floor(dur / 60).toString().padStart(2, "0");
  const sec = (dur % 60).toString().padStart(2, "0");

  return (
    <div
      className="absolute inset-0 z-50 flex flex-col bg-[#070A13]"
      style={{ boxShadow: "inset 0 0 40px rgba(56,189,248,0.05)" }}
    >
      {/* Header */}
      <div className="px-4 pt-4 pb-2 border-b border-white/[0.06]">
        <div className="font-orbitron text-[9px] font-semibold text-white/40 tracking-[0.2em]">INCOMING CALL</div>
      </div>

      {/* Main */}
      <div className="flex-1 flex flex-col items-center justify-center gap-3">
        {/* Pulse rings */}
        <div className="relative flex items-center justify-center">
          <div className="absolute w-20 h-20 rounded-full border border-sky-400/20 animate-ripple" />
          <div className="absolute w-20 h-20 rounded-full border border-sky-400/10" style={{ animation: "ripple 1.5s ease-out 0.4s infinite" }} />
          <div className="w-16 h-16 rounded-full bg-sky-400/10 flex items-center justify-center border border-sky-400/40">
            <span className="text-[28px]">📞</span>
          </div>
        </div>

        <div className="flex gap-1.5">
          {[...Array(3)].map((_, i) => (
            <div
              key={i}
              className="w-1.5 h-1.5 rounded-full bg-sky-400"
              style={{ animation: `pulse 1s ease-in-out ${i * 0.2}s infinite` }}
            />
          ))}
        </div>

        <div className="font-orbitron text-[13px] font-black text-white">VOICE INTERCOM</div>
        <div className="font-mono-jb text-[18px] font-bold text-sky-400" style={{ textShadow: "0 0 10px #38BDF8" }}>
          {min}:{sec}
        </div>

        <div className="flex items-center gap-2">
          <div className={`w-2 h-2 rounded-full ${state.voipMuted ? "bg-[#ef4444]" : "bg-[#4ade80]"}`}
            style={{ boxShadow: state.voipMuted ? "0 0 4px #ef4444" : "0 0 4px #4ade80" }} />
          <span className="font-mono-jb text-[8px] text-white/40">MIC {state.voipMuted ? "MUTED" : "ON"}</span>
        </div>
      </div>

      {/* Controls */}
      <div className="px-4 pb-4 flex gap-3">
        <button
          onClick={() => dispatch({ type: "VOIP_MUTE_TOGGLE" })}
          className="interactive-tap flex-1 py-2.5 rounded-xl font-mono-jb text-[8px] flex flex-col items-center gap-1"
          style={{ background: state.voipMuted ? "rgba(239,68,68,0.2)" : "rgba(255,255,255,0.08)", border: `1px solid ${state.voipMuted ? "rgba(239,68,68,0.4)" : "rgba(255,255,255,0.12)"}` }}
        >
          <span className="text-[14px]">{state.voipMuted ? "🔇" : "🎙️"}</span>
          <span className="text-white/60">{state.voipMuted ? "UNMUTE" : "MUTE"}</span>
        </button>
        <button
          onClick={() => dispatch({ type: "DISMISS_OVERLAY" })}
          className="interactive-tap flex-1 py-2.5 rounded-xl font-mono-jb text-[8px] flex flex-col items-center gap-1"
          style={{ background: "rgba(239,68,68,0.2)", border: "1px solid rgba(239,68,68,0.4)" }}
        >
          <span className="text-[14px]">📴</span>
          <span className="text-[#ef4444]">END</span>
        </button>
      </div>
    </div>
  );
}

export default function GlobalOverlays({ state, dispatch }: Props) {
  if (!state.overlay) return null;

  return (
    <>
      {state.overlay === "notification" && <NotificationOverlay state={state} dispatch={dispatch} />}
      {state.overlay === "partner" && <PartnerEventOverlay state={state} dispatch={dispatch} />}
      {state.overlay === "voip" && <VoipOverlay state={state} dispatch={dispatch} />}
    </>
  );
}
