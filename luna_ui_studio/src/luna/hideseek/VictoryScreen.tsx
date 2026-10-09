import React from 'react';
import { sounds } from './soundEffects';

interface VictoryScreenProps {
  timeRemaining: number;
  totalSeconds: number;
  onPlayAgain: () => void;
  onSetup: () => void;
}

export default function VictoryScreen({
  timeRemaining,
  totalSeconds,
  onPlayAgain,
  onSetup,
}: VictoryScreenProps) {
  const score = Math.max(100, timeRemaining * 40 + 500);
  const stars = timeRemaining > totalSeconds * 0.6 ? 3 : timeRemaining > totalSeconds * 0.25 ? 2 : 1;

  return (
    <div className="relative w-full h-full overflow-hidden select-none bg-[#1A0B2E] flex flex-col justify-between items-center py-3 px-3">
      {/* 3D Celebration Stage Background */}
      <img
        src="/hide_seek/win_bg.jpg"
        alt="Victory Celebration"
        className="absolute inset-0 w-full h-full object-cover pointer-events-none"
        style={{
          filter: 'brightness(0.92) contrast(1.05)',
        }}
      />

      {/* Ambient Gradient */}
      <div
        className="absolute inset-0 pointer-events-none"
        style={{
          background: 'linear-gradient(180deg, rgba(20,5,40,0.6) 0%, transparent 45%, rgba(15,2,30,0.85) 100%)',
        }}
      />

      {/* Top Banner: VICTORY! */}
      <div className="relative z-10 flex flex-col items-center gap-0.5 mt-1">
        <span
          className="text-[20px] font-black tracking-wide text-amber-300 uppercase leading-none"
          style={{
            fontFamily: '"Outfit", sans-serif',
            textShadow: '0 2px 0 #78350F, 0 4px 8px rgba(0,0,0,0.8)',
          }}
        >
          YOU FOUND ME!
        </span>
        <span className="text-[10px] text-purple-200 font-bold tracking-widest uppercase">
          BOMB DEFUSED IN TIME!
        </span>

        {/* 3 Stars */}
        <div className="flex gap-1.5 mt-1">
          {[1, 2, 3].map((s) => (
            <span
              key={s}
              className={`text-[22px] transition-transform ${
                s <= stars ? 'text-amber-300 filter drop-shadow-[0_0_6px_#F59E0B]' : 'text-gray-500/50'
              }`}
            >
              ★
            </span>
          ))}
        </div>
      </div>

      {/* Score Card (Bottom Center) */}
      <div className="relative z-10 w-full flex flex-col items-center gap-2">
        <div
          className="w-full px-4 py-2 rounded-2xl flex items-center justify-between border border-purple-400/40 backdrop-blur-md"
          style={{
            background: 'rgba(46, 16, 101, 0.75)',
            boxShadow: '0 4px 12px rgba(0,0,0,0.4)',
          }}
        >
          <div className="flex flex-col">
            <span className="text-[9px] text-purple-200/80 font-bold uppercase">SCORE</span>
            <span className="text-[20px] font-black text-amber-300 font-outfit leading-none">
              {score.toLocaleString()}
            </span>
          </div>

          <div className="flex flex-col items-end">
            <span className="text-[9px] text-purple-200/80 font-bold uppercase">TIME LEFT</span>
            <span className="text-[16px] font-black text-white font-mono leading-none">
              {timeRemaining}s
            </span>
          </div>
        </div>

        {/* Action Buttons */}
        <div className="w-full flex gap-2">
          <button
            type="button"
            onClick={() => {
              sounds.playTap();
              onPlayAgain();
            }}
            className="flex-1 py-2.5 rounded-full font-black text-[12px] text-white tracking-wider uppercase cursor-pointer active:scale-95 transition-all"
            style={{
              background: 'linear-gradient(135deg, #10B981, #059669)',
              boxShadow: '0 0 12px rgba(16,185,129,0.5)',
              border: '1.5px solid #6EE7B7',
            }}
          >
            PLAY AGAIN
          </button>

          <button
            type="button"
            onClick={() => {
              sounds.playTap();
              onSetup();
            }}
            className="flex-1 py-2.5 rounded-full font-bold text-[12px] text-purple-200 tracking-wider uppercase cursor-pointer active:scale-95 transition-all bg-purple-900/60 hover:bg-purple-800 border border-purple-400/30"
          >
            TIMER SETUP
          </button>
        </div>
      </div>
    </div>
  );
}
