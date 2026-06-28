import 'dart:async';
import 'dart:io';
import 'dart:math';
import 'dart:typed_data';
import 'package:audioplayers/audioplayers.dart';
import 'package:path_provider/path_provider.dart';

class AudioSynthService {
  final AudioPlayer _audioPlayer = AudioPlayer();
  bool _isPlaying = false;
  File? _tempWavFile;

  static const Map<String, double> noteFreqs = {
    'C3': 130.81, 'CS3': 138.59, 'D3': 146.83, 'DS3': 155.56, 'E3': 164.81, 'F3': 174.61, 'FS3': 185.00, 'G3': 196.00, 'GS3': 207.65, 'A3': 220.00, 'AS3': 233.08, 'B3': 246.94,
    'C4': 261.63, 'CS4': 277.18, 'D4': 293.66, 'DS4': 311.13, 'E4': 329.63, 'F4': 349.23, 'FS4': 369.99, 'G4': 392.00, 'GS4': 415.30, 'A4': 440.00, 'AS4': 466.16, 'B4': 493.88,
    'C5': 523.25, 'CS5': 554.37, 'D5': 587.33, 'DS5': 622.25, 'E5': 659.25, 'F5': 698.46, 'FS5': 739.99, 'G5': 783.99, 'GS5': 830.61, 'A5': 880.00, 'AS5': 932.33, 'B5': 987.77,
    'C6': 1046.50, 'CS6': 1108.73, 'D6': 1174.66, 'DS6': 1244.51, 'E6': 1318.51, 'F6': 1396.91, 'FS6': 1479.98, 'G6': 1567.98, 'GS6': 1661.22, 'A6': 1760.00, 'AS6': 1864.66, 'B6': 1975.53,
    'C7': 2093.00, 'CS7': 2217.46, 'D7': 2349.32, 'DS7': 2489.02, 'E7': 2637.02, 'F7': 2793.83, 'FS7': 2959.96, 'G7': 3135.96, 'GS7': 3322.44, 'A7': 3520.00, 'AS7': 3729.31, 'B7': 3951.07,
    'C8': 4186.01, 'CS8': 4434.92, 'D8': 4698.63, 'DS8': 4978.03, 'E8': 5274.04, 'F8': 5587.65, 'FS8': 5919.91, 'G8': 6271.93, 'GS8': 6644.88, 'A8': 7040.00, 'AS8': 7458.62, 'B8': 7902.13
  };

  bool get isPlaying => _isPlaying;

  AudioSynthService() {
    _audioPlayer.onPlayerStateChanged.listen((state) {
      if (state == PlayerState.completed || state == PlayerState.stopped) {
        _isPlaying = false;
      }
    });
  }

  // Parse notation and play
  Future<void> playMelody(String sequenceStr, {double volume = 0.08, Function()? onCompleted}) async {
    await stop();
    _isPlaying = true;

    try {
      final wavBytes = _compileMelodyToWav(sequenceStr, volume: volume);
      if (wavBytes.isEmpty) {
        _isPlaying = false;
        if (onCompleted != null) onCompleted();
        return;
      }

      final tempDir = await getTemporaryDirectory();
      _tempWavFile = File('${tempDir.path}/temp_melody.wav');
      await _tempWavFile!.writeAsBytes(wavBytes, flush: true);

      // Play the generated WAV file
      await _audioPlayer.play(DeviceFileSource(_tempWavFile!.path));

      // Listen for complete to call callbacks
      if (onCompleted != null) {
        late StreamSubscription sub;
        sub = _audioPlayer.onPlayerStateChanged.listen((state) {
          if (state == PlayerState.completed || state == PlayerState.stopped) {
            onCompleted();
            sub.cancel();
          }
        });
      }
    } catch (e) {
      print("Synthesis/Playback failed: $e");
      _isPlaying = false;
      if (onCompleted != null) onCompleted();
    }
  }

  Future<void> stop() async {
    if (_isPlaying) {
      await _audioPlayer.stop();
      _isPlaying = false;
    }
  }

