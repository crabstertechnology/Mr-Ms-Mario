import React, { useState } from 'react';
import LunaWatchFrame from './LunaWatchFrame';
import TitleScreen from './TitleScreen';
import SetupScreen from './SetupScreen';
import HidingCountdownScreen from './HidingCountdownScreen';
import SeekGameScreen from './SeekGameScreen';
import VictoryScreen from './VictoryScreen';
import ExplosionScreen from './ExplosionScreen';
import { sounds } from './soundEffects';

type GameScreen = 'title' | 'setup' | 'countdown' | 'seek' | 'victory' | 'timeout';

export default function HideSeekApp() {
  const [currentScreen, setCurrentScreen] = useState<GameScreen>('title');
  const [bombSeconds, setBombSeconds] = useState(60);
  const [gameMode, setGameMode] = useState<'room_search' | 'watch_beacon'>('room_search');
  const [remainingTime, setRemainingTime] = useState(60);
  const [muted, setMuted] = useState(false);
  const [zoomScale, setZoomScale] = useState<1 | 1.15 | 1.3>(1.15);

  const toggleMute = () => {
    sounds.enabled = !sounds.enabled;
    setMuted(!sounds.enabled);
  };

  const handleStartFromTitle = () => {
    setCurrentScreen('setup');
  };

  const handleArmAndHide = (seconds: number, mode: 'room_search' | 'watch_beacon') => {
    setBombSeconds(seconds);
    setGameMode(mode);
    setCurrentScreen('countdown');
  };

  const handleFinishHiding = () => {
    setCurrentScreen('seek');
  };

  const handleFoundCat = (timeLeft: number) => {
    setRemainingTime(timeLeft);
    setCurrentScreen('victory');
  };

  const handleTimeExpired = () => {
    setCurrentScreen('timeout');
  };

  const handlePlayAgain = () => {
    setCurrentScreen('countdown');
  };

  const handleBackToSetup = () => {
    setCurrentScreen('setup');
  };

  const handleBackToTitle = () => {
    setCurrentScreen('title');
  };

  // Dimensions for screen
  const screenW = Math.round(260 * zoomScale);
  const screenH = Math.round(320 * zoomScale);

  return (
    <div className="flex flex-col items-center justify-center min-h-screen py-8 px-4 bg-gradient-to-b from-[#110722] via-[#090214] to-[#040108] text-white">
      {/* Top Controls & Navigation Header */}
      <div className="w-full max-w-2xl flex flex-wrap items-center justify-between gap-3 mb-6 px-4 py-3 rounded-2xl bg-[#1C0D36]/80 border border-purple-500/30 backdrop-blur-md shadow-2xl">
        <div className="flex items-center gap-2">
          <span className="text-xl">🐱</span>
          <div>
            <h1 className="text-sm font-extrabold tracking-wider text-purple-200 uppercase font-outfit">
              Luna OS · Hide &amp; Seek
            </h1>
            <p className="text-[10px] text-purple-400/80">
              Waveshare ESP32-S3 1.69&quot; (240×280) Exact Interactive UI
            </p>
          </div>
        </div>

        {/* Quick Screen Jump Pills */}
        <div className="flex items-center gap-1 bg-[#120624] p-1 rounded-xl border border-purple-900/60">
          {(
            [
              { id: 'title', label: '1. Title' },
              { id: 'setup', label: '2. Setup' },
              { id: 'countdown', label: '3. Hide' },
              { id: 'seek', label: '4. Seek' },
              { id: 'victory', label: '5. Win' },
            ] as const
          ).map((s) => (
            <button
              key={s.id}
              type="button"
              onClick={() => {
                sounds.playTap();
                setCurrentScreen(s.id);
              }}
              className={`px-2.5 py-1 rounded-lg text-[11px] font-bold cursor-pointer transition-all ${
                currentScreen === s.id
                  ? 'bg-[#9333EA] text-white shadow-md'
                  : 'text-purple-300 hover:text-white hover:bg-white/5'
              }`}
            >
              {s.label}
            </button>
          ))}
        </div>

        {/* Action Toggles: Mute + Scale */}
        <div className="flex items-center gap-2">
          <button
            type="button"
            onClick={toggleMute}
            className="px-2.5 py-1.5 rounded-xl bg-purple-900/50 hover:bg-purple-800 text-xs font-bold text-purple-200 border border-purple-500/40 cursor-pointer flex items-center gap-1.5"
            title="Toggle Web Audio SFX"
          >
            {muted ? '🔇 Muted' : '🔊 Sound'}
          </button>

          <select
            value={zoomScale}
            onChange={(e) => setZoomScale(parseFloat(e.target.value) as 1 | 1.15 | 1.3)}
            className="px-2 py-1.5 rounded-xl bg-purple-900/50 text-xs font-bold text-purple-200 border border-purple-500/40 cursor-pointer"
          >
            <option value={1}>Scale 1.0x</option>
            <option value={1.15}>Scale 1.15x</option>
            <option value={1.3}>Scale 1.3x</option>
          </select>
        </div>
      </div>

      {/* Main Watch Simulator Display */}
      <div className="relative my-2 transition-transform duration-200">
        <LunaWatchFrame width={screenW} height={screenH}>
          {currentScreen === 'title' && (
            <TitleScreen onStart={handleStartFromTitle} />
          )}

          {currentScreen === 'setup' && (
            <SetupScreen
              initialSeconds={bombSeconds}
              onArmAndHide={handleArmAndHide}
              onBackToTitle={handleBackToTitle}
            />
          )}

          {currentScreen === 'countdown' && (
            <HidingCountdownScreen
              bombSeconds={bombSeconds}
              onFinishHiding={handleFinishHiding}
            />
          )}

          {currentScreen === 'seek' && (
            <SeekGameScreen
              bombSeconds={bombSeconds}
              onFoundCat={handleFoundCat}
              onTimeExpired={handleTimeExpired}
            />
          )}

          {currentScreen === 'victory' && (
            <VictoryScreen
              timeRemaining={remainingTime}
              totalSeconds={bombSeconds}
              onPlayAgain={handlePlayAgain}
              onSetup={handleBackToSetup}
            />
          )}

          {currentScreen === 'timeout' && (
            <ExplosionScreen
              onRetry={handlePlayAgain}
              onSetup={handleBackToSetup}
            />
          )}
        </LunaWatchFrame>
      </div>

      {/* Interactive Helper Notes */}
      <div className="mt-8 text-center text-xs text-purple-300/70 max-w-md">
        <p className="font-semibold text-purple-200 mb-1">
          ✨ Exact UI recreation of Luna Smartwatch Hide &amp; Seek
        </p>
        <p>
          Features interactive draggable dial scrubber, quick presets (30s/60s/90s/2m),
          fine adjusters, synthesized Web Audio sound effects, and 7-spot isometric room search.
        </p>
      </div>
    </div>
  );
}
