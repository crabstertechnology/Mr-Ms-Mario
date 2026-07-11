package com.mrmsluna.controller.mobile_flutter

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.content.ComponentName
import android.content.pm.PackageManager
import android.provider.Settings
import io.flutter.embedding.android.FlutterActivity
import io.flutter.embedding.engine.FlutterEngine
import io.flutter.plugin.common.MethodChannel
import io.flutter.plugin.common.EventChannel
import android.media.MediaCodec
import android.media.MediaExtractor
import android.media.MediaFormat
import java.io.ByteArrayOutputStream
import java.io.File
import java.nio.ByteBuffer
import java.nio.ByteOrder

class MainActivity: FlutterActivity() {
    private val CHANNEL = "com.mrmsluna/notifications"
    private val AUDIO_STREAM_CHANNEL = "com.mrmsluna/audio_stream"
    private var methodChannel: MethodChannel? = null
    private var audioStreamThread: Thread? = null
    @Volatile private var stopStreamRequested = false

    companion object {
        var flutterEngine: FlutterEngine? = null
    }

    override fun shouldDestroyEngineWithHost(): Boolean {
        return false
    }



    override fun configureFlutterEngine(flutterEngine: FlutterEngine) {
        super.configureFlutterEngine(flutterEngine)
        Companion.flutterEngine = flutterEngine
        methodChannel = MethodChannel(flutterEngine.dartExecutor.binaryMessenger, CHANNEL)

        // Streaming PCM EventChannel — sends chunks as codec decodes them (zero wait)
        EventChannel(flutterEngine.dartExecutor.binaryMessenger, AUDIO_STREAM_CHANNEL)
            .setStreamHandler(object : EventChannel.StreamHandler {
                override fun onListen(arguments: Any?, events: EventChannel.EventSink) {
                    val path = (arguments as? Map<*, *>)?.get("path") as? String ?: return
                    val targetSampleRate = (arguments as? Map<*, *>)?.get("targetSampleRate") as? Int ?: 16000
                    stopStreamRequested = false
                    audioStreamThread?.interrupt()
                    audioStreamThread = Thread {
                        streamAudioChunks(path, targetSampleRate, events)
                    }.also { it.start() }
                }
                override fun onCancel(arguments: Any?) {
                    stopStreamRequested = true
                    audioStreamThread?.interrupt()
                }
            })

        // Force rebind NotificationListenerService to avoid Android binding issues on upgrade
        try {
            val pm = packageManager
            val componentName = ComponentName(this, MyNotificationListener::class.java)
            pm.setComponentEnabledSetting(
                componentName,
                PackageManager.COMPONENT_ENABLED_STATE_DISABLED,
                PackageManager.DONT_KILL_APP
            )
            pm.setComponentEnabledSetting(
                componentName,
                PackageManager.COMPONENT_ENABLED_STATE_ENABLED,
                PackageManager.DONT_KILL_APP
            )
        } catch (e: Exception) {}
        
        // Start background service only if permissions are already granted to prevent SecurityException
        if (hasConnectedDevicePermissions()) {
            val serviceIntent = Intent(this, LunaBackgroundService::class.java)
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
                startForegroundService(serviceIntent)
            } else {
                startService(serviceIntent)
            }
        }
        
