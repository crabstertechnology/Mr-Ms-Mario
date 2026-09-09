import { useEffect, useRef, useState, useCallback } from "react";

const W = 220, H = 180;
const BASKET_W = 38, BASKET_H = 16;
const COIN_R = 7;

interface Coin { x: number; y: number; speed: number; color: string; value: number }

export default function CoinCatcher({ onGameOver, highScore }: { onGameOver: (score: number) => void; highScore: number }) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const state = useRef({ basket: W / 2, coins: [] as Coin[], score: 0, lives: 3, spawnTimer: 0, frame: 0, alive: true, speed: 60 });
  const [score, setScore] = useState(0);
  const [lives, setLives] = useState(3);
  const scoreRef = useRef(0);

  const move = useCallback((dir: "left" | "right") => {
    const s = state.current;
    s.basket = Math.max(BASKET_W / 2, Math.min(W - BASKET_W / 2, s.basket + (dir === "left" ? -24 : 24)));
  }, []);

  useEffect(() => {
    const canvas = canvasRef.current!;
    const ctx = canvas.getContext("2d")!;
    let raf = 0, last = 0;
    const s = state.current;
    const COIN_COLORS = ["#fbbf24", "#f59e0b", "#a3e635", "#38BDF8", "#F472B6"];
    const COIN_VALUES = [1, 1, 2, 3, 5];

    function spawnCoin() {
      const idx = Math.floor(Math.random() * COIN_COLORS.length);
      s.coins.push({ x: COIN_R + Math.random() * (W - COIN_R * 2), y: -COIN_R, speed: s.speed * (0.8 + Math.random() * 0.5), color: COIN_COLORS[idx], value: COIN_VALUES[idx] });
    }

    function loop(now: number) {
      const dt = Math.min((now - last) / 1000, 0.05);
      last = now;
      s.frame++;
      if (!s.alive) return;

      s.speed = 60 + s.score * 0.3;
      s.spawnTimer += dt;
      if (s.spawnTimer > Math.max(0.35, 1.0 - s.score * 0.002)) {
        s.spawnTimer = 0;
        spawnCoin();
      }

      const BY = H - BASKET_H - 4;
      for (const coin of s.coins) { coin.y += coin.speed * dt; }

      const caught: Coin[] = [];
      const missed: Coin[] = [];
      for (const coin of s.coins) {
        if (coin.y + COIN_R > BY && coin.y - COIN_R < BY + BASKET_H && coin.x > s.basket - BASKET_W / 2 && coin.x < s.basket + BASKET_W / 2) {
          caught.push(coin);
        } else if (coin.y > H + COIN_R) {
          missed.push(coin);
        }
      }
      s.coins = s.coins.filter(c => !caught.includes(c) && !missed.includes(c));
      for (const c of caught) { s.score += c.value; scoreRef.current = s.score; setScore(s.score); }
      if (missed.length > 0) {
        s.lives -= missed.length;
        setLives(Math.max(0, s.lives));
        if (s.lives <= 0) { s.alive = false; onGameOver(scoreRef.current); return; }
      }

      // Draw
      ctx.fillStyle = "#070A13";
      ctx.fillRect(0, 0, W, H);

      // Stars BG
      ctx.fillStyle = "rgba(56,189,248,0.2)";
      for (let i = 0; i < 15; i++) ctx.fillRect((i * 41 + s.frame * 0.1) % W, (i * 29 + s.frame * 0.05) % (H - 30), 1, 1);

      // Coins
      for (const coin of s.coins) {
        const pulse = Math.sin(s.frame * 0.15 + coin.x) * 0.15 + 0.85;
        ctx.save();
        ctx.beginPath(); ctx.arc(coin.x, coin.y, COIN_R * pulse, 0, Math.PI * 2);
        ctx.fillStyle = coin.color;
        ctx.shadowColor = coin.color; ctx.shadowBlur = 6;
        ctx.fill();
        ctx.shadowBlur = 0;
        ctx.fillStyle = "rgba(255,255,255,0.6)";
        ctx.font = `bold ${COIN_R - 1}px monospace`;
        ctx.textAlign = "center"; ctx.textBaseline = "middle";
        ctx.fillText(`${coin.value}`, coin.x, coin.y);
        ctx.restore();
      }

      // Basket
      ctx.save();
      ctx.fillStyle = "#1e3a5f";
      ctx.strokeStyle = "#38BDF8";
      ctx.lineWidth = 1.5;
      ctx.shadowColor = "#38BDF8"; ctx.shadowBlur = 6;
      ctx.beginPath(); ctx.roundRect(s.basket - BASKET_W / 2, BY, BASKET_W, BASKET_H, 3);
      ctx.fill(); ctx.stroke();
      ctx.shadowBlur = 0;
      ctx.restore();

      // Ground
      ctx.fillStyle = "rgba(56,189,248,0.1)";
      ctx.fillRect(0, H - 4, W, 4);

      // HUD
      ctx.fillStyle = "#38BDF8";
      ctx.font = "bold 10px 'JetBrains Mono', monospace";
      ctx.textAlign = "left"; ctx.textBaseline = "top";
      ctx.fillText(`${s.score}`, 6, 4);
      ctx.textAlign = "right";
      ctx.fillStyle = "#ef4444";
      ctx.fillText("❤".repeat(Math.max(0, s.lives)), W - 4, 4);

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
