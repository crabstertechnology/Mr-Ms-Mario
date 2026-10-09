import React from 'react';
import { sounds } from './soundEffects';

interface PresetOption {
  label: string;
  seconds: number;
}

const PRESETS: PresetOption[] = [
  { label: '30s', seconds: 30 },
  { label: '60s', seconds: 60 },
  { label: '90s', seconds: 90 },
  { label: '2m', seconds: 120 },
];

interface QuickPresetRowProps {
  currentSeconds: number;
  onSelect: (secs: number) => void;
}

export default function QuickPresetRow({ currentSeconds, onSelect }: QuickPresetRowProps) {
  const handleClick = (secs: number) => {
    sounds.playPreset();
    onSelect(secs);
  };

  return (
    <div className="flex items-center justify-center gap-2 select-none w-full px-2">
      {PRESETS.map((p) => {
        const isSelected = currentSeconds === p.seconds;
        return (
          <button
            key={p.seconds}
            type="button"
            onClick={() => handleClick(p.seconds)}
            className="flex-1 py-1.5 px-1 rounded-full font-bold text-[13px] transition-all duration-200 cursor-pointer active:scale-95 flex items-center justify-center"
            style={
              isSelected
                ? {
                    background: 'linear-gradient(135deg, #A855F7 0%, #7E22CE 100%)',
                    color: '#FFFFFF',
                    boxShadow: '0 0 16px rgba(168,85,247,0.75), 0 3px 6px rgba(0,0,0,0.25)',
                    border: '1.5px solid #D8B4FE',
                  }
                : {
                    background: 'rgba(255, 255, 255, 0.92)',
                    color: '#3B0764',
                    boxShadow: '0 2px 6px rgba(0,0,0,0.15), inset 0 1px 1px rgba(255,255,255,0.8)',
                    border: '1px solid rgba(255, 255, 255, 0.6)',
                  }
            }
          >
            {p.label}
          </button>
        );
      })}
    </div>
  );
}
