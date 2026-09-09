import type React from "react";

interface ProgressRingProps {
  progress: number; // 0–1
  size?: number;
  strokeWidth?: number;
  color?: string;
  trackColor?: string;
  children?: React.ReactNode;
  glow?: boolean;
}

export default function ProgressRing({
  progress,
  size = 120,
  strokeWidth = 6,
  color = "#38BDF8",
  trackColor = "rgba(56,189,248,0.12)",
  children,
  glow = true,
}: ProgressRingProps) {
  const r = (size - strokeWidth) / 2;
  const circ = 2 * Math.PI * r;
  const offset = circ * (1 - Math.max(0, Math.min(1, progress)));
  const cx = size / 2;

  return (
    <div className="relative flex items-center justify-center" style={{ width: size, height: size }}>
      <svg width={size} height={size} style={{ position: "absolute", top: 0, left: 0, transform: "rotate(-90deg)" }}>
        <circle cx={cx} cy={cx} r={r} fill="none" stroke={trackColor} strokeWidth={strokeWidth} />
        <circle
          cx={cx}
          cy={cx}
          r={r}
          fill="none"
          stroke={color}
          strokeWidth={strokeWidth}
          strokeDasharray={circ}
          strokeDashoffset={offset}
          strokeLinecap="round"
          style={{
            transition: "stroke-dashoffset 0.5s ease",
            filter: glow ? `drop-shadow(0 0 4px ${color})` : undefined,
          }}
        />
      </svg>
      <div className="flex items-center justify-center" style={{ width: size - strokeWidth * 2 - 8, height: size - strokeWidth * 2 - 8 }}>
        {children}
      </div>
    </div>
  );
}
