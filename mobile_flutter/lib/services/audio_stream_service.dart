import 'dart:async';
import 'dart:io';
import 'dart:typed_data';
import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';
import 'package:record/record.dart';
import 'package:flutter_pcm_sound/flutter_pcm_sound.dart';
import 'bluetooth_service.dart';

class AudioStreamService with ChangeNotifier {
  WebSocket? _socket;
  final AudioRecorder _recorder = AudioRecorder();
  StreamSubscription<List<int>>? _recordSub;
  StreamSubscription? _bleAudioSub;
  
  bool _isCalling = false;
  bool _isStreamingMusic = false;
  bool _isBleStreaming = false;
  bool _isLoopbackActive = false;
  String? _statusMessage;

  bool _isMusicPaused = false;
  int _musicStreamOffset = 0;
  int _musicStreamTotalSize = 0;
  int _musicTimerStartTime = 0;
  int _pauseStartTime = 0;
  int _totalPauseDuration = 0;
  Timer? _musicTimer;
  Uint8List? _pcmBytes;

  // Volume (0–100) and bass boost (0–10) — sent to ESP32 via BLE text command
  int _volume = 70;
  int _bass = 3;

  int get volume => _volume;
  int get bass => _bass;

  void setVolume(int v) {
    _volume = v.clamp(0, 100);
    notifyListeners();
  }

  void setBass(int b) {
    _bass = b.clamp(0, 10);
    notifyListeners();
  }

  // Streaming path fields
  StreamSubscription? _pcmEventSub;

  bool get isCalling => _isCalling;
  bool get isStreamingMusic => _isStreamingMusic;
  bool get isBleStreaming => _isBleStreaming;
  bool get isLoopbackActive => _isLoopbackActive;
  bool get isMusicPaused => _isMusicPaused;
  int get musicStreamOffset => _musicStreamOffset;
  int get musicStreamTotalSize => _musicStreamTotalSize;

  String? get statusMessage => _statusMessage;
  set statusMessage(String? val) {
    _statusMessage = val;
    notifyListeners();
  }

  // ─── PCM accumulator (filled by codec, drained by timer at audio clock rate) ─
  final List<int> _pcmAccumulator = [];
  bool _decodeComplete = false;

  // Notify UI only when offset advances by this many bytes (~100ms of audio)
  // to avoid rebuilding the widget tree every 20ms tick.
  static const int _notifyEveryBytes = 3200;

