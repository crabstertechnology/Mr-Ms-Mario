import type React from "react";
import type { LunaState, LunaAction, LunaSettings } from "../types";
import StatusBar from "../components/StatusBar";

interface Props { state: LunaState; dispatch: React.Dispatch<LunaAction> }

interface SettingRow {
  key: keyof LunaSettings;
  label: string;
  icon: string;
  type: "toggle" | "select" | "number" | "action";
  options?: string[];
  min?: number;
  max?: number;
  step?: number;
  unit?: string;
  dangerous?: boolean;
}

const SETTINGS_DEFS: SettingRow[] = [
  { key: "bluetooth",     label: "Bluetooth BLE",       icon: "📡", type: "toggle" },
  { key: "brightness",    label: "Brightness",          icon: "☀️", type: "select", options: ["LOW", "MED", "HIGH"] },
  { key: "invertColors",  label: "Invert Colors",       icon: "🌑", type: "toggle" },
  { key: "clockStyle",    label: "Clock Style",         icon: "🕐", type: "select", options: ["MINIMAL", "8-BIT", "CYBER", "ANALOG"] },
  { key: "gifSpeed",      label: "Mochi GIF Speed",     icon: "🎞️", type: "number", min: 50, max: 500, step: 10, unit: "ms" },
  { key: "dnd",           label: "Silent / DND",        icon: "🔕", type: "toggle" },
  { key: "audioLoopback", label: "Mic / Audio Loopback",icon: "🎙️", type: "toggle" },
];

function getDisplayValue(key: keyof LunaSettings, settings: LunaSettings): string {
  const v = settings[key];
  if (key === "clockStyle") return ["MINIMAL", "8-BIT", "CYBER", "ANALOG"][v as number];
  if (key === "brightness") return v as string;
  if (key === "gifSpeed") return `${v}ms`;
  return v ? "ON" : "OFF";
}

function Toggle({ on, onChange }: { on: boolean; onChange: () => void }) {
  return (
    <button
      onClick={onChange}
      className="interactive-tap relative w-10 h-5 rounded-full transition-all duration-200"
      style={{ background: on ? "#38BDF8" : "rgba(255,255,255,0.1)", boxShadow: on ? "0 0 8px #38BDF850" : "none" }}
    >
      <div className="absolute top-0.5 w-4 h-4 rounded-full bg-white shadow transition-all duration-200"
        style={{ left: on ? "calc(100% - 18px)" : "2px" }} />
    </button>
  );
}

function SettingEditor({ def, settings, dispatch, onBack }: { def: SettingRow; settings: LunaSettings; dispatch: React.Dispatch<LunaAction>; onBack: () => void }) {
  const val = settings[def.key];

  function change(v: unknown) {
    dispatch({ type: "CHANGE_SETTING", key: def.key, value: v });
  }

  return (
    <div className="flex flex-col h-full">
      <div className="flex items-center gap-2 px-3 py-2 border-b border-white/[0.06]">
        <button onClick={onBack} className="font-mono-jb text-[8px] text-sky-400 interactive-tap">← BACK</button>
        <span className="font-dm text-[10px] font-semibold text-white ml-2">{def.icon} {def.label}</span>
      </div>
      <div className="flex-1 flex flex-col items-center justify-center gap-4 px-4">
        {def.type === "toggle" && (
          <div className="flex flex-col items-center gap-3">
            <span className="font-orbitron text-[18px] font-black text-sky-400">{val ? "ON" : "OFF"}</span>
            <Toggle on={val as boolean} onChange={() => change(!val)} />
            <span className="font-dm text-[9px] text-white/30">Tap to toggle</span>
          </div>
        )}
        {def.type === "select" && def.options && (
          <div className="w-full space-y-2">
            {def.options.map((opt, i) => {
              const isActive = def.key === "clockStyle" ? i === val : opt === val;
              return (
                <button
                  key={opt}
                  onClick={() => change(def.key === "clockStyle" ? i : opt)}
                  className="interactive-tap w-full py-2.5 rounded-lg font-mono-jb text-[9px] text-left px-3"
                  style={{
                    background: isActive ? "rgba(56,189,248,0.15)" : "rgba(255,255,255,0.04)",
                    border: `1px solid ${isActive ? "rgba(56,189,248,0.4)" : "rgba(255,255,255,0.06)"}`,
                    color: isActive ? "#38BDF8" : "rgba(255,255,255,0.6)",
                  }}
                >
                  {isActive ? "● " : "○ "}{opt}
                </button>
              );
            })}
          </div>
        )}
        {def.type === "number" && (
          <div className="flex flex-col items-center gap-3">
            <span className="font-orbitron text-[28px] font-black text-sky-400">{val}{def.unit}</span>
            <div className="flex gap-3">
              <button
                onClick={() => change(Math.max(def.min!, (val as number) - def.step!))}
                className="interactive-tap w-10 h-10 rounded-full bg-white/10 flex items-center justify-center font-orbitron text-white text-[16px]"
              >–</button>
              <button
                onClick={() => change(Math.min(def.max!, (val as number) + def.step!))}
                className="interactive-tap w-10 h-10 rounded-full bg-sky-400/20 border border-sky-400/40 flex items-center justify-center font-orbitron text-sky-400 text-[16px]"
              >+</button>
            </div>
          </div>
        )}
      </div>
      <div className="px-3 py-2 border-t border-white/[0.06]">
        <button
          onClick={onBack}
          className="w-full py-1.5 rounded-lg font-mono-jb text-[8px] text-sky-400 interactive-tap"
          style={{ background: "rgba(56,189,248,0.1)", border: "1px solid rgba(56,189,248,0.25)" }}
        >
          SAVE & BACK
        </button>
      </div>
    </div>
  );
}

