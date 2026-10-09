import React from 'react';

interface LunaWatchFrameProps {
  children: React.ReactNode;
  width?: number;
  height?: number;
}

export default function LunaWatchFrame({
  children,
  width = 280,
  height = 340,
}: LunaWatchFrameProps) {
  return (
    <div className="relative inline-flex items-center justify-center select-none filter drop-shadow-[0_25px_60px_rgba(20,5,40,0.6)]">
      {/* Right Hardware Side Button (Matching photo) */}
      <div
        className="absolute -right-3.5 top-28 w-4 h-16 rounded-r-xl transition-transform active:translate-x-[-2px]"
        style={{
          background: 'linear-gradient(90deg, #9F75DE 0%, #B894EB 50%, #C4A4F2 100%)',
          boxShadow: '2px 0 5px rgba(0,0,0,0.25), inset 1px 0 2px rgba(255,255,255,0.4)',
          borderTop: '1px solid #D8B4FE',
          borderBottom: '1px solid #7E22CE',
        }}
      />

      {/* Left Hardware Grip / Button Accent */}
      <div
        className="absolute -left-2.5 top-36 w-3 h-14 rounded-l-lg opacity-80"
        style={{
          background: 'linear-gradient(270deg, #9F75DE 0%, #B894EB 60%, #C4A4F2 100%)',
          boxShadow: '-1px 0 4px rgba(0,0,0,0.2)',
        }}
      />

      {/* Main Lavender Watch Body (Exact shape from photo) */}
      <div
        className="relative flex flex-col items-center justify-between p-3.5 transition-all"
        style={{
          borderRadius: 48,
          background: 'linear-gradient(150deg, #D4B6F7 0%, #C29EF2 35%, #B48CEB 70%, #A275E5 100%)',
          boxShadow: [
            '0 12px 36px rgba(46, 16, 101, 0.45)',
            'inset 0 2px 4px rgba(255, 255, 255, 0.75)',
            'inset 0 -4px 8px rgba(91, 33, 182, 0.4)',
            '0 0 0 1px rgba(255, 255, 255, 0.4)',
          ].join(', '),
        }}
      >
        {/* Black Display Bezel Frame */}
        <div
          className="relative flex flex-col items-center justify-between pt-2.5 pb-2 px-2"
          style={{
            borderRadius: 38,
            background: 'linear-gradient(180deg, #090314 0%, #05010B 100%)',
            boxShadow: [
              'inset 0 1px 2px rgba(255,255,255,0.25)',
              '0 4px 12px rgba(0,0,0,0.6)',
              '0 0 0 1.5px #1E0842',
            ].join(', '),
          }}
        >
          {/* Top Sensor / Camera Dot */}
          <div
            className="w-2.5 h-2.5 rounded-full mb-1.5 flex items-center justify-center"
            style={{
              background: '#0B0416',
              border: '0.8px solid #2E1065',
              boxShadow: 'inset 0 1px 1px rgba(255,255,255,0.2)',
            }}
          >
            <div className="w-1 h-1 rounded-full bg-[#3B1569]" />
          </div>

          {/* Active Screen Display Port */}
          <div
            className="relative overflow-hidden rounded-[26px] bg-black shadow-inner"
            style={{
              width,
              height,
            }}
          >
            {children}

            {/* Subtle Screen Glass Sheen */}
            <div
              className="absolute inset-0 pointer-events-none rounded-[26px]"
              style={{
                background: 'linear-gradient(135deg, rgba(255,255,255,0.06) 0%, transparent 40%)',
              }}
            />
          </div>

          {/* Bottom Bezel: Engraved "Luna" Logo */}
          <div className="mt-2 mb-0.5 flex items-center justify-center select-none">
            <span
              className="font-medium text-[13px] tracking-wide"
              style={{
                fontFamily: '"Outfit", "Inter", sans-serif',
                color: '#D1D5DB',
                textShadow: '0 1px 1px rgba(0,0,0,0.8), 0 0 4px rgba(255,255,255,0.2)',
              }}
            >
              Luna
            </span>
          </div>
        </div>
      </div>
    </div>
  );
}
