import React, { useState, useEffect } from 'react';
import { sounds } from './soundEffects';

interface HidingCountdownScreenProps {
  bombSeconds: number;
  onFinishHiding: () => void;
}

export default function HidingCountdownScreen({
  bombSeconds,
  onFinishHiding,
}: HidingCountdownScreenProps) {
  const [timeLeft, setTimeLeft] = useState(10);
  const [catFrame, setCatFrame] = useState(0);

  useEffect(() => {
    // Tick down 10s
    if (timeLeft <= 0) {
      sounds.playMeow();
      sounds.playCountBeep(1200);
      onFinishHiding();
      return;
    }

    sounds.playCountBeep(timeLeft <= 3 ? 1040 : 700);

    const timer = setTimeout(() => {
      setTimeLeft((t) => t - 1);
    }, 1000);

    return () => clearTimeout(timer);
  }, [timeLeft, onFinishHiding]);

  // Running cat frame toggle
  useEffect(() => {
    const anim = setInterval(() => {
      setCatFrame((f) => (f + 1) % 4);
    }, 180);
    return () => clearInterval(anim);
  }, []);

  return (
    <div className="relative w-full h-full bg-[#16062B] text-white flex flex-col justify-between items-center py-4 px-3 select-none overflow-hidden">
      {/* Top Hazard Stripes */}
      <div className="w-full flex h-3 rounded-full overflow-hidden opacity-80 shadow-sm">
        {[...Array(16)].map((_, i) => (
          <div
            key={i}
            className="flex-1 h-full"
            style={{
              background: i % 2 === 0 ? '#F59E0B' : '#3B0764',
            }}
          />
        ))}
      </div>

      {/* Main Banner */}
      <div className="flex flex-col items-center gap-1 z-10 -mt-1">
        <span className="text-[20px] font-black tracking-wider text-amber-300 font-outfit uppercase animate-pulse">
          QUICK! GO HIDE!
        </span>
        <span className="text-[10px] text-purple-200/80 font-mono tracking-widest uppercase">
          STEALTH COUNTDOWN
        </span>
      </div>

      {/* Animated Scampering White Cat */}
      <div className="relative w-full h-20 flex items-center justify-center">
        {/* Cat scamper path */}
        <div
          className="transition-transform duration-150 flex flex-col items-center"
          style={{
            transform: `translateX(${(catFrame - 1.5) * 22}px) rotate(${catFrame % 2 === 0 ? 4 : -4}deg)`,
          }}
        >
          {/* White Kitten Silhouette / Cartoon Emoji */}
          <div className="text-[44px] filter drop-shadow-[0_4px_12px_rgba(168,85,247,0.7)]">
            🐱💨
          </div>
          <div className="text-[10px] text-amber-200 font-bold bg-[#4C1D95]/80 px-2.5 py-0.5 rounded-full border border-purple-400/30">
            Finding a spot...
          </div>
        </div>
      </div>

      {/* Giant Countdown Digit */}
      <div className="relative flex items-center justify-center">
        <div className="absolute w-28 h-28 rounded-full bg-purple-600/20 blur-xl animate-ping" />
        <div
          className="relative w-24 h-24 rounded-full border-4 border-amber-400 bg-gradient-to-b from-[#4C1D95] to-[#1E0842] flex items-center justify-center"
          style={{
            boxShadow: '0 0 25px rgba(251, 191, 36, 0.6), inset 0 2px 6px rgba(255,255,255,0.4)',
          }}
        >
          <span
            className="text-[52px] font-black text-amber-300 leading-none"
            style={{
              fontFamily: '"Outfit", sans-serif',
              textShadow: '0 3px 0 #B45309, 0 6px 12px rgba(0,0,0,0.8)',
            }}
          >
            {timeLeft}
          </span>
        </div>
      </div>

      {/* Bomb time duration info */}
      <div className="text-center text-[10px] text-purple-200 font-medium">
        Bomb set to <span className="font-bold text-amber-300">{bombSeconds}s</span> · Ready or not!
      </div>

      {/* Skip Button */}
      <button
        type="button"
        onClick={() => {
          sounds.playTap();
          onFinishHiding();
        }}
        className="w-full py-2.5 rounded-full bg-purple-900/60 hover:bg-purple-800 border border-purple-400/40 text-[11px] font-extrabold text-white tracking-wider uppercase cursor-pointer active:scale-95 transition-all shadow-md"
      >
        I'M HIDDEN! SEEK NOW ➔
      </button>
    </div>
  );
}
