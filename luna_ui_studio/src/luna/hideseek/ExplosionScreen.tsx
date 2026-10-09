import React from 'react';
import { sounds } from './soundEffects';

interface ExplosionScreenProps {
  onRetry: () => void;
  onSetup: () => void;
}

export default function ExplosionScreen({ onRetry, onSetup }: ExplosionScreenProps) {
  return (
    <div className="relative w-full h-full overflow-hidden select-none bg-[#1A0520] flex flex-col justify-between items-center py-5 px-3">
      {/* Cartoon Explosion Flash Backdrop */}
      <div
        className="absolute inset-0 pointer-events-none"
        style={{
          background: 'radial-gradient(circle at 50% 45%, rgba(239,68,68,0.4) 0%, rgba(147,51,234,0.3) 50%, rgba(20,5,30,0.95) 100%)',
        }}
      />

      {/* Floating Sparkles & Dust Puffs */}
      <div className="absolute top-8 left-8 text-amber-300 text-lg animate-ping">💥</div>
      <div className="absolute top-12 right-8 text-amber-400 text-base animate-ping" style={{ animationDuration: '1.5s' }}>💥</div>

      {/* Comic BOOM Header */}
      <div className="relative z-10 flex flex-col items-center gap-1 mt-2">
        <span
          className="text-[34px] font-black text-amber-300 tracking-wider leading-none"
          style={{
            fontFamily: '"Outfit", sans-serif',
            textShadow: '0 3px 0 #DC2626, 0 6px 0 #991B1B, 0 8px 16px rgba(0,0,0,0.8)',
          }}
        >
          BOOM!
        </span>
        <span className="text-[14px] font-extrabold text-red-300 tracking-widest uppercase">
          TIME RAN OUT!
        </span>
      </div>

      {/* Cartoon Dizzy Cat Illustration */}
      <div className="relative z-10 flex flex-col items-center">
        <div className="w-24 h-24 rounded-full bg-purple-900/40 border-2 border-red-500/50 flex items-center justify-center text-[50px] shadow-2xl relative">
          😵‍💫
          {/* Smoke puff badge */}
          <div className="absolute -top-2 -right-2 text-[20px] animate-bounce">
            💨
          </div>
        </div>
        <p className="mt-2 text-center text-[11px] font-semibold text-purple-200/90 max-w-[200px]">
          The hider was too sneaky! The time bomb went off in a puff of confetti!
        </p>
      </div>

      {/* Actions */}
      <div className="relative z-10 w-full flex flex-col gap-2">
        <button
          type="button"
          onClick={() => {
            sounds.playTap();
            onRetry();
          }}
          className="w-full py-2.5 rounded-full font-black text-[13px] text-white tracking-wider uppercase cursor-pointer active:scale-95 transition-all"
          style={{
            background: 'linear-gradient(135deg, #EF4444, #B91C1C)',
            boxShadow: '0 0 16px rgba(239,68,68,0.5)',
            border: '1.5px solid #FCA5A5',
          }}
        >
          TRY AGAIN
        </button>

        <button
          type="button"
          onClick={() => {
            sounds.playTap();
            onSetup();
          }}
          className="w-full py-2 rounded-full font-bold text-[11px] text-purple-300 uppercase cursor-pointer hover:bg-purple-900/40 border border-purple-500/30 transition-all text-center"
        >
          CHANGE TIMER DURATION
        </button>
      </div>
    </div>
  );
}
