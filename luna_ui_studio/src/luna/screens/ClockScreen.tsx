import type React from "react";
import type { LunaState, LunaAction } from "../types";
import StatusBar from "../components/StatusBar";
import LunaMascot from "../components/LunaMascot";

interface Props { state: LunaState; dispatch: React.Dispatch<LunaAction> }

function pad(n: number) { return n.toString().padStart(2, "0"); }

// ─── Style 0: Minimal Digital ───────────────────────────────────────────────
function MinimalClock({ time }: { time: Date }) {
  const h = pad(time.getHours());
  const m = pad(time.getMinutes());
  const s = pad(time.getSeconds());
  const ampm = time.getHours() >= 12 ? "PM" : "AM";
  return (
    <div className="flex flex-col items-center gap-1">
      <div className="flex items-end">
        <span
          className="font-orbitron font-black leading-none text-white"
          style={{ fontSize: 58, letterSpacing: "-2px", textShadow: "0 0 40px rgba(56,189,248,0.25)" }}
        >
          {h}:{m}
        </span>
        <div className="flex flex-col items-start mb-2 ml-1.5 gap-0.5">
          <span className="font-orbitron text-[18px] font-bold text-sky-400">:{s}</span>
          <span className="font-mono-jb text-[9px] text-white/30">{ampm}</span>
        </div>
      </div>
    </div>
  );
}

// ─── Style 1: Retro 8-Bit ───────────────────────────────────────────────────
function RetroClock({ time }: { time: Date }) {
  const h = pad(time.getHours());
  const m = pad(time.getMinutes());
  const s = pad(time.getSeconds());
  const blinkColon = time.getSeconds() % 2 === 0;
  const DAYS_SHORT = ["SUN","MON","TUE","WED","THU","FRI","SAT"];
  const curDay = time.getDay();
  return (
    <div
      className="crt-effect relative w-full rounded-lg overflow-hidden"
      style={{ background: "#000a00", border: "1px solid #00ff0018", boxShadow: "0 0 30px #00ff0012, inset 0 0 30px #00ff0006" }}
    >
      {/* Scanline overlay handled by CSS */}
      <div className="p-3 flex flex-col items-center gap-2">
        <div className="font-pixel text-[7px] tracking-[0.25em]" style={{ color: "#00cc00" }}>
          ◂ LUNA-OS v1.0 ▸
        </div>
        <div className="font-pixel leading-none flex items-center gap-1">
          <span style={{ fontSize: 34, color: "#00ff00", textShadow: "0 0 8px #00ff00, 0 0 20px #00ff0040" }}>{h}</span>
          <span style={{ fontSize: 34, color: blinkColon ? "#00ff00" : "#003300", textShadow: blinkColon ? "0 0 8px #00ff00" : "none" }}>:</span>
          <span style={{ fontSize: 34, color: "#00ff00", textShadow: "0 0 8px #00ff00, 0 0 20px #00ff0040" }}>{m}</span>
          <span style={{ fontSize: 18, color: "#00aa00", textShadow: "0 0 6px #00aa00", marginLeft: 4 }}>:{s}</span>
        </div>
        {/* Day indicator */}
        <div className="flex gap-1.5">
          {DAYS_SHORT.map((d, i) => (
            <span
              key={d}
              className="font-pixel text-[5px]"
              style={{ color: i === curDay ? "#00ff00" : "#003300", textShadow: i === curDay ? "0 0 4px #00ff00" : "none" }}
            >
              {d}
            </span>
          ))}
        </div>
        {/* Battery bar simulation */}
        <div className="flex items-center gap-1.5">
          <span className="font-pixel text-[5px]" style={{ color: "#00aa00" }}>BAT</span>
          <div className="flex gap-0.5">
            {[...Array(10)].map((_, i) => (
              <div key={i} className="w-1.5 h-2" style={{ background: i < 8 ? "#00ff00" : "#003300", boxShadow: i < 8 ? "0 0 2px #00ff00" : "none" }} />
            ))}
          </div>
          <div className="w-0.5 h-1.5 rounded-r-sm" style={{ background: "#00aa00" }} />
        </div>
      </div>
    </div>
  );
}