function ResetConfirm({ dispatch }: { dispatch: React.Dispatch<LunaAction> }) {
  return (
    <div className="absolute inset-0 bg-black/80 flex flex-col items-center justify-center z-20 px-4 gap-4" style={{ backdropFilter: "blur(4px)" }}>
      <div className="text-[18px]">⚠️</div>
      <div className="font-orbitron text-[11px] font-black text-[#ef4444] text-center tracking-wider">
        RESET DEVICE?
      </div>
      <div className="font-dm text-[9px] text-white/50 text-center leading-relaxed">
        This will erase all saved settings and return Luna to defaults.
      </div>
      <div className="flex gap-3 w-full">
        <button
          onClick={() => dispatch({ type: "CANCEL_RESET" })}
          className="flex-1 py-2 rounded-lg font-mono-jb text-[8px] text-white/60 interactive-tap"
          style={{ background: "rgba(255,255,255,0.08)", border: "1px solid rgba(255,255,255,0.12)" }}
        >
          CANCEL
        </button>
        <button
          onClick={() => dispatch({ type: "EXECUTE_RESET" })}
          className="flex-1 py-2 rounded-lg font-mono-jb text-[8px] text-white interactive-tap"
          style={{ background: "#ef4444", boxShadow: "0 0 16px #ef444460" }}
        >
          RESET
        </button>
      </div>
    </div>
  );
}

