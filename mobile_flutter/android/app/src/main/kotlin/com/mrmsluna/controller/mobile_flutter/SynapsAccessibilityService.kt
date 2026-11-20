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
        var autoHomeSeconds: Int = 5

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
    private var lastCheckedPackage: String? = null
    private var isCurrentlyBlocking = false

    override fun onServiceConnected() {
        super.onServiceConnected()
        instance = this
        Log.i(TAG, "SynapsAccessibilityService CONNECTED")

        // Load persisted preferences
        try {
            val prefs = getSharedPreferences("SynapsDetachPrefs", Context.MODE_PRIVATE)
            detachEnabled = prefs.getBoolean("detach_enabled", true)
            customMessage = prefs.getString("custom_message", "Get back to what matters.") ?: "Get back to what matters."
            autoHomeSeconds = prefs.getInt("auto_home_seconds", 5)

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

        // 1. Check if the foreground application is directly in the blocked apps list
        if (blockedApps.contains(pkgName.lowercase())) {
            handleBlockedApp(pkgName)
            return
        }

        // 2. Check if a web browser is in the foreground
        if (browserPackages.contains(pkgName)) {
            handleBrowserInspection(pkgName, event)
            return
        }

        // 3. If leaving a blocked app / browser to home screen or another safe app, dismiss overlay
        if (isCurrentlyBlocking && pkgName != packageName && !browserPackages.contains(pkgName) && !blockedApps.contains(pkgName.lowercase())) {
            Log.i(TAG, "User exited blocked context to: $pkgName. Dismissing overlay.")
            isCurrentlyBlocking = false
            DetachOverlayManager.dismiss()
        }
    }

    private fun handleBlockedApp(pkgName: String) {
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
            // First check explicit URL bar IDs
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

            // Fallback: search node hierarchy for address text
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
                    val browserName = getAppLabel(browserPkg)
                    val sourceDesc = "$matchedDomain in $browserName"
                    Log.i(TAG, "RESTRICTED WEBSITE DETECTED: $sourceDesc (Raw: $currentUrl)")

                    isCurrentlyBlocking = true
                    DetachOverlayManager.show(
                        context = this,
                        sourceInfo = sourceDesc,
                        customMessage = customMessage,
                        autoHomeSeconds = autoHomeSeconds
                    )

                    notifyFlutterOfBlock(browserPkg, sourceDesc, "website")
                } else {
                    // Safe website in browser — if overlay was previously shown for a blocked URL in this browser, dismiss it
                    if (isCurrentlyBlocking) {
                        Log.i(TAG, "Safe URL detected in $browserPkg ($currentUrl). Dismissing overlay.")
                        isCurrentlyBlocking = false
                        DetachOverlayManager.dismiss()
                    }
                }
            }
        } catch (e: Exception) {
            Log.e(TAG, "Error inspecting browser window: ${e.message}")
        }
    }

    private fun searchForUrlInTree(node: AccessibilityNodeInfo?, maxDepth: Int): String? {
        if (node == null || maxDepth <= 0) return null

        val text = node.text?.toString()
        if (!text.isNullOrBlank()) {
            val lower = text.lowercase()
            for (domain in blockedDomains) {
                if (lower.contains(domain)) {
                    return domain
                }
            }
        }

        val desc = node.contentDescription?.toString()
        if (!desc.isNullOrBlank()) {
            val lower = desc.lowercase()
            for (domain in blockedDomains) {
                if (lower.contains(domain)) {
                    return domain
                }
            }
        }

        val count = node.childCount
        for (i in 0 until count) {
            val child = node.getChild(i)
            val found = searchForUrlInTree(child, maxDepth - 1)
            if (found != null) return found
        }
        return null
    }

    private fun notifyFlutterOfBlock(pkg: String, sourceDesc: String, type: String) {
        val engine = MainActivity.flutterEngine ?: return
        mainHandler.post {
            try {
                val channel = MethodChannel(engine.dartExecutor.binaryMessenger, "com.mrmsluna/notifications")
                channel.invokeMethod("onDetachBlocked", mapOf(
                    "package" to pkg,
                    "source" to sourceDesc,
                    "type" to type
                ))
            } catch (e: Exception) {
                Log.e(TAG, "Error notifying Flutter: ${e.message}")
            }
        }
    }

    private fun getAppLabel(pkg: String): String {
        return try {
            val pm = packageManager
            val info = pm.getApplicationInfo(pkg, 0)
            pm.getApplicationLabel(info).toString()
        } catch (e: Exception) {
            when {
                pkg.contains("chrome") -> "Chrome"
                pkg.contains("firefox") -> "Firefox"
                pkg.contains("sbrowser") -> "Samsung Internet"
                pkg.contains("emmx") -> "Edge"
                pkg.contains("instagram") -> "Instagram"
                pkg.contains("youtube") -> "YouTube"
                else -> pkg
            }
        }
    }
}
