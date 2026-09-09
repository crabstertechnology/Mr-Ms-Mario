interface LunaMascotProps {
  mood?: "default" | "focus" | "break" | "happy" | "surprised" | "cool" | "sleep" | "love" | "game" | "analyze";
  size?: "xs" | "sm" | "md" | "lg";
  animate?: boolean;
  label?: string;
}

const MOODS = {
  default:   { face: "(• ‿ •)", color: "#38BDF8" },
  focus:     { face: "(⌐■_■)", color: "#38BDF8" },
  break:     { face: "(¬‿¬) ♪", color: "#4ade80" },
  happy:     { face: "(◕‿◕✿)", color: "#fbbf24" },
  surprised: { face: "(⊙▽⊙)!", color: "#F472B6" },
  cool:      { face: "( •_•)>⌐■", color: "#38BDF8" },
  sleep:     { face: "(￣Θ￣) z z", color: "#8B5CF6" },
  love:      { face: "( > 3 < )♥", color: "#F472B6" },
  game:      { face: "(ง •_•)ง", color: "#fbbf24" },
  analyze:   { face: "(-_-)ノ 📐", color: "#06B6D4" },
};

const SIZES = {
  xs: "text-[8px]",
  sm: "text-[9px]",
  md: "text-[11px]",
  lg: "text-[14px]",
};

export default function LunaMascot({ mood = "default", size = "md", animate = true, label }: LunaMascotProps) {
  const m = MOODS[mood];
  return (
    <div className={`flex flex-col items-center gap-0.5 ${animate ? "animate-float" : ""}`}>
      <div
        className={`font-mono-jb font-bold ${SIZES[size]} select-none`}
        style={{ color: m.color, textShadow: `0 0 8px ${m.color}60` }}
      >
        {m.face}
      </div>
      {label && (
        <span className="font-dm text-[7px] text-white/40 tracking-wide uppercase">{label}</span>
      )}
    </div>
  );
}