        methodChannel?.setMethodCallHandler { call, result ->
            when (call.method) {
                "isPermissionGranted" -> {
                    result.success(isNotificationServiceEnabled())
                }
                "openSettings" -> {
                    startActivity(Intent("android.settings.ACTION_NOTIFICATION_LISTENER_SETTINGS"))
                    result.success(true)
                }
                "isPostNotificationsPermissionGranted" -> {
                    if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.TIRAMISU) {
                        val granted = checkSelfPermission(android.Manifest.permission.POST_NOTIFICATIONS) == android.content.pm.PackageManager.PERMISSION_GRANTED
                        result.success(granted)
                    } else {
                        result.success(true)
                    }
                }
                "requestPostNotificationsPermission" -> {
                    if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.TIRAMISU) {
                        requestPermissions(arrayOf(android.Manifest.permission.POST_NOTIFICATIONS), 101)
                    }
                    result.success(true)
                }
                "startBackgroundService" -> {
                    if (hasConnectedDevicePermissions()) {
                        val serviceIntent = Intent(this, LunaBackgroundService::class.java)
                        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
                            startForegroundService(serviceIntent)
                        } else {
                            startService(serviceIntent)
                        }
                        result.success(true)
                    } else {
                        result.success(false)
                    }
                }
                "updateConnectionStatus" -> {
                    val connected = call.argument<Boolean>("connected") ?: false
                    if (hasConnectedDevicePermissions()) {
                        val serviceIntent = Intent(this, LunaBackgroundService::class.java).apply {
                            putExtra("connected", connected)
                        }
                        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
                            startForegroundService(serviceIntent)
                        } else {
                            startService(serviceIntent)
                        }
                        result.success(true)
                    } else {
                        result.success(false)
                    }
                }
                "getInstalledApps" -> {
                    Thread {
                        try {
                            val appsList = ArrayList<Map<String, String>>()
                            val pm = packageManager
                            val packages = pm.getInstalledApplications(PackageManager.GET_META_DATA)
                            for (packageInfo in packages) {
                                val pkgName = packageInfo.packageName
                                val isSystem = (packageInfo.flags and android.content.pm.ApplicationInfo.FLAG_SYSTEM) != 0
                                val isSystemUpdate = (packageInfo.flags and android.content.pm.ApplicationInfo.FLAG_UPDATED_SYSTEM_APP) != 0
                                
                                // Skip non-launchable / background system services to speed up loading
                                if (isSystem && !isSystemUpdate && 
                                    !pkgName.contains("whatsapp") && 
                                    !pkgName.contains("instagram") && 
                                    !pkgName.contains("telegram") && 
                                    !pkgName.contains("messenger") && 
                                    !pkgName.contains("youtube") && 
                                    !pkgName.contains("gmail") && 
                                    !pkgName.contains("message") && 
                                    !pkgName.contains("dialer") && 
                                    !pkgName.contains("phone")) {
                                    continue
                                }
                                
                                val label = packageInfo.loadLabel(pm).toString()
                                val appMap = HashMap<String, String>()
                                appMap["name"] = label
                                appMap["packageName"] = pkgName
                                appsList.add(appMap)
                            }
                            appsList.sortBy { it["name"]?.lowercase() ?: "" }
                            runOnUiThread {
                                result.success(appsList)
                            }
                        } catch (e: Exception) {
                            runOnUiThread {
                                result.error("ERROR", e.message, null)
                            }
                        }
                    }.start()
                }
                "getLocalAudioFiles" -> {
                    Thread {
                        try {
                            val audioList = ArrayList<Map<String, String>>()
                            val uri = android.provider.MediaStore.Audio.Media.EXTERNAL_CONTENT_URI
                            val projection = arrayOf(
                                android.provider.MediaStore.Audio.Media.DATA,
                                android.provider.MediaStore.Audio.Media.TITLE,
                                android.provider.MediaStore.Audio.Media.DISPLAY_NAME
                            )
                            val selection = "${android.provider.MediaStore.Audio.Media.IS_MUSIC} != 0 OR ${android.provider.MediaStore.Audio.Media.IS_AUDIOBOOK} != 0 OR ${android.provider.MediaStore.Audio.Media.IS_RECORDING} != 0 OR ${android.provider.MediaStore.Audio.Media.IS_PODCAST} != 0"
                            contentResolver.query(uri, projection, selection, null, null)?.use { cursor ->
                                val dataCol = cursor.getColumnIndexOrThrow(android.provider.MediaStore.Audio.Media.DATA)
                                val titleCol = cursor.getColumnIndexOrThrow(android.provider.MediaStore.Audio.Media.TITLE)
                                val nameCol = cursor.getColumnIndex(android.provider.MediaStore.Audio.Media.DISPLAY_NAME)
                                while (cursor.moveToNext()) {
                                    val path = cursor.getString(dataCol) ?: continue
                                    val file = File(path)
                                    if (file.exists() && (path.endsWith(".mp3", true) || path.endsWith(".wav", true))) {
                                        val title = cursor.getString(titleCol) ?: file.name
                                        val name = if (nameCol != -1) cursor.getString(nameCol) else file.name
                                        val map = HashMap<String, String>()
                                        map["path"] = path
                                        map["title"] = title
                                        map["name"] = name
                                        audioList.add(map)
                                    }
                                }
                            }
                            runOnUiThread {
                                result.success(audioList)
                            }
                        } catch (e: Exception) {
                            runOnUiThread {
                                result.error("QUERY_ERROR", e.message, null)
                            }
                        }
                    }.start()
                }
                "decodeAudioToPcm" -> {
                    val path = call.argument<String>("path") ?: ""
                    Thread {
                        try {
                            val pcm = decodeAudioToPcm16kMono(path)
                            runOnUiThread {
                                if (pcm != null) {
                                    result.success(pcm)
                                } else {
                                    result.error("DECODE_ERROR", "Failed to decode audio file to 16k Mono PCM", null)
                                }
                            }
                        } catch (e: Exception) {
                            runOnUiThread {
                                result.error("DECODE_ERROR", e.message, null)
                            }
                        }
                    }.start()
                }
                else -> result.notImplemented()
            }
        }
    }

    override fun onDestroy() {
        stopStreamRequested = true
        audioStreamThread?.interrupt()
        super.onDestroy()
    }

    /**
     * Streams decoded + resampled PCM chunks to Flutter via EventChannel.
     * Each event is a ByteArray chunk ready to send over BLE.
     * Playback can begin after the very first chunk (~50ms of audio).
     */
    private fun streamAudioChunks(filePath: String, targetSampleRate: Int, sink: EventChannel.EventSink) {
        val file = File(filePath)
        if (!file.exists()) {
            runOnUiThread { sink.error("FILE_NOT_FOUND", "File not found: $filePath", null) }
            return
        }

        val extractor = MediaExtractor()
        try { extractor.setDataSource(file.absolutePath) }
        catch (e: Exception) {
            runOnUiThread { sink.error("EXTRACTOR_ERROR", e.message, null) }
            return
        }

        var trackIndex = -1
        for (i in 0 until extractor.trackCount) {
            val fmt = extractor.getTrackFormat(i)
            if ((fmt.getString(MediaFormat.KEY_MIME) ?: "").startsWith("audio/")) {
                trackIndex = i; break
            }
        }
        if (trackIndex == -1) {
            extractor.release()
            runOnUiThread { sink.error("NO_AUDIO_TRACK", "No audio track found", null) }
            return
        }

        extractor.selectTrack(trackIndex)
        val inputFormat = extractor.getTrackFormat(trackIndex)
        val mime = inputFormat.getString(MediaFormat.KEY_MIME) ?: ""
        val srcRate = if (inputFormat.containsKey(MediaFormat.KEY_SAMPLE_RATE)) inputFormat.getInteger(MediaFormat.KEY_SAMPLE_RATE) else 44100
        val srcCh   = if (inputFormat.containsKey(MediaFormat.KEY_CHANNEL_COUNT)) inputFormat.getInteger(MediaFormat.KEY_CHANNEL_COUNT) else 2

        android.util.Log.d("AudioStream", "Decoding: srcRate=$srcRate srcCh=$srcCh -> ${targetSampleRate}Hz mono")

        val codec = MediaCodec.createDecoderByType(mime)
        codec.configure(inputFormat, null, null, 0)
        codec.start()

        val info = MediaCodec.BufferInfo()
        var inputEOS = false
        var outputEOS = false

        val accum = ByteArrayOutputStream()

        try {
            while (!outputEOS && !stopStreamRequested && !Thread.currentThread().isInterrupted) {
                // Feed compressed data to codec
                if (!inputEOS) {
                    val idx = codec.dequeueInputBuffer(5000)
                    if (idx >= 0) {
                        val buf = codec.getInputBuffer(idx)!!
                        val sz = extractor.readSampleData(buf, 0)
                        if (sz < 0) {
                            codec.queueInputBuffer(idx, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM)
                            inputEOS = true
                        } else {
                            codec.queueInputBuffer(idx, 0, sz, extractor.sampleTime, 0)
                            extractor.advance()
                        }
                    }
                }

                // Pull decoded PCM from codec — emit via runOnUiThread in larger blocks to avoid UI thread bottleneck.
                var outIdx = codec.dequeueOutputBuffer(info, 5000)
                while (outIdx >= 0) {
                    if ((info.flags and MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) outputEOS = true
                    val outBuf = codec.getOutputBuffer(outIdx)
                    if (outBuf != null && info.size > 0) {
                        outBuf.position(info.offset); outBuf.limit(info.offset + info.size)
                        val raw = ByteArray(info.size); outBuf.get(raw)
                        val chunk = resampleToMono(raw, srcRate, targetSampleRate, srcCh)
                        accum.write(chunk)
                        
                        if (accum.size() >= 9600 || outputEOS) {
                            val toSend = accum.toByteArray()
                            accum.reset()
                            runOnUiThread { if (!stopStreamRequested) sink.success(toSend) }
                        }
                    }
                    codec.releaseOutputBuffer(outIdx, false)
                    if (outputEOS) break
                    outIdx = codec.dequeueOutputBuffer(info, 0)
                }
                if (outIdx == MediaCodec.INFO_TRY_AGAIN_LATER && inputEOS) outputEOS = true
            }
            if (accum.size() > 0) {
                val toSend = accum.toByteArray()
                runOnUiThread { if (!stopStreamRequested) sink.success(toSend) }
            }
        } catch (e: Exception) {
            android.util.Log.e("MainActivity", "streamAudioChunks error: ", e)
            runOnUiThread { if (!stopStreamRequested) sink.error("DECODE_ERROR", e.message ?: "Unknown decode error", null) }
        }
        finally {
            try { codec.stop() } catch (_: Exception) {}
            codec.release()
            extractor.release()
            runOnUiThread { if (!stopStreamRequested) sink.endOfStream() }
        }
    }

    private fun isNotificationServiceEnabled(): Boolean {
        val pkgName = packageName
        val flat = Settings.Secure.getString(contentResolver, "enabled_notification_listeners")
        if (flat != null) {
            val names = flat.split(":")
            for (name in names) {
                val cn = android.content.ComponentName.unflattenFromString(name)
                if (cn != null && cn.packageName == pkgName) {
                    return true
                }
            }
        }
        return false
    }

    private fun hasConnectedDevicePermissions(): Boolean {
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.S) {
            val hasConnect = checkSelfPermission(android.Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED
            val hasScan = checkSelfPermission(android.Manifest.permission.BLUETOOTH_SCAN) == PackageManager.PERMISSION_GRANTED
            return hasConnect || hasScan
        }
        return true
    }

    private fun decodeAudioToPcm16kMono(filePath: String): ByteArray? {
        val file = File(filePath)
        if (!file.exists()) return null

        val extractor = MediaExtractor()
        try {
            extractor.setDataSource(file.absolutePath)
        } catch (e: Exception) {
            return null
        }

        var trackIndex = -1
        for (i in 0 until extractor.trackCount) {
            val format = extractor.getTrackFormat(i)
            val mime = format.getString(MediaFormat.KEY_MIME) ?: ""
            if (mime.startsWith("audio/")) {
                trackIndex = i
                break
            }
        }

        if (trackIndex == -1) {
            extractor.release()
            return null
        }

        extractor.selectTrack(trackIndex)
        val inputFormat = extractor.getTrackFormat(trackIndex)
        val mime = inputFormat.getString(MediaFormat.KEY_MIME) ?: return null

        val inputSampleRate = if (inputFormat.containsKey(MediaFormat.KEY_SAMPLE_RATE)) {
            inputFormat.getInteger(MediaFormat.KEY_SAMPLE_RATE)
        } else {
            44100
        }
        val inputChannels = if (inputFormat.containsKey(MediaFormat.KEY_CHANNEL_COUNT)) {
            inputFormat.getInteger(MediaFormat.KEY_CHANNEL_COUNT)
        } else {
            2
        }

        val codec = MediaCodec.createDecoderByType(mime)
        codec.configure(inputFormat, null, null, 0)
        codec.start()

        val rawPcmStream = ByteArrayOutputStream()
        val info = MediaCodec.BufferInfo()
        var isInputEOS = false
        var isOutputEOS = false

        while (!isOutputEOS) {
            if (!isInputEOS) {
                val inputBufferIndex = codec.dequeueInputBuffer(5000)
                if (inputBufferIndex >= 0) {
                    val inputBuffer = codec.getInputBuffer(inputBufferIndex)
                    if (inputBuffer != null) {
                        val sampleSize = extractor.readSampleData(inputBuffer, 0)
                        if (sampleSize < 0) {
                            codec.queueInputBuffer(inputBufferIndex, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM)
                            isInputEOS = true
                        } else {
                            codec.queueInputBuffer(inputBufferIndex, 0, sampleSize, extractor.sampleTime, 0)
                            extractor.advance()
                        }
                    }
                }
            }

            var outputBufferIndex = codec.dequeueOutputBuffer(info, 5000)
            while (outputBufferIndex >= 0) {
                if ((info.flags and MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                    isOutputEOS = true
                }
                val outputBuffer = codec.getOutputBuffer(outputBufferIndex)
                if (outputBuffer != null && info.size > 0) {
                    outputBuffer.position(info.offset)
                    outputBuffer.limit(info.offset + info.size)
                    val chunk = ByteArray(info.size)
                    outputBuffer.get(chunk)
                    rawPcmStream.write(chunk)
                }
                codec.releaseOutputBuffer(outputBufferIndex, false)
                if (isOutputEOS) break
                outputBufferIndex = codec.dequeueOutputBuffer(info, 5000)
            }
            if (outputBufferIndex == MediaCodec.INFO_TRY_AGAIN_LATER && isInputEOS) {
                isOutputEOS = true
            }
        }

        try {
            codec.stop()
        } catch (e: Exception) {}
        codec.release()
        extractor.release()

        val rawPcm = rawPcmStream.toByteArray()
        return resampleToMono(rawPcm, inputSampleRate, 16000, inputChannels)
    }

    private fun resampleToMono(rawPcm: ByteArray, sourceSampleRate: Int, targetSampleRate: Int, channels: Int): ByteArray {
        val numSamples = rawPcm.size / 2
        val shorts = ShortArray(numSamples)
        ByteBuffer.wrap(rawPcm).order(ByteOrder.LITTLE_ENDIAN).asShortBuffer().get(shorts)

        // Step 1: Mix down to mono by averaging all channels
        val monoShorts = if (channels > 1) {
            val mixed = ShortArray(numSamples / channels)
            for (i in mixed.indices) {
                var sum = 0L
                for (c in 0 until channels) {
                    val idx = i * channels + c
                    if (idx < shorts.size) sum += shorts[idx].toLong()
                }
                mixed[i] = (sum / channels).toShort()
            }
            mixed
        } else shorts

        if (sourceSampleRate == targetSampleRate) {
            val resultBytes = ByteArray(monoShorts.size * 2)
            ByteBuffer.wrap(resultBytes).order(ByteOrder.LITTLE_ENDIAN).asShortBuffer().put(monoShorts)
            return resultBytes
        }

        // Step 2: Anti-aliased decimation using box filter (moving average)
        // For each output sample, average ALL input samples that fall in that window.
        // This is a proper low-pass filter that prevents aliasing noise.
        val ratio = sourceSampleRate.toDouble() / targetSampleRate.toDouble()
        val targetSize = (monoShorts.size / ratio).toInt()
        val resampled = ShortArray(targetSize)

        for (i in 0 until targetSize) {
            val srcStart = (i * ratio).toInt()
            val srcEnd = ((i + 1) * ratio).toInt().coerceAtMost(monoShorts.size)
            var sum = 0L
            var count = 0
            for (j in srcStart until srcEnd) {
                sum += monoShorts[j].toLong()
                count++
            }
            resampled[i] = if (count > 0) (sum / count).toShort() else 0
        }

        val resultBytes = ByteArray(resampled.size * 2)
        ByteBuffer.wrap(resultBytes).order(ByteOrder.LITTLE_ENDIAN).asShortBuffer().put(resampled)
        return resultBytes
    }
}
