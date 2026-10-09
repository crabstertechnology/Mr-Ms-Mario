import React, { useState } from 'react';
import TitleScreen from '../hideseek/TitleScreen';
import SetupScreen from '../hideseek/SetupScreen';
import HidingCountdownScreen from '../hideseek/HidingCountdownScreen';
import SeekGameScreen from '../hideseek/SeekGameScreen';
import VictoryScreen from '../hideseek/VictoryScreen';
import ExplosionScreen from '../hideseek/ExplosionScreen';

interface HideSeekGameProps {
  onGameOver: (score: number) => void;
  highScore: number;
}

export default function HideSeek({ onGameOver, highScore }: HideSeekGameProps) {
  const [screen, setScreen] = useState<'title' | 'setup' | 'countdown' | 'seek' | 'victory' | 'timeout'>('title');
  const [bombSeconds, setBombSeconds] = useState(60);
  const [timeRemaining, setRemainingTime] = useState(60);

  const handleStartTitle = () => setScreen('setup');
  const handleArm = (secs: number) => {
    setBombSeconds(secs);
    setScreen('countdown');
  };
  const handleFinishHiding = () => setScreen('seek');
  const handleFoundCat = (timeLeft: number) => {
    setRemainingTime(timeLeft);
    const score = timeLeft * 50 + 500;
    if (score > highScore) {
      onGameOver(score);
    }
    setScreen('victory');
  };
  const handleTimeout = () => {
    onGameOver(0);
    setScreen('timeout');
  };

  return (
    <div className="relative w-full h-full overflow-hidden">
      {screen === 'title' && <TitleScreen onStart={handleStartTitle} />}
      {screen === 'setup' && (
        <SetupScreen
          initialSeconds={bombSeconds}
          onArmAndHide={handleArm}
          onBackToTitle={() => setScreen('title')}
        />
      )}
      {screen === 'countdown' && (
        <HidingCountdownScreen bombSeconds={bombSeconds} onFinishHiding={handleFinishHiding} />
      )}
      {screen === 'seek' && (
        <SeekGameScreen
          bombSeconds={bombSeconds}
          onFoundCat={handleFoundCat}
          onTimeExpired={handleTimeout}
        />
      )}
      {screen === 'victory' && (
        <VictoryScreen
          timeRemaining={timeRemaining}
          totalSeconds={bombSeconds}
          onPlayAgain={() => setScreen('countdown')}
          onSetup={() => setScreen('setup')}
        />
      )}
      {screen === 'timeout' && (
        <ExplosionScreen
          onRetry={() => setScreen('countdown')}
          onSetup={() => setScreen('setup')}
        />
      )}
    </div>
  );
}
