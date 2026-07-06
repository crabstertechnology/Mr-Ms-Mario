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
import io.flutter.plugin.common.MethodChannel

class LunaBackgroundService : Service() {
    private val CHANNEL_ID = "LunaBackgroundChannel"
    private val NOTIFICATION_ID = 99182

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
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            registerReceiver(receiver, filter, Context.RECEIVER_NOT_EXPORTED)
        } else {
            registerReceiver(receiver, filter)
        }

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
}
