package com.mrmsluna.controller.mobile_flutter

import android.accessibilityservice.AccessibilityService
import android.content.Context
import android.content.Intent
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.accessibility.AccessibilityEvent
import android.view.accessibility.AccessibilityNodeInfo
import io.flutter.plugin.common.MethodChannel
import java.util.concurrent.ConcurrentHashMap

class SynapsAccessibilityService : AccessibilityService() {

    companion object {
        private const val TAG = "SynapsAccessibility"
        var instance: SynapsAccessibilityService? = null

        // Default configurations
        var detachEnabled: Boolean = true
        var customMessage: String = "Get back to what matters."
        var autoHomeSeconds: Int = 4

        // Blocked domain strings (substring matching on browser URL bar)
        val blockedDomains = ConcurrentHashMap.newKeySet<String>().apply {
            add("youtube.com")
            add("m.youtube.com")
            add("youtu.be")
            add("instagram.com")
            add("m.instagram.com")
        }

        // Blocked app package names
        val blockedApps = ConcurrentHashMap.newKeySet<String>().apply {
            add("com.instagram.android")
            add("com.google.android.youtube")
        }

        // Known browser package names
        val browserPackages = setOf(
            "com.android.chrome",
            "com.chrome.beta",
            "com.chrome.dev",
            "com.chrome.canary",
            "org.mozilla.firefox",
            "org.mozilla.firefox_beta",
            "com.sec.android.app.sbrowser",
            "com.microsoft.emmx",
            "com.brave.browser",
            "com.opera.browser",
            "com.opera.mini.native",
            "com.opera.touch",
            "com.duckduckgo.mobile.android"
        )

        // Known URL bar view IDs across popular browsers
        private val urlBarViewIds = listOf(
            "com.android.chrome:id/url_bar",
            "org.mozilla.firefox:id/url_bar_title",
            "com.sec.android.app.sbrowser:id/location_bar_edit_text",
            "com.microsoft.emmx:id/url_bar",
            "com.brave.browser:id/url_bar",
            "com.opera.browser:id/url_field"
        )

        val paymentApps = setOf(
            "com.google.android.apps.nbu.paisa.user", // Google Pay
            "com.phonepe.app",                       // PhonePe
            "net.one97.paytm",                       // Paytm
            "in.org.npci.upiapp",                    // BHIM UPI
            "com.dreamplug.androidapp"               // CRED
        )

        fun isRunning(): Boolean = instance != null

        fun setAccessibilityServiceEnabled(context: Context, enable: Boolean): Boolean {
            return try {
                val cr = context.contentResolver
                val expectedComponentName = android.content.ComponentName(context, SynapsAccessibilityService::class.java).flattenToString()
                val enabledServices = android.provider.Settings.Secure.getString(cr, android.provider.Settings.Secure.ENABLED_ACCESSIBILITY_SERVICES) ?: ""
                val set = mutableSetOf<String>()
                for (service in enabledServices.split(':')) {
                    if (service.isNotBlank() && service != expectedComponentName) {
                        set.add(service)
                    }
                }
                if (enable) {
                    set.add(expectedComponentName)
                }
                val newSetting = set.joinToString(":")
                android.provider.Settings.Secure.putString(cr, android.provider.Settings.Secure.ENABLED_ACCESSIBILITY_SERVICES, newSetting)
                if (enable) {
                    android.provider.Settings.Secure.putInt(cr, android.provider.Settings.Secure.ACCESSIBILITY_ENABLED, 1)
                }
                true
            } catch (e: Exception) {
                false
            }
        }

        fun disableService(): Boolean {
            return try {
                if (instance != null) {
                    instance?.disableSelf()
                    instance = null
                    Log.i(TAG, "SynapsAccessibilityService disabled via disableSelf()")
                    true
                } else {
                    false
                }
            } catch (e: Exception) {
                Log.e(TAG, "Failed to disableSelf: ${e.message}")
                false
            }
        }

        fun updateConfig(
            context: Context?,
            enabled: Boolean,
            domains: List<String>,
            apps: List<String>,
            message: String?,
            autoHome: Int?
        ) {
            detachEnabled = enabled
            if (message != null) customMessage = message
            if (autoHome != null) autoHomeSeconds = autoHome

            blockedDomains.clear()
            for (d in domains) {
                blockedDomains.add(d.lowercase().trim())
            }

            blockedApps.clear()
            for (a in apps) {
                blockedApps.add(a.lowercase().trim())
            }

            Log.i(TAG, "Config updated: enabled=$enabled, domains=${blockedDomains.size}, apps=${blockedApps.size}")

            if (context != null) {
                try {
                    val prefs = context.getSharedPreferences("SynapsDetachPrefs", Context.MODE_PRIVATE)
                    prefs.edit()
                        .putBoolean("detach_enabled", enabled)
                        .putStringSet("blocked_domains", blockedDomains)
                        .putStringSet("blocked_apps", blockedApps)
                        .putString("custom_message", customMessage)
                        .putInt("auto_home_seconds", autoHomeSeconds)
                        .apply()
                } catch (e: Exception) {
                    Log.e(TAG, "Error saving detach prefs: ${e.message}")
                }
            }
        }
    }

