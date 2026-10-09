import React, { useState, useEffect, useRef } from 'react';
import { sounds } from './soundEffects';

interface HidingSpot {
  id: string;
  name: string;
  x: number; // percentage from left
  y: number; // percentage from top
  width: number; // px
  height: number; // px
  emptyItem: { icon: string; name: string };
}

const HIDING_SPOTS: HidingSpot[] = [
  { id: 'wardrobe', name: 'Wardrobe', x: 22, y: 44, width: 56, height: 74, emptyItem: { icon: '👔', name: 'Warm Coats' } },
  { id: 'toy_chest', name: 'Toy Chest', x: 53, y: 40, width: 52, height: 44, emptyItem: { icon: '🧸', name: 'Teddy Bear' } },
  { id: 'cardboard_box', name: 'Box', x: 68, y: 52, width: 48, height: 44, emptyItem: { icon: '📦', name: 'Packing Peanuts' } },
  { id: 'bed', name: 'Bed Pillows', x: 40, y: 68, width: 70, height: 52, emptyItem: { icon: '⭐', name: 'Star Pillow' } },
  { id: 'curtains', name: 'Curtains', x: 84, y: 36, width: 38, height: 60, emptyItem: { icon: '🌺', name: 'Flower Drape' } },
  { id: 'armchair', name: 'Armchair', x: 78, y: 66, width: 50, height: 50, emptyItem: { icon: '🧶', name: 'Yarn Ball' } },
  { id: 'nightstand', name: 'Nightstand', x: 36, y: 48, width: 36, height: 40, emptyItem: { icon: '⏰', name: 'Alarm Clock' } },
];

interface SeekGameScreenProps {
  bombSeconds: number;
  onFoundCat: (timeRemaining: number) => void;
  onTimeExpired: () => void;
}

