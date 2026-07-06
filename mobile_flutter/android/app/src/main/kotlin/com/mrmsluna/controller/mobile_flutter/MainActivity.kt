package com.mrmario.controller.mobile_flutter

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

class MainActivity: FlutterActivity() {
    private val CHANNEL = "com.mrmario/notifications"
    private var methodChannel: MethodChannel? = null

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
            val serviceIntent = Intent(this, MrMarioBackgroundService::class.java)
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
                        val serviceIntent = Intent(this, MrMarioBackgroundService::class.java)
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
                        val serviceIntent = Intent(this, MrMarioBackgroundService::class.java).apply {
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
                    try {
                        val appsList = ArrayList<Map<String, String>>()
                        val pm = packageManager
                        val packages = pm.getInstalledApplications(PackageManager.GET_META_DATA)
                        for (packageInfo in packages) {
                            val label = packageInfo.loadLabel(pm).toString()
                            val packageName = packageInfo.packageName
                            
                            val appMap = HashMap<String, String>()
                            appMap["name"] = label
                            appMap["packageName"] = packageName
                            appsList.add(appMap)
                        }
                        appsList.sortBy { it["name"]?.lowercase() ?: "" }
                        result.success(appsList)
                    } catch (e: Exception) {
                        result.error("ERROR", e.message, null)
                    }
                }
                else -> result.notImplemented()
        }
    }
    }

    override fun onDestroy() {
        super.onDestroy()
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
}
