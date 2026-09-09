import type React from "react";
import type { LunaState } from "../types";

interface StatusBarProps {
  state: LunaState;
  label?: string;
  right?: React.ReactNode;
}

function BatteryIcon({ level }: { level: number }) {
  const fill = Math.max(0, Math.min(100, level));
  const color = fill > 50 ? "#4ade80" : fill > 20 ? "#fbbf24" : "#ef4444";
  return (
    <div className="flex items-center gap-0.5">
      <div className="relative w-5 h-2.5 rounded-sm border border-white/40 overflow-visible">
        <div
          className="absolute left-0 top-0 bottom-0 rounded-[1px]"
          style={{ width: `${fill}%`, background: color }}
        />
      </div>
      <div className="w-0.5 h-1.5 rounded-r-sm bg-white/40" />
      <span className="font-mono-jb text-[7px] ml-0.5" style={{ color }}>{fill}%</span>
    </div>
  );
}

function BleIcon({ connected }: { connected: boolean }) {
  return (
    <svg width="8" height="10" viewBox="0 0 8 10" fill="none">
      <path
        d="M4 1L7 3.5L4 6M4 1L1 3.5L4 6M4 6V9M4 6L7 8.5M4 6L1 8.5"
        stroke={connected ? "#38BDF8" : "#ffffff30"}
        strokeWidth="1.2"
        strokeLinecap="round"
        strokeLinejoin="round"
      />
    </svg>
  );
}

function PartnerDot({ online }: { online: boolean }) {
  return (
    <div className="flex items-center gap-0.5">
      <div
        className={`w-1.5 h-1.5 rounded-full ${online ? "bg-[#F472B6]" : "bg-white/20"}`}
        style={online ? { boxShadow: "0 0 4px #F472B6" } : {}}
      />
    </div>
  );
}

export default function StatusBar({ state, label, right }: StatusBarProps) {
  const h = state.currentTime.getHours().toString().padStart(2, "0");
  const m = state.currentTime.getMinutes().toString().padStart(2, "0");

  return (
    <div className="flex items-center justify-between px-3 py-1.5 border-b border-white/[0.06]">
      <div className="flex items-center gap-2">
        <BleIcon connected={state.bleConnected} />
        <PartnerDot online={state.partnerOnline} />
      </div>

      <span className="font-orbitron text-[8px] font-semibold tracking-[0.15em] text-white/70 uppercase">
        {label || "LUNA"}
      </span>

      <div className="flex items-center gap-2">
        {right || (
          <span className="font-mono-jb text-[8px] text-white/50">{h}:{m}</span>
        )}
        <BatteryIcon level={state.batteryLevel} />
      </div>
    </div>
  );
}