export default function SeekGameScreen({
  bombSeconds,
  onFoundCat,
  onTimeExpired,
}: SeekGameScreenProps) {
  const [timeRemaining, setTimeRemaining] = useState(bombSeconds);
  const [targetSpotId, setTargetSpotId] = useState<string>('');
  const [inspectedSpots, setInspectedSpots] = useState<Record<string, boolean>>({});
  const [activePopup, setActivePopup] = useState<{ spotId: string; isCat: boolean; item: string } | null>(null);
  const [proximityText, setProximityText] = useState('Explore the room to find the cat!');
  const [hotColdLevel, setHotColdLevel] = useState<'cold' | 'warm' | 'hot' | 'burning'>('cold');

  // Randomize hiding spot on mount
  useEffect(() => {
    const randomSpot = HIDING_SPOTS[Math.floor(Math.random() * HIDING_SPOTS.length)].id;
    setTargetSpotId(randomSpot);
  }, []);

  // Timer loop
  useEffect(() => {
    if (timeRemaining <= 0) {
      sounds.playExplode();
      onTimeExpired();
      return;
    }

    if (timeRemaining <= 5) {
      sounds.playCountBeep(1100);
    }

    const timer = setInterval(() => {
      setTimeRemaining((t) => t - 1);
    }, 1000);

    return () => clearInterval(timer);
  }, [timeRemaining, onTimeExpired]);

  const targetSpot = HIDING_SPOTS.find((s) => s.id === targetSpotId);

  const handleSpotClick = (spot: HidingSpot) => {
    if (inspectedSpots[spot.id] || timeRemaining <= 0) return;

    setInspectedSpots((prev) => ({ ...prev, [spot.id]: true }));

    if (spot.id === targetSpotId) {
      // FOUND THE CAT!
      sounds.playMeow();
      sounds.playVictory();
      setActivePopup({ spotId: spot.id, isCat: true, item: '🐱 FOUND WHITE CAT!' });
      setTimeout(() => {
        onFoundCat(timeRemaining);
      }, 1400);
    } else {
      // Empty spot
      sounds.playSqueak();
      setActivePopup({ spotId: spot.id, isCat: false, item: `${spot.emptyItem.icon} Just a ${spot.emptyItem.name}!` });
      setTimeout(() => setActivePopup(null), 1200);

      // Calculate distance to target spot
      if (targetSpot) {
        const dx = spot.x - targetSpot.x;
        const dy = spot.y - targetSpot.y;
        const dist = Math.sqrt(dx * dx + dy * dy);

        if (dist < 22) {
          setHotColdLevel('burning');
          setProximityText('🔥 BURNING HOT! Super close!');
          sounds.playMeow();
        } else if (dist < 38) {
          setHotColdLevel('hot');
          setProximityText('☀️ Hot! You are nearby!');
        } else if (dist < 55) {
          setHotColdLevel('warm');
          setProximityText('🌤️ Warm... keep searching!');
        } else {
          setHotColdLevel('cold');
          setProximityText('❄️ Brrr, freezing cold!');
        }
      }
    }
  };

  // Format MM:SS
  const mins = Math.floor(timeRemaining / 60);
  const secs = timeRemaining % 60;
  const timeStr = `${mins.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;

  return (
    <div className="relative w-full h-full overflow-hidden select-none bg-[#1C0F38]">
      {/* 3D Isometric Gameplay Bedroom Background */}
      <img
        src="/hide_seek/room_bg.jpg"
        alt="Isometric Bedroom"
        className="absolute inset-0 w-full h-full object-cover pointer-events-none"
        style={{
          filter: 'brightness(0.95) contrast(1.08)',
        }}
      />

      {/* Top HUD: Status & Time Bomb Countdown */}
      <div className="absolute top-2 inset-x-2 z-30 flex items-center justify-between pointer-events-none">
        {/* Radar / Clue Badge */}
        <div
          className="px-2.5 py-1 rounded-full text-[10px] font-black flex items-center gap-1 shadow-md pointer-events-auto"
          style={{
            background:
              hotColdLevel === 'burning'
                ? 'linear-gradient(135deg, #EF4444, #DC2626)'
                : hotColdLevel === 'hot'
                ? 'linear-gradient(135deg, #F59E0B, #D97706)'
                : hotColdLevel === 'warm'
                ? 'linear-gradient(135deg, #3B82F6, #2563EB)'
                : 'rgba(30, 10, 60, 0.85)',
            border: '1px solid rgba(255,255,255,0.4)',
            color: '#FFFFFF',
          }}
        >
          <span>{hotColdLevel === 'burning' ? '🔥' : hotColdLevel === 'hot' ? '☀️' : '🔍'}</span>
          <span className="truncate max-w-[120px]">{proximityText}</span>
        </div>

        {/* Ticking Time Bomb Capsule */}
        <div
          className="flex items-center gap-1.5 px-2.5 py-1 rounded-full border border-purple-300/40 shadow-lg"
          style={{
            background: timeRemaining <= 10 ? 'linear-gradient(135deg, #DC2626, #991B1B)' : 'linear-gradient(135deg, #6B21A8, #4C1D95)',
          }}
        >
          <div className="w-2 h-2 rounded-full bg-amber-300 animate-ping" />
          <span className="font-mono font-black text-[13px] text-white tracking-wider">
            {timeStr}
          </span>
        </div>
      </div>

      {/* Clickable Furniture Hiding Spot Targets */}
      {HIDING_SPOTS.map((spot) => {
        const isChecked = inspectedSpots[spot.id];
        return (
          <div
            key={spot.id}
            onClick={() => handleSpotClick(spot)}
            className={`absolute -translate-x-1/2 -translate-y-1/2 rounded-2xl cursor-pointer transition-transform duration-150 active:scale-90 ${
              isChecked ? 'pointer-events-none' : 'hover:scale-105'
            }`}
            style={{
              left: `${spot.x}%`,
              top: `${spot.y}%`,
              width: spot.width,
              height: spot.height,
            }}
            title={`Check ${spot.name}`}
          >
            {/* Subtle Pulsing Clue Halo */}
            {!isChecked && (
              <div
                className="w-full h-full rounded-2xl border-2 border-dashed border-amber-300/60 animate-pulse flex items-center justify-center"
                style={{
                  background: 'radial-gradient(circle, rgba(251,191,36,0.18) 0%, transparent 70%)',
                }}
              >
                <span className="text-[14px] opacity-80 filter drop-shadow">❓</span>
              </div>
            )}

            {/* Checked Icon */}
            {isChecked && (
              <div className="w-full h-full rounded-2xl flex items-center justify-center bg-black/30 backdrop-blur-[1px]">
                <div className="w-5 h-5 rounded-full bg-emerald-500 text-white text-[11px] font-black flex items-center justify-center shadow-md">
                  ✓
                </div>
              </div>
            )}
          </div>
        );
      })}

      {/* Pop-up feedback message */}
      {activePopup && (
        <div className="absolute inset-x-4 top-1/2 -translate-y-1/2 z-40 flex justify-center animate-bounce">
          <div
            className="px-4 py-2.5 rounded-2xl text-white font-black text-center shadow-2xl border-2 flex flex-col items-center gap-1"
            style={{
              background: activePopup.isCat
                ? 'linear-gradient(135deg, #10B981, #059669)'
                : 'linear-gradient(135deg, #4C1D95, #3B0764)',
              borderColor: activePopup.isCat ? '#6EE7B7' : '#A855F7',
            }}
          >
            <span className="text-[14px]">{activePopup.item}</span>
            {activePopup.isCat && (
              <span className="text-[11px] text-amber-200">DEFUSING BOMB... 🎉</span>
            )}
          </div>
        </div>
      )}

      {/* Bottom Hint Banner */}
      <div className="absolute bottom-2 inset-x-2 z-20 flex justify-center">
        <div className="px-3 py-1 rounded-full bg-[#2E1065]/90 border border-purple-400/40 text-[10px] font-bold text-purple-200 shadow-md">
          Tap suspicious furniture to inspect!
        </div>
      </div>
    </div>
  );
}
