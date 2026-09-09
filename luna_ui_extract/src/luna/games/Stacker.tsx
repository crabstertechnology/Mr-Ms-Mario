import { useEffect, useRef, useState, useCallback } from "react";

const W = 220, H = 180;
const COLS = 7, MAX_ROWS = 10;
const CELL = Math.floor(W / COLS);
const BLOCK_H = 14;

interface Block { col: number; width: number; y: number }
interface StackedBlock { col: number; width: number; row: number }

export default function Stacker({ onGameOver, highScore }: { onGameOver: (score: number) => void; highScore: number }) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const state = useRef({
    currentBlock: { col: 0, width: COLS, y: BLOCK_H } as Block,
    stacked: [] as StackedBlock[],
    dir: 1,
    score: 0,
    speed: 2.5,
    alive: true,
    row: 0,
    frame: 0,
  });
  const [score, setScore] = useState(0);
  const scoreRef = useRef(0);

  const drop = useCallback(() => {
    const s = state.current;
    if (!s.alive) return;

    if (s.stacked.length === 0) {
      s.stacked.push({ col: s.currentBlock.col, width: s.currentBlock.width, row: 0 });
    } else {
      const prev = s.stacked[s.stacked.length - 1];
      const curLeft = s.currentBlock.col;
      const curRight = s.currentBlock.col + s.currentBlock.width;
      const prevLeft = prev.col;
      const prevRight = prev.col + prev.width;
      const overlapLeft = Math.max(curLeft, prevLeft);
      const overlapRight = Math.min(curRight, prevRight);
      if (overlapLeft >= overlapRight) {
        s.alive = false;
        onGameOver(scoreRef.current);
        return;
      }
      const newWidth = overlapRight - overlapLeft;
      s.stacked.push({ col: overlapLeft, width: newWidth, row: s.stacked.length });
      s.score += newWidth;
      scoreRef.current = s.score;
      setScore(s.score);
      s.speed = Math.min(8, 2.5 + s.stacked.length * 0.2);
      if (s.stacked.length >= MAX_ROWS) { s.alive = false; onGameOver(scoreRef.current + 100); return; }
      s.currentBlock = { col: 0, width: newWidth, y: BLOCK_H };
    }
  }, [onGameOver]);

  useEffect(() => {
    const canvas = canvasRef.current!;
    const ctx = canvas.getContext("2d")!;
    let raf = 0;
    const s = state.current;
    const COLORS = ["#38BDF8", "#8B5CF6", "#F472B6", "#4ade80", "#fbbf24", "#ef4444", "#06B6D4", "#a78bfa"];

    function getColor(row: number) { return COLORS[row % COLORS.length]; }

    function drawBlock(col: number, width: number, yPx: number, color: string) {
      const x = col * CELL + 2;
      const w = width * CELL - 4;
      ctx.fillStyle = color;
      ctx.shadowColor = color; ctx.shadowBlur = 4;
      ctx.beginPath(); ctx.roundRect(x, yPx, w, BLOCK_H - 2, 2); ctx.fill();
      ctx.shadowBlur = 0;
      ctx.fillStyle = "rgba(255,255,255,0.25)";
      ctx.fillRect(x + 2, yPx + 2, w - 4, 3);
    }

    function loop() {
      s.frame++;
      if (!s.alive) return;

      // Move block
      s.currentBlock.col += s.dir * s.speed;
      if (s.currentBlock.col + s.currentBlock.width > COLS) {
        s.currentBlock.col = COLS - s.currentBlock.width;
        s.dir = -1;
      }
      if (s.currentBlock.col < 0) {
        s.currentBlock.col = 0;
        s.dir = 1;
      }

      const stackHeight = s.stacked.length;
      const baseY = H - BLOCK_H;
      const currentY = baseY - stackHeight * BLOCK_H;

      ctx.fillStyle = "#070A13";
      ctx.fillRect(0, 0, W, H);

      // Draw stacked blocks
      for (const block of s.stacked) {
        const y = baseY - block.row * BLOCK_H;
        drawBlock(block.col, block.width, y, getColor(block.row));
      }

      // Draw moving block
      drawBlock(s.currentBlock.col, s.currentBlock.width, currentY, getColor(stackHeight));

      // Grid guide lines
      ctx.strokeStyle = "rgba(255,255,255,0.04)";
      ctx.lineWidth = 1;
      for (let c = 0; c <= COLS; c++) {
        ctx.beginPath(); ctx.moveTo(c * CELL, 0); ctx.lineTo(c * CELL, H); ctx.stroke();
      }

      // Score
      ctx.fillStyle = "#38BDF8";
      ctx.font = "bold 10px 'JetBrains Mono', monospace";
      ctx.textAlign = "left"; ctx.textBaseline = "top";
      ctx.fillText(`SCORE ${s.score}`, 6, 4);
      ctx.fillStyle = "rgba(255,255,255,0.3)";
      ctx.fillText(`LVL ${stackHeight + 1}`, 6, 16);

      // Height indicator
      for (let r = 0; r < MAX_ROWS; r++) {
        const isFilled = r < stackHeight;
        ctx.fillStyle = isFilled ? getColor(r) : "rgba(255,255,255,0.06)";
        ctx.fillRect(W - 8, H - (r + 1) * BLOCK_H, 5, BLOCK_H - 2);
      }

      raf = requestAnimationFrame(loop);
    }
    raf = requestAnimationFrame(loop);
    return () => cancelAnimationFrame(raf);
  }, []);

  useEffect(() => {
    function onKey(e: KeyboardEvent) { if (e.code === "Space" || e.key === "Enter") drop(); }
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [drop]);

  return (
    <canvas ref={canvasRef} width={W} height={H} className="game-canvas block cursor-pointer" style={{ width: W, height: H }} onClick={drop} />
  );
}