// ─── Style 2: Cyberpunk Meter ────────────────────────────────────────────────
function CyberpunkClock({ time }: { time: Date }) {
  const h = time.getHours();
  const m = time.getMinutes();
  const s = time.getSeconds();
  const hourPct = ((h % 12) / 12) * 100;
  const minPct = (m / 60) * 100;
  const secPct = (s / 60) * 100;

  function Gauge({ label, value, pct, color, unit }: { label: string; value: number; pct: number; color: string; unit: string }) {
    return (
      <div className="flex items-center gap-2">
        <span className="font-mono-jb text-[6px] text-white/30 w-4 uppercase">{label}</span>
        <div className="flex-1 relative h-2.5 rounded-full overflow-hidden" style={{ background: "rgba(255,255,255,0.04)", border: "1px solid rgba(255,255,255,0.06)" }}>
          <div
            className="absolute inset-y-0 left-0 rounded-full transition-all duration-500"
            style={{
              width: `${pct}%`,
              background: `linear-gradient(90deg, ${color}60, ${color})`,
              boxShadow: `0 0 8px ${color}`,
            }}
          />
          {/* Tick marks */}
          {[25, 50, 75].map(t => (
            <div key={t} className="absolute top-0 bottom-0 w-px" style={{ left: `${t}%`, background: "rgba(0,0,0,0.4)" }} />
          ))}
        </div>
        <span className="font-orbitron text-[10px] font-bold w-6 text-right" style={{ color }}>{pad(value)}<span className="font-mono-jb text-[6px] text-white/30">{unit}</span></span>
      </div>
    );
  }

  return (
    <div className="w-full flex flex-col items-center gap-2">
      {/* Large time */}
      <div className="flex items-baseline gap-1">
        <span className="font-orbitron font-black text-[46px] leading-none text-sky-400" style={{ textShadow: "0 0 20px #38BDF8, 0 0 40px #38BDF840", letterSpacing: "-1px" }}>
          {pad(h)}:{pad(m)}
        </span>
        <div className="flex flex-col mb-1">
          <span className="font-orbitron text-[14px] font-bold text-[#fbbf24]">:{pad(s)}</span>
        </div>
      </div>
      {/* Angular decorative bracket */}
      <div className="flex items-center gap-2 w-full">
        <div className="flex-1 h-px" style={{ background: "linear-gradient(90deg, transparent, #38BDF840)" }} />
        <span className="font-pixel text-[6px] text-sky-400/50">CHRONO</span>
        <div className="flex-1 h-px" style={{ background: "linear-gradient(90deg, #38BDF840, transparent)" }} />
      </div>
      {/* Gauges */}
      <div className="w-full space-y-1.5">
        <Gauge label="HR" value={h % 12 || 12} pct={hourPct} color="#F472B6" unit="h" />
        <Gauge label="MIN" value={m} pct={minPct} color="#38BDF8" unit="m" />
        <Gauge label="SEC" value={s} pct={secPct} color="#fbbf24" unit="s" />
      </div>
    </div>
  );
}