  // Compiles note tokens into a raw WAV byte array (16-bit PCM Mono @ 22050Hz)
  Uint8List _compileMelodyToWav(String sequenceStr, {double volume = 0.08, int sampleRate = 22050}) {
    final tokens = sequenceStr.trim().split(RegExp(r'\s+'));
    final notes = <_Note>[];

    for (int i = 0; i < tokens.length; i += 3) {
      if (i + 2 >= tokens.length) break;
      final noteName = tokens[i].toUpperCase().replaceAll('#', 'S');
      final duration = int.tryParse(tokens[i + 1]) ?? 0;
      final rest = int.tryParse(tokens[i + 2]) ?? 0;
      notes.add(_Note(noteName, duration, rest));
    }

    if (notes.isEmpty) return Uint8List(0);

    // Calculate total samples
    int totalSamples = 0;
    for (var note in notes) {
      int toneSamples = (sampleRate * (note.durationMs / 1000.0)).toInt();
      int restSamples = (sampleRate * (note.restMs / 1000.0)).toInt();
      totalSamples += toneSamples + restSamples;
    }

    int dataSize = totalSamples * 2; // 16-bit PCM = 2 bytes per sample
    int fileSize = 36 + dataSize;

    final header = ByteData(44);
    // RIFF
    header.setUint8(0, 0x52); // R
    header.setUint8(1, 0x49); // I
    header.setUint8(2, 0x46); // F
    header.setUint8(3, 0x46); // F
    header.setUint32(4, fileSize, Endian.little);
    // WAVE
    header.setUint8(8, 0x57); // W
    header.setUint8(9, 0x41); // A
    header.setUint8(10, 0x56); // V
    header.setUint8(11, 0x45); // E
    // fmt 
    header.setUint8(12, 0x66); // f
    header.setUint8(13, 0x6d); // m
    header.setUint8(14, 0x74); // t
    header.setUint8(15, 0x20); // ' '
    header.setUint32(16, 16, Endian.little);
    header.setUint16(20, 1, Endian.little); // Format: PCM
    header.setUint16(22, 1, Endian.little); // Channels: 1 (Mono)
    header.setUint32(24, sampleRate, Endian.little);
    header.setUint32(28, sampleRate * 2, Endian.little); // Byte rate
    header.setUint16(32, 2, Endian.little); // Block align
    header.setUint16(34, 16, Endian.little); // Bits per sample
    // data
    header.setUint8(36, 0x64); // d
    header.setUint8(37, 0x61); // a
    header.setUint8(38, 0x74); // t
    header.setUint8(39, 0x61); // a
    header.setUint32(40, dataSize, Endian.little);

    final wavBytes = BytesBuilder();
    wavBytes.add(header.buffer.asUint8List());

    final dataList = Int16List(totalSamples);
    int currentOffset = 0;
    final maxAmplitude = (32767 * volume).toInt();

    for (var note in notes) {
      final freq = noteFreqs[note.noteName] ?? 0.0;
      final toneSamples = (sampleRate * (note.durationMs / 1000.0)).toInt();
      final restSamples = (sampleRate * (note.restMs / 1000.0)).toInt();

      if (freq > 0) {
        final double period = sampleRate / freq;
        final int halfPeriod = (period / 2).round();

        for (int i = 0; i < toneSamples; i++) {
          if ((i % period) < halfPeriod) {
            dataList[currentOffset + i] = maxAmplitude;
          } else {
            dataList[currentOffset + i] = -maxAmplitude;
          }
        }
      } else {
        // Silent note/rest
        for (int i = 0; i < toneSamples; i++) {
          dataList[currentOffset + i] = 0;
        }
      }
      currentOffset += toneSamples;

      // Rest interval (silence)
      for (int i = 0; i < restSamples; i++) {
        dataList[currentOffset + i] = 0;
      }
      currentOffset += restSamples;
    }

    wavBytes.add(dataList.buffer.asUint8List());
    return wavBytes.toBytes();
  }

  // AI melody composer generator mapping
  String generateAiMelody(String prompt) {
    final lowerPrompt = prompt.toLowerCase();
    if (lowerPrompt.contains('victory') || lowerPrompt.contains('fanfare')) {
      return "C5 100 20 C5 100 20 C5 100 20 C5 300 60 G4 300 60 A4 150 20 B4 150 20 C5 400 100";
    } else if (lowerPrompt.contains('coin') || lowerPrompt.contains('jump')) {
      return "B5 80 10 E6 280 20";
    } else if (lowerPrompt.contains('sad') || lowerPrompt.contains('death')) {
      return "C5 150 50 G4 150 50 E4 150 50 A4 120 20 B4 120 20 A4 120 20 GS4 300 200";
    }
    // Fallback default melody
    return "C5 100 20 E5 100 20 G5 200 60";
  }
}

class _Note {
  final String noteName;
  final int durationMs;
  final int restMs;
  _Note(this.noteName, this.durationMs, this.restMs);
}
