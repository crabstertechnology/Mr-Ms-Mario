import { useEffect, useRef, useState, useCallback } from "react";

interface Props {
  onGameOver: (score: number) => void;
  highScore: number;
}

const W = 220, H = 180;
const LANES = [W * 0.25, W * 0.5, W * 0.75];
const CAR_W = 22, CAR_H = 36;

interface Car { x: number; y: number; lane: number; color: string; speed: number }
interface PlayerCar { lane: number; x: number; targetX: number }

const CAR_COLORS = ["#ef4444", "#fbbf24", "#8B5CF6", "#ec4899", "#10b981"];

export default function LunaRacer({ onGameOver, highScore }: Props) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const state = useRef({
    player: { lane: 1, x: LANES[1], targetX: LANES[1] } as PlayerCar,
    cars: [] as Car[],
    score: 0,
    speed: 120,
    spawnTimer: 0,
    roadOffset: 0,
    alive: true,
  });
  const [score, setScore] = useState(0);
  const scoreRef = useRef(0);

  const handleInput = useCallback((dir: "left" | "right") => {
    const s = state.current;
    if (!s.alive) return;
    const newLane = dir === "left" ? Math.max(0, s.player.lane - 1) : Math.min(2, s.player.lane + 1);
    s.player.lane = newLane;
    s.player.targetX = LANES[newLane];
  }, []);

  useEffect(() => {
    const canvas = canvasRef.current!;
    const ctx = canvas.getContext("2d")!;
    let raf = 0;
    let last = 0;
    const s = state.current;

    function spawnCar() {
      const lane = Math.floor(Math.random() * 3);
      s.cars.push({ x: LANES[lane], y: -CAR_H, lane, color: CAR_COLORS[Math.floor(Math.random() * CAR_COLORS.length)], speed: s.speed * (0.8 + Math.random() * 0.6) });
    }

    function drawCar(x: number, y: number, color: string, isPlayer = false) {
      ctx.save();
      // Body
      ctx.fillStyle = color;
      ctx.beginPath(); ctx.roundRect(x - CAR_W/2, y - CAR_H/2, CAR_W, CAR_H, 4); ctx.fill();
      // Windows
      ctx.fillStyle = isPlayer ? "rgba(56,189,248,0.8)" : "rgba(0,0,0,0.6)";
      ctx.beginPath(); ctx.roundRect(x - CAR_W/2 + 3, y - CAR_H/2 + 6, CAR_W - 6, 10, 2); ctx.fill();
      // Headlights
      if (isPlayer) {
        ctx.fillStyle = "#fbbf24";
        ctx.fillRect(x - CAR_W/2 + 2, y + CAR_H/2 - 4, 5, 3);
        ctx.fillRect(x + CAR_W/2 - 7, y + CAR_H/2 - 4, 5, 3);
      }
      ctx.restore();
    }

    function loop(now: number) {
      const dt = Math.min((now - last) / 1000, 0.05);
      last = now;

      if (!s.alive) return;
      // Update speed
      s.speed = Math.min(300, 120 + s.score * 0.05);
      s.score += dt * 10;
      scoreRef.current = Math.floor(s.score);
      setScore(Math.floor(s.score));

      // Road animation
      s.roadOffset = (s.roadOffset + s.speed * dt) % 40;

      // Move player
      s.player.x += (s.player.targetX - s.player.x) * Math.min(1, dt * 12);

      // Spawn cars
      s.spawnTimer += dt;
      if (s.spawnTimer > Math.max(0.5, 1.5 - s.score * 0.0002)) {
        s.spawnTimer = 0;
        spawnCar();
      }

      // Move cars
      for (const car of s.cars) {
        car.y += (s.speed + car.speed) * dt;
      }
      s.cars = s.cars.filter(c => c.y < H + CAR_H);

      // Collision
      const py = H - 50;
      for (const car of s.cars) {
        if (Math.abs(car.x - s.player.x) < CAR_W - 4 && Math.abs(car.y - py) < CAR_H - 6) {
          s.alive = false;
          onGameOver(scoreRef.current);
          return;
        }
      }

      // Draw
      ctx.fillStyle = "#0a0a0a";
      ctx.fillRect(0, 0, W, H);

      // Road
      ctx.fillStyle = "#1a1a2e";
      ctx.fillRect(W * 0.15, 0, W * 0.7, H);

      // Lane lines
      ctx.strokeStyle = "rgba(255,255,255,0.15)";
      ctx.setLineDash([20, 20]);
      ctx.lineWidth = 1;
      for (let i = 0; i < 2; i++) {
        const x = LANES[i] + (LANES[1] - LANES[0]) / 2;
        ctx.beginPath();
        for (let y = -s.roadOffset; y < H; y += 40) {
          ctx.moveTo(x, y); ctx.lineTo(x, y + 20);
        }
        ctx.stroke();
      }
      ctx.setLineDash([]);

      // Enemy cars
      for (const car of s.cars) drawCar(car.x, car.y, car.color);

      // Player car (blue)
      drawCar(s.player.x, py, "#38BDF8", true);

      // Score
      ctx.fillStyle = "#38BDF8";
      ctx.font = "bold 10px 'JetBrains Mono', monospace";
      ctx.textAlign = "left";
      ctx.fillText(`${Math.floor(s.score)}`, 6, 14);

      raf = requestAnimationFrame(loop);
    }

    raf = requestAnimationFrame(loop);
    return () => cancelAnimationFrame(raf);
  }, [onGameOver]);

  useEffect(() => {
    function onKey(e: KeyboardEvent) {
      if (e.key === "ArrowLeft" || e.key === "a") handleInput("left");
      if (e.key === "ArrowRight" || e.key === "d") handleInput("right");
    }
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [handleInput]);

  let tapStart = 0;
  return (
    <div className="relative">
      <canvas
        ref={canvasRef}
        width={W}
        height={H}
        className="game-canvas block"
        style={{ width: W, height: H }}
        onPointerDown={e => {
          const rect = (e.target as HTMLCanvasElement).getBoundingClientRect();
          const x = e.clientX - rect.left;
          tapStart = x;
        }}
        onPointerUp={e => {
          const rect = (e.target as HTMLCanvasElement).getBoundingClientRect();
          const x = e.clientX - rect.left;
          if (x < tapStart) handleInput("left");
          else if (x > tapStart) handleInput("right");
          else handleInput(x < W / 2 ? "left" : "right");
        }}
      />
      <div className="absolute bottom-1 left-0 right-0 flex justify-center gap-4">
        <button onClick={() => handleInput("left")} className="font-pixel text-[8px] text-white/50 px-2 py-0.5 bg-white/10 rounded interactive-tap">◄</button>
        <button onClick={() => handleInput("right")} className="font-pixel text-[8px] text-white/50 px-2 py-0.5 bg-white/10 rounded interactive-tap">►</button>
      </div>
    </div>
  );
}
