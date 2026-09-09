import { useEffect, useRef, useState, useCallback } from "react";

const W = 220, H = 180;
const BIRD_X = 50, GRAVITY = 600, JUMP = -250, PIPE_W = 28, GAP = 55, PIPE_SPEED = 110;

interface Pipe { x: number; top: number; scored: boolean }

export default function FlappyMochy({ onGameOver, highScore }: { onGameOver: (score: number) => void; highScore: number }) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const state = useRef({ vy: 0, y: H / 2, pipes: [] as Pipe[], score: 0, frame: 0, alive: true, started: false });
  const [score, setScore] = useState(0);
  const scoreRef = useRef(0);

  const flap = useCallback(() => {
    const s = state.current;
    if (!s.alive) return;
    s.started = true;
    s.vy = JUMP;
  }, []);

  useEffect(() => {
    const canvas = canvasRef.current!;
    const ctx = canvas.getContext("2d")!;
    let raf = 0, last = 0;
    const s = state.current;

    function spawnPipe() {
      const top = 20 + Math.random() * (H - GAP - 40);
      s.pipes.push({ x: W + PIPE_W, top, scored: false });
    }

    function drawBird(x: number, y: number, vy: number, frame: number) {
      ctx.save();
      ctx.translate(x, y);
      const angle = Math.max(-30, Math.min(60, vy / 8));
      ctx.rotate(angle * Math.PI / 180);
      // Body
      ctx.fillStyle = "#F472B6";
      ctx.beginPath(); ctx.ellipse(0, 0, 12, 9, 0, 0, Math.PI * 2); ctx.fill();
      // Wing flap
      ctx.fillStyle = "#ec4899";
      ctx.beginPath();
      ctx.ellipse(-2, frame % 8 < 4 ? 3 : 0, 8, 4, -0.3, 0, Math.PI * 2);
      ctx.fill();
      // Eye
      ctx.fillStyle = "white"; ctx.beginPath(); ctx.arc(5, -3, 3, 0, Math.PI * 2); ctx.fill();
      ctx.fillStyle = "#1a1a2e"; ctx.beginPath(); ctx.arc(6, -3, 1.5, 0, Math.PI * 2); ctx.fill();
      // Beak
      ctx.fillStyle = "#fbbf24";
      ctx.beginPath(); ctx.moveTo(10, -1); ctx.lineTo(15, 0); ctx.lineTo(10, 2); ctx.closePath(); ctx.fill();
      ctx.restore();
    }

    spawnPipe();

    function loop(now: number) {
      const dt = Math.min((now - last) / 1000, 0.05);
      last = now;
      s.frame++;

      if (!s.alive) return;

      if (s.started) {
        s.vy += GRAVITY * dt;
        s.y += s.vy * dt;
      }

      if (s.pipes.length === 0 || s.pipes[s.pipes.length - 1].x < W - 100) spawnPipe();

      for (const pipe of s.pipes) {
        pipe.x -= PIPE_SPEED * dt;
        if (!pipe.scored && pipe.x + PIPE_W < BIRD_X) {
          pipe.scored = true;
          s.score++;
          scoreRef.current = s.score;
          setScore(s.score);
        }
      }
      s.pipes = s.pipes.filter(p => p.x + PIPE_W > 0);

      // Collision
      if (s.y > H - 12 || s.y < 12) { s.alive = false; onGameOver(scoreRef.current); return; }
      for (const p of s.pipes) {
        if (BIRD_X + 10 > p.x && BIRD_X - 10 < p.x + PIPE_W) {
          if (s.y - 9 < p.top || s.y + 9 > p.top + GAP) { s.alive = false; onGameOver(scoreRef.current); return; }
        }
      }

      // Draw sky
      ctx.fillStyle = "#070A13";
      ctx.fillRect(0, 0, W, H);

      // Stars
      ctx.fillStyle = "rgba(255,255,255,0.3)";
      for (let i = 0; i < 20; i++) {
        ctx.fillRect((i * 37 + s.frame * 0.2) % W, (i * 23) % H, 1, 1);
      }

      // Pipes
      for (const p of s.pipes) {
        const grad = ctx.createLinearGradient(p.x, 0, p.x + PIPE_W, 0);
        grad.addColorStop(0, "#166534"); grad.addColorStop(1, "#15803d");
        ctx.fillStyle = grad;
        ctx.fillRect(p.x, 0, PIPE_W, p.top);
        ctx.fillRect(p.x, p.top + GAP, PIPE_W, H - p.top - GAP);
        ctx.fillStyle = "#14532d";
        ctx.fillRect(p.x - 3, p.top - 8, PIPE_W + 6, 8);
        ctx.fillRect(p.x - 3, p.top + GAP, PIPE_W + 6, 8);
      }

      // Ground
      ctx.fillStyle = "#16a34a";
      ctx.fillRect(0, H - 8, W, 8);

      drawBird(BIRD_X, s.y, s.vy, s.frame);

      // Score
      ctx.fillStyle = "white";
      ctx.font = "bold 12px 'JetBrains Mono', monospace";
      ctx.textAlign = "center";
      ctx.fillText(`${s.score}`, W / 2, 20);

      if (!s.started) {
        ctx.fillStyle = "rgba(255,255,255,0.6)";
        ctx.font = "8px 'Press Start 2P', monospace";
        ctx.fillText("TAP TO START", W / 2, H / 2);
      }

      raf = requestAnimationFrame(loop);
    }

    raf = requestAnimationFrame(loop);
    return () => cancelAnimationFrame(raf);
  }, [onGameOver]);

  useEffect(() => {
    function onKey(e: KeyboardEvent) { if (e.code === "Space") flap(); }
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [flap]);

  return (
    <canvas
      ref={canvasRef}
      width={W}
      height={H}
      className="game-canvas block cursor-pointer"
      style={{ width: W, height: H }}
      onClick={flap}
    />
  );
}