  // ─── Streaming BLE from file path ────────────────────────────────────────
  // Phase 1: EventChannel decodes into _pcmAccumulator (fast, background)
  // Phase 2: Timer drains accumulator at 32000 bytes/sec (16kHz 16-bit mono)
  Future<bool> startMusicStreamBLEFromPath(BLEService ble, String filePath) async {
    if (_isCalling) return false;
    // If already streaming, stop instantly before switching song
    if (_isStreamingMusic) {
      _musicTimer?.cancel();
      _musicTimerRunning = false;
      _pcmEventSub?.cancel();
      _pcmAccumulator.clear();
      _decodeComplete = false;
      _isStreamingMusic = false;
      await ble.transmitStopMusicInstant();
      await Future.delayed(const Duration(milliseconds: 80));
    }

    _isStreamingMusic = true;
    _isBleStreaming = true;
    _isMusicPaused = false;
    _musicStreamOffset = 0;
    _musicStreamTotalSize = 0;
    _pcmAccumulator.clear();
    _decodeComplete = false;
    _statusMessage = "Buffering...";
    _totalPauseDuration = 0;
    _pauseStartTime = 0;
    _musicTimerStartTime = 0;
    notifyListeners();

    // ── Phase 1: Stream-decode into accumulator ──────────────────────────────
    const eventChannel = EventChannel('com.mrmsluna/audio_stream');
    _pcmEventSub?.cancel();
    _pcmEventSub = eventChannel
        .receiveBroadcastStream({
          'path': filePath,
          'targetSampleRate': 16000, // 16kHz for normal speed playback
        })
        .listen(
      (dynamic data) {
        if (data is Map) {
          final totalBytes = data['totalPcmBytes'];
          if (totalBytes is num) {
            _musicStreamTotalSize = totalBytes.toInt();
            notifyListeners();
          }
        } else if (data is Uint8List) {
          _pcmAccumulator.addAll(data);
          if (_musicStreamTotalSize == 0) {
            _musicStreamTotalSize = _pcmAccumulator.length;
          }

          // Start rate-controlled sender once we have ~375ms of audio buffered (12000 bytes)
          // to align with the hardware's 12000-byte prebuffer threshold.
          if (!_musicTimerRunning && _pcmAccumulator.length >= 12000) {
            _startRateTimer(ble);
          }
          notifyListeners();
        }
      },
      onError: (e) {
        _statusMessage = "Decode error: $e";
        _isStreamingMusic = false;
        _isBleStreaming = false;
        notifyListeners();
      },
      onDone: () {
        _decodeComplete = true;
        if (_musicStreamTotalSize == 0) {
          _musicStreamTotalSize = _pcmAccumulator.length;
        }
        // If timer hasn't started yet (very short file), start it now
        if (!_musicTimerRunning && _pcmAccumulator.isNotEmpty) {
          _startRateTimer(ble);
        }
        notifyListeners();
      },
      cancelOnError: true,
    );

    await ble.transmitStartMusic();
    return true;
  }

  bool _musicTimerRunning = false;
  int _lastNotifiedOffset = 0;

  // ── Phase 2: Rate-controlled sender ─────────────────────────────────────────
  // Sends PCM at EXACTLY the I2S playback rate: 16kHz × 16-bit × 1ch = 32,000 bytes/sec.
  // We use system-time absolute pacing (DateTime.now()) to be immune to timer jitter,
  // preventing buffer overflow (dropped audio/glitches) and underflow.
  void _startRateTimer(BLEService ble) {
    if (_musicTimerRunning) return;
    _musicTimerRunning = true;
    _lastNotifiedOffset = 0;
    _statusMessage = "Streaming Music...";
    notifyListeners();

    _musicTimerStartTime = DateTime.now().millisecondsSinceEpoch;
    _totalPauseDuration = 0;
    _pauseStartTime = 0;

    _musicTimer?.cancel();
    
    // Async backpressure loop
    Future.microtask(() async {
      while (_isStreamingMusic && _musicTimerRunning) {
        if (_isMusicPaused) {
          await Future.delayed(const Duration(milliseconds: 15));
          continue;
        }

        final elapsedMs = DateTime.now().millisecondsSinceEpoch - _musicTimerStartTime - _totalPauseDuration;
        final targetOffset = (elapsedMs * 32) ~/ 2 * 2;

        if (_musicStreamOffset >= _pcmAccumulator.length) {
          if (_decodeComplete) {
            _musicTimerRunning = false;
            _isStreamingMusic = false;
            _isBleStreaming = false;
            _statusMessage = "Playback complete.";
            notifyListeners();
            break;
          }
          await Future.delayed(const Duration(milliseconds: 15));
          continue;
        }

        if (targetOffset > _musicStreamOffset) {
          int bytesToSend = targetOffset - _musicStreamOffset;
          if (bytesToSend > 480) bytesToSend = 480; // Limit burst size to 480 bytes

          final available = _pcmAccumulator.length - _musicStreamOffset;
          if (available <= 0) {
            await Future.delayed(const Duration(milliseconds: 5));
            continue;
          }

          int toSend = bytesToSend < available ? bytesToSend : available;
          final chunk = Uint8List.fromList(
            _pcmAccumulator.sublist(_musicStreamOffset, _musicStreamOffset + toSend),
          );
          _musicStreamOffset += toSend;

          if (_musicStreamOffset - _lastNotifiedOffset >= _notifyEveryBytes) {
            _lastNotifiedOffset = _musicStreamOffset;
            notifyListeners();
          }

          final success = await ble.transmitAudioChunk(chunk);
          if (!success) {
            await Future.delayed(const Duration(milliseconds: 5));
          }
        } else {
          await Future.delayed(const Duration(milliseconds: 10));
        }
      }
    });
  }

