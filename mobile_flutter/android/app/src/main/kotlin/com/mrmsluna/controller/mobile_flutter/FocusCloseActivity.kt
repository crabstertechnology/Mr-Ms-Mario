package com.mrmsluna.controller.mobile_flutter

import android.app.Activity
import android.content.Intent
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log

class FocusCloseActivity : Activity() {

    private val targetPkg: String? get() = intent?.getStringExtra("close_pkg")
    private var homeSent = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Log.i("FocusGuard", "FocusCloseActivity: onCreate for $targetPkg")

        // Kill the target app background processes immediately
        killTarget()

        // If window focus doesn't arrive within 500ms, go home anyway
        Handler(Looper.getMainLooper()).postDelayed({
            if (!homeSent) {
                Log.i("FocusGuard", "FocusCloseActivity: timeout fallback — going home now")
                goHome()
            }
        }, 500)
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus && !homeSent) {
            Log.i("FocusGuard", "FocusCloseActivity: got window focus — going home for $targetPkg")
            goHome()
        }
    }

    override fun onResume() {
        super.onResume()
        // Extra safety — if onResume fires and we haven't gone home yet
        if (!homeSent) {
            Handler(Looper.getMainLooper()).postDelayed({
                if (!homeSent) {
                    Log.i("FocusGuard", "FocusCloseActivity: onResume fallback — going home")
                    goHome()
                }
            }, 300)
        }
    }

    private fun goHome() {
        if (homeSent) return
        homeSent = true
        try {
            val homeIntent = Intent(Intent.ACTION_MAIN).apply {
                addCategory(Intent.CATEGORY_HOME)
                flags = Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK
            }
            startActivity(homeIntent)
            Log.i("FocusGuard", "FocusCloseActivity: home intent sent for $targetPkg")
        } catch (e: Exception) {
            Log.e("FocusGuard", "FocusCloseActivity: error sending home intent: ${e.message}")
        }
        // Kill again after going home to ensure the process is cleared
        Handler(Looper.getMainLooper()).postDelayed({
            killTarget()
            finish()
        }, 200)
    }

    private fun killTarget() {
        val pkg = targetPkg ?: return
        try {
            val am = getSystemService(ACTIVITY_SERVICE) as? android.app.ActivityManager
            am?.killBackgroundProcesses(pkg)
        } catch (e: Exception) {
            Log.e("FocusGuard", "FocusCloseActivity: error killing $pkg: ${e.message}")
        }
    }
}