    private val mainHandler = Handler(Looper.getMainLooper())
    var isCurrentlyBlocking = false
    private var lastInterceptTimestamp = 0L

    override fun onServiceConnected() {
        super.onServiceConnected()
        instance = this
        Log.i(TAG, "SynapsAccessibilityService CONNECTED")

        // Load persisted preferences
        try {
            val prefs = getSharedPreferences("SynapsDetachPrefs", Context.MODE_PRIVATE)
            detachEnabled = prefs.getBoolean("detach_enabled", true)
            customMessage = prefs.getString("custom_message", "Get back to what matters.") ?: "Get back to what matters."
            autoHomeSeconds = prefs.getInt("auto_home_seconds", 4)

            val savedDomains = prefs.getStringSet("blocked_domains", null)
            if (!savedDomains.isNullOrEmpty()) {
                blockedDomains.clear()
                blockedDomains.addAll(savedDomains)
            }

            val savedApps = prefs.getStringSet("blocked_apps", null)
            if (!savedApps.isNullOrEmpty()) {
                blockedApps.clear()
                blockedApps.addAll(savedApps)
            }
        } catch (e: Exception) {
            Log.e(TAG, "Error loading saved prefs: ${e.message}")
        }
    }

    override fun onDestroy() {
        instance = null
        DetachOverlayManager.dismiss()
        super.onDestroy()
    }

    override fun onInterrupt() {
        Log.w(TAG, "SynapsAccessibilityService INTERRUPTED")
    }

    override fun onAccessibilityEvent(event: AccessibilityEvent?) {
        if (!detachEnabled || event == null) return

        val pkgName = event.packageName?.toString() ?: return

        // 0. Auto-disable if user opens Google Pay or other UPI payment apps
        if (paymentApps.contains(pkgName.lowercase())) {
            Log.i(TAG, "GPay / Payment app detected ($pkgName)! Auto-disabling accessibility to unblock payments...")
            disableSelf()
            instance = null
            return
        }

        // 1. Check direct blocked app
        if (blockedApps.contains(pkgName.lowercase())) {
            handleBlockedApp(pkgName)
            return
        }

        // 2. Check browser
        if (browserPackages.contains(pkgName)) {
            handleBrowserInspection(browserPkg = pkgName, event = event)
            return
        }

        // 3. User exited blocked context
        if (isCurrentlyBlocking && pkgName != packageName && !browserPackages.contains(pkgName) && !blockedApps.contains(pkgName.lowercase())) {
            Log.i(TAG, "User exited blocked context to: $pkgName. Dismissing overlay.")
            isCurrentlyBlocking = false
            DetachOverlayManager.dismiss()
        }
    }

    private fun handleBlockedApp(pkgName: String) {
        val now = System.currentTimeMillis()
        if (now - lastInterceptTimestamp < 1500L && isCurrentlyBlocking) return
        lastInterceptTimestamp = now

        val appName = getAppLabel(pkgName)
        Log.i(TAG, "Direct blocked app opened: $appName ($pkgName)")
        isCurrentlyBlocking = true

        DetachOverlayManager.show(
            context = this,
            sourceInfo = appName,
            customMessage = customMessage,
            autoHomeSeconds = autoHomeSeconds
        )

        notifyFlutterOfBlock(pkgName, appName, "app")
    }

    private fun handleBrowserInspection(browserPkg: String, event: AccessibilityEvent) {
        val rootNode = rootInActiveWindow ?: return

        try {
            var currentUrl: String? = null
            for (viewId in urlBarViewIds) {
                val nodes = rootNode.findAccessibilityNodeInfosByViewId(viewId)
                if (!nodes.isNullOrEmpty()) {
                    for (node in nodes) {
                        val text = node.text?.toString()
                        if (!text.isNullOrBlank()) {
                            currentUrl = text
                            break
                        }
                    }
                }
                if (currentUrl != null) break
            }

            if (currentUrl == null) {
                currentUrl = searchForUrlInTree(rootNode, maxDepth = 6)
            }

            if (currentUrl != null) {
                val normalizedUrl = currentUrl.lowercase()
                var matchedDomain: String? = null

                for (domain in blockedDomains) {
                    if (normalizedUrl.contains(domain)) {
                        matchedDomain = domain
                        break
                    }
                }

                if (matchedDomain != null) {
                    val now = System.currentTimeMillis()
                    if (now - lastInterceptTimestamp < 1500L && isCurrentlyBlocking) return
                    lastInterceptTimestamp = now

                    val browserName = getAppLabel(browserPkg)
                    val sourceDesc = "$matchedDomain in $browserName"
                    Log.i(TAG, "RESTRICTED WEBSITE DETECTED: $sourceDesc (Raw: $currentUrl)")

                    isCurrentlyBlocking = true

                    // 1. Show full-screen overlay
                    DetachOverlayManager.show(
                        context = this,
                        sourceInfo = sourceDesc,
                        customMessage = customMessage,
                        autoHomeSeconds = autoHomeSeconds
                    )

                    // 2. Close the restricted browser tab
                    closeRestrictedBrowserTab(browserPkg)

                    notifyFlutterOfBlock(browserPkg, sourceDesc, "website")
                }
            }
        } catch (e: Exception) {
            Log.e(TAG, "Error inspecting browser window: ${e.message}")
        }
    }

