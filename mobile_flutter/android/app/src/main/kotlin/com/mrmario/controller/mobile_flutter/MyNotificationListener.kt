package com.mrmario.controller.mobile_flutter

import android.service.notification.NotificationListenerService
import android.service.notification.StatusBarNotification
import android.content.Intent
import android.app.Notification
import android.os.Bundle
import android.os.Handler
import android.os.Looper

class MyNotificationListener : NotificationListenerService() {
    private val handler = Handler(Looper.getMainLooper())

    override fun onNotificationPosted(sbn: StatusBarNotification?) {
        super.onNotificationPosted(sbn)
        if (sbn == null) return

        val extras = sbn.notification.extras
        val title = extras.getString(Notification.EXTRA_TITLE) ?: ""
        val text = extras.getCharSequence(Notification.EXTRA_TEXT)?.toString() ?: ""
        val packageName = sbn.packageName ?: ""

        // Only process if there's actual content
        if (title.isNotEmpty() || text.isNotEmpty()) {
            val intent = Intent("com.mrmario.NOTIFICATION_RECEIVED")
            intent.setPackage(this.packageName)
            intent.putExtra("title", title)
            intent.putExtra("text", text)
            intent.putExtra("package", packageName)
            sendBroadcast(intent)
        }
    }

    override fun onNotificationRemoved(sbn: StatusBarNotification?) {
        super.onNotificationRemoved(sbn)
        if (sbn == null) return
        val packageName = sbn.packageName ?: ""

        if (packageName == "com.google.android.apps.maps") {
            // Debounce exit check by 1.5 seconds to prevent race conditions during updates
            handler.postDelayed({
                var hasMapsNotif = false
                try {
                    val activeNotifs = activeNotifications
                    if (activeNotifs != null) {
                        for (n in activeNotifs) {
                            if (n.packageName == "com.google.android.apps.maps") {
                                hasMapsNotif = true
                                break
                            }
                        }
                    }
                } catch (e: Exception) {}

                if (!hasMapsNotif) {
                    val intent = Intent("com.mrmario.NOTIFICATION_REMOVED")
                    intent.setPackage(this.packageName)
                    intent.putExtra("package", packageName)
                    sendBroadcast(intent)
                }
            }, 1500)
        } else {
            val intent = Intent("com.mrmario.NOTIFICATION_REMOVED")
            intent.setPackage(this.packageName)
            intent.putExtra("package", packageName)
            sendBroadcast(intent)
        }
    }
}
