import { useEffect, useRef, useState, useCallback } from "react";

const W = 220, H = 180;
const GROUND_Y = H - 20;
const GRAVITY = 700, JUMP_V = -280;
const MOCHY_X = 45, MOCHY_W = 18, MOCHY_H = 20;

interface Obstacle { x: number; w: number; h: number; type: "rock" | "spike" | "cactus" }

export default function MochyJump({ onGameOver, highScore }: { onGameOver: (score: number) => void; highScore: number }) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const state = useRef({ y: GROUND_Y - MOCHY_H, vy: 0, onGround: true, obstacles: [] as Obstacle[], score: 0, speed: 100, spawnTimer: 0, frame: 0, alive: true, started: false });
  const scoreRef = useRef(0);
  const [score, setScore] = useState(0);

  const jump = useCallback(() => {
    const s = state.current;
    if (!s.alive) return;
    s.started = true;
    if (s.onGround) { s.vy = JUMP_V; s.onGround = false; }
  }, []);

  useEffect(() => {
    const canvas = canvasRef.current!;
    const ctx = canvas.getContext("2d")!;
    let raf = 0, last = 0;
    const s = state.current;

    function spawnObstacle() {
      const types: Obstacle["type"][] = ["rock", "spike", "cactus"];
      const type = types[Math.floor(Math.random() * types.length)];
      const w = type === "spike" ? 12 : type === "rock" ? 18 : 14;
      const h = type === "spike" ? 20 : type === "rock" ? 16 : 24;
      s.obstacles.push({ x: W + w, w, h, type });
    }

    function drawMochy(x: number, y: number, frame: number) {
      ctx.save(); ctx.translate(x, y);
      const legToggle = Math.floor(frame / 8) % 2;
      // Legs
      ctx.fillStyle = "#F472B6";
      ctx.fillRect(-4, MOCHY_H - 8, 5, 8);
      ctx.fillRect(3, MOCHY_H - 8 + (legToggle ? 4 : 0), 5, 8 - (legToggle ? 4 : 0));
      // Body
      ctx.fillStyle = "#ec4899";
      ctx.beginPath(); ctx.roundRect(-MOCHY_W/2, 0, MOCHY_W, MOCHY_H - 6, 5); ctx.fill();
      // Face
      ctx.fillStyle = "#fce7f3";
      ctx.beginPath(); ctx.arc(0, MOCHY_H / 2 - 8, 7, 0, Math.PI * 2); ctx.fill();
      ctx.fillStyle = "#be185d";
      ctx.fillRect(-2, MOCHY_H / 2 - 10, 1.5, 1.5);
      ctx.fillRect(1, MOCHY_H / 2 - 10, 1.5, 1.5);
      // Ears
      ctx.fillStyle = "#ec4899";
      ctx.beginPath(); ctx.moveTo(-6, -2); ctx.lineTo(-9, -8); ctx.lineTo(-3, -4); ctx.closePath(); ctx.fill();
      ctx.beginPath(); ctx.moveTo(6, -2); ctx.lineTo(9, -8); ctx.lineTo(3, -4); ctx.closePath(); ctx.fill();
      ctx.restore();
    }

    function drawObstacle(o: Obstacle) {
      ctx.save(); ctx.translate(o.x, GROUND_Y - o.h);
      if (o.type === "rock") {
        ctx.fillStyle = "#6b7280";
        ctx.beginPath(); ctx.roundRect(-o.w/2, 0, o.w, o.h, 4); ctx.fill();
        ctx.fillStyle = "rgba(255,255,255,0.15)"; ctx.fillRect(-o.w/2 + 3, 3, o.w/2, 4);
      } else if (o.type === "spike") {
        ctx.fillStyle = "#ef4444";
        ctx.beginPath(); ctx.moveTo(0, 0); ctx.lineTo(-o.w/2, o.h); ctx.lineTo(o.w/2, o.h); ctx.closePath(); ctx.fill();
        ctx.strokeStyle = "#fca5a5"; ctx.lineWidth = 1; ctx.stroke();
      } else {
        ctx.fillStyle = "#16a34a";
        ctx.fillRect(-o.w/2, o.h/3, o.w, o.h * 2/3);
        ctx.beginPath(); ctx.arc(0, o.h/3, o.w/2, 0, Math.PI * 2); ctx.fill();
        ctx.beginPath(); ctx.arc(-o.w/2, o.h * 0.5, o.w/3, 0, Math.PI * 2); ctx.fill();
        ctx.beginPath(); ctx.arc(o.w/2, o.h * 0.5, o.w/3, 0, Math.PI * 2); ctx.fill();
      }
      ctx.restore();
    }

    function loop(now: number) {
      const dt = Math.min((now - last) / 1000, 0.05);
      last = now;
      s.frame++;
      if (!s.alive) return;

      if (s.started) {
        s.score += dt * 12;
        scoreRef.current = Math.floor(s.score);
        setScore(Math.floor(s.score));
        s.speed = Math.min(250, 100 + s.score * 0.15);
        s.vy += GRAVITY * dt;
        s.y += s.vy * dt;
        if (s.y >= GROUND_Y - MOCHY_H) { s.y = GROUND_Y - MOCHY_H; s.vy = 0; s.onGround = true; }
        s.spawnTimer += dt;
        if (s.spawnTimer > Math.max(0.8, 2.5 - s.score * 0.003)) { s.spawnTimer = 0; spawnObstacle(); }
        for (const o of s.obstacles) o.x -= s.speed * dt;
        s.obstacles = s.obstacles.filter(o => o.x > -o.w);
        // Collision
        for (const o of s.obstacles) {
          if (Math.abs(o.x - MOCHY_X) < (o.w/2 + MOCHY_W/2 - 4) && GROUND_Y - o.h < s.y + MOCHY_H - 4) {
            s.alive = false; onGameOver(scoreRef.current); return;
          }
        }
      }

      // Draw
      ctx.fillStyle = "#0a0a14";
      ctx.fillRect(0, 0, W, H);
      // Background hills
      ctx.fillStyle = "#0f172a";
      for (let i = 0; i < 4; i++) {
        const ox = (i * 60 - (s.score * 0.3) % 60);
        ctx.beginPath(); ctx.arc(ox, GROUND_Y - 10, 30, 0, Math.PI * 2); ctx.fill();
      }
      // Ground
      ctx.fillStyle = "#1e293b";
      ctx.fillRect(0, GROUND_Y, W, H - GROUND_Y);
      ctx.fillStyle = "rgba(56,189,248,0.3)";
      ctx.fillRect(0, GROUND_Y, W, 2);

      for (const o of s.obstacles) drawObstacle(o);
      drawMochy(MOCHY_X, s.y, s.frame);

      if (!s.started) {
        ctx.fillStyle = "rgba(255,255,255,0.5)";
        ctx.font = "7px 'Press Start 2P', monospace";
        ctx.textAlign = "center";
        ctx.fillText("TAP / SPACE TO START", W/2, H/2);
      }

      ctx.fillStyle = "#38BDF8";
      ctx.font = "bold 10px 'JetBrains Mono', monospace";
      ctx.textAlign = "right"; ctx.textBaseline = "top";
      ctx.fillText(`${Math.floor(s.score)} m`, W - 4, 4);

      raf = requestAnimationFrame(loop);
    }
    raf = requestAnimationFrame(loop);
    return () => cancelAnimationFrame(raf);
  }, [onGameOver]);

  useEffect(() => {
    function onKey(e: KeyboardEvent) { if (e.code === "Space" || e.key === "ArrowUp") jump(); }
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [jump]);

  return <canvas ref={canvasRef} width={W} height={H} className="game-canvas block cursor-pointer" style={{ width: W, height: H }} onClick={jump} />;
}
