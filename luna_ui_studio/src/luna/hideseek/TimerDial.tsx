import React, { useRef, useCallback } from 'react';
import { sounds } from './soundEffects';

interface TimerDialProps {
  seconds: number;
  onChange: (secs: number) => void;
  minSeconds?: number;
  maxSeconds?: number;
}

export default function TimerDial({
  seconds,
  onChange,
  minSeconds = 10,
  maxSeconds = 300,
}: TimerDialProps) {
  const dialRef = useRef<HTMLDivElement>(null);
  const isDraggingRef = useRef(false);

  // Convert seconds to angle (0s = 0°, 300s = 360°, or 60s at top 0°/360°)
  // For standard 60s timer at top:
  // Let angle = (seconds % 60) / 60 * 360 or proportional to maxSeconds
  // In Image 2, 01:00 has the scrubber knob at the exact top (12 o'clock)!
  const progressRatio = Math.min(1, Math.max(0, (seconds - minSeconds) / (maxSeconds - minSeconds)));
  const angleDeg = progressRatio * 360 - 90; // -90 puts 0% at top
  const knobAngleRad = (angleDeg * Math.PI) / 180;

  // Radius for knob path
  const size = 156;
  const radius = 66;
  const cx = size / 2;
  const cy = size / 2;
  const knobX = cx + radius * Math.cos(knobAngleRad);
  const knobY = cy + radius * Math.sin(knobAngleRad);

  const calculateSecondsFromEvent = useCallback(
    (clientX: number, clientY: number) => {
      if (!dialRef.current) return;
      const rect = dialRef.current.getBoundingClientRect();
      const x = clientX - (rect.left + rect.width / 2);
      const y = clientY - (rect.top + rect.height / 2);

      // Angle relative to top (-90 deg / 12 o'clock)
      let deg = (Math.atan2(y, x) * 180) / Math.PI + 90;
      if (deg < 0) deg += 360;

      // Fraction of full circle
      const fraction = deg / 360;
      let newSec = Math.round((minSeconds + fraction * (maxSeconds - minSeconds)) / 5) * 5;
      newSec = Math.max(minSeconds, Math.min(maxSeconds, newSec));

      if (newSec !== seconds) {
        sounds.playTick();
        onChange(newSec);
      }
    },
    [minSeconds, maxSeconds, seconds, onChange]
  );

  const handlePointerDown = (e: React.PointerEvent) => {
    isDraggingRef.current = true;
    (e.target as HTMLElement).setPointerCapture(e.pointerId);
    calculateSecondsFromEvent(e.clientX, e.clientY);
  };

  const handlePointerMove = (e: React.PointerEvent) => {
    if (!isDraggingRef.current) return;
    calculateSecondsFromEvent(e.clientX, e.clientY);
  };

  const handlePointerUp = (e: React.PointerEvent) => {
    isDraggingRef.current = false;
    try {
      (e.target as HTMLElement).releasePointerCapture(e.pointerId);
    } catch {}
  };

  // Format MM:SS
  const mins = Math.floor(seconds / 60);
  const secs = seconds % 60;
  const minsStr = mins.toString().padStart(2, '0');
  const secsStr = secs.toString().padStart(2, '0');

  return (
    <div
      ref={dialRef}
      onPointerDown={handlePointerDown}
      onPointerMove={handlePointerMove}
      onPointerUp={handlePointerUp}
      className="relative select-none cursor-pointer touch-none flex items-center justify-center"
      style={{ width: size, height: size }}
    >
      {/* Outer Glowing Halo Ring */}
      <div
        className="absolute inset-0 rounded-full"
        style={{
          background: 'radial-gradient(circle, rgba(168,85,247,0.25) 0%, rgba(147,51,234,0.1) 60%, transparent 80%)',
          filter: 'blur(8px)',
        }}
      />

      {/* SVG Ring Track */}
      <svg width={size} height={size} className="absolute inset-0 pointer-events-none">
        <defs>
          <linearGradient id="ringTrack" x1="0%" y1="0%" x2="100%" y2="100%">
            <stop offset="0%" stopColor="#C084FC" />
            <stop offset="50%" stopColor="#9333EA" />
            <stop offset="100%" stopColor="#7E22CE" />
          </linearGradient>
          <linearGradient id="cloudGrad" x1="0%" y1="0%" x2="0%" y2="100%">
            <stop offset="0%" stopColor="#E9D5FF" stopOpacity="0.85" />
            <stop offset="100%" stopColor="#C084FC" stopOpacity="0.95" />
          </linearGradient>
          <filter id="neonGlow" x="-20%" y="-20%" width="140%" height="140%">
            <feGaussianBlur stdDeviation="3" result="blur" />
            <feComposite in="SourceGraphic" in2="blur" operator="over" />
          </filter>
        </defs>

        {/* Outer Shadow Ring */}
        <circle
          cx={cx}
          cy={cy}
          r={radius}
          fill="none"
          stroke="#4C1D95"
          strokeWidth="7"
          opacity="0.5"
        />

        {/* Luminous Active Ring */}
        <circle
          cx={cx}
          cy={cy}
          r={radius}
          fill="none"
          stroke="url(#ringTrack)"
          strokeWidth="6"
          strokeLinecap="round"
          filter="url(#neonGlow)"
        />

        {/* Highlight Accent Track */}
        <circle
          cx={cx}
          cy={cy}
          r={radius - 4}
          fill="none"
          stroke="#D8B4FE"
          strokeWidth="1.5"
          opacity="0.6"
        />
      </svg>

      {/* Dial Interior Face Container */}
      <div
        className="relative rounded-full flex flex-col items-center justify-center overflow-hidden"
        style={{
          width: size - 28,
          height: size - 28,
          background: 'linear-gradient(180deg, #F5F3FF 0%, #EDE9FE 40%, #DDD6FE 100%)',
          boxShadow: 'inset 0 3px 8px rgba(0,0,0,0.18), inset 0 -3px 6px rgba(147,51,234,0.25), 0 4px 12px rgba(0,0,0,0.25)',
        }}
      >
        {/* Dreamy clouds at bottom */}
        <div
          className="absolute -bottom-3 inset-x-0 h-14 pointer-events-none opacity-90"
          style={{
            background: 'radial-gradient(ellipse 110% 80% at 50% 100%, #C4B5FD 0%, #DDD6FE 60%, transparent 100%)',
          }}
        >
          {/* Subtle wave contours */}
          <svg viewBox="0 0 100 40" preserveAspectRatio="none" className="w-full h-full opacity-60">
            <path
              d="M0,25 C30,15 70,35 100,20 L100,40 L0,40 Z"
              fill="#A78BFA"
            />
            <path
              d="M0,30 C40,22 65,34 100,28 L100,40 L0,40 Z"
              fill="#8B5CF6"
            />
          </svg>
        </div>

        {/* Cute star sparkles inside dial */}
        <div className="absolute top-4 left-6 text-[#A855F7] text-[9px] pointer-events-none opacity-70 animate-pulse">
          ✦
        </div>
        <div className="absolute top-5 right-7 text-[#C084FC] text-[8px] pointer-events-none opacity-80 animate-pulse">
          ✦
        </div>
        <div className="absolute bottom-5 left-10 text-[#7C3AED] text-[7px] pointer-events-none opacity-60">
          ✦
        </div>

        {/* Big Bold Digital Time Display (Image 2) */}
        <div className="relative z-10 flex items-center justify-center font-black tracking-tight select-none -mt-1">
          {/* MM in deep navy/slate */}
          <span
            className="text-[32px] text-[#1E1B4B]"
            style={{
              fontFamily: '"Outfit", sans-serif',
              fontWeight: 800,
              letterSpacing: '-0.03em',
            }}
          >
            {minsStr}
          </span>

          {/* Colon in vivid purple */}
          <span
            className="text-[30px] text-[#9333EA] mx-0.5"
            style={{
              fontFamily: '"Outfit", sans-serif',
              fontWeight: 800,
            }}
          >
            :
          </span>

          {/* SS in vibrant electric purple/magenta */}
          <span
            className="text-[32px] text-[#9333EA]"
            style={{
              fontFamily: '"Outfit", sans-serif',
              fontWeight: 800,
              letterSpacing: '-0.03em',
            }}
          >
            {secsStr}
          </span>
        </div>
      </div>

      {/* Draggable Illuminated White Scrubber Knob */}
      <div
        className="absolute rounded-full pointer-events-none transition-transform duration-75"
        style={{
          width: 18,
          height: 18,
          left: knobX - 9,
          top: knobY - 9,
          background: 'linear-gradient(135deg, #FFFFFF 0%, #F5F3FF 60%, #DDD6FE 100%)',
          border: '2px solid #C084FC',
          boxShadow: '0 0 10px rgba(192,132,252,0.9), 0 2px 5px rgba(0,0,0,0.3)',
        }}
      >
        {/* Center dot glow */}
        <div className="w-1.5 h-1.5 rounded-full bg-[#9333EA] mx-auto mt-1" />
      </div>
    </div>
  );
}