  // Stop music immediately. Sends MUSIC:STOP without ACK so ESP32 reacts in <10ms.
  Future<void> stopMusicStreamBLEFromPath(BLEService ble) async {
    // Cancel Dart-side timer and decode stream instantly
    _musicTimer?.cancel();
    _musicTimer = null;
    _musicTimerRunning = false;
    _pcmEventSub?.cancel();
    _pcmEventSub = null;
    _pcmAccumulator.clear();
    _decodeComplete = false;
    _isStreamingMusic = false;
    _isBleStreaming = false;
    _isMusicPaused = false;
    _musicStreamOffset = 0;
    _musicStreamTotalSize = 0;
    _statusMessage = "Stopped.";
    notifyListeners();
    // Send stop to ESP32 immediately (fire-and-forget, no ACK wait)
    await ble.transmitStopMusicInstant();
  }

  void pauseMusic() {
    if (_isStreamingMusic && !_isMusicPaused) {
      _isMusicPaused = true;
      _pauseStartTime = DateTime.now().millisecondsSinceEpoch;
      _statusMessage = "Music Paused";
      notifyListeners();
    }
  }

  void resumeMusic() {
    if (_isStreamingMusic && _isMusicPaused) {
      _isMusicPaused = false;
      if (_pauseStartTime > 0) {
        _totalPauseDuration += DateTime.now().millisecondsSinceEpoch - _pauseStartTime;
        _pauseStartTime = 0;
      }
      _statusMessage = "Streaming Music...";
      notifyListeners();
    }
  }

  void seekMusic(int newOffset) {
    if (_isStreamingMusic) {
      int alignedOffset = (newOffset ~/ 2) * 2;
      _musicStreamOffset = alignedOffset.clamp(0, _musicStreamTotalSize);
      _musicTimerStartTime = DateTime.now().millisecondsSinceEpoch - _totalPauseDuration - (_musicStreamOffset ~/ 32);
      notifyListeners();
    }
  }
  
  // Convert binary byte buffer to signed 16-bit integer array for the player
  List<int> bytesToInt16List(Uint8List bytes) {
    final int16List = Int16List.view(bytes.buffer, bytes.offsetInBytes, bytes.lengthInBytes ~/ 2);
    return int16List.toList();
  }

  // Set up audio player (FlutterPcmSound)
  Future<void> initPlayer() async {
    await FlutterPcmSound.setup(
      sampleRate: 16000,
      channelCount: 1, // Mono
    );
  }

