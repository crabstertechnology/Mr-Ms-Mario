import React from 'react';

interface LogoProps {
  variant?: 'title' | 'compact';
  hasCatEars?: boolean;
}

export default function HideSeekLogo({ variant = 'title', hasCatEars = false }: LogoProps) {
  if (variant === 'compact') {
    return (
      <div className="relative inline-flex flex-col items-center select-none filter drop-shadow-[0_4px_12px_rgba(30,10,60,0.6)]">
        {/* Cat ears over H & I */}
        {hasCatEars && (
          <div className="absolute -top-3 left-3 z-10 flex gap-2 pointer-events-none">
            <div className="w-3.5 h-3.5 bg-[#3B1569] rounded-tl-full rounded-tr-md rotate-[-12deg] border border-[#5B21B6] relative flex items-center justify-center">
              <div className="w-1.5 h-1.5 bg-[#F472B6] rounded-tl-full" />
            </div>
            <div className="w-3.5 h-3.5 bg-[#3B1569] rounded-tr-full rounded-tl-md rotate-[12deg] border border-[#5B21B6] relative flex items-center justify-center">
              <div className="w-1.5 h-1.5 bg-[#F472B6] rounded-tr-full" />
            </div>
          </div>
        )}

        {/* Comic excitement rays */}
        <div className="absolute -top-1 -right-3 text-amber-300 font-black text-[10px] animate-pulse">
          ✦
        </div>
        <div className="absolute -bottom-1 -left-3 text-amber-300 font-black text-[9px] animate-pulse">
          ✦
        </div>

        {/* Compact Logo container */}
        <div
          className="relative px-3 py-1 rounded-2xl flex items-center gap-1.5 border-2 border-[#581C87]"
          style={{
            background: 'linear-gradient(180deg, #2E1065 0%, #1E0842 100%)',
            boxShadow: '0 4px 10px rgba(0,0,0,0.4), inset 0 2px 2px rgba(255,255,255,0.25)',
          }}
        >
          {/* HIDE */}
          <span
            className="font-black text-[18px] tracking-wide text-white"
            style={{
              fontFamily: '"Outfit", "Fredoka", sans-serif',
              textShadow: '0 2px 0 #3B0764, 0 3px 0 #2E0854, 0 4px 6px rgba(0,0,0,0.6)',
              filter: 'drop-shadow(0 1px 1px rgba(255,255,255,0.8))',
            }}
          >
            HIDE
          </span>

          {/* & */}
          <span
            className="font-black text-[12px] text-[#D8B4FE]"
            style={{
              textShadow: '0 1px 0 #3B0764',
            }}
          >
            &amp;
          </span>

          {/* SEEK */}
          <span
            className="font-black text-[18px] tracking-wide text-[#FBBF24]"
            style={{
              fontFamily: '"Outfit", "Fredoka", sans-serif',
              textShadow: '0 2px 0 #B45309, 0 3px 0 #78350F, 0 4px 6px rgba(0,0,0,0.6)',
              filter: 'drop-shadow(0 1px 1px rgba(254,240,138,0.8))',
            }}
          >
            SEEK
          </span>
        </div>
      </div>
    );
  }

  // Large 3D Title Logo (Screen 1)
  return (
    <div className="relative inline-flex flex-col items-center select-none filter drop-shadow-[0_8px_20px_rgba(20,5,45,0.85)]">
      {/* Surrounding cartoon stars */}
      <div className="absolute -top-3 -left-3 text-[#FBBF24] text-xs font-black animate-spin" style={{ animationDuration: '6s' }}>
        ✦
      </div>
      <div className="absolute -top-2 -right-4 text-[#FDE047] text-sm font-black animate-pulse">
        ★
      </div>
      <div className="absolute bottom-2 -right-3 text-[#E9D5FF] text-xs font-black animate-pulse">
        ✦
      </div>

      {/* Main 3D Logo Badge */}
      <div
        className="relative px-5 py-2.5 rounded-3xl flex flex-col items-center border-[3px] border-[#4C1D95]"
        style={{
          background: 'linear-gradient(180deg, #3B0764 0%, #1E0742 70%, #15032E 100%)',
          boxShadow: '0 8px 24px rgba(0,0,0,0.6), inset 0 2px 3px rgba(255,255,255,0.3), inset 0 -3px 6px rgba(0,0,0,0.7)',
        }}
      >
        {/* Upper Row: HIDE & */}
        <div className="flex items-center gap-1.5 leading-none">
          <span
            className="font-black text-[34px] tracking-tight text-white relative"
            style={{
              fontFamily: '"Outfit", "Fredoka", sans-serif',
              letterSpacing: '-0.02em',
              textShadow: [
                '0 1px 0 #ffffff',
                '0 2px 0 #D8B4FE',
                '0 4px 0 #7E22CE',
                '0 6px 0 #581C87',
                '0 7px 0 #3B0764',
                '0 10px 14px rgba(0,0,0,0.8)',
              ].join(', '),
            }}
          >
            HIDE
          </span>

          <div
            className="w-5 h-5 rounded-full bg-[#7E22CE] border border-[#C084FC] flex items-center justify-center font-black text-[11px] text-white shadow-md -mt-1"
            style={{
              boxShadow: '0 2px 4px rgba(0,0,0,0.4), inset 0 1px 1px rgba(255,255,255,0.4)',
            }}
          >
            &amp;
          </div>
        </div>

        {/* Lower Row: SEEK */}
        <div className="leading-none -mt-1">
          <span
            className="font-black text-[38px] tracking-wider text-[#FCD34D]"
            style={{
              fontFamily: '"Outfit", "Fredoka", sans-serif',
              textShadow: [
                '0 1px 0 #FEF08A',
                '0 2px 0 #F59E0B',
                '0 4px 0 #D97706',
                '0 6px 0 #B45309',
                '0 7px 0 #78350F',
                '0 10px 14px rgba(0,0,0,0.8)',
              ].join(', '),
            }}
          >
            SEEK
          </span>
        </div>
      </div>
    </div>
  );
}
