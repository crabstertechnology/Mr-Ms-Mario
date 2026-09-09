import { useEffect, useRef, useState, useCallback } from "react";

const W = 220, H = 170;
const CELL = 20, ROWS = 7, COLS = 9;
const BALL_R = 5;

// Simple maze: 0=open, 1=wall
const MAZE = [
  [1,1,1,1,1,1,1,1,1],
  [1,0,0,0,1,0,0,0,1],
  [1,0,1,0,0,0,1,0,1],
  [1,0,1,1,1,0,1,0,1],
  [1,0,0,0,1,0,0,0,1],
  [1,0,1,0,0,0,1,0,1],
  [1,1,1,1,1,1,1,1,1],
];

const OX = (W - COLS * CELL) / 2;
const OY = 10;

interface Ball { x: number; y: number; vx: number; vy: number }

function cellCenter(col: number, row: number) {
  return { x: OX + col * CELL + CELL / 2, y: OY + row * CELL + CELL / 2 };
}

export default function TiltMaze({ onGameOver, highScore }: { onGameOver: (score: number) => void; highScore: number }) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const state = useRef<{ ball: Ball; time: number; won: boolean; alive: boolean; tilt: { x: number; y: number } }>({
    ball: { ...cellCenter(1, 1), vx: 0, vy: 0 },
    time: 0,
    won: false,
    alive: true,
    tilt: { x: 0, y: 0 },
  });
  const [elapsed, setElapsed] = useState(0);

  const handleMouse = useCallback((e: MouseEvent) => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const rect = canvas.getBoundingClientRect();
    const cx = rect.left + W / 2;
    const cy = rect.top + H / 2;
    const dx = (e.clientX - cx) / (W / 2);
    const dy = (e.clientY - cy) / (H / 2);
    state.current.tilt = { x: dx * 2, y: dy * 2 };
  }, []);

  const handleKey = useCallback((e: KeyboardEvent) => {
    const STEP = 1;
    const t = state.current.tilt;
    if (e.key === "ArrowLeft") state.current.tilt = { ...t, x: Math.max(-1, t.x - STEP) };
    if (e.key === "ArrowRight") state.current.tilt = { ...t, x: Math.min(1, t.x + STEP) };
    if (e.key === "ArrowUp") state.current.tilt = { ...t, y: Math.max(-1, t.y - STEP) };
    if (e.key === "ArrowDown") state.current.tilt = { ...t, y: Math.min(1, t.y + STEP) };
  }, []);

  useEffect(() => {
    window.addEventListener("mousemove", handleMouse);
    window.addEventListener("keydown", handleKey);
    return () => { window.removeEventListener("mousemove", handleMouse); window.removeEventListener("keydown", handleKey); };
  }, [handleMouse, handleKey]);

  useEffect(() => {
    const canvas = canvasRef.current!;
    const ctx = canvas.getContext("2d")!;
    let raf = 0, last = 0;
    const s = state.current;
    const FRICTION = 0.82, GRAVITY_SCALE = 300;

    function isWall(cx: number, cy: number) {
      const col = Math.floor((cx - OX) / CELL);
      const row = Math.floor((cy - OY) / CELL);
      if (row < 0 || row >= ROWS || col < 0 || col >= COLS) return true;
      return MAZE[row][col] === 1;
    }

    function loop(now: number) {
      const dt = Math.min((now - last) / 1000, 0.05);
      last = now;
      if (!s.alive || s.won) return;

      s.time += dt;
      setElapsed(Math.floor(s.time));
      s.ball.vx += s.tilt.x * GRAVITY_SCALE * dt;
      s.ball.vy += s.tilt.y * GRAVITY_SCALE * dt;
      s.ball.vx *= FRICTION;
      s.ball.vy *= FRICTION;
      const speed = Math.hypot(s.ball.vx, s.ball.vy);
      const MAX_SPEED = 120;
      if (speed > MAX_SPEED) { s.ball.vx *= MAX_SPEED / speed; s.ball.vy *= MAX_SPEED / speed; }

      let nx = s.ball.x + s.ball.vx * dt;
      let ny = s.ball.y + s.ball.vy * dt;

      if (isWall(nx, s.ball.y)) { s.ball.vx *= -0.4; nx = s.ball.x; }
      if (isWall(s.ball.x, ny)) { s.ball.vy *= -0.4; ny = s.ball.y; }
      s.ball.x = nx; s.ball.y = ny;

      // Goal: bottom-right open cell
      const goal = cellCenter(7, 5);
      if (Math.hypot(s.ball.x - goal.x, s.ball.y - goal.y) < CELL / 2) {
        s.won = true;
        const score = Math.max(10, Math.round(100 - s.time * 2));
        onGameOver(score);
        return;
      }

      // Draw
      ctx.fillStyle = "#070A13";
      ctx.fillRect(0, 0, W, H + 10);

      for (let r = 0; r < ROWS; r++) {
        for (let c = 0; c < COLS; c++) {
          const x = OX + c * CELL, y = OY + r * CELL;
          if (MAZE[r][c] === 1) {
            ctx.fillStyle = "#0f1729";
            ctx.fillRect(x, y, CELL, CELL);
            ctx.strokeStyle = "rgba(56,189,248,0.2)";
            ctx.lineWidth = 0.5;
            ctx.strokeRect(x, y, CELL, CELL);
          } else {
            ctx.fillStyle = "#0a0f1e";
            ctx.fillRect(x, y, CELL, CELL);
          }
        }
      }

      // Goal cell
      const g = cellCenter(7, 5);
      ctx.fillStyle = "rgba(74,222,128,0.25)";
      ctx.fillRect(OX + 7 * CELL, OY + 5 * CELL, CELL, CELL);
      ctx.fillStyle = "#4ade80";
      ctx.font = "10px monospace";
      ctx.textAlign = "center";
      ctx.fillText("⬛", g.x, g.y + 4);

      // Start marker
      ctx.fillStyle = "rgba(56,189,248,0.15)";
      ctx.fillRect(OX + 1 * CELL, OY + 1 * CELL, CELL, CELL);

      // Ball
      const ballGrad = ctx.createRadialGradient(s.ball.x - 2, s.ball.y - 2, 1, s.ball.x, s.ball.y, BALL_R);
      ballGrad.addColorStop(0, "#7dd3fc"); ballGrad.addColorStop(1, "#0284C7");
      ctx.fillStyle = ballGrad;
      ctx.shadowColor = "#38BDF8"; ctx.shadowBlur = 8;
      ctx.beginPath(); ctx.arc(s.ball.x, s.ball.y, BALL_R, 0, Math.PI * 2); ctx.fill();
      ctx.shadowBlur = 0;

      // Timer
      ctx.fillStyle = "#38BDF8";
      ctx.font = "bold 10px 'JetBrains Mono', monospace";
      ctx.textAlign = "right"; ctx.textBaseline = "top";
      ctx.fillText(`${Math.floor(s.time)}s`, W - 4, 0);

      ctx.fillStyle = "rgba(255,255,255,0.2)";
      ctx.font = "6px 'JetBrains Mono', monospace";
      ctx.textAlign = "left";
      ctx.fillText("MOVE MOUSE / ARROWS", 4, 0);

      raf = requestAnimationFrame(loop);
    }
    raf = requestAnimationFrame(loop);
    return () => cancelAnimationFrame(raf);
  }, [onGameOver]);

  return <canvas ref={canvasRef} width={W} height={H} className="game-canvas block" style={{ width: W, height: H }} />;
}