  // Connect to the WebSocket broker or direct BLE link
  Future<bool> startCall(String serverIp, String robotMac, {BLEService? ble}) async {
    if (_isCalling || _isStreamingMusic) return false;
    
    final bool useBle = (ble != null && (serverIp.isEmpty || serverIp == "localhost" || serverIp == "0.0.0.0"));
    _statusMessage = useBle ? "Connecting Direct BLE Call..." : "Connecting to VoIP broker...";
    notifyListeners();
    
    try {
      // 1. Initialize player
      await initPlayer();
      
      _isCalling = true;
      
      if (useBle) {
        _statusMessage = "BLE Call Connected!";
        notifyListeners();
        
        // Listen to incoming audio from the watch via BLE notifications
        _bleAudioSub = ble.audioStreamData.listen((data) async {
          if (_isCalling) {
            final samples = bytesToInt16List(Uint8List.fromList(data));
            if (samples.isNotEmpty) {
              await FlutterPcmSound.feed(PcmArrayInt16.fromList(samples));
            }
          }
        });
      } else {
        final cleanMac = robotMac.replaceAll(':', '').toUpperCase();
        final urlStr = 'ws://$serverIp:8001/ws?mac=$cleanMac&variant=phone';
        
        _socket = await WebSocket.connect(urlStr).timeout(const Duration(seconds: 5));
        _statusMessage = "Call Connected!";
        notifyListeners();
        
        _socket!.listen(
          (data) async {
            if (data is List<int> && _isCalling) {
              final samples = bytesToInt16List(Uint8List.fromList(data));
              if (samples.isNotEmpty) {
                await FlutterPcmSound.feed(PcmArrayInt16.fromList(samples));
              }
            }
          },
          onError: (err) {
            stopCall();
          },
          onDone: () {
            stopCall();
          },
        );
      }
      
      // 3. Request mic permissions & Start Recording mic input
      final hasPermission = await _recorder.hasPermission();
      if (!hasPermission) {
        _statusMessage = "Mic permission denied.";
        stopCall();
        return false;
      }
      
      final recordConfig = const RecordConfig(
        encoder: AudioEncoder.pcm16bits,
        sampleRate: 16000,
        numChannels: 1,
      );
      
      final recordStream = await _recorder.startStream(recordConfig);
      _recordSub = recordStream.listen(
        (chunk) {
          if (_isCalling) {
            if (useBle) {
              ble.transmitAudioChunk(chunk);
            } else if (_socket != null) {
              _socket!.add(chunk);
            }
          }
        },
        onError: (err) {
          stopCall();
        },
      );
      
      return true;
    } catch (e) {
      _statusMessage = "Call connection failed: $e";
      _isCalling = false;
      notifyListeners();
      return false;
    }
  }
  
  Future<void> stopCall() async {
    _statusMessage = "Call stopped.";
    _isCalling = false;
    notifyListeners();
    
    await _recordSub?.cancel();
    _recordSub = null;
    
    await _bleAudioSub?.cancel();
    _bleAudioSub = null;
    
    await _recorder.stop();
    await FlutterPcmSound.release();
    
    await _socket?.close();
    _socket = null;
  }

  Future<void> startLoopback(BLEService ble) async {
    if (_isCalling || _isStreamingMusic || _isLoopbackActive) return;
    _isLoopbackActive = true;
    _statusMessage = "Loopback Test Active (Mic -> Speaker)";
    notifyListeners();
    await ble.transmitStartLoopback();
  }

  Future<void> stopLoopback(BLEService ble) async {
    if (!_isLoopbackActive) return;
    _isLoopbackActive = false;
    _statusMessage = "Loopback Stopped";
    notifyListeners();
    await ble.transmitStopLoopback();
  }

  // Stream picked music (WAV / parsed PCM)
  Future<bool> startMusicStream(String serverIp, String robotMac, Uint8List pcmBytes) async {
    if (_isCalling || _isStreamingMusic) return false;
    _statusMessage = "Connecting to music broker...";
    notifyListeners();
    
    final cleanMac = robotMac.replaceAll(':', '').toUpperCase();
    final urlStr = 'ws://$serverIp:8001/ws?mac=$cleanMac&variant=phone';
    
    try {
      _socket = await WebSocket.connect(urlStr).timeout(const Duration(seconds: 5));
      _isStreamingMusic = true;
      _isMusicPaused = false;
      _pcmBytes = pcmBytes;
      _musicStreamOffset = 0;
      _musicStreamTotalSize = pcmBytes.length;
      _statusMessage = "Streaming Music...";
      notifyListeners();
      
      // Send music in chunks to avoid overloading the ESP32 buffer
      // 16kHz, 16-bit, Mono PCM = 32,000 bytes per second.
      // Let's send chunks of 2048 bytes every 64ms.
      const chunkSize = 2048;
      
      _musicTimer?.cancel();
      _musicTimer = Timer.periodic(const Duration(milliseconds: 64), (timer) {
        if (!_isStreamingMusic || _socket == null) {
          timer.cancel();
          return;
        }

        if (_isMusicPaused) {
          return;
        }
        
        if (_musicStreamOffset >= _musicStreamTotalSize) {
          stopMusicStream();
          timer.cancel();
          return;
        }
        
        int end = _musicStreamOffset + chunkSize;
        if (end > _musicStreamTotalSize) end = _musicStreamTotalSize;
        
        final chunk = _pcmBytes!.sublist(_musicStreamOffset, end);
        _socket!.add(chunk);
        _musicStreamOffset = end;
        notifyListeners();
      });
      
      return true;
    } catch (e) {
      _statusMessage = "Music connection failed: $e";
      _isStreamingMusic = false;
      notifyListeners();
      return false;
    }
  }
  
