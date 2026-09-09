import type React from "react";
import { useEffect, useRef, useState, useCallback } from "react";
import type { LunaState, LunaAction } from "../types";
import StatusBar from "../components/StatusBar";

interface Props { state: LunaState; dispatch: React.Dispatch<LunaAction> }

function clamp(v: number, min: number, max: number) { return Math.max(min, Math.min(max, v)); }
function lerp(a: number, b: number, t: number) { return a + (b - a) * t; }

export default function SpiritLevelScreen({ state, dispatch }: Props) {
  const containerRef = useRef<HTMLDivElement>(null);
  const targetPos = useRef({ x: 0, y: 0 });
  const smoothPos = useRef({ x: 0, y: 0 });
  const animRef = useRef<number | null>(null);
  const [renderPos, setRenderPos] = useState({ x: 0, y: 0 });

  const onMouseMove = useCallback((e: MouseEvent) => {
    const el = containerRef.current;
    if (!el) return;
    const rect = el.getBoundingClientRect();
    const cx = rect.left + rect.width / 2;
    const cy = rect.top + rect.height / 2;
    const dx = clamp((e.clientX - cx) / (rect.width / 2.5), -1, 1);
    const dy = clamp((e.clientY - cy) / (rect.height / 2.5), -1, 1);
    targetPos.current = { x: dx, y: dy };
    dispatch({ type: "UPDATE_LEVEL", pitch: dy * 15, roll: dx * 15 });
  }, [dispatch]);

  useEffect(() => {
    window.addEventListener("mousemove", onMouseMove);
    return () => window.removeEventListener("mousemove", onMouseMove);
  }, [onMouseMove]);

  useEffect(() => {
    function tick() {
      const t = targetPos.current, s = smoothPos.current;
      const nx = lerp(s.x, t.x, 0.07);
      const ny = lerp(s.y, t.y, 0.07);
      smoothPos.current = { x: nx, y: ny };
      setRenderPos({ x: nx, y: ny });
      animRef.current = requestAnimationFrame(tick);
    }
    animRef.current = requestAnimationFrame(tick);
    return () => { if (animRef.current) cancelAnimationFrame(animRef.current); };
  }, []);

  const { pitch, roll, isLeveled, calibrating, calibStep } = state;
  const MAX = 38;
  const bx = renderPos.x * MAX;
  const by = renderPos.y * MAX;
  const cx = 76, cy = 76, R = 70;

  function statusLabel() {
    if (isLeveled) return "PERFECTLY LEVEL";
    const ap = Math.abs(pitch), ar = Math.abs(roll);
    if (ap > ar) return pitch > 0 ? "TILT FORWARD ↓" : "TILT BACK ↑";
    return roll > 0 ? "TILT RIGHT →" : "TILT LEFT ←";
  }

  if (calibrating) {
    return (
      <div className="flex flex-col h-full bg-black font-dm">
        <StatusBar state={state} label="6-AXIS LEVEL" />
        <div className="flex-1 flex flex-col items-center justify-center gap-4">
          <div className="relative">
            <div className="w-20 h-20 rounded-full border-2 border-sky-400/30 absolute inset-0 animate-calibrate" />
            <div className="w-20 h-20 rounded-full border border-sky-400/15 absolute inset-0 animate-pulse" />
            <div className="w-20 h-20 rounded-full flex items-center justify-center">
              <span className="font-orbitron text-[28px] font-black text-sky-400 animate-calibrate">
                {calibStep}
              </span>
            </div>
          </div>
          <div className="flex flex-col items-center gap-1">
            <div className="font-orbitron text-[10px] font-bold text-sky-400 tracking-[0.2em] animate-calibrate">CALIBRATING...</div>
            <div className="font-dm text-[9px] text-white/30">HOLD DEVICE PERFECTLY STILL</div>
          </div>
          <div className="w-32 h-1 rounded-full overflow-hidden bg-white/10">
            <div
              className="h-full rounded-full bg-sky-400 transition-all duration-900"
              style={{ width: `${((3 - calibStep) / 3) * 100}%`, boxShadow: "0 0 6px #38BDF8" }}
            />
          </div>
          <div className="font-mono-jb text-[8px] text-sky-400/60">
            {calibStep > 0 ? `ZEROING IN ${calibStep}...` : "✓ ZEROED"}
          </div>
        </div>
      </div>
    );
  }

  return (
    <div className="flex flex-col h-full bg-black font-dm" ref={containerRef}>
      <StatusBar
        state={state}
        label="6-AXIS LEVEL"
        right={
          <button
            onClick={() => dispatch({ type: "CALIBRATE_START" })}
            className="interactive-tap font-mono-jb text-[7px] text-sky-400 px-2 py-0.5 rounded"
            style={{ background: "rgba(56,189,248,0.1)", border: "1px solid rgba(56,189,248,0.3)" }}
          >
            ZERO
          </button>
        }
      />

      <div className="flex-1 flex flex-col items-center justify-between px-3 py-3">
        {/* Main instrument */}
        <div className="flex-1 flex items-center justify-center">
          <div className="relative" style={{ width: 152, height: 152 }}>
            <svg width="152" height="152" className="absolute inset-0">
              {/* Outer ring */}
              <circle cx={cx} cy={cy} r={R} fill="none" stroke="rgba(56,189,248,0.15)" strokeWidth="1.5"/>
              {/* Inner rings */}
              {[55, 40, 25].map(r => (
                <circle key={r} cx={cx} cy={cy} r={r} fill="none" stroke="rgba(56,189,248,0.06)" strokeWidth="0.5"/>
              ))}
              {/* Crosshair */}
              <line x1={cx} y1={cy - R} x2={cx} y2={cy + R} stroke="rgba(56,189,248,0.15)" strokeWidth="0.5"/>
              <line x1={cx - R} y1={cy} x2={cx + R} y2={cy} stroke="rgba(56,189,248,0.15)" strokeWidth="0.5"/>
              {/* Target ring (level zone) */}
              <circle
                cx={cx} cy={cy} r={10}
                fill={isLeveled ? "rgba(74,222,128,0.08)" : "none"}
                stroke={isLeveled ? "#4ade80" : "rgba(56,189,248,0.4)"}
                strokeWidth="1.5"
                style={{ filter: isLeveled ? "drop-shadow(0 0 6px #4ade80)" : undefined, transition: "all 0.4s ease" }}
              />
              {/* Degree markings */}
              {[0, 90, 180, 270].map(deg => {
                const a = (deg - 90) * Math.PI / 180;
                const x1 = cx + (R - 2) * Math.cos(a), y1 = cy + (R - 2) * Math.sin(a);
                const x2 = cx + (R - 10) * Math.cos(a), y2 = cy + (R - 10) * Math.sin(a);
                return <line key={deg} x1={x1} y1={y1} x2={x2} y2={y2} stroke="rgba(56,189,248,0.5)" strokeWidth="1.5" strokeLinecap="round"/>;
              })}
              {[45, 135, 225, 315].map(deg => {
                const a = (deg - 90) * Math.PI / 180;
                const x1 = cx + (R - 2) * Math.cos(a), y1 = cy + (R - 2) * Math.sin(a);
                const x2 = cx + (R - 6) * Math.cos(a), y2 = cy + (R - 6) * Math.sin(a);
                return <line key={deg} x1={x1} y1={y1} x2={x2} y2={y2} stroke="rgba(56,189,248,0.2)" strokeWidth="1" strokeLinecap="round"/>;
              })}
              {/* Cardinal labels */}
              {[{l:"N",d:0},{l:"E",d:90},{l:"S",d:180},{l:"W",d:270}].map(({l,d}) => {
                const a = (d - 90) * Math.PI / 180;
                return <text key={l} x={cx + (R + 6) * Math.cos(a) + (l === "N" || l === "S" ? 0 : l === "E" ? 1 : -1)} y={cy + (R + 6) * Math.sin(a) + 3} textAnchor="middle" fill="rgba(56,189,248,0.45)" fontSize="7" fontFamily="JetBrains Mono, monospace" fontWeight="600">{l}</text>;
              })}
            </svg>

            {/* Bubble */}
            <div
              style={{
                position: "absolute",
                left: cx - 11 + bx,
                top: cy - 11 + by,
                width: 22,
                height: 22,
                borderRadius: "50%",
                background: isLeveled
                  ? "radial-gradient(circle at 38% 35%, #86efac, #4ade80 60%, #16a34a)"
                  : "radial-gradient(circle at 38% 35%, #7dd3fc, #38BDF8 60%, #0284C7)",
                boxShadow: isLeveled
                  ? "0 0 14px #4ade80, 0 0 28px #4ade8050"
                  : "0 0 12px #38BDF8, 0 0 24px #38BDF840",
                border: "1.5px solid rgba(255,255,255,0.5)",
                transition: "background 0.5s ease, box-shadow 0.5s ease",
              }}
            >
              {/* Bubble highlight */}
              <div
                style={{
                  position: "absolute",
                  top: 3, left: 3,
                  width: 8, height: 6,
                  borderRadius: "50%",
                  background: "rgba(255,255,255,0.5)",
                  filter: "blur(1px)",
                }}
              />
            </div>
          </div>
        </div>

        {/* Readout */}
        <div className="w-full space-y-2">
          {/* Pitch / Roll */}
          <div className="grid grid-cols-2 gap-2">
            {[
              { label: "PITCH", value: pitch, unit: "°", axis: "Y" },
              { label: "ROLL",  value: roll,  unit: "°", axis: "X" },
            ].map(({ label, value, unit, axis }) => {
              const ok = Math.abs(value) < 0.5;
              return (
                <div key={label} className="rounded-lg px-2.5 py-2" style={{ background: "rgba(255,255,255,0.04)", border: `1px solid ${ok ? "rgba(74,222,128,0.2)" : "rgba(255,255,255,0.06)"}` }}>
                  <div className="flex items-center justify-between mb-0.5">
                    <span className="font-mono-jb text-[6px] text-white/30 uppercase">{label}</span>
                    <span className="font-mono-jb text-[6px] text-white/20">{axis}</span>
                  </div>
                  <div className="font-orbitron text-[14px] font-bold leading-none" style={{ color: ok ? "#4ade80" : "white" }}>
                    {value >= 0 ? "+" : ""}{value.toFixed(1)}{unit}
                  </div>
                </div>
              );
            })}
          </div>

          {/* Status */}
          <div
            className="flex items-center justify-center gap-2 py-1.5 rounded-lg transition-all duration-400"
            style={{
              background: isLeveled ? "rgba(74,222,128,0.06)" : "rgba(251,191,36,0.06)",
              border: `1px solid ${isLeveled ? "rgba(74,222,128,0.2)" : "rgba(251,191,36,0.15)"}`,
            }}
          >
            <div
              className="w-2 h-2 rounded-full"
              style={{
                background: isLeveled ? "#4ade80" : "#fbbf24",
                boxShadow: `0 0 8px ${isLeveled ? "#4ade80" : "#fbbf24"}`,
                transition: "all 0.4s ease",
              }}
            />
            <span
              className="font-orbitron text-[9px] font-bold tracking-[0.1em]"
              style={{ color: isLeveled ? "#4ade80" : "#fbbf24", transition: "color 0.4s ease" }}
            >
              {statusLabel()}
            </span>
          </div>

          <div className="font-mono-jb text-[6px] text-white/15 text-center">
            MOVE MOUSE TO SIMULATE IMU · TAP ZERO TO CALIBRATE
          </div>
        </div>
      </div>
    </div>
  );
}
