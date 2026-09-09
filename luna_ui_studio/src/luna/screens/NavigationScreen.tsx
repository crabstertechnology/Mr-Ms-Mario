import type React from "react";
import { useState } from "react";
import type { LunaState, LunaAction } from "../types";
import StatusBar from "../components/StatusBar";

interface Props { state: LunaState; dispatch: React.Dispatch<LunaAction> }

interface NavStep {
  direction: string;
  arrowSvg: React.ReactNode;
  distance: string;
  street: string;
  eta: string;
  remaining: string;
  total: string;
}

function ArrowStraight({ color }: { color: string }) {
  return (
    <svg width="70" height="70" viewBox="0 0 70 70" fill="none">
      <line x1="35" y1="60" x2="35" y2="15" stroke={color} strokeWidth="6" strokeLinecap="round"/>
      <polyline points="18,32 35,12 52,32" stroke={color} strokeWidth="6" strokeLinecap="round" strokeLinejoin="round" fill="none"/>
    </svg>
  );
}
function ArrowRight({ color }: { color: string }) {
  return (
    <svg width="70" height="70" viewBox="0 0 70 70" fill="none">
      <path d="M15 55 L15 30 Q15 15 30 15 L50 15" stroke={color} strokeWidth="6" strokeLinecap="round" strokeLinejoin="round" fill="none"/>
      <polyline points="34,2 50,18 34,34" stroke={color} strokeWidth="6" strokeLinecap="round" strokeLinejoin="round" fill="none"/>
    </svg>
  );
}
function ArrowLeft({ color }: { color: string }) {
  return (
    <svg width="70" height="70" viewBox="0 0 70 70" fill="none">
      <path d="M55 55 L55 30 Q55 15 40 15 L20 15" stroke={color} strokeWidth="6" strokeLinecap="round" strokeLinejoin="round" fill="none"/>
      <polyline points="36,2 20,18 36,34" stroke={color} strokeWidth="6" strokeLinecap="round" strokeLinejoin="round" fill="none"/>
    </svg>
  );
}
function ArrowHardRight({ color }: { color: string }) {
  return (
    <svg width="70" height="70" viewBox="0 0 70 70" fill="none">
      <path d="M15 58 L15 35 Q15 20 30 15 L55 15" stroke={color} strokeWidth="6" strokeLinecap="round" strokeLinejoin="round" fill="none"/>
      <polyline points="40,3 58,15 50,32" stroke={color} strokeWidth="6" strokeLinecap="round" strokeLinejoin="round" fill="none"/>
    </svg>
  );
}
function ArrowUTurn({ color }: { color: string }) {
  return (
    <svg width="70" height="70" viewBox="0 0 70 70" fill="none">
      <path d="M50 58 L50 28 Q50 10 35 10 Q20 10 20 28 L20 40" stroke={color} strokeWidth="6" strokeLinecap="round" fill="none"/>
      <polyline points="8,26 20,42 32,26" stroke={color} strokeWidth="6" strokeLinecap="round" strokeLinejoin="round" fill="none"/>
    </svg>
  );
}
function ArrowRoundabout({ color }: { color: string }) {
  return (
    <svg width="70" height="70" viewBox="0 0 70 70" fill="none">
      <circle cx="35" cy="35" r="16" stroke={color} strokeWidth="5" strokeLinecap="round" strokeDasharray="6 4" fill="none"/>
      <line x1="35" y1="8" x2="35" y2="19" stroke={color} strokeWidth="5" strokeLinecap="round"/>
      <polyline points="24,12 35,6 46,12" stroke={color} strokeWidth="5" strokeLinecap="round" strokeLinejoin="round" fill="none"/>
    </svg>
  );
}
function ArrowDestination({ color }: { color: string }) {
  return (
    <svg width="70" height="70" viewBox="0 0 70 70" fill="none">
      <path d="M35 10 C20 10 12 20 12 30 C12 45 35 62 35 62 C35 62 58 45 58 30 C58 20 50 10 35 10Z" stroke={color} strokeWidth="4" fill={color + "30"}/>
      <circle cx="35" cy="30" r="8" fill={color}/>
    </svg>
  );
}

const NAV_STEPS: NavStep[] = [
  {
    direction: "TURN RIGHT",
    arrowSvg: <ArrowRight color="#38BDF8" />,
    distance: "150 m",
    street: "Kings Avenue",
    eta: "12:45 PM",
    remaining: "4.2 km",
    total: "5.8 km",
  },
  {
    direction: "CONTINUE",
    arrowSvg: <ArrowStraight color="#38BDF8" />,
    distance: "600 m",
    street: "Victoria Road",
    eta: "12:49 PM",
    remaining: "3.6 km",
    total: "5.8 km",
  },
  {
    direction: "TURN LEFT",
    arrowSvg: <ArrowLeft color="#38BDF8" />,
    distance: "80 m",
    street: "Park Street",
    eta: "12:52 PM",
    remaining: "2.8 km",
    total: "5.8 km",
  },
  {
    direction: "ROUNDABOUT",
    arrowSvg: <ArrowRoundabout color="#38BDF8" />,
    distance: "200 m",
    street: "Central Square",
    eta: "12:55 PM",
    remaining: "2.1 km",
    total: "5.8 km",
  },
  {
    direction: "SHARP RIGHT",
    arrowSvg: <ArrowHardRight color="#fbbf24" />,
    distance: "30 m",
    street: "Station Lane",
    eta: "12:57 PM",
    remaining: "0.6 km",
    total: "5.8 km",
  },
  {
    direction: "U-TURN",
    arrowSvg: <ArrowUTurn color="#ef4444" />,
    distance: "—",
    street: "Recalculating...",
    eta: "12:59 PM",
    remaining: "0.6 km",
    total: "5.8 km",
  },
  {
    direction: "DESTINATION",
    arrowSvg: <ArrowDestination color="#4ade80" />,
    distance: "50 m",
    street: "You have arrived",
    eta: "01:00 PM",
    remaining: "0.0 km",
    total: "5.8 km",
  },
];