  Future<void> stopMusicStream() async {
    _statusMessage = "Music stream stopped.";
    _isStreamingMusic = false;
    _isBleStreaming = false;
    _isMusicPaused = false;
    _musicStreamOffset = 0;
    _musicStreamTotalSize = 0;
    _pcmBytes = null;
    notifyListeners();
    
    _musicTimer?.cancel();
    _musicTimer = null;
    
    await _socket?.close();
    _socket = null;
  }

  Future<bool> startMusicStreamBLE(BLEService ble, Uint8List pcmBytes) async {
    if (_isCalling || _isStreamingMusic) return false;
    _statusMessage = "Connecting Bluetooth Audio...";
    notifyListeners();
    
    _isStreamingMusic = true;
    _isBleStreaming = true;
    _isMusicPaused = false;
    _pcmBytes = pcmBytes;
    _musicStreamOffset = 0;
    _musicStreamTotalSize = pcmBytes.length;
    _statusMessage = "Streaming Music...";
    notifyListeners();
    
    _musicTimerStartTime = DateTime.now().millisecondsSinceEpoch;
    _totalPauseDuration = 0;
    _pauseStartTime = 0;
    
    _musicTimer?.cancel();
    _musicTimerRunning = true;
    
    // Async backpressure loop
    Future.microtask(() async {
      while (_isStreamingMusic && _isBleStreaming && _musicTimerRunning) {
        if (_isMusicPaused) {
          await Future.delayed(const Duration(milliseconds: 15));
          continue;
        }
        
        final elapsedMs = DateTime.now().millisecondsSinceEpoch - _musicTimerStartTime - _totalPauseDuration;
        final targetOffset = (elapsedMs * 32) ~/ 2 * 2;
        
        if (_musicStreamOffset >= _musicStreamTotalSize) {
          stopMusicStreamBLE();
          break;
        }
        
        if (targetOffset > _musicStreamOffset) {
          int bytesToSend = targetOffset - _musicStreamOffset;
          if (bytesToSend > 480) bytesToSend = 480; // Limit burst size to 480 bytes
          
          int end = _musicStreamOffset + bytesToSend;
          if (end > _musicStreamTotalSize) end = _musicStreamTotalSize;
          
          final chunk = _pcmBytes!.sublist(_musicStreamOffset, end);
          _musicStreamOffset = end;
          notifyListeners();
          
          final success = await ble.transmitAudioChunk(chunk);
          if (!success) {
            await Future.delayed(const Duration(milliseconds: 5));
          }
        } else {
          await Future.delayed(const Duration(milliseconds: 10));
        }
      }
    });
    
    return true;
  }

  Future<void> stopMusicStreamBLE() async {
    _statusMessage = "Music stream stopped.";
    _isStreamingMusic = false;
    _isBleStreaming = false;
    _isMusicPaused = false;
    _musicStreamOffset = 0;
    _musicStreamTotalSize = 0;
    _pcmBytes = null;
    _musicTimerRunning = false;
    notifyListeners();
    
    _musicTimer?.cancel();
    _musicTimer = null;
  }
}
