import React, { useState } from 'react';
import HideSeekLogo from './HideSeekLogo';
import TimerDial from './TimerDial';
import QuickPresetRow from './QuickPresetRow';
import FineAdjusterBar from './FineAdjusterBar';
import { sounds } from './soundEffects';

interface SetupScreenProps {
  initialSeconds?: number;
  onArmAndHide: (durationSeconds: number, gameMode: 'room_search' | 'watch_beacon') => void;
  onBackToTitle?: () => void;
}

export default function SetupScreen({
  initialSeconds = 60,
  onArmAndHide,
  onBackToTitle,
}: SetupScreenProps) {
  const [seconds, setSeconds] = useState(initialSeconds);
  const [gameMode, setGameMode] = useState<'room_search' | 'watch_beacon'>('room_search');

  const handleStart = () => {
    sounds.playCountBeep(980);
    onArmAndHide(seconds, gameMode);
  };

  return (
    <div className="relative w-full h-full overflow-hidden select-none bg-[#241042] flex flex-col justify-between">
      {/* Dreamy Purple Night Background */}
      <img
        src="/hide_seek/setup_bg.jpg"
        alt="Hide & Seek Setup"
        className="absolute inset-0 w-full h-full object-cover pointer-events-none"
        style={{
          filter: 'brightness(0.96) contrast(1.05)',
        }}
      />

      {/* Atmospheric Ambient Gradient */}
      <div
        className="absolute inset-0 pointer-events-none"
        style={{
          background: 'linear-gradient(180deg, rgba(20,5,40,0.3) 0%, transparent 40%, rgba(30,10,60,0.4) 80%, rgba(20,5,40,0.7) 100%)',
        }}
      />

      {/* Comic excitement rays for cats */}
      <div className="absolute top-[82px] left-[66px] pointer-events-none">
        <div className="flex gap-0.5">
          <div className="w-2.5 h-1 bg-amber-300 rounded-full rotate-[-45deg] animate-pulse" />
          <div className="w-2.5 h-1 bg-amber-300 rounded-full rotate-[-20deg] animate-pulse" />
        </div>
      </div>
      <div className="absolute top-[82px] right-[66px] pointer-events-none">
        <div className="flex gap-0.5">
          <div className="w-2.5 h-1 bg-amber-300 rounded-full rotate-[20deg] animate-pulse" />
          <div className="w-2.5 h-1 bg-amber-300 rounded-full rotate-[45deg] animate-pulse" />
        </div>
      </div>

      {/* Top Header: HIDE & SEEK with Cat Ears */}
      <div className="relative z-10 pt-2 flex items-center justify-between px-3">
        {onBackToTitle ? (
          <button
            type="button"
            onClick={onBackToTitle}
            className="w-6 h-6 rounded-full bg-white/20 hover:bg-white/30 text-white text-xs flex items-center justify-center cursor-pointer transition-colors"
            title="Back to title"
          >
            ‹
          </button>
        ) : (
          <div className="w-6" />
        )}

        <div className="flex-1 flex justify-center">
          <HideSeekLogo variant="compact" hasCatEars={true} />
        </div>

        <div className="w-6" />
      </div>

      {/* Center Section: Circular Timer Dial */}
      <div className="relative z-10 flex flex-col items-center justify-center -my-1">
        <TimerDial seconds={seconds} onChange={setSeconds} />
      </div>

      {/* Controls Section: Presets + Stepper Bar */}
      <div className="relative z-10 flex flex-col items-center gap-2 px-3 w-full">
        {/* Quick Presets: 30s | 60s | 90s | 2m */}
        <QuickPresetRow currentSeconds={seconds} onSelect={setSeconds} />

        {/* Fine Adjustment Bar: [-] (dots + clock) [+] */}
        <FineAdjusterBar seconds={seconds} onChange={setSeconds} step={5} />
      </div>

      {/* Mode Selector Pill (Room Hunt vs Watch Beacon) */}
      <div className="relative z-10 flex justify-center px-4 -mb-1">
        <div
          className="flex items-center p-0.5 rounded-full text-[10px] font-bold text-white/90"
          style={{
            background: 'rgba(59, 7, 100, 0.75)',
            border: '1px solid rgba(168, 85, 247, 0.4)',
          }}
        >
          <button
            type="button"
            onClick={() => { sounds.playTap(); setGameMode('room_search'); }}
            className={`px-2.5 py-0.5 rounded-full transition-all cursor-pointer ${
              gameMode === 'room_search'
                ? 'bg-[#9333EA] text-white shadow-sm'
                : 'text-purple-200/70 hover:text-white'
            }`}
          >
            🔍 Room Hunt
          </button>
          <button
            type="button"
            onClick={() => { sounds.playTap(); setGameMode('watch_beacon'); }}
            className={`px-2.5 py-0.5 rounded-full transition-all cursor-pointer ${
              gameMode === 'watch_beacon'
                ? 'bg-[#9333EA] text-white shadow-sm'
                : 'text-purple-200/70 hover:text-white'
            }`}
          >
            ⏱ Watch Alarm
          </button>
        </div>
      </div>

      {/* Bottom Wavy Card & Swipe/Tap Handle */}
      <div
        className="relative z-10 w-full pt-1 pb-2 flex flex-col items-center cursor-pointer transition-transform hover:scale-[1.02] active:scale-[0.98]"
        onClick={handleStart}
        style={{
          background: 'linear-gradient(180deg, rgba(168, 85, 247, 0.3) 0%, rgba(126, 34, 206, 0.6) 100%)',
          backdropFilter: 'blur(8px)',
          borderTop: '1px solid rgba(216, 180, 254, 0.5)',
          boxShadow: '0 -4px 16px rgba(126, 34, 206, 0.4)',
        }}
      >
        {/* Upward Chevron ^ with glowing pulse */}
        <div className="flex items-center justify-center text-purple-100 text-[12px] font-black -mb-0.5 animate-bounce">
          ▲
        </div>

        {/* Start Label */}
        <span
          className="text-white font-extrabold text-[12px] tracking-wider uppercase"
          style={{
            fontFamily: '"Outfit", sans-serif',
            textShadow: '0 0 10px rgba(168,85,247,0.9)',
          }}
        >
          ARM &amp; HIDE
        </span>

        {/* Illuminated Bottom Pill Capsule Handle */}
        <div
          className="w-16 h-1.5 rounded-full mt-1"
          style={{
            background: 'linear-gradient(90deg, #E9D5FF 0%, #FFFFFF 50%, #E9D5FF 100%)',
            boxShadow: '0 0 8px rgba(255, 255, 255, 0.9)',
          }}
        />
      </div>
    </div>
  );
}
