import { useState, useEffect, useCallback } from "react";

interface Props { onGameOver: (score: number) => void; highScore: number }

const EMOJIS = ["🌙", "⭐", "🚀", "🎮", "💎", "🔥", "⚡", "🎯"];

function shuffle<T>(arr: T[]): T[] {
  const a = [...arr];
  for (let i = a.length - 1; i > 0; i--) {
    const j = Math.floor(Math.random() * (i + 1));
    [a[i], a[j]] = [a[j], a[i]];
  }
  return a;
}

interface Card { id: number; emoji: string; matched: boolean; flipped: boolean }

function createCards(count: number): Card[] {
  const emojis = EMOJIS.slice(0, count / 2);
  const pairs = [...emojis, ...emojis];
  return shuffle(pairs).map((emoji, i) => ({ id: i, emoji, matched: false, flipped: false }));
}

export default function MemoryMatrix({ onGameOver, highScore }: Props) {
  const [cards, setCards] = useState<Card[]>(() => createCards(16));
  const [flipped, setFlipped] = useState<number[]>([]);
  const [moves, setMoves] = useState(0);
  const [score, setScore] = useState(0);
  const [locked, setLocked] = useState(false);
  const [won, setWon] = useState(false);

  const handleFlip = useCallback((id: number) => {
    if (locked || won) return;
    const card = cards.find(c => c.id === id);
    if (!card || card.flipped || card.matched) return;
    if (flipped.length === 1 && flipped[0] === id) return;

    const newFlipped = [...flipped, id];
    setCards(cs => cs.map(c => c.id === id ? { ...c, flipped: true } : c));

    if (newFlipped.length === 2) {
      setLocked(true);
      setMoves(m => m + 1);
      const [a, b] = newFlipped;
      const ca = cards.find(c => c.id === a)!;
      const cb = cards.find(c => c.id === b)!;
      if (ca.emoji === cb.emoji) {
        setTimeout(() => {
          setCards(cs => cs.map(c => c.id === a || c.id === b ? { ...c, matched: true } : c));
          const newScore = score + 10;
          setScore(newScore);
          setFlipped([]);
          setLocked(false);
          const allMatched = cards.filter(c => c.id !== a && c.id !== b && !c.matched).length === 0;
          if (allMatched) { setWon(true); onGameOver(newScore + Math.max(0, 100 - moves * 5)); }
        }, 500);
      } else {
        setTimeout(() => {
          setCards(cs => cs.map(c => c.id === a || c.id === b ? { ...c, flipped: false } : c));
          setFlipped([]);
          setLocked(false);
        }, 800);
      }
    } else {
      setFlipped(newFlipped);
    }
  }, [cards, flipped, locked, won, score, moves, onGameOver]);

  const matched = cards.filter(c => c.matched).length / 2;

  return (
    <div className="flex flex-col items-center gap-2 p-2 w-full" style={{ height: 180 }}>
      {/* Header */}
      <div className="flex justify-between w-full px-1">
        <span className="font-mono-jb text-[8px] text-[#F472B6]">PAIRS {matched}/8</span>
        <span className="font-mono-jb text-[8px] text-sky-400">MOVES {moves}</span>
      </div>
      {/* Grid */}
      <div className="grid gap-1" style={{ gridTemplateColumns: "repeat(4, 1fr)", width: 200 }}>
        {cards.map(card => (
          <button
            key={card.id}
            onClick={() => handleFlip(card.id)}
            className="interactive-tap flex items-center justify-center rounded"
            style={{
              width: 46,
              height: 36,
              background: card.matched
                ? "rgba(244,114,182,0.2)"
                : card.flipped
                  ? "rgba(56,189,248,0.2)"
                  : "rgba(255,255,255,0.06)",
              border: card.matched
                ? "1px solid rgba(244,114,182,0.4)"
                : card.flipped
                  ? "1px solid rgba(56,189,248,0.4)"
                  : "1px solid rgba(255,255,255,0.08)",
              fontSize: card.flipped || card.matched ? "18px" : "14px",
              transition: "all 0.2s ease",
            }}
          >
            {card.flipped || card.matched ? card.emoji : "?"}
          </button>
        ))}
      </div>

      {won && (
        <div className="text-center">
          <div className="font-pixel text-[7px] text-[#4ade80]">YOU WIN!</div>
        </div>
      )}
    </div>
  );
}
