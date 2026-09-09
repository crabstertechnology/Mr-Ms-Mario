import type React from "react";
import { useState } from "react";
import type { LunaState, LunaAction, CalendarEvent } from "../types";
import StatusBar from "../components/StatusBar";

interface Props { state: LunaState; dispatch: React.Dispatch<LunaAction> }

const TYPE_COLOR: Record<string, string> = {
  video:      "#38BDF8",
  "in-person":"#4ade80",
  personal:   "#fbbf24",
  birthday:   "#F472B6",
};
const TYPE_ICON: Record<string, string> = {
  video: "📹", "in-person": "🏢", personal: "⭐", birthday: "🎂",
};

function EventDetail({ ev, onBack }: { ev: CalendarEvent; onBack: () => void }) {
  const color = TYPE_COLOR[ev.type];
  return (
    <div className="flex flex-col h-full font-dm">
      <div className="flex items-center gap-2 px-3 py-2 border-b border-white/[0.06]">
        <button onClick={onBack} className="font-mono-jb text-[8px] text-sky-400 interactive-tap">← BACK</button>
      </div>
      <div className="flex-1 px-3 py-3 space-y-3 overflow-y-auto">
        {/* Event header */}
        <div className="flex items-start gap-2.5">
          <div
            className="w-9 h-9 rounded-xl flex items-center justify-center text-[16px] flex-shrink-0 mt-0.5"
            style={{ background: color + "18", border: `1px solid ${color}30` }}
          >
            {TYPE_ICON[ev.type]}
          </div>
          <div>
            <div className="font-dm text-[13px] font-bold text-white leading-tight">{ev.title}</div>
            <div className="font-mono-jb text-[8px] mt-0.5" style={{ color }}>{ev.time} · {ev.duration}</div>
          </div>
        </div>

        <div className="h-px bg-white/[0.06]" />

        {/* Details */}
        <div className="space-y-2">
          {ev.location && (
            <div className="flex items-center gap-2 rounded-lg px-2.5 py-2" style={{ background: "rgba(255,255,255,0.04)", border: "1px solid rgba(255,255,255,0.06)" }}>
              <span className="text-[11px]">📍</span>
              <span className="font-dm text-[9px] text-white/70">{ev.location}</span>
            </div>
          )}
          <div className="flex items-center gap-2 rounded-lg px-2.5 py-2" style={{ background: "rgba(255,255,255,0.04)", border: "1px solid rgba(255,255,255,0.06)" }}>
            <span className="text-[11px]">⏱</span>
            <span className="font-dm text-[9px] text-white/70">{ev.duration}</span>
          </div>
          <div className="flex items-center gap-2 rounded-lg px-2.5 py-2" style={{ background: ev.completed ? "rgba(74,222,128,0.06)" : "rgba(251,191,36,0.06)", border: `1px solid ${ev.completed ? "rgba(74,222,128,0.2)" : "rgba(251,191,36,0.15)"}` }}>
            <span className="text-[11px]">{ev.completed ? "✅" : "🔔"}</span>
            <span className="font-mono-jb text-[8px]" style={{ color: ev.completed ? "#4ade80" : "#fbbf24" }}>
              {ev.completed ? "COMPLETED" : "UPCOMING"}
            </span>
          </div>
        </div>
      </div>
    </div>
  );
}

function Timeline({ events }: { events: CalendarEvent[] }) {
  const [selected, setSelected] = useState<number | null>(null);
  const ev = events.find(e => e.id === selected);
  if (ev) return <EventDetail ev={ev} onBack={() => setSelected(null)} />;

  return (
    <div className="flex-1 overflow-y-auto">
      {events.map(ev => {
        const color = TYPE_COLOR[ev.type];
        const isCurrent = !!ev.isCurrent;
        return (
          <div
            key={ev.id}
            className="interactive-tap cursor-pointer flex gap-2 px-3 py-2.5 border-b border-white/[0.04] active:bg-white/[0.03]"
            style={{ background: isCurrent ? "rgba(56,189,248,0.03)" : undefined }}
            onClick={() => setSelected(ev.id)}
          >
            {/* Time column */}
            <div className="w-10 flex-shrink-0 pt-0.5">
              <div className="font-mono-jb text-[8px] text-white/35 text-right">{ev.time}</div>
            </div>

            {/* Timeline line + dot */}
            <div className="flex flex-col items-center w-4 flex-shrink-0">
              <div
                className="w-2.5 h-2.5 rounded-full flex-shrink-0 mt-0.5"
                style={{
                  background: ev.completed ? "rgba(255,255,255,0.15)" : color,
                  boxShadow: ev.completed ? "none" : `0 0 ${isCurrent ? "8px" : "4px"} ${color}`,
                  border: ev.completed ? `1px solid ${color}40` : "none",
                }}
              />
              <div className="flex-1 w-px mt-1" style={{ background: color + "20", minHeight: 8 }} />
            </div>

            {/* Content */}
            <div className="flex-1 min-w-0">
              <div className={`font-dm text-[9px] font-semibold leading-tight ${ev.completed ? "text-white/25 line-through" : isCurrent ? "text-white" : "text-white/70"}`}>
                {ev.title}
              </div>
              <div className="flex items-center gap-1.5 mt-0.5 flex-wrap">
                {ev.location && (
                  <span className="font-dm text-[7px] text-white/25">{ev.location}</span>
                )}
                {ev.location && <span className="text-white/15 text-[7px]">·</span>}
                <span className="font-mono-jb text-[7px]" style={{ color: color + "80" }}>{ev.duration}</span>
              </div>
            </div>

            {/* Current indicator */}
            {isCurrent && (
              <div
                className="w-5 h-5 rounded-full flex items-center justify-center flex-shrink-0"
                style={{ background: "rgba(56,189,248,0.15)", border: "1px solid rgba(56,189,248,0.4)" }}
              >
                <span className="text-[9px]">{TYPE_ICON[ev.type]}</span>
              </div>
            )}
          </div>
        );
      })}
      <div className="px-3 py-2">
        <div className="font-mono-jb text-[7px] text-white/15 text-center">+ MORE EVENTS TODAY</div>
      </div>
    </div>
  );
}

