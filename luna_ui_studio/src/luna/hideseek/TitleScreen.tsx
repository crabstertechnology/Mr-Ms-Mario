import React, { useState, useEffect } from 'react';
import HideSeekLogo from './HideSeekLogo';
import { sounds } from './soundEffects';

interface TitleScreenProps {
  onStart: () => void;
}

export default function TitleScreen({ onStart }: TitleScreenProps) {
  const [fuseSparkFrame, setFuseSparkFrame] = useState(0);

  // Sizzling spark animation loop
  useEffect(() => {
    const timer = setInterval(() => {
      setFuseSparkFrame((f) => (f + 1) % 6);
    }, 120);
    return () => clearInterval(timer);
  }, []);

  const handlePlayClick = () => {
    sounds.playTap();
    sounds.playMeow();
    onStart();
  };

  return (
    <div className="relative w-full h-full overflow-hidden select-none bg-[#1A0B2E]">
      {/* 3D Isometric Background Room */}
      <img
        src="/hide_seek/title_bg.jpg"
        alt="Hide & Seek Room"
        className="absolute inset-0 w-full h-full object-cover pointer-events-none"
        style={{
          filter: 'brightness(0.95) contrast(1.05)',
        }}
      />

      {/* Ambient purple gradient overlay for text readability */}
      <div
        className="absolute inset-0 pointer-events-none"
        style={{
          background: 'radial-gradient(ellipse 90% 70% at 50% 45%, transparent 30%, rgba(20,5,40,0.45) 85%, rgba(15,2,30,0.7) 100%)',
        }}
      />

      {/* Floating 4-pointed stars */}
      <div className="absolute top-12 left-6 text-amber-200 text-xs animate-pulse pointer-events-none">✦</div>
      <div className="absolute top-8 right-10 text-amber-100 text-xs animate-ping pointer-events-none" style={{ animationDuration: '3s' }}>✦</div>
      <div className="absolute top-24 right-5 text-purple-200 text-xs animate-pulse pointer-events-none">★</div>

      {/* Left White Cat Excitement Comic Rays */}
      <div className="absolute top-[138px] left-[74px] pointer-events-none z-10">
        <div className="relative">
          <div className="absolute -top-3 -left-2 w-2.5 h-1 bg-amber-300 rounded-full rotate-[-45deg] animate-pulse" />
          <div className="absolute -top-5 left-1 w-3 h-1 bg-amber-300 rounded-full rotate-[-75deg] animate-pulse" />
          <div className="absolute -top-3 left-4 w-2.5 h-1 bg-amber-300 rounded-full rotate-[-20deg] animate-pulse" />
        </div>
      </div>

      {/* Right Black Cat Excitement Comic Rays */}
      <div className="absolute top-[134px] right-[76px] pointer-events-none z-10">
        <div className="relative">
          <div className="absolute -top-3 -left-1 w-2.5 h-1 bg-amber-300 rounded-full rotate-[30deg] animate-pulse" />
          <div className="absolute -top-5 left-2 w-3 h-1 bg-amber-300 rounded-full rotate-[75deg] animate-pulse" />
          <div className="absolute -top-3 left-5 w-2.5 h-1 bg-amber-300 rounded-full rotate-[50deg] animate-pulse" />
        </div>
      </div>

      {/* 3D Title "HIDE & SEEK" Logo (Centered above the bomb) */}
      <div className="absolute top-11 inset-x-0 flex justify-center z-20">
        <HideSeekLogo variant="title" />
      </div>

      {/* Sizzling Fuse Spark Layer over the Bomb Centerpiece */}
      <div className="absolute top-[198px] left-[138px] pointer-events-none z-10">
        {/* Dynamic Spark Star */}
        <div
          className="relative flex items-center justify-center transition-transform"
          style={{
            transform: `scale(${0.9 + (fuseSparkFrame % 3) * 0.2}) rotate(${fuseSparkFrame * 45}deg)`,
          }}
        >
          {/* Intense glowing center core */}
          <div className="w-4 h-4 rounded-full bg-amber-200 shadow-[0_0_12px_#FDE047,0_0_24px_#F59E0B]" />
          {/* Radiating fire spark rays */}
          <div className="absolute w-7 h-1 bg-amber-300 rounded-full blur-[0.5px]" />
          <div className="absolute h-7 w-1 bg-amber-300 rounded-full blur-[0.5px]" />
          <div className="absolute w-5 h-1 bg-yellow-100 rounded-full rotate-45" />
          <div className="absolute w-5 h-1 bg-yellow-100 rounded-full -rotate-45" />
        </div>
      </div>

      {/* Primary Action Button: "TAP TO PLAY" (Bottom Center) */}
      <div className="absolute bottom-6 inset-x-0 flex flex-col items-center justify-center px-4 z-20">
        <div className="relative group cursor-pointer" onClick={handlePlayClick}>
          {/* Yellow comic rays radiating from sides of button */}
          <div className="absolute -left-3.5 top-1/2 -translate-y-1/2 flex flex-col gap-1 pointer-events-none">
            <div className="w-2.5 h-1 bg-amber-300 rounded-full rotate-[-25deg] animate-pulse" />
            <div className="w-2 h-1 bg-amber-300 rounded-full rotate-[15deg] animate-pulse" />
          </div>
          <div className="absolute -right-3.5 top-1/2 -translate-y-1/2 flex flex-col gap-1 pointer-events-none">
            <div className="w-2.5 h-1 bg-amber-300 rounded-full rotate-[25deg] animate-pulse" />
            <div className="w-2 h-1 bg-amber-300 rounded-full rotate-[-15deg] animate-pulse" />
          </div>

          {/* Neon Purple Pill Button */}
          <button
            type="button"
            className="flex items-center gap-2.5 px-6 py-3 rounded-full font-black text-white text-[15px] tracking-wider transition-all duration-200 active:scale-95 hover:scale-105 cursor-pointer"
            style={{
              fontFamily: '"Outfit", sans-serif',
              background: 'linear-gradient(135deg, #9333EA 0%, #A855F7 50%, #7E22CE 100%)',
              border: '2px solid #E9D5FF',
              boxShadow: '0 0 24px rgba(168,85,247,0.85), 0 0 40px rgba(147,51,234,0.5), inset 0 2px 4px rgba(255,255,255,0.6)',
            }}
          >
            {/* White Circular Badge with Play Triangle Icon */}
            <div
              className="w-7 h-7 rounded-full bg-white flex items-center justify-center shadow-md -ml-1"
              style={{
                boxShadow: '0 2px 4px rgba(0,0,0,0.25)',
              }}
            >
              <svg width="12" height="12" viewBox="0 0 24 24" fill="#7E22CE" className="ml-0.5">
                <path d="M5 3l14 9-14 9V3z" />
              </svg>
            </div>

            {/* Uppercase Label */}
            <span
              className="tracking-wider uppercase"
              style={{
                textShadow: '0 2px 4px rgba(0,0,0,0.4)',
              }}
            >
              TAP TO PLAY
            </span>
          </button>
        </div>

        {/* Subtle hint */}
        <div className="mt-2 text-[9px] font-semibold text-purple-200/70 tracking-widest uppercase">
          PRESS TO BEGIN
        </div>
      </div>
    </div>
  );
}