// ─── Style 3: Analog Designer ───────────────────────────────────────────────
function AnalogClock({ time }: { time: Date }) {
  const h = time.getHours() % 12 + time.getMinutes() / 60;
  const m = time.getMinutes() + time.getSeconds() / 60;
  const s = time.getSeconds() + (Date.now() % 1000) / 1000;
  const hDeg = h * 30;
  const mDeg = m * 6;
  const sDeg = s * 6;
  const R = 68, cx = 75, cy = 75;

  function hand(deg: number, len: number, color: string, w: number, glow?: boolean) {
    const a = (deg - 90) * (Math.PI / 180);
    const x = cx + len * Math.cos(a);
    const y = cy + len * Math.sin(a);
    // Counter-weight
    const cx2 = cx + (len * 0.2) * Math.cos(a + Math.PI);
    const cy2 = cy + (len * 0.2) * Math.sin(a + Math.PI);
    return (
      <g>
        {glow && <line x1={cx2} y1={cy2} x2={x} y2={y} stroke={color} strokeWidth={w + 2} strokeLinecap="round" opacity="0.25" style={{ filter: `blur(3px)` }} />}
        <line x1={cx2} y1={cy2} x2={x} y2={y} stroke={color} strokeWidth={w} strokeLinecap="round" />
      </g>
    );
  }

  return (
    <div className="flex flex-col items-center">
      <svg width={150} height={150} viewBox="0 0 150 150">
        {/* Outer ring */}
        <circle cx={cx} cy={cy} r={R} fill="none" stroke="rgba(56,189,248,0.12)" strokeWidth="1.5" />
        <circle cx={cx} cy={cy} r={R - 8} fill="rgba(56,189,248,0.02)" stroke="rgba(56,189,248,0.05)" strokeWidth="0.5" />

        {/* Hour ticks */}
        {[...Array(12)].map((_, i) => {
          const a = (i * 30 - 90) * (Math.PI / 180);
          const isMajor = i % 3 === 0;
          const outer = R - 1, inner = R - (isMajor ? 10 : 5);
          return (
            <line
              key={i}
              x1={cx + outer * Math.cos(a)} y1={cy + outer * Math.sin(a)}
              x2={cx + inner * Math.cos(a)} y2={cy + inner * Math.sin(a)}
              stroke={isMajor ? "rgba(56,189,248,0.9)" : "rgba(56,189,248,0.3)"}
              strokeWidth={isMajor ? 2 : 1}
              strokeLinecap="round"
            />
          );
        })}

        {/* Minute ticks */}
        {[...Array(60)].map((_, i) => {
          if (i % 5 === 0) return null;
          const a = (i * 6 - 90) * (Math.PI / 180);
          return <line key={i} x1={cx + (R-1) * Math.cos(a)} y1={cy + (R-1) * Math.sin(a)} x2={cx + (R-3.5) * Math.cos(a)} y2={cy + (R-3.5) * Math.sin(a)} stroke="rgba(56,189,248,0.15)" strokeWidth={0.5} />;
        })}

        {/* Hour numbers (cardinal) */}
        {[12, 3, 6, 9].map((n, i) => {
          const a = (i * 90 - 90) * (Math.PI / 180);
          const r = R - 20;
          return (
            <text key={n} x={cx + r * Math.cos(a)} y={cy + r * Math.sin(a) + 3} textAnchor="middle" fill="rgba(56,189,248,0.6)" fontSize="8" fontFamily="Orbitron, monospace" fontWeight="700">{n}</text>
          );
        })}

        {/* Hands */}
        {hand(hDeg, 38, "#ffffff", 3, true)}
        {hand(mDeg, 52, "#38BDF8", 2, true)}
        {hand(sDeg, 58, "#F472B6", 1, true)}

        {/* Center cap */}
        <circle cx={cx} cy={cy} r={5} fill="#070A13" stroke="#38BDF8" strokeWidth="1.5" style={{ filter: "drop-shadow(0 0 4px #38BDF8)" }} />
        <circle cx={cx} cy={cy} r={2.5} fill="#38BDF8" />
      </svg>
    </div>
  );
}

const STYLE_NAMES = ["MINIMAL", "8-BIT", "CYBER", "ANALOG"];
const DAYS = ["SUN","MON","TUE","WED","THU","FRI","SAT"];
const MONTHS = ["JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"];