    private fun closeRestrictedBrowserTab(browserPkg: String) {
        try {
            val root = rootInActiveWindow ?: return

            val closeButtons = root.findAccessibilityNodeInfosByViewId("$browserPkg:id/close_button")
            if (!closeButtons.isNullOrEmpty()) {
                for (btn in closeButtons) {
                    if (btn.isClickable) {
                        btn.performAction(AccessibilityNodeInfo.ACTION_CLICK)
                        Log.i(TAG, "Closed tab via close_button in $browserPkg")
                        return
                    }
                }
            }

            val tabSwitcherNodes = root.findAccessibilityNodeInfosByViewId("$browserPkg:id/tab_switcher_button")
            var clickedSwitcher = false
            if (!tabSwitcherNodes.isNullOrEmpty()) {
                for (node in tabSwitcherNodes) {
                    if (node.isClickable) {
                        node.performAction(AccessibilityNodeInfo.ACTION_CLICK)
                        clickedSwitcher = true
                        Log.i(TAG, "Clicked tab_switcher_button in $browserPkg")
                        break
                    }
                }
            }

            if (clickedSwitcher) {
                mainHandler.postDelayed({
                    closeTabInTabSwitcher(browserPkg)
                }, 120L)
            } else {
                performGlobalAction(GLOBAL_ACTION_BACK)
                Log.i(TAG, "Executed GLOBAL_ACTION_BACK to exit tab in $browserPkg")
            }

        } catch (e: Exception) {
            Log.e(TAG, "Error closing tab: ${e.message}")
        }
    }

    private fun closeTabInTabSwitcher(browserPkg: String) {
        try {
            val root = rootInActiveWindow ?: return
            val actionButtons = root.findAccessibilityNodeInfosByViewId("$browserPkg:id/action_button")
            if (!actionButtons.isNullOrEmpty()) {
                for (btn in actionButtons) {
                    val desc = btn.contentDescription?.toString()?.lowercase() ?: ""
                    if (desc.contains("close") || desc.contains("dismiss")) {
                        btn.performAction(AccessibilityNodeInfo.ACTION_CLICK)
                        Log.i(TAG, "Clicked close tab button in switcher: $desc")
                        return
                    }
                }
            }
            performGlobalAction(GLOBAL_ACTION_BACK)
        } catch (e: Exception) {
            Log.e(TAG, "Error closing tab in switcher: ${e.message}")
        }
    }

    private fun searchForUrlInTree(node: AccessibilityNodeInfo, maxDepth: Int): String? {
        if (maxDepth <= 0) return null
        val text = node.text?.toString()
        if (!text.isNullOrBlank()) {
            val lower = text.lowercase()
            for (domain in blockedDomains) {
                if (lower.contains(domain)) return text
            }
            if (lower.startsWith("http://") || lower.startsWith("https://") || lower.contains(".com") || lower.contains(".org") || lower.contains(".net")) {
                return text
            }
        }
        for (i in 0 until node.childCount) {
            val child = node.getChild(i) ?: continue
            val result = searchForUrlInTree(child, maxDepth - 1)
            if (result != null) return result
        }
        return null
    }

    private fun getAppLabel(packageName: String): String {
        return try {
            val pm = applicationContext.packageManager
            val appInfo = pm.getApplicationInfo(packageName, 0)
            pm.getApplicationLabel(appInfo).toString()
        } catch (_: Exception) {
            packageName
        }
    }

    private fun notifyFlutterOfBlock(pkgName: String, source: String, type: String) {
        mainHandler.post {
            try {
                val engine = MainActivity.flutterEngine
                if (engine != null) {
                    val channel = MethodChannel(engine.dartExecutor.binaryMessenger, "com.mrmsluna/notifications")
                    channel.invokeMethod("onDetachBlocked", mapOf(
                        "package" to pkgName,
                        "source" to source,
                        "type" to type
                    ))
                }
            } catch (e: Exception) {
                Log.e(TAG, "Error notifying Flutter: ${e.message}")
            }
        }
    }
}