export default function SettingsScreen({ state, dispatch }: Props) {
  const editIdx = state.editingSettingIdx;
  const editDef = editIdx !== null && editIdx < SETTINGS_DEFS.length ? SETTINGS_DEFS[editIdx] : null;

  if (editDef) {
    return (
      <div className="flex flex-col h-full bg-black relative">
        <StatusBar state={state} label="SETTINGS" />
        <SettingEditor
          def={editDef}
          settings={state.settings}
          dispatch={dispatch}
          onBack={() => dispatch({ type: "SAVE_SETTING" })}
        />
      </div>
    );
  }

  return (
    <div className="flex flex-col h-full bg-black relative">
      <StatusBar
        state={state}
        label="SETTINGS"
        right={
          <span className="font-mono-jb text-[7px] text-sky-400">SYSTEM</span>
        }
      />

      <div className="flex-1 overflow-y-auto">
        {/* Section label */}
        <div className="px-3 pt-2 pb-1">
          <span className="font-mono-jb text-[7px] text-white/20 tracking-[0.2em]">DEVICE</span>
        </div>

        {/* Settings list */}
        {SETTINGS_DEFS.map((def, i) => {
          const val = state.settings[def.key];
          const displayVal = getDisplayValue(def.key, state.settings);
          const isToggleOn = def.type === "toggle" && !!val;
          const isToggle = def.type === "toggle";
          return (
            <div
              key={def.key}
              className="interactive-tap cursor-pointer flex items-center gap-2.5 px-3 py-2.5 border-b border-white/[0.04] active:bg-white/[0.03]"
              onClick={() => dispatch({ type: "EDIT_SETTING", idx: i })}
            >
              {/* Icon badge */}
              <div
                className="w-7 h-7 rounded-lg flex items-center justify-center text-[11px] flex-shrink-0"
                style={{ background: "rgba(56,189,248,0.08)", border: "1px solid rgba(56,189,248,0.12)" }}
              >
                {def.icon}
              </div>
              <div className="flex-1 min-w-0">
                <div className="font-dm text-[9px] font-medium text-white/80">{def.label}</div>
              </div>
              <div className="flex items-center gap-1.5 flex-shrink-0">
                {isToggle ? (
                  <div
                    className="relative w-8 h-4 rounded-full transition-all duration-200"
                    style={{ background: isToggleOn ? "#38BDF8" : "rgba(255,255,255,0.1)", boxShadow: isToggleOn ? "0 0 6px #38BDF840" : "none" }}
                  >
                    <div
                      className="absolute top-0.5 w-3 h-3 rounded-full bg-white transition-all duration-200"
                      style={{ left: isToggleOn ? "calc(100% - 14px)" : "2px" }}
                    />
                  </div>
                ) : (
                  <>
                    <span className="font-mono-jb text-[8px] text-sky-400">{displayVal}</span>
                    <span className="text-white/20 text-[9px]">›</span>
                  </>
                )}
              </div>
            </div>
          );
        })}

        {/* Danger zone label */}
        <div className="px-3 pt-3 pb-1">
          <span className="font-mono-jb text-[7px] text-[#ef4444]/40 tracking-[0.2em]">DANGER ZONE</span>
        </div>

        {/* Factory reset */}
        <div
          className="interactive-tap cursor-pointer flex items-center gap-2.5 px-3 py-2.5 border-b border-[#ef4444]/[0.08]"
          style={{ background: "rgba(239,68,68,0.03)" }}
          onClick={() => dispatch({ type: "SHOW_RESET_CONFIRM" })}
        >
          <div className="w-7 h-7 rounded-lg flex items-center justify-center text-[11px] flex-shrink-0"
            style={{ background: "rgba(239,68,68,0.12)", border: "1px solid rgba(239,68,68,0.2)" }}>
            🔄
          </div>
          <div className="flex-1">
            <div className="font-dm text-[9px] font-medium text-[#ef4444]/80">Factory Reset</div>
            <div className="font-mono-jb text-[6px] text-white/20 mt-0.5">ERASES ALL DATA</div>
          </div>
          <span className="text-[#ef4444]/30 text-[9px]">›</span>
        </div>

        {/* System info card */}
        <div className="px-3 py-3">
          <div className="rounded-xl px-3 py-2.5 space-y-1.5" style={{ background: "rgba(255,255,255,0.03)", border: "1px solid rgba(255,255,255,0.05)" }}>
            <div className="flex justify-between">
              <span className="font-mono-jb text-[7px] text-white/20">FIRMWARE</span>
              <span className="font-mono-jb text-[7px] text-white/40">LUNA OS v1.0.0</span>
            </div>
            <div className="flex justify-between">
              <span className="font-mono-jb text-[7px] text-white/20">BUILD</span>
              <span className="font-mono-jb text-[7px] text-sky-400/50">20260909-REL</span>
            </div>
            <div className="flex justify-between">
              <span className="font-mono-jb text-[7px] text-white/20">BATTERY</span>
              <span className="font-mono-jb text-[7px] text-white/40">{state.batteryLevel}%</span>
            </div>
          </div>
        </div>
      </div>

      <div className="px-3 py-1.5 border-t border-white/[0.04]">
        <div className="font-mono-jb text-[6px] text-white/15 text-center">TAP · EDIT SETTING</div>
      </div>

      {state.showResetConfirm && <ResetConfirm dispatch={dispatch} />}
    </div>
  );
}