export default function ClockScreen({ state, dispatch }: Props) {
  const { currentTime, clockStyle, steps, temperature, partnerOnline, notifications, calEvents, batteryLevel } = state;
  const unread = notifications.filter(n => !n.isRead).length;
  const nextEvent = calEvents.find(e => !e.completed && e.hour > currentTime.getHours());
  const day = DAYS[currentTime.getDay()];
  const date = currentTime.getDate().toString().padStart(2, "0");
  const month = MONTHS[currentTime.getMonth()];
  const year = currentTime.getFullYear();
  const kcal = Math.round(steps * 0.059);

  return (
    <div className="flex flex-col h-full bg-black font-dm">
      <StatusBar state={state} label={STYLE_NAMES[clockStyle]} />

      {/* Main clock area — tappable to cycle */}
      <div
        className="flex-1 flex flex-col px-3.5 pt-3 pb-0 gap-2.5 cursor-pointer interactive-tap select-none"
        onClick={() => dispatch({ type: "CYCLE_CLOCK_STYLE" })}
      >
        {/* Clock face */}
        <div className="flex-1 flex flex-col items-center justify-center min-h-0">
          {clockStyle === 0 && <MinimalClock time={currentTime} />}
          {clockStyle === 1 && <RetroClock time={currentTime} />}
          {clockStyle === 2 && <CyberpunkClock time={currentTime} />}
          {clockStyle === 3 && <AnalogClock time={currentTime} />}

          {/* Date strip */}
          <div className="flex items-center gap-1.5 mt-2">
            <span className="font-orbitron text-[9px] font-bold text-sky-400 tracking-[0.2em]">{day}</span>
            <span className="w-0.5 h-0.5 rounded-full bg-white/20" />
            <span className="font-mono-jb text-[9px] text-white/50">{date} {month} {year}</span>
          </div>

          {/* Style indicator dots */}
          <div className="flex gap-1 mt-2">
            {[0, 1, 2, 3].map(i => (
              <div
                key={i}
                className="rounded-full transition-all duration-200"
                style={{
                  width: i === clockStyle ? 10 : 4,
                  height: 4,
                  background: i === clockStyle ? "#38BDF8" : "rgba(255,255,255,0.12)",
                  boxShadow: i === clockStyle ? "0 0 6px #38BDF8" : "none",
                }}
              />
            ))}
          </div>
        </div>

        {/* Stats grid */}
        <div
          className="rounded-xl p-2.5"
          style={{ background: "rgba(255,255,255,0.03)", border: "1px solid rgba(255,255,255,0.06)" }}
        >
          <div className="grid grid-cols-2 gap-x-4 gap-y-2">
            {[
              { icon: "👟", value: steps.toLocaleString(), label: "STEPS",   color: "#38BDF8" },
              { icon: "🔥", value: `${kcal}`,             label: "KCAL",    color: "#fbbf24" },
              { icon: "🌡️", value: `${temperature}°C`,    label: "TEMP",    color: "#4ade80" },
              { icon: null, value: partnerOnline ? "ONLINE" : "AWAY", label: "PARTNER", color: partnerOnline ? "#F472B6" : "rgba(255,255,255,0.2)" },
            ].map(({ icon, value, label, color }) => (
              <div key={label} className="flex items-center gap-1.5">
                {icon
                  ? <span className="text-[10px] w-4">{icon}</span>
                  : <div className="w-2 h-2 rounded-full flex-shrink-0 mt-px" style={{ background: color, boxShadow: partnerOnline ? `0 0 6px ${color}` : "none" }} />
                }
                <div>
                  <div className="font-orbitron text-[10px] font-bold leading-none" style={{ color }}>{value}</div>
                  <div className="font-mono-jb text-[6px] text-white/25 uppercase tracking-wider">{label}</div>
                </div>
              </div>
            ))}
          </div>
        </div>

        {/* Next event */}
        {nextEvent ? (
          <div className="flex items-center gap-2 pb-0.5">
            <div className="w-0.5 h-6 rounded-full bg-sky-400 flex-shrink-0" style={{ boxShadow: "0 0 4px #38BDF8" }} />
            <div className="flex-1 min-w-0">
              <div className="font-mono-jb text-[7px] text-white/30 uppercase">NEXT EVENT</div>
              <div className="font-dm text-[9px] text-white font-medium truncate">{nextEvent.time} · {nextEvent.title}</div>
            </div>
            {unread > 0 && (
              <div className="w-5 h-5 rounded-full flex items-center justify-center flex-shrink-0" style={{ background: "#F472B6", boxShadow: "0 0 8px #F472B6" }}>
                <span className="font-pixel text-[5px] text-black font-black">{unread}</span>
              </div>
            )}
          </div>
        ) : (
          <div className="flex items-center justify-center pb-0.5">
            <LunaMascot mood="default" size="xs" animate />
          </div>
        )}
      </div>

      {/* Bottom hint */}
      <div className="px-3 py-1.5 border-t border-white/[0.04] flex justify-between items-center">
        <span className="font-mono-jb text-[6px] text-white/15">TAP · CHANGE STYLE</span>
        <span className="font-mono-jb text-[6px] text-white/15">← SWIPE →</span>
      </div>
    </div>
  );
}
