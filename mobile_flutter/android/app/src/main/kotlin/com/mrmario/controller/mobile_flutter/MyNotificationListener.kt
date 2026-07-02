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
        val title = extras.getCharSequence(Notification.EXTRA_TITLE)?.toString() ?: ""
        val text = extras.getCharSequence(Notification.EXTRA_TEXT)?.toString() ?: ""
        val subText = extras.getCharSequence(Notification.EXTRA_SUB_TEXT)?.toString() ?: ""
        val bigText = extras.getCharSequence(Notification.EXTRA_BIG_TEXT)?.toString() ?: ""
        val packageName = sbn.packageName ?: ""

        var smallIconName = ""
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {
            try {
                val smallIcon = sbn.notification.smallIcon
                if (smallIcon != null && smallIcon.type == android.graphics.drawable.Icon.TYPE_RESOURCE) {
                    val resPackage = smallIcon.resPackage ?: packageName
                    val resId = smallIcon.resId
                    if (resId != 0) {
                        val res = packageManager.getResourcesForApplication(resPackage)
                        smallIconName = res.getResourceEntryName(resId)
                    }
                }
            } catch (e: Exception) {
                println("Error getting small icon name: ${e.message}")
            }
        }

        println("MyNotificationListener - Posted: pkg=$packageName, title=$title, text=$text, smallIconName=$smallIconName")

        if (packageName == "com.google.android.apps.maps") {
            for (key in extras.keySet()) {
                try {
                    val value = extras.get(key)
                    println("MapsExtra - $key: $value")
                } catch (e: Exception) {
                    println("MapsExtra - $key: <Error: ${e.message}>")
                }
            }
        }

        // Only process if there's actual content
        if (title.isNotEmpty() || text.isNotEmpty() || subText.isNotEmpty() || bigText.isNotEmpty() || smallIconName.isNotEmpty()) {
            val intent = Intent("com.mrmario.NOTIFICATION_RECEIVED")
            intent.setPackage(this.packageName)
            intent.putExtra("title", title)
            intent.putExtra("text", text)
            intent.putExtra("subText", subText)
            intent.putExtra("bigText", bigText)
            intent.putExtra("package", packageName)
            intent.putExtra("smallIcon", smallIconName)
            sendBroadcast(intent)
        }
    }

    override fun onNotificationRemoved(sbn: StatusBarNotification?) {
        super.onNotificationRemoved(sbn)
        if (sbn == null) return
        val packageName = sbn.packageName ?: ""
        println("MyNotificationListener - Removed: pkg=$packageName")

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
