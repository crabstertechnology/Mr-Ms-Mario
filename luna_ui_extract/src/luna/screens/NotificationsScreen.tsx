import type React from "react";
import type { LunaState, LunaAction, LunaNotification, AppIcon } from "../types";
import StatusBar from "../components/StatusBar";

interface Props { state: LunaState; dispatch: React.Dispatch<LunaAction> }

const APP_META: Record<AppIcon, { abbr: string; label: string; color: string; bg: string; emoji: string }> = {
  WA:   { abbr: "WA",    label: "WhatsApp",   color: "#25D366", bg: "rgba(37,211,102,0.12)", emoji: "💬" },
  SLACK:{ abbr: "SL",    label: "Slack",      color: "#E01E5A", bg: "rgba(224,30,90,0.12)",  emoji: "💼" },
  MAIL: { abbr: "✉",     label: "Mail",       color: "#38BDF8", bg: "rgba(56,189,248,0.12)", emoji: "📧" },
  PHONE:{ abbr: "☎",     label: "Phone",      color: "#4ade80", bg: "rgba(74,222,128,0.12)", emoji: "📞" },
  CAL:  { abbr: "📅",    label: "Calendar",   color: "#F472B6", bg: "rgba(244,114,182,0.12)",emoji: "📅" },
};

function AppIcon({ app }: { app: AppIcon }) {
  const m = APP_META[app];
  return (
    <div
      className="w-8 h-8 rounded-xl flex items-center justify-center font-mono-jb font-bold text-[8px] flex-shrink-0"
      style={{ background: m.bg, color: m.color, border: `1px solid ${m.color}30` }}
    >
      {m.abbr}
    </div>
  );
}

function NotifRow({ notif, onTap }: { notif: LunaNotification; onTap: () => void }) {
  const m = APP_META[notif.app];
  return (
    <div
      className="interactive-tap cursor-pointer flex items-start gap-2.5 px-3 py-2.5 border-b border-white/[0.04] active:bg-white/[0.03] relative overflow-hidden"
      onClick={onTap}
    >
      {/* Unread left bar */}
      {!notif.isRead && (
        <div className="absolute left-0 top-2 bottom-2 w-0.5 rounded-r-full" style={{ background: m.color, boxShadow: `0 0 4px ${m.color}` }} />
      )}
      <AppIcon app={notif.app} />
      <div className="flex-1 min-w-0">
        <div className="flex items-baseline justify-between mb-0.5 gap-2">
          <span className={`font-dm text-[9px] font-semibold truncate ${notif.isRead ? "text-white/50" : "text-white"}`}>
            {notif.sender}
          </span>
          <span className="font-mono-jb text-[7px] text-white/25 flex-shrink-0">{notif.time}</span>
        </div>
        <p className={`font-dm text-[8px] leading-snug line-clamp-2 ${notif.isRead ? "text-white/25" : "text-white/55"}`}>
          {notif.preview}
        </p>
      </div>
      {!notif.isRead && (
        <div className="w-1.5 h-1.5 rounded-full flex-shrink-0 mt-1" style={{ background: m.color, boxShadow: `0 0 5px ${m.color}` }} />
      )}
    </div>
  );
}

function NotifDetail({ notif, onBack }: { notif: LunaNotification; onBack: () => void }) {
  const m = APP_META[notif.app];
  return (
    <div className="flex flex-col h-full font-dm">
      {/* Header */}
      <div className="flex items-center gap-3 px-3 py-2.5 border-b border-white/[0.06]">
        <button
          onClick={onBack}
          className="interactive-tap font-mono-jb text-[8px] text-sky-400 flex items-center gap-1"
        >
          ← BACK
        </button>
        <div className="flex items-center gap-1.5 flex-1">
          <div className="text-[10px]">{m.emoji}</div>
          <span className="font-mono-jb text-[7px] text-white/30">{m.label}</span>
        </div>
        <AppIcon app={notif.app} />
      </div>

      {/* Body */}
      <div className="flex-1 overflow-y-auto px-3 py-3 space-y-3">
        <div>
          <div className="font-dm text-[13px] font-bold text-white leading-tight">{notif.sender}</div>
          <div className="font-mono-jb text-[7px] text-white/30 mt-0.5">{notif.time}</div>
        </div>
        <div className="h-px bg-white/[0.06]" />
        <div
          className="rounded-xl p-3"
          style={{ background: m.bg, border: `1px solid ${m.color}20` }}
        >
          <p className="font-dm text-[9px] text-white/75 leading-relaxed">{notif.full}</p>
        </div>
      </div>

      {/* Actions */}
      <div className="px-3 py-2.5 border-t border-white/[0.06]">
        <button
          onClick={onBack}
          className="interactive-tap w-full py-2 rounded-xl font-mono-jb text-[8px] font-semibold"
          style={{ background: "rgba(56,189,248,0.1)", border: "1px solid rgba(56,189,248,0.25)", color: "#38BDF8" }}
        >
          DISMISS
        </button>
      </div>
    </div>
  );
}

export default function NotificationsScreen({ state, dispatch }: Props) {
  const unread = state.notifications.filter(n => !n.isRead).length;
  const selected = state.notifications.find(n => n.id === state.selectedNotifId);

  if (state.notifView === "detail" && selected) {
    return (
      <div className="flex flex-col h-full bg-black">
        <StatusBar state={state} label="MESSAGE" />
        <NotifDetail notif={selected} onBack={() => dispatch({ type: "CLOSE_NOTIF" })} />
      </div>
    );
  }

  return (
    <div className="flex flex-col h-full bg-black font-dm">
      <StatusBar
        state={state}
        label="NOTIFICATIONS"
        right={
          unread > 0 ? (
            <div className="flex items-center gap-1">
              <div
                className="w-4 h-4 rounded-full flex items-center justify-center"
                style={{ background: "#F472B6", boxShadow: "0 0 8px #F472B6" }}
              >
                <span className="font-pixel text-[5px] text-black">{unread}</span>
              </div>
              <span className="font-mono-jb text-[7px] text-[#F472B6]">NEW</span>
            </div>
          ) : undefined
        }
      />

      {/* Filter tabs */}
      <div className="flex gap-1 px-3 pt-2 pb-1">
        <div className="px-2 py-0.5 rounded font-mono-jb text-[7px]" style={{ background: "rgba(56,189,248,0.12)", color: "#38BDF8", border: "1px solid rgba(56,189,248,0.3)" }}>
          ALL ({state.notifications.length})
        </div>
        {unread > 0 && (
          <div className="px-2 py-0.5 rounded font-mono-jb text-[7px]" style={{ background: "rgba(244,114,182,0.08)", color: "#F472B6", border: "1px solid rgba(244,114,182,0.2)" }}>
            UNREAD ({unread})
          </div>
        )}
      </div>

      <div className="flex-1 overflow-y-auto">
        {state.notifications.length === 0 ? (
          <div className="flex flex-col items-center justify-center h-full gap-2 text-white/20">
            <span className="text-2xl">🔔</span>
            <span className="font-dm text-[9px]">All clear</span>
          </div>
        ) : (
          state.notifications.map(notif => (
            <NotifRow
              key={notif.id}
              notif={notif}
              onTap={() => dispatch({ type: "OPEN_NOTIF", id: notif.id })}
            />
          ))
        )}
      </div>

      <div className="px-3 py-1.5 border-t border-white/[0.04] flex justify-between items-center">
        <span className="font-mono-jb text-[6px] text-white/15">LONG PRESS · CLEAR ALL</span>
        <span className="font-mono-jb text-[6px] text-white/15">TAP · READ</span>
      </div>
    </div>
  );
}
