package com.mrmsluna.controller.mobile_flutter

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.app.Service
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.Build
import android.os.IBinder
import android.app.AppOpsManager
import android.app.usage.UsageStatsManager
import android.app.usage.UsageEvents
import android.app.ActivityOptions
import android.util.Log
import java.util.concurrent.ConcurrentHashMap
import io.flutter.plugin.common.MethodChannel

class LunaBackgroundService : Service() {
    private val CHANNEL_ID = "LunaBackgroundChannel"
    private val NOTIFICATION_ID = 99182

    companion object {
        var focusGuardEnabled: Boolean = true
        val focusLimits = ConcurrentHashMap<String, Int>().apply {
            put("com.instagram.android", 5)  // 5 minutes
            put("com.google.android.youtube", 5)
        }
        var currentTrackedPkg: String? = null
        var currentTrackedSeconds: Int = 0
        var alertAlreadySentForSession: Boolean = false
        var lastKnownForegroundPkg: String? = null

        fun updateFocusLimits(context: Context?, enabled: Boolean, limits: Map<String, Int>) {
            focusGuardEnabled = enabled
            focusLimits.clear()
            focusLimits.putAll(limits)
            currentTrackedPkg = null
            currentTrackedSeconds = 0
            alertAlreadySentForSession = false
            println("LunaBackgroundService - Focus Guard updated: enabled=$enabled, limits=${limits.size}")

            if (context != null) {
                try {
                    val prefs = context.getSharedPreferences("LunaFocusPrefs", Context.MODE_PRIVATE)
                    val editor = prefs.edit()
                    editor.putBoolean("focus_enabled", enabled)
                    val json = org.json.JSONObject()
                    for ((k, v) in limits) {
                        json.put(k, v)
                    }
                    editor.putString("focus_limits_json", json.toString())
                    editor.apply()
                } catch (_: Exception) {}
            }
        }
    }

