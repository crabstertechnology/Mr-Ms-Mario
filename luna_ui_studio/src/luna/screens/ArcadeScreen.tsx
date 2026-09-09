import type React from "react";
import { lazy, Suspense, useCallback } from "react";
import type { LunaState, LunaAction } from "../types";
import { GAMES } from "../types";
import StatusBar from "../components/StatusBar";
import LunaMascot from "../components/LunaMascot";

const LunaRacer = lazy(() => import("../games/LunaRacer"));
const LunaSpace = lazy(() => import("../games/LunaSpace"));
const FlappyMochy = lazy(() => import("../games/FlappyMochy"));
const CoinCatcher = lazy(() => import("../games/CoinCatcher"));
const MochyJump = lazy(() => import("../games/MochyJump"));
const Stacker = lazy(() => import("../games/Stacker"));
const MemoryMatrix = lazy(() => import("../games/MemoryMatrix"));
const TiltMaze = lazy(() => import("../games/TiltMaze"));

interface Props { state: LunaState; dispatch: React.Dispatch<LunaAction> }

function GameOverScreen({ state, dispatch }: Props) {
  const game = GAMES[state.activeGame ?? state.selectedGame];
  const latestScore = state.highScores[game.key];
  return (
    <div className="flex-1 flex flex-col items-center justify-center gap-3 px-4">
      <div className="text-[32px] mb-1">{game.icon}</div>

      <div className="flex flex-col items-center gap-1">
        <div className="font-pixel text-[8px] tracking-[0.2em] animate-pulse" style={{ color: game.color }}>
          ★ PERSONAL BEST ★
        </div>
        <div className="font-orbitron text-[32px] font-black" style={{ color: game.color, textShadow: `0 0 24px ${game.color}70` }}>
          {latestScore.toLocaleString()}
        </div>
        <div className="font-dm text-[9px] text-white/30">{game.name}</div>
      </div>

      <LunaMascot mood="surprised" size="sm" />

      <div className="font-pixel text-[8px] text-white/20 tracking-[0.2em]">GAME OVER</div>

      <div className="flex gap-2.5 w-full">
        <button
          onClick={() => dispatch({ type: "LAUNCH_GAME" })}
          className="interactive-tap flex-1 py-2.5 rounded-xl font-pixel text-[7px]"
          style={{ background: game.color + "18", border: `1px solid ${game.color}40`, color: game.color }}
        >
          REPLAY
        </button>
        <button
          onClick={() => dispatch({ type: "EXIT_GAME" })}
          className="interactive-tap flex-1 py-2.5 rounded-xl font-mono-jb text-[8px] text-white/40 border border-white/[0.08]"
        >
          ← ARCADE
        </button>
      </div>
    </div>
  );
}

function GamePlay({ state, dispatch }: Props) {
  const game = GAMES[state.activeGame ?? 0];

  const handleGameOver = useCallback((score: number) => {
    dispatch({ type: "SAVE_HIGH_SCORE", gameKey: game.key, score });
  }, [dispatch, game.key]);

  const GameComponent = [LunaRacer, LunaSpace, FlappyMochy, CoinCatcher, MochyJump, Stacker, MemoryMatrix, TiltMaze][state.activeGame ?? 0];

  return (
    <div className="flex-1 flex flex-col">
      {/* Minimal game header */}
      <div className="flex items-center justify-between px-3 py-1.5 border-b border-white/[0.06]">
        <span className="font-pixel text-[7px]" style={{ color: game.color }}>{game.icon} {game.name.toUpperCase()}</span>
        <button
          onClick={() => dispatch({ type: "EXIT_GAME" })}
          className="interactive-tap font-mono-jb text-[7px] text-white/30 px-1.5 py-0.5 rounded border border-white/10"
        >
          EXIT
        </button>
      </div>

      {/* Game area */}
      <div className="flex-1 flex flex-col items-center justify-center bg-black overflow-hidden">
        <Suspense fallback={<div className="font-pixel text-[8px] text-white/40 animate-pulse">LOADING...</div>}>
          <GameComponent
            onGameOver={handleGameOver}
            highScore={state.highScores[game.key]}
          />
        </Suspense>
      </div>

      <div className="px-3 py-1 border-t border-white/[0.06]">
        <div className="font-mono-jb text-[6px] text-white/20 text-center">LONG PRESS · EXIT</div>
      </div>
    </div>
  );
}

