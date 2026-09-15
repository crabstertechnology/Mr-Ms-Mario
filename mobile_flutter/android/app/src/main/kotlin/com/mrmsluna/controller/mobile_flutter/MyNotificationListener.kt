package com.mrmsluna.controller.mobile_flutter

import android.service.notification.NotificationListenerService
import android.service.notification.StatusBarNotification
import android.content.Intent
import android.content.Context
import android.app.Notification
import android.app.RemoteInput
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import java.util.concurrent.ConcurrentHashMap

class MyNotificationListener : NotificationListenerService() {
    private val handler = Handler(Looper.getMainLooper())

    companion object {
        var instance: MyNotificationListener? = null
        // Cache direct reply actions: package -> Pair<Notification.Action, RemoteInput>
        val directReplyActions = ConcurrentHashMap<String, Pair<Notification.Action, RemoteInput>>()
        // Cache active incoming call decline action and package
        var activeCallDeclineAction: Notification.Action? = null
        var activeCallPackage: String = ""

        fun sendQuickReply(context: Context, targetPackage: String, replyText: String): Boolean {
            val pair = directReplyActions[targetPackage] ?: run {
                if (targetPackage.contains("whatsapp")) {
                    directReplyActions["com.whatsapp"] ?: directReplyActions["com.whatsapp.w4b"]
                } else null
            } ?: run {
                // Fallback to any available direct reply action
                directReplyActions.values.firstOrNull()
            } ?: return false

            val action = pair.first
            val remoteInput = pair.second
            return try {
                val intent = Intent()
                val bundle = Bundle()
                bundle.putCharSequence(remoteInput.resultKey, replyText)
                RemoteInput.addResultsToIntent(arrayOf(remoteInput), intent, bundle)
                action.actionIntent.send(context, 0, intent)
                println("MyNotificationListener - Quick Reply sent: '$replyText' to ${action.title}")
                true
            } catch (e: Exception) {
                println("MyNotificationListener - Error sending quick reply: ${e.message}")
                false
            }
        }

        fun declineActiveCall(): Boolean {
            activeCallDeclineAction?.let { action ->
                try {
                    action.actionIntent.send()
                    println("MyNotificationListener - Executed decline action: '${action.title}'")
                    activeCallDeclineAction = null
                    return true
                } catch (e: Exception) {
                    println("MyNotificationListener - Error declining call: ${e.message}")
                }
            }
            return false
        }
    }

    override fun onCreate() {
        super.onCreate()
        instance = this
    }

    override fun onListenerConnected() {
        super.onListenerConnected()
        instance = this
        println("MyNotificationListener - Connected")
    }

    override fun onListenerDisconnected() {
        super.onListenerDisconnected()
        instance = null
    }

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

        var directionFromIcon = ""
        if (packageName == "com.google.android.apps.maps") {
            for (key in extras.keySet()) {
                try {
                    val value = extras.get(key)
                    println("MapsExtra - $key: $value")
                } catch (e: Exception) {
                    println("MapsExtra - $key: <Error: ${e.message}>")
                }
            }

            try {
                val largeIcon = sbn.notification.getLargeIcon()
                if (largeIcon != null) {
                    val drawable = largeIcon.loadDrawable(this)
                    if (drawable != null) {
                        val bitmap = drawableToBitmap(drawable)
                        directionFromIcon = analyzeDirectionFromBitmap(bitmap)
                        println("MyNotificationListener - Analyzed direction from largeIcon: $directionFromIcon")
                    }
                }
            } catch (e: Exception) {
                println("Error analyzing large icon: ${e.message}")
            }
        }

        // Cache any available Direct Reply action (e.g. WhatsApp, WhatsApp Business, SMS)
        val actions = sbn.notification.actions
        if (actions != null) {
            for (action in actions) {
                val remoteInputs = action.remoteInputs
                if (remoteInputs != null && remoteInputs.isNotEmpty()) {
                    directReplyActions[packageName] = Pair(action, remoteInputs[0])
                    println("MyNotificationListener - Cached direct reply action for $packageName ('${action.title}')")
                    break
                }
            }
        }

        // Detect incoming phone call notifications (ODialer, system dialers, telecom)
        val pkgLower = packageName.lowercase()
        val isCallCategory = sbn.notification.category == Notification.CATEGORY_CALL
        val isCallPkg = pkgLower.contains("dialer") || pkgLower.contains("telecom") || pkgLower.contains("phone") || pkgLower.contains("incallui")
        val combinedNotifText = "$title $text $subText $bigText".lowercase()
        val isIncomingCall = isCallCategory || (isCallPkg && (
            combinedNotifText.contains("incoming") || combinedNotifText.contains("calling") || sbn.isOngoing
        ))

        if (isIncomingCall) {
            activeCallPackage = packageName
            if (actions != null) {
                for (action in actions) {
                    val actTitle = action.title?.toString()?.lowercase() ?: ""
                    if (actTitle.contains("decline") || actTitle.contains("reject") || actTitle.contains("dismiss") || actTitle.contains("hang") || actTitle.contains("cut")) {
                        activeCallDeclineAction = action
                        println("MyNotificationListener - Cached call decline action: '${action.title}'")
                        break
                    }
                }
            }
            val callerName = when {
                title.isNotEmpty() && !title.lowercase().contains("call") -> title
                text.isNotEmpty() && !text.lowercase().contains("call") -> text
                else -> if (title.isNotEmpty()) title else "Incoming Call"
            }
            val callIntent = Intent("com.mrmsluna.INCOMING_CALL")
            callIntent.setPackage(this.packageName)
            callIntent.putExtra("caller", callerName)
            callIntent.putExtra("package", packageName)
            sendBroadcast(callIntent)
            println("MyNotificationListener - Broadcasted INCOMING_CALL: caller='$callerName', pkg=$packageName")
        }