function MonthGrid({ time }: { time: Date }) {
  const MONTHS = ["JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"];
  const year = time.getFullYear(), month = time.getMonth(), today = time.getDate();
  const firstDay = new Date(year, month, 1).getDay();
  const daysInMonth = new Date(year, month + 1, 0).getDate();
  const busyDays = new Set([2, 5, 9, 11, 14, 18, 22, 25, 28]);

  const cells: (number | null)[] = [];
  for (let i = 0; i < firstDay; i++) cells.push(null);
  for (let i = 1; i <= daysInMonth; i++) cells.push(i);

  return (
    <div className="flex-1 overflow-y-auto px-3 py-2">
      <div className="flex items-center justify-between mb-3">
        <span className="font-orbitron text-[12px] font-black text-white tracking-wider">{MONTHS[month]}</span>
        <span className="font-mono-jb text-[9px] text-white/30">{year}</span>
      </div>
      <div className="grid grid-cols-7 gap-y-0.5">
        {["S","M","T","W","T","F","S"].map((d, i) => (
          <div key={i} className="text-center font-mono-jb text-[7px] text-white/25 pb-1.5">{d}</div>
        ))}
        {cells.map((day, i) => (
          <div key={i} className="flex items-center justify-center py-0.5">
            {day !== null && (
              <div
                className="w-6 h-6 flex items-center justify-center rounded-full relative cursor-pointer interactive-tap"
                style={{
                  background: day === today ? "#38BDF8" : "transparent",
                  boxShadow: day === today ? "0 0 10px #38BDF8" : "none",
                }}
              >
                <span className={`font-mono-jb text-[8px] ${day === today ? "text-black font-bold" : "text-white/50"}`}>{day}</span>
                {busyDays.has(day) && day !== today && (
                  <div className="absolute bottom-0.5 left-1/2 -translate-x-1/2 w-1 h-1 rounded-full bg-sky-400/60" />
                )}
              </div>
            )}
          </div>
        ))}
      </div>
    </div>
  );
}

export default function CalendarScreen({ state, dispatch }: Props) {
  const MONTHS = ["JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"];
  const d = state.currentTime;
  const day = d.getDate().toString().padStart(2, "0");
  const month = MONTHS[d.getMonth()];
  const upcoming = state.calEvents.filter(e => !e.completed).length;

  return (
    <div className="flex flex-col h-full bg-black font-dm">
      <StatusBar state={state} label="CALENDAR" />

      {/* Header */}
      <div className="flex items-center justify-between px-3 py-2 border-b border-white/[0.06]">
        <div>
          <div className="font-orbitron text-[9px] font-bold text-sky-400 tracking-[0.2em]">
            TODAY · {day} {month}
          </div>
          <div className="font-mono-jb text-[7px] text-white/25 mt-0.5">{upcoming} UPCOMING</div>
        </div>
        <button
          onClick={() => dispatch({ type: "TOGGLE_CAL_VIEW" })}
          className="interactive-tap font-mono-jb text-[7px] px-2.5 py-1 rounded-lg"
          style={{
            background: "rgba(56,189,248,0.08)",
            border: "1px solid rgba(56,189,248,0.25)",
            color: "#38BDF8",
          }}
        >
          {state.calView === "timeline" ? "MONTH" : "TODAY"}
        </button>
      </div>

      {/* Content */}
      <div className="flex-1 overflow-hidden flex flex-col">
        {state.calView === "timeline"
          ? <Timeline events={state.calEvents} />
          : <MonthGrid time={state.currentTime} />
        }
      </div>

      <div className="px-3 py-1 border-t border-white/[0.04]">
        <div className="font-mono-jb text-[6px] text-white/15 text-center">TAP · TOGGLE · TAP EVENT · DETAILS</div>
      </div>
    </div>
  );
}