export default function NavigationScreen({ state, dispatch }: Props) {
  const [stepIdx, setStepIdx] = useState(0);
  const step = NAV_STEPS[stepIdx];
  const isDestination = stepIdx === NAV_STEPS.length - 1;
  const remKm = parseFloat(step.remaining);
  const totalKm = parseFloat(step.total);
  const routePct = totalKm > 0 ? Math.max(0, Math.min(100, ((totalKm - remKm) / totalKm) * 100)) : 0;

  function next() { setStepIdx(i => (i + 1) % NAV_STEPS.length); }

  return (
    <div className="flex flex-col h-full font-dm" style={{ background: "#070A13" }}>
      <StatusBar
        state={state}
        label="NAVIGATION"
        right={
          <div className="flex items-center gap-1">
            <div className="w-1.5 h-1.5 rounded-full bg-[#4ade80]" style={{ boxShadow: "0 0 5px #4ade80", animation: "pulse 2s ease-in-out infinite" }} />
            <span className="font-mono-jb text-[8px] text-[#4ade80]">GPS ●</span>
          </div>
        }
      />

      <div className="flex-1 flex flex-col items-center justify-between px-4 py-3">
        {/* Distance + arrow + label */}
        <div
          className="flex-1 flex flex-col items-center justify-center gap-1 cursor-pointer interactive-tap w-full"
          onClick={next}
        >
          {/* Distance */}
          <div
            className="font-orbitron text-[11px] font-semibold tracking-[0.25em]"
            style={{ color: "rgba(255,255,255,0.4)" }}
          >
            {step.distance}
          </div>

          {/* Giant SVG arrow — key glanceable element */}
          <div
            className="flex items-center justify-center transition-all duration-300"
            style={{
              width: 100,
              height: 100,
              filter: `drop-shadow(0 0 16px ${isDestination ? "#4ade80" : "#38BDF8"}60)`,
              transform: "scale(1.3)",
            }}
          >
            {step.arrowSvg}
          </div>

          {/* Direction label */}
          <div
            className="font-orbitron text-[13px] font-black tracking-[0.15em] text-center"
            style={{ color: isDestination ? "#4ade80" : "#ffffff" }}
          >
            {step.direction}
          </div>

          {/* Street name */}
          <div className="font-dm text-[10px] font-medium text-white/50 text-center tracking-wide">
            {step.street}
          </div>

          <div className="font-mono-jb text-[7px] text-white/20 mt-1">TAP TO ADVANCE</div>
        </div>

        {/* Bottom info panel */}
        <div className="w-full space-y-2">
          <div className="w-full h-px" style={{ background: "rgba(255,255,255,0.06)" }} />

          {/* ETA + Distance row */}
          <div className="grid grid-cols-2 gap-2">
            <div className="rounded-lg px-2.5 py-2 text-center" style={{ background: "rgba(255,255,255,0.04)", border: "1px solid rgba(255,255,255,0.06)" }}>
              <div className="font-mono-jb text-[7px] text-white/30 uppercase mb-0.5">ETA</div>
              <div className="font-orbitron text-[15px] font-bold text-sky-400">{step.eta}</div>
            </div>
            <div className="rounded-lg px-2.5 py-2 text-center" style={{ background: "rgba(255,255,255,0.04)", border: "1px solid rgba(255,255,255,0.06)" }}>
              <div className="font-mono-jb text-[7px] text-white/30 uppercase mb-0.5">LEFT</div>
              <div className="font-orbitron text-[15px] font-bold text-white">{step.remaining}</div>
            </div>
          </div>

          {/* Route progress */}
          <div className="space-y-1">
            <div className="flex justify-between font-mono-jb text-[7px] text-white/20">
              <span>ROUTE</span>
              <span>{Math.round(routePct)}%</span>
            </div>
            <div className="h-1.5 rounded-full overflow-hidden" style={{ background: "rgba(255,255,255,0.06)" }}>
              <div
                className="h-full rounded-full transition-all duration-500"
                style={{ width: `${routePct}%`, background: "linear-gradient(90deg, #0284C7, #38BDF8)", boxShadow: "0 0 6px #38BDF8" }}
              />
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