function ArcadeMenu({ state, dispatch }: Props) {
  return (
    <div className="flex-1 flex flex-col overflow-hidden">
      {/* Selected game preview */}
      <div className="px-3 py-2 border-b border-white/[0.06]">
        {(() => {
          const game = GAMES[state.selectedGame];
          const hs = state.highScores[game.key];
          return (
            <div className="flex items-center gap-3">
              <div
                className="w-10 h-10 rounded-xl flex items-center justify-center text-[20px] flex-shrink-0"
                style={{ background: game.color + "20", border: `1px solid ${game.color}40`, boxShadow: `0 0 12px ${game.color}20` }}
              >
                {game.icon}
              </div>
              <div className="flex-1">
                <div className="font-orbitron text-[10px] font-bold text-white">{game.name}</div>
                <div className="font-dm text-[8px] text-white/40">{game.desc}</div>
              </div>
              <div className="text-right">
                <div className="font-pixel text-[6px] text-white/30">BEST</div>
                <div className="font-orbitron text-[10px] font-bold" style={{ color: game.color }}>{hs.toLocaleString()}</div>
              </div>
            </div>
          );
        })()}
      </div>

      {/* Game list */}
      <div className="flex-1 overflow-y-auto">
        {GAMES.map((game, i) => {
          const hs = state.highScores[game.key];
          const isSelected = state.selectedGame === i;
          return (
            <div
              key={game.id}
              className="interactive-tap cursor-pointer flex items-center gap-2.5 px-3 py-2 border-b border-white/[0.04]"
              style={{ background: isSelected ? game.color + "10" : "transparent" }}
              onClick={() => {
                if (isSelected) {
                  dispatch({ type: "LAUNCH_GAME" });
                } else {
                  dispatch({ type: "SELECT_GAME", idx: i });
                }
              }}
            >
              <div
                className="w-6 h-6 rounded-lg flex items-center justify-center text-[12px] flex-shrink-0"
                style={{
                  background: isSelected ? game.color + "30" : "rgba(255,255,255,0.04)",
                  border: `1px solid ${isSelected ? game.color + "60" : "rgba(255,255,255,0.06)"}`,
                }}
              >
                {game.icon}
              </div>
              <span
                className={`font-dm text-[9px] font-medium flex-1 ${isSelected ? "text-white" : "text-white/60"}`}
              >
                {game.name}
              </span>
              <span className="font-mono-jb text-[8px]" style={{ color: isSelected ? game.color : "rgba(255,255,255,0.25)" }}>
                {hs.toLocaleString()}
              </span>
              {isSelected && (
                <div className="w-1.5 h-1.5 rounded-full flex-shrink-0" style={{ background: game.color, boxShadow: `0 0 4px ${game.color}` }} />
              )}
            </div>
          );
        })}
      </div>

      <div className="px-3 py-1.5 border-t border-white/[0.06] flex items-center justify-between">
        <LunaMascot mood="game" size="xs" animate />
        <span className="font-mono-jb text-[7px] text-white/20">TAP · SELECT · TAP AGAIN · PLAY</span>
      </div>
    </div>
  );
}

export default function ArcadeScreen({ state, dispatch }: Props) {
  return (
    <div className="flex flex-col h-full bg-black">
      <StatusBar
        state={state}
        label="LUNA ARCADE"
        right={
          <span className="font-pixel text-[7px] text-[#fbbf24]">
            {state.highScores[GAMES[state.selectedGame].key].toLocaleString()}
          </span>
        }
      />

      {state.arcadeMode === "menu" && <ArcadeMenu state={state} dispatch={dispatch} />}
      {state.arcadeMode === "playing" && <GamePlay state={state} dispatch={dispatch} />}
      {state.arcadeMode === "gameover" && (
        <div className="flex flex-col h-full">
          <GameOverScreen state={state} dispatch={dispatch} />
        </div>
      )}
    </div>
  );
}