    private val receiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context?, intent: Intent?) {
            val action = intent?.action
            if (action == "com.mrmsluna.NOTIFICATION_RECEIVED") {
                val title = intent.getStringExtra("title") ?: ""
                val text = intent.getStringExtra("text") ?: ""
                val subText = intent.getStringExtra("subText") ?: ""
                val bigText = intent.getStringExtra("bigText") ?: ""
                val packageName = intent.getStringExtra("package") ?: ""
                val smallIcon = intent.getStringExtra("smallIcon") ?: ""
                val directionFromIcon = intent.getStringExtra("directionFromIcon") ?: ""

                // Forward to Flutter engine if it's alive
                val engine = MainActivity.flutterEngine
                if (engine != null) {
                    val channel = MethodChannel(engine.dartExecutor.binaryMessenger, "com.mrmsluna/notifications")
                    // Invoke on main thread
                    channel.invokeMethod("onNotification", mapOf(
                        "title" to title,
                        "text" to text,
                        "subText" to subText,
                        "bigText" to bigText,
                        "package" to packageName,
                        "smallIcon" to smallIcon,
                        "directionFromIcon" to directionFromIcon
                    ))
                }
            } else if (action == "com.mrmsluna.INCOMING_CALL") {
                val caller = intent.getStringExtra("caller") ?: "Incoming Call"
                val packageName = intent.getStringExtra("package") ?: ""
                val engine = MainActivity.flutterEngine
                if (engine != null) {
                    val channel = MethodChannel(engine.dartExecutor.binaryMessenger, "com.mrmsluna/notifications")
                    channel.invokeMethod("onIncomingCall", mapOf(
                        "caller" to caller,
                        "package" to packageName
                    ))
                }
            } else if (action == "com.mrmsluna.CALL_ENDED") {
                val engine = MainActivity.flutterEngine
                if (engine != null) {
                    val channel = MethodChannel(engine.dartExecutor.binaryMessenger, "com.mrmsluna/notifications")
                    channel.invokeMethod("onCallEnded", emptyMap<String, String>())
                }
            } else if (action == "com.mrmsluna.NOTIFICATION_REMOVED") {
                val packageName = intent.getStringExtra("package") ?: ""
                val engine = MainActivity.flutterEngine
                if (engine != null) {
                    val channel = MethodChannel(engine.dartExecutor.binaryMessenger, "com.mrmsluna/notifications")
                    channel.invokeMethod("onNotificationRemoved", mapOf(
                        "package" to packageName
                    ))
                }
            }
        }
    }

    private var isConnectedToRobot = false
    private var lastConnectedStatus: Boolean? = null
    private val handler = android.os.Handler(android.os.Looper.getMainLooper())
    private val updateRunnable = object : Runnable {
        override fun run() {
            val engine = MainActivity.flutterEngine
            val connected = if (engine == null) false else isConnectedToRobot
            updateNotificationStatus(connected)
            if (focusGuardEnabled && focusLimits.isNotEmpty()) {
                checkFocusGuard()
            }
            handler.postDelayed(this, 1000)
        }
    }

    override fun onCreate() {
        super.onCreate()
        createNotificationChannel()
        startMyForeground()

        // Register broadcast receiver to intercept notifications in the background service itself
        val filter = IntentFilter()
        filter.addAction("com.mrmsluna.NOTIFICATION_RECEIVED")
        filter.addAction("com.mrmsluna.NOTIFICATION_REMOVED")
        filter.addAction("com.mrmsluna.INCOMING_CALL")
        filter.addAction("com.mrmsluna.CALL_ENDED")
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            registerReceiver(receiver, filter, Context.RECEIVER_NOT_EXPORTED)
        } else {
            registerReceiver(receiver, filter)
        }

        // Restore focus settings from native prefs
        try {
            val prefs = getSharedPreferences("LunaFocusPrefs", Context.MODE_PRIVATE)
            focusGuardEnabled = prefs.getBoolean("focus_enabled", true)
            val jsonStr = prefs.getString("focus_limits_json", null)
            if (!jsonStr.isNullOrEmpty()) {
                val json = org.json.JSONObject(jsonStr)
                focusLimits.clear()
                val keys = json.keys()
                while (keys.hasNext()) {
                    val k = keys.next()
                    focusLimits[k] = json.getInt(k)
                }
                println("LunaBackgroundService - Restored ${focusLimits.size} focus limits from native prefs")
            }
        } catch (_: Exception) {}

        // Start 1-second background connection status updater
        handler.post(updateRunnable)
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        if (intent != null && intent.hasExtra("connected")) {
            isConnectedToRobot = intent.getBooleanExtra("connected", false)
        }
        val engine = MainActivity.flutterEngine
        val connected = if (engine == null) false else isConnectedToRobot
        updateNotificationStatus(connected)
        
        return START_STICKY
    }

    override fun onDestroy() {
        super.onDestroy()
        handler.removeCallbacks(updateRunnable)
        try {
            unregisterReceiver(receiver)
        } catch (e: Exception) {}
    }

    override fun onBind(intent: Intent?): IBinder? {
        return null
    }

    private fun createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val name = "Mr.&Ms Luna Controller"
            val descriptionText = "Keeps Mr.&Ms Luna background sync active"
            val importance = NotificationManager.IMPORTANCE_DEFAULT
            val channel = NotificationChannel(CHANNEL_ID, name, importance).apply {
                description = descriptionText
            }
            val notificationManager: NotificationManager =
                getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
            notificationManager.createNotificationChannel(channel)
        }
    }

    private fun startMyForeground() {
        val engine = MainActivity.flutterEngine
        val connected = if (engine == null) false else isConnectedToRobot
        updateNotificationStatus(connected)
    }

    private fun updateNotificationStatus(connected: Boolean) {
        if (lastConnectedStatus == connected) {
            return
        }
        lastConnectedStatus = connected

        val notificationIntent = Intent(this, MainActivity::class.java)
        val pendingIntent = PendingIntent.getActivity(
            this, 0, notificationIntent,
            PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_UPDATE_CURRENT
        )

        val iconId = resources.getIdentifier("ic_launcher", "mipmap", packageName)
        val smallIcon = if (iconId != 0) iconId else android.R.drawable.stat_notify_sync

        val statusText = if (connected) "Status: Connected to Robot" else "Status: Disconnected from Robot"

        val notification: Notification = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            Notification.Builder(this, CHANNEL_ID)
                .setContentTitle("Mr.&Ms Luna Controller")
                .setContentText(statusText)
                .setSmallIcon(smallIcon)
                .setContentIntent(pendingIntent)
                .setOngoing(true) // Prevents user swiping it away
                .setOnlyAlertOnce(true) // Prevents continuous alert chirps
                .build()
        } else {
            @Suppress("DEPRECATION")
            Notification.Builder(this)
                .setContentTitle("Mr.&Ms Luna Controller")
                .setContentText(statusText)
                .setSmallIcon(smallIcon)
                .setContentIntent(pendingIntent)
                .setOngoing(true)
                .setOnlyAlertOnce(true)
                .build()
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            startForeground(NOTIFICATION_ID, notification, android.content.pm.ServiceInfo.FOREGROUND_SERVICE_TYPE_CONNECTED_DEVICE)
        } else {
            startForeground(NOTIFICATION_ID, notification)
        }
    }

    private fun checkFocusGuard() {
        val currentPkg = getForegroundPackage() ?: run {
            if (currentTrackedPkg != null) {
                currentTrackedPkg = null
                currentTrackedSeconds = 0
                alertAlreadySentForSession = false
            }
            return
        }

        val limitMins = focusLimits[currentPkg]
        if (limitMins != null && limitMins > 0) {
            val limitSecs = limitMins * 60
            if (currentTrackedPkg == currentPkg) {
                currentTrackedSeconds++
                Log.i("FocusGuard", "Active $currentPkg: ${currentTrackedSeconds}s / ${limitSecs}s")
                if (currentTrackedSeconds >= limitSecs && !alertAlreadySentForSession) {
                    alertAlreadySentForSession = true
                    val label = getAppLabel(currentPkg)
                    Log.i("FocusGuard", "Focus limit reached for $label — sending Luna alert")
                    triggerFocusAlert(currentPkg, label, limitMins)
                    // Reset timer so alert fires again next session
                    currentTrackedSeconds = 0
                    alertAlreadySentForSession = false
                }
            } else {
                currentTrackedPkg = currentPkg
                currentTrackedSeconds = 1
                alertAlreadySentForSession = false
                Log.i("FocusGuard", "Started tracking $currentPkg (limit: ${limitMins}m = ${limitSecs}s)")
            }
        } else {
            if (currentTrackedPkg != null) {
                currentTrackedPkg = null
                currentTrackedSeconds = 0
                alertAlreadySentForSession = false
            }
        }
    }

    private fun getAppLabel(pkg: String): String {
        return try {
            val pm = packageManager
            val info = pm.getApplicationInfo(pkg, 0)
            pm.getApplicationLabel(info).toString()
        } catch (e: Exception) {
            if (pkg.contains("instagram")) "Instagram"
            else if (pkg.contains("youtube")) "YouTube"
            else pkg
        }
    }

    private fun triggerFocusAlert(pkg: String, label: String, limitMinutes: Int) {
        Log.i("FocusGuard", "Focus limit exceeded for $label ($pkg), limit=$limitMinutes min")
        val engine = MainActivity.flutterEngine
        if (engine != null) {
            val channel = MethodChannel(engine.dartExecutor.binaryMessenger, "com.mrmsluna/notifications")
            channel.invokeMethod("onFocusLimitExceeded", mapOf(
                "package" to pkg,
                "label" to label,
                "limitMinutes" to limitMinutes
            ))
        }
        // Do not display notification text banner — user requested only angry emoji and alert on Luna
    }

    private fun sendFocusWarningNotification(label: String, limitMinutes: Int) {
        val notificationManager = getSystemService(Context.NOTIFICATION_SERVICE) as? NotificationManager ?: return
        val notifIntent = Intent(this, MainActivity::class.java)
        val pendingIntent = PendingIntent.getActivity(
            this, 99183, notifIntent,
            PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_UPDATE_CURRENT
        )
        val iconId = resources.getIdentifier("ic_launcher", "mipmap", packageName)
        val smallIcon = if (iconId != 0) iconId else android.R.drawable.stat_notify_sync

        val notif = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            Notification.Builder(this, CHANNEL_ID)
                .setContentTitle("⚠️ Luna Focus Alert: $label")
                .setContentText("You've been on $label for $limitMinutes min! Put your phone down.")
                .setSmallIcon(smallIcon)
                .setContentIntent(pendingIntent)
                .setAutoCancel(true)
                .build()
        } else {
            @Suppress("DEPRECATION")
            Notification.Builder(this)
                .setContentTitle("⚠️ Luna Focus Alert: $label")
                .setContentText("You've been on $label for $limitMinutes min! Put your phone down.")
                .setSmallIcon(smallIcon)
                .setContentIntent(pendingIntent)
                .setAutoCancel(true)
                .build()
        }
        notificationManager.notify(99184, notif)
    }

    private var lastLoggedFgPkg: String? = "UNSET"
    private fun getForegroundPackage(): String? {
        return try {
            val appOps = getSystemService(Context.APP_OPS_SERVICE) as? AppOpsManager ?: return null
            val mode = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                appOps.unsafeCheckOpNoThrow(AppOpsManager.OPSTR_GET_USAGE_STATS, android.os.Process.myUid(), packageName)
            } else {
                appOps.checkOpNoThrow(AppOpsManager.OPSTR_GET_USAGE_STATS, android.os.Process.myUid(), packageName)
            }
            if (mode != AppOpsManager.MODE_ALLOWED) {
                Log.e("FocusGuard", "getForegroundPackage: Usage Access DENIED")
                return null
            }

            val usm = getSystemService(Context.USAGE_STATS_SERVICE) as? UsageStatsManager ?: return null
            val now = System.currentTimeMillis()

            // === PRIMARY: queryUsageStats with short windows (more reliable on MIUI) ===
            // Try a tight 3-second window first — if app is truly active it'll appear here
            var result: String? = null
            for (windowMs in listOf(3_000L, 10_000L, 30_000L)) {
                val stats = usm.queryUsageStats(UsageStatsManager.INTERVAL_BEST, now - windowMs, now)
                if (!stats.isNullOrEmpty()) {
                    val best = stats
                        .filter { it.lastTimeUsed > now - windowMs }
                        .filter { it.packageName != packageName } // exclude ourselves
                        .maxByOrNull { it.lastTimeUsed }
                    if (best != null) {
                        result = best.packageName
                        break
                    }
                }
            }

            // === FALLBACK: queryEvents (180s window) ===
            if (result == null) {
                val events = usm.queryEvents(now - 180_000L, now)
                val event = UsageEvents.Event()
                val resumeTimes = HashMap<String, Long>()
                val pauseTimes = HashMap<String, Long>()
                while (events.hasNextEvent()) {
                    events.getNextEvent(event)
                    val pkg = event.packageName ?: continue
                    when (event.eventType) {
                        UsageEvents.Event.ACTIVITY_RESUMED -> resumeTimes[pkg] = event.timeStamp
                        UsageEvents.Event.ACTIVITY_PAUSED, UsageEvents.Event.ACTIVITY_STOPPED -> pauseTimes[pkg] = event.timeStamp
                    }
                }
                result = resumeTimes.entries
                    .filter { (pkg, resumeTime) -> resumeTime > (pauseTimes[pkg] ?: 0L) }
                    .maxByOrNull { it.value }
                    ?.key

                // Sticky fallback: if no transition events, trust lastKnownForegroundPkg unless it was explicitly paused
                if (result == null) {
                    val lastPkg = lastKnownForegroundPkg
                    if (lastPkg != null) {
                        val lastPause = pauseTimes[lastPkg] ?: 0L
                        val lastResume = resumeTimes[lastPkg] ?: 0L
                        if (lastPause > 0 && lastPause > lastResume) {
                            lastKnownForegroundPkg = null
                        } else {
                            result = lastPkg
                        }
                    }
                }
            }

            // Log on change only (to reduce MIUI log quota pressure)
            if (result != lastLoggedFgPkg) {
                Log.i("FocusGuard", ">>> FG CHANGE: $lastLoggedFgPkg -> $result")
                lastLoggedFgPkg = result
            }
            if (result != null) lastKnownForegroundPkg = result

            result
        } catch (e: Exception) {
            Log.e("FocusGuard", "getForegroundPackage exception: ${e.message}")
            null
        }
    }
}