        // Only process if there's actual content
        if (title.isNotEmpty() || text.isNotEmpty() || subText.isNotEmpty() || bigText.isNotEmpty() || smallIconName.isNotEmpty()) {
            val intent = Intent("com.mrmsluna.NOTIFICATION_RECEIVED")
            intent.setPackage(this.packageName)
            intent.putExtra("title", title)
            intent.putExtra("text", text)
            intent.putExtra("subText", subText)
            intent.putExtra("bigText", bigText)
            intent.putExtra("package", packageName)
            intent.putExtra("smallIcon", smallIconName)
            intent.putExtra("directionFromIcon", directionFromIcon)
            sendBroadcast(intent)
        }
    }

    override fun onNotificationRemoved(sbn: StatusBarNotification?) {
        super.onNotificationRemoved(sbn)
        if (sbn == null) return
        val packageName = sbn.packageName ?: ""
        println("MyNotificationListener - Removed: pkg=$packageName")

        if (packageName == activeCallPackage || sbn.notification.category == Notification.CATEGORY_CALL) {
            activeCallDeclineAction = null
            activeCallPackage = ""
            val callEndIntent = Intent("com.mrmsluna.CALL_ENDED")
            callEndIntent.setPackage(this.packageName)
            sendBroadcast(callEndIntent)
            println("MyNotificationListener - Broadcasted CALL_ENDED")
        }

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
                    val intent = Intent("com.mrmsluna.NOTIFICATION_REMOVED")
                    intent.setPackage(this.packageName)
                    intent.putExtra("package", packageName)
                    sendBroadcast(intent)
                }
            }, 1500)
        } else {
            val intent = Intent("com.mrmsluna.NOTIFICATION_REMOVED")
            intent.setPackage(this.packageName)
            intent.putExtra("package", packageName)
            sendBroadcast(intent)
        }
    }

    private fun drawableToBitmap(drawable: android.graphics.drawable.Drawable): android.graphics.Bitmap {
        if (drawable is android.graphics.drawable.BitmapDrawable) {
            if (drawable.bitmap != null) {
                return drawable.bitmap
            }
        }
        val bitmap = if (drawable.intrinsicWidth <= 0 || drawable.intrinsicHeight <= 0) {
            android.graphics.Bitmap.createBitmap(1, 1, android.graphics.Bitmap.Config.ARGB_8888)
        } else {
            android.graphics.Bitmap.createBitmap(drawable.intrinsicWidth, drawable.intrinsicHeight, android.graphics.Bitmap.Config.ARGB_8888)
        }
        val canvas = android.graphics.Canvas(bitmap)
        drawable.setBounds(0, 0, canvas.width, canvas.height)
        drawable.draw(canvas)
        return bitmap
    }

    private fun analyzeDirectionFromBitmap(bitmap: android.graphics.Bitmap): String {
        try {
            val width = bitmap.width
            val height = bitmap.height
            if (width <= 0 || height <= 0) return ""

            val scaled = android.graphics.Bitmap.createScaledBitmap(bitmap, 16, 16, true)

            var hasTransparency = false
            for (y in 0 until 16) {
                for (x in 0 until 16) {
                    val pixel = scaled.getPixel(x, y)
                    val alpha = android.graphics.Color.alpha(pixel)
                    if (alpha < 200) {
                        hasTransparency = true
                        break
                    }
                }
                if (hasTransparency) break
            }

            val grid = Array(16) { BooleanArray(16) }
            
            if (hasTransparency) {
                for (y in 0 until 16) {
                    for (x in 0 until 16) {
                        val pixel = scaled.getPixel(x, y)
                        grid[y][x] = android.graphics.Color.alpha(pixel) > 50
                    }
                }
            } else {
                var totalBrightness = 0L
                val brightnesses = Array(16) { IntArray(16) }
                for (y in 0 until 16) {
                    for (x in 0 until 16) {
                        val pixel = scaled.getPixel(x, y)
                        val r = android.graphics.Color.red(pixel)
                        val g = android.graphics.Color.green(pixel)
                        val b = android.graphics.Color.blue(pixel)
                        val brightness = (r + g + b) / 3
                        brightnesses[y][x] = brightness
                        totalBrightness += brightness
                    }
                }
                val avgBrightness = totalBrightness / 256
                for (y in 0 until 16) {
                    for (x in 0 until 16) {
                        grid[y][x] = brightnesses[y][x] > (avgBrightness + 15)
                    }
                }
            }

            println("--- Maneuver Icon Art ---")
            for (y in 0 until 16) {
                val sb = java.lang.StringBuilder()
                for (x in 0 until 16) {
                    sb.append(if (grid[y][x]) "#" else ".")
                }
                println(sb.toString())
            }
            println("-------------------------")

            var topLeft = 0
            var topRight = 0
            for (y in 0 until 8) { // Top half (rows 0-7)
                for (x in 0 until 16) {
                    if (grid[y][x]) {
                        if (x < 8) topLeft++
                        else topRight++
                    }
                }
            }

            println("Icon Analysis: topLeft=$topLeft, topRight=$topRight")

            val diff = topRight - topLeft
            if (diff > 4) {
                return "RIGHT"
            } else if (diff < -4) {
                return "LEFT"
            }
            return "STRAIGHT"
        } catch (e: Exception) {
            println("Error in analyzeDirectionFromBitmap: ${e.message}")
        }
        return ""
    }
}
