import { useEffect, useRef, useState, useCallback } from "react";

const W = 220, H = 180;
const SHIP_W = 18, SHIP_H = 22;

interface Bullet { x: number; y: number }
interface Asteroid { x: number; y: number; r: number; vx: number; vy: number; rot: number; rotV: number }
interface Star { x: number; y: number; b: number }

export default function LunaSpace({ onGameOver, highScore }: { onGameOver: (score: number) => void; highScore: number }) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const state = useRef({
    ship: W / 2, targetX: W / 2,
    bullets: [] as Bullet[],
    asteroids: [] as Asteroid[],
    stars: Array.from({ length: 30 }, () => ({ x: Math.random() * W, y: Math.random() * H, b: Math.random() })) as Star[],
    score: 0, lives: 3, shootTimer: 0, spawnTimer: 0, frame: 0, alive: true,
  });
  const [score, setScore] = useState(0);
  const [lives, setLives] = useState(3);
  const scoreRef = useRef(0);

  const move = useCallback((dir: "left" | "right") => {
    const s = state.current;
    s.targetX = Math.max(SHIP_W, Math.min(W - SHIP_W, s.targetX + (dir === "left" ? -28 : 28)));
  }, []);

  useEffect(() => {
    const canvas = canvasRef.current!;
    const ctx = canvas.getContext("2d")!;
    let raf = 0, last = 0;
    const s = state.current;

    function spawnAsteroid() {
      const r = 8 + Math.random() * 12;
      s.asteroids.push({ x: Math.random() * W, y: -r, r, vx: (Math.random() - 0.5) * 60, vy: 30 + Math.random() * 50, rot: 0, rotV: (Math.random() - 0.5) * 3 });
    }

    function drawShip(x: number, y: number) {
      ctx.save(); ctx.translate(x, y);
      ctx.fillStyle = "#38BDF8";
      ctx.shadowColor = "#38BDF8"; ctx.shadowBlur = 8;
      ctx.beginPath(); ctx.moveTo(0, -SHIP_H / 2); ctx.lineTo(SHIP_W / 2, SHIP_H / 2); ctx.lineTo(0, SHIP_H / 3); ctx.lineTo(-SHIP_W / 2, SHIP_H / 2); ctx.closePath(); ctx.fill();
      ctx.shadowBlur = 0;
      ctx.fillStyle = "#0284C7";
      ctx.beginPath(); ctx.moveTo(0, -SHIP_H / 2 + 4); ctx.lineTo(5, SHIP_H / 3); ctx.lineTo(0, SHIP_H / 4); ctx.lineTo(-5, SHIP_H / 3); ctx.closePath(); ctx.fill();
      // Engine flame
      if (s.frame % 4 < 2) {
        ctx.fillStyle = "#fbbf24"; ctx.shadowColor = "#fbbf24"; ctx.shadowBlur = 6;
        ctx.beginPath(); ctx.moveTo(-5, SHIP_H / 2); ctx.lineTo(5, SHIP_H / 2); ctx.lineTo(0, SHIP_H / 2 + 8); ctx.closePath(); ctx.fill();
      }
      ctx.restore();
    }

    function drawAsteroid(a: Asteroid) {
      ctx.save(); ctx.translate(a.x, a.y); ctx.rotate(a.rot);
      ctx.strokeStyle = "#8B5CF6"; ctx.fillStyle = "rgba(139,92,246,0.3)";
      ctx.lineWidth = 1.5;
      ctx.shadowColor = "#8B5CF6"; ctx.shadowBlur = 4;
      ctx.beginPath();
      for (let i = 0; i < 8; i++) {
        const angle = (i / 8) * Math.PI * 2;
        const r = a.r * (0.7 + Math.sin(i * 37 + a.rot) * 0.3);
        if (i === 0) ctx.moveTo(r * Math.cos(angle), r * Math.sin(angle));
        else ctx.lineTo(r * Math.cos(angle), r * Math.sin(angle));
      }
      ctx.closePath(); ctx.fill(); ctx.stroke();
      ctx.restore();
    }

    function loop(now: number) {
      const dt = Math.min((now - last) / 1000, 0.05);
      last = now;
      s.frame++;
      if (!s.alive) return;

      s.ship += (s.targetX - s.ship) * Math.min(1, dt * 10);
      s.shootTimer += dt;
      if (s.shootTimer > 0.22) { s.shootTimer = 0; s.bullets.push({ x: s.ship, y: H - 30 }); }
      s.spawnTimer += dt;
      if (s.spawnTimer > Math.max(0.4, 1.2 - s.score * 0.001)) { s.spawnTimer = 0; spawnAsteroid(); }

      for (const b of s.bullets) b.y -= 220 * dt;
      s.bullets = s.bullets.filter(b => b.y > -10);
      for (const a of s.asteroids) { a.x += a.vx * dt; a.y += a.vy * dt; a.rot += a.rotV * dt; if (a.x < -a.r) a.x = W + a.r; if (a.x > W + a.r) a.x = -a.r; }
      s.asteroids = s.asteroids.filter(a => a.y < H + a.r);

      const toRemoveB = new Set<Bullet>();
      const toRemoveA = new Set<Asteroid>();
      for (const b of s.bullets) {
        for (const a of s.asteroids) {
          if (Math.hypot(b.x - a.x, b.y - a.y) < a.r) { toRemoveB.add(b); toRemoveA.add(a); s.score += Math.round(10 + a.r); scoreRef.current = s.score; setScore(s.score); }
        }
      }
      s.bullets = s.bullets.filter(b => !toRemoveB.has(b));
      s.asteroids = s.asteroids.filter(a => !toRemoveA.has(a));

      const sy = H - 30;
      for (const a of s.asteroids) {
        if (Math.hypot(a.x - s.ship, a.y - sy) < a.r + 10) { s.lives--; setLives(s.lives); if (s.lives <= 0) { s.alive = false; onGameOver(scoreRef.current); return; } s.asteroids = s.asteroids.filter(x => x !== a); break; }
      }

      // Draw
      ctx.fillStyle = "#020409";
      ctx.fillRect(0, 0, W, H);
      for (const star of s.stars) {
        const b = (Math.sin(s.frame * 0.03 + star.b * 10) + 1) / 2 * 0.6 + 0.1;
        ctx.fillStyle = `rgba(255,255,255,${b})`;
        ctx.fillRect(star.x, star.y, 1, 1);
      }
      for (const b of s.bullets) { ctx.fillStyle = "#fbbf24"; ctx.shadowColor = "#fbbf24"; ctx.shadowBlur = 4; ctx.fillRect(b.x - 1.5, b.y - 6, 3, 6); ctx.shadowBlur = 0; }
      for (const a of s.asteroids) drawAsteroid(a);
      drawShip(s.ship, H - 30);

      ctx.fillStyle = "#38BDF8";
      ctx.font = "bold 10px 'JetBrains Mono', monospace";
      ctx.textAlign = "left"; ctx.textBaseline = "top";
      ctx.fillText(`${s.score}`, 6, 4);
      ctx.textAlign = "right";
      ctx.fillStyle = "#ef4444";
      ctx.fillText("♥".repeat(Math.max(0, s.lives)), W - 4, 4);

      raf = requestAnimationFrame(loop);
    }
    raf = requestAnimationFrame(loop);
    return () => cancelAnimationFrame(raf);
  }, [onGameOver]);

  useEffect(() => {
    function onKey(e: KeyboardEvent) {
      if (e.key === "ArrowLeft" || e.key === "a") move("left");
      if (e.key === "ArrowRight" || e.key === "d") move("right");
    }
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [move]);

  return (
    <div>
      <canvas ref={canvasRef} width={W} height={H} className="game-canvas block" style={{ width: W, height: H }}
        onPointerDown={e => {
          const rect = (e.target as HTMLElement).getBoundingClientRect();
          move(e.clientX - rect.left < W / 2 ? "left" : "right");
        }}
      />
      <div className="flex justify-center gap-4 mt-1">
        <button onClick={() => move("left")} className="font-pixel text-[8px] text-white/50 px-2 py-0.5 bg-white/10 rounded interactive-tap">◄</button>
        <button onClick={() => move("right")} className="font-pixel text-[8px] text-white/50 px-2 py-0.5 bg-white/10 rounded interactive-tap">►</button>
      </div>
    </div>
  );
}
