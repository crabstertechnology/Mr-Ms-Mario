package com.mrmario.controller.mobile_flutter

import android.service.notification.NotificationListenerService
import android.service.notification.StatusBarNotification
import android.content.Intent
import android.app.Notification
import android.os.Bundle

class MyNotificationListener : NotificationListenerService() {
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
}
