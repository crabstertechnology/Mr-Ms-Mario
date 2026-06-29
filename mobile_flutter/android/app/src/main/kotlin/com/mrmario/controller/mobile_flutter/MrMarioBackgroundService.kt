package com.mrmario.controller.mobile_flutter

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

class MrMarioBackgroundService : Service() {
    private val CHANNEL_ID = "MrMarioBackgroundChannel"
    private val NOTIFICATION_ID = 99182

    private val receiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context?, intent: Intent?) {
            if (intent?.action == "com.mrmario.NOTIFICATION_RECEIVED") {
                val title = intent.getStringExtra("title") ?: ""
                val text = intent.getStringExtra("text") ?: ""
                val packageName = intent.getStringExtra("package") ?: ""

                // Forward to Flutter engine if it's alive
                val engine = MainActivity.flutterEngine
                if (engine != null) {
                    val channel = MethodChannel(engine.dartExecutor.binaryMessenger, "com.mrmario/notifications")
                    // Invoke on main thread
                    channel.invokeMethod("onNotification", mapOf(
                        "title" to title,
                        "text" to text,
                        "package" to packageName
                    ))
                }
            }
        }
    }

    override fun onCreate() {
        super.onCreate()
        createNotificationChannel()
        startMyForeground()

        // Register broadcast receiver to intercept notifications in the background service itself
        val filter = IntentFilter("com.mrmario.NOTIFICATION_RECEIVED")
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            registerReceiver(receiver, filter, Context.RECEIVER_NOT_EXPORTED)
        } else {
            registerReceiver(receiver, filter)
        }
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        // Refresh foreground notification in case permission was granted after service started
        startMyForeground()
        // Return START_STICKY to keep service running when app process is pushed out of memory
        return START_STICKY
    }

    override fun onDestroy() {
        super.onDestroy()
        try {
            unregisterReceiver(receiver)
        } catch (e: Exception) {}
    }

    override fun onBind(intent: Intent?): IBinder? {
        return null
    }

    private fun createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val name = "Mr. Mario Controller"
            val descriptionText = "Keeps Mr. Mario background sync active"
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
        val notificationIntent = Intent(this, MainActivity::class.java)
        val pendingIntent = PendingIntent.getActivity(
            this, 0, notificationIntent,
            PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_UPDATE_CURRENT
        )

        val iconId = resources.getIdentifier("ic_launcher", "mipmap", packageName)
        val smallIcon = if (iconId != 0) iconId else android.R.drawable.stat_notify_sync

        val notification: Notification = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            Notification.Builder(this, CHANNEL_ID)
                .setContentTitle("Mr. Mario Background Sync")
                .setContentText("Connected to Mr. Mario Robot in background")
                .setSmallIcon(smallIcon)
                .setContentIntent(pendingIntent)
                .build()
        } else {
            @Suppress("DEPRECATION")
            Notification.Builder(this)
                .setContentTitle("Mr. Mario Background Sync")
                .setContentText("Connected to Mr. Mario Robot in background")
                .setSmallIcon(smallIcon)
                .setContentIntent(pendingIntent)
                .build()
        }

        startForeground(NOTIFICATION_ID, notification)
    }
}
