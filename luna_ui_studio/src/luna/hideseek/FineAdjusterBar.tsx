import React from 'react';
import { sounds } from './soundEffects';

interface FineAdjusterBarProps {
  seconds: number;
  onChange: (secs: number) => void;
  minSeconds?: number;
  maxSeconds?: number;
  step?: number;
}

export default function FineAdjusterBar({
  seconds,
  onChange,
  minSeconds = 10,
  maxSeconds = 300,
  step = 5,
}: FineAdjusterBarProps) {
  const handleMinus = () => {
    const next = Math.max(minSeconds, seconds - step);
    if (next !== seconds) {
      sounds.playTap();
      onChange(next);
    }
  };

  const handlePlus = () => {
    const next = Math.min(maxSeconds, seconds + step);
    if (next !== seconds) {
      sounds.playTap();
      onChange(next);
    }
  };

  return (
    <div
      className="flex items-center justify-between px-3 py-2 rounded-full select-none w-full"
      style={{
        background: 'rgba(255, 255, 255, 0.55)',
        backdropFilter: 'blur(12px)',
        border: '1.5px solid rgba(255, 255, 255, 0.8)',
        boxShadow: '0 4px 16px rgba(126, 34, 206, 0.15), inset 0 1px 2px rgba(255, 255, 255, 0.9)',
      }}
    >
      {/* Minus Button: Circular soft white */}
      <button
        type="button"
        onClick={handleMinus}
        disabled={seconds <= minSeconds}
        className="w-9 h-9 rounded-full bg-white flex items-center justify-center transition-all duration-150 cursor-pointer active:scale-90 disabled:opacity-40 disabled:cursor-not-allowed"
        style={{
          boxShadow: '0 2px 6px rgba(0, 0, 0, 0.12), inset 0 1px 1px rgba(255, 255, 255, 1)',
          border: '1px solid rgba(220, 210, 245, 0.7)',
        }}
        title="Decrease time (-5s)"
      >
        <span className="text-[#3B0764] font-black text-[20px] leading-none -mt-0.5">−</span>
      </button>

      {/* Center Indicator: Arc of dots with clock icon */}
      <div className="flex flex-col items-center justify-center relative px-2">
        {/* Arc of dots */}
        <div className="flex items-center gap-1 mb-1">
          {[...Array(9)].map((_, i) => {
            const isCenter = i === 4;
            const distFromCenter = Math.abs(i - 4);
            const opacity = 1 - distFromCenter * 0.15;
            return (
              <div
                key={i}
                className="rounded-full bg-white transition-opacity"
                style={{
                  width: isCenter ? 4 : 3,
                  height: isCenter ? 4 : 3,
                  opacity,
                  boxShadow: '0 0 3px rgba(255,255,255,0.8)',
                }}
              />
            );
          })}
        </div>

        {/* Small Clock Icon */}
        <div
          className="w-5 h-5 rounded-full flex items-center justify-center border border-[#A855F7]"
          style={{
            background: 'linear-gradient(135deg, #7E22CE 0%, #581C87 100%)',
            boxShadow: '0 1px 3px rgba(0,0,0,0.2)',
          }}
        >
          <svg width="10" height="10" viewBox="0 0 24 24" fill="none" stroke="white" strokeWidth="3" strokeLinecap="round">
            <circle cx="12" cy="12" r="9" />
            <path d="M12 7v5l3 2" />
          </svg>
        </div>
      </div>

      {/* Plus Button: Circular vibrant purple */}
      <button
        type="button"
        onClick={handlePlus}
        disabled={seconds >= maxSeconds}
        className="w-9 h-9 rounded-full flex items-center justify-center transition-all duration-150 cursor-pointer active:scale-90 disabled:opacity-40 disabled:cursor-not-allowed"
        style={{
          background: 'linear-gradient(135deg, #A855F7 0%, #7E22CE 100%)',
          boxShadow: '0 0 12px rgba(168, 85, 247, 0.6), 0 2px 6px rgba(0,0,0,0.2)',
          border: '1.5px solid #D8B4FE',
        }}
        title="Increase time (+5s)"
      >
        <span className="text-white font-black text-[20px] leading-none -mt-0.5">+</span>
      </button>
    </div>
  );
}
