package com.mrmsluna.controller.mobile_flutter

import android.content.Context
import android.content.Intent
import android.graphics.Color
import android.graphics.PixelFormat
import android.graphics.Typeface
import android.graphics.drawable.GradientDrawable
import android.os.Build
import android.os.CountDownTimer
import android.os.Handler
import android.os.Looper
import android.provider.Settings
import android.util.Log
import android.util.TypedValue
import android.view.Gravity
import android.view.View
import android.view.ViewGroup
import android.view.WindowManager
import android.widget.Button
import android.widget.FrameLayout
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.TextView

object DetachOverlayManager {
    private const val TAG = "DetachOverlay"

    private var windowManager: WindowManager? = null
    private var overlayView: View? = null
    private var isOverlayVisible = false
    private var countdownTimer: CountDownTimer? = null
    private val mainHandler = Handler(Looper.getMainLooper())

    @Volatile
    var lastBlockedSource: String = ""

    fun isShowing(): Boolean = isOverlayVisible

    /**
     * Display the full-screen interruption overlay.
     * Prevents any touch interaction with the underlying application.
     */
    fun show(
        context: Context,
        sourceInfo: String,
        customMessage: String? = null,
        autoHomeSeconds: Int = 0
    ) {
        mainHandler.post {
            lastBlockedSource = sourceInfo

            // Check if overlay permission is granted
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M && !Settings.canDrawOverlays(context)) {
                Log.w(TAG, "Overlay permission not granted! Falling back to DetachBlockActivity.")
                launchBlockActivity(context, sourceInfo, customMessage)
                return@post
            }

            if (isOverlayVisible && overlayView != null) {
                // Update text if already visible
                updateOverlayContent(sourceInfo, customMessage)
                return@post
            }

            try {
                val wm = context.getSystemService(Context.WINDOW_SERVICE) as? WindowManager
                if (wm == null) {
                    Log.e(TAG, "WindowManager is null, fallback to Activity")
                    launchBlockActivity(context, sourceInfo, customMessage)
                    return@post
                }
                windowManager = wm

                val layoutType = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                    WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY
                } else {
                    @Suppress("DEPRECATION")
                    WindowManager.LayoutParams.TYPE_PHONE
                }

                val params = WindowManager.LayoutParams(
                    WindowManager.LayoutParams.MATCH_PARENT,
                    WindowManager.LayoutParams.MATCH_PARENT,
                    layoutType,
                    WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL or
                    WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN or
                    WindowManager.LayoutParams.FLAG_FULLSCREEN or
                    WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS or
                    WindowManager.LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH,
                    PixelFormat.TRANSLUCENT
                ).apply {
                    gravity = Gravity.FILL
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                        layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES
                    }
                }

                val view = createOverlayView(context, sourceInfo, customMessage, autoHomeSeconds)
                // Consume all touch events completely
                view.setOnTouchListener { _, _ -> true }

                wm.addView(view, params)
                overlayView = view
                isOverlayVisible = true
                Log.i(TAG, "Full-screen interruption overlay successfully displayed for: $sourceInfo")

            } catch (e: Exception) {
                Log.e(TAG, "Failed to display overlay: ${e.message}, falling back to Activity", e)
                launchBlockActivity(context, sourceInfo, customMessage)
            }
        }
    }

    /**
     * Dismiss the overlay.
     */
    fun dismiss() {
        mainHandler.post {
            try {
                countdownTimer?.cancel()
                countdownTimer = null

                if (isOverlayVisible && overlayView != null && windowManager != null) {
                    windowManager?.removeViewImmediate(overlayView)
                    Log.i(TAG, "Overlay dismissed")
                }
            } catch (e: Exception) {
                Log.e(TAG, "Error dismissing overlay: ${e.message}")
            } finally {
                overlayView = null
                isOverlayVisible = false
            }
        }
    }

    private fun launchBlockActivity(
        context: Context,
        sourceInfo: String,
        customMessage: String?
    ) {
        try {
            val intent = Intent(context, DetachBlockActivity::class.java).apply {
                putExtra("source_info", sourceInfo)
                putExtra("custom_message", customMessage ?: "Get back to what matters.")
                addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TOP or Intent.FLAG_ACTIVITY_SINGLE_TOP)
            }
            context.startActivity(intent)
        } catch (e: Exception) {
            Log.e(TAG, "Failed to launch DetachBlockActivity: ${e.message}")
        }
    }

    private fun updateOverlayContent(sourceInfo: String, customMessage: String?) {
        val root = overlayView as? ViewGroup ?: return
        val detailView = root.findViewWithTag<TextView>("detach_detail")
        val messageView = root.findViewWithTag<TextView>("detach_message")

        detailView?.text = "Access to $sourceInfo is restricted."
        if (customMessage != null) {
            messageView?.text = customMessage
        }
    }

    private fun createOverlayView(
        context: Context,
        sourceInfo: String,
        customMessage: String?,
        autoHomeSeconds: Int
    ): View {
        val root = FrameLayout(context).apply {
            setBackgroundColor(Color.BLACK) // Pure Jet-Black Background
            isClickable = true
            isFocusable = true
        }

        val contentLayout = LinearLayout(context).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER_HORIZONTAL or Gravity.CENTER_VERTICAL
            setPadding(dpToPx(context, 28), dpToPx(context, 48), dpToPx(context, 28), dpToPx(context, 48))
        }

        // 1. Shield / Lock Icon
        val icon = ImageView(context).apply {
            setImageResource(android.R.drawable.ic_lock_lock)
            setColorFilter(Color.parseColor("#FF6D00")) // Deep Orange Accent
            val size = dpToPx(context, 56)
            layoutParams = LinearLayout.LayoutParams(size, size).apply {
                bottomMargin = dpToPx(context, 20)
            }
        }
        contentLayout.addView(icon)

        // 2. Pill Badge
        val badge = TextView(context).apply {
            text = "SYNAPS DETACH • HABIT INTERRUPT"
            setTextColor(Color.parseColor("#FF9100"))
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 10.5f)
            typeface = Typeface.create("sans-serif-medium", Typeface.BOLD)
            gravity = Gravity.CENTER
            setPadding(dpToPx(context, 12), dpToPx(context, 4), dpToPx(context, 12), dpToPx(context, 4))
            background = GradientDrawable().apply {
                setColor(Color.parseColor("#261A0E"))
                cornerRadius = dpToPx(context, 12).toFloat()
                setStroke(dpToPx(context, 1), Color.parseColor("#4DFF9100"))
            }
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply {
                bottomMargin = dpToPx(context, 24)
            }
        }
        contentLayout.addView(badge)

        // 3. Primary Motivational Title: "Get back to what matters."
        val title = TextView(context).apply {
            tag = "detach_message"
            text = customMessage ?: "Get back to what matters."
            setTextColor(Color.WHITE)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 26f)
            typeface = Typeface.create("sans-serif", Typeface.BOLD)
            gravity = Gravity.CENTER
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply {
                bottomMargin = dpToPx(context, 14)
            }
        }
        contentLayout.addView(title)

        // 4. Source detail
        val detail = TextView(context).apply {
            tag = "detach_detail"
            text = "Access to $sourceInfo is restricted."
            setTextColor(Color.parseColor("#B0BEC5"))
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 13.5f)
            gravity = Gravity.CENTER
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply {
                bottomMargin = dpToPx(context, 8)
            }
        }
        contentLayout.addView(detail)

        // 5. Mindful Reminder
        val reminder = TextView(context).apply {
            text = "Notice the reflex. Reclaim your focus."
            setTextColor(Color.parseColor("#78909C"))
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 12f)
            gravity = Gravity.CENTER
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply {
                bottomMargin = dpToPx(context, 36)
            }
        }
        contentLayout.addView(reminder)

        // 6. Action Button: "RETURN TO HOME SCREEN"
        val btnHome = Button(context).apply {
            text = "RETURN TO HOME SCREEN"
            setTextColor(Color.WHITE)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 13f)
            typeface = Typeface.create("sans-serif-medium", Typeface.BOLD)
            background = GradientDrawable().apply {
                setColor(Color.parseColor("#FF5722")) // Deep Orange Accent
                cornerRadius = dpToPx(context, 24).toFloat()
            }
            setPadding(dpToPx(context, 28), dpToPx(context, 14), dpToPx(context, 28), dpToPx(context, 14))
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply {
                bottomMargin = dpToPx(context, 16)
            }
            setOnClickListener {
                goHome(context)
            }
        }
        contentLayout.addView(btnHome)

        // 7. Auto-return countdown (e.g. 4 seconds)
        val effectiveAutoHome = if (autoHomeSeconds > 0) autoHomeSeconds else 4
        val countdownView = TextView(context).apply {
            text = "Auto-returning home in ${effectiveAutoHome}s..."
            setTextColor(Color.parseColor("#546E7A"))
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 11f)
            gravity = Gravity.CENTER
        }
        contentLayout.addView(countdownView)

        countdownTimer?.cancel()
        countdownTimer = object : CountDownTimer((effectiveAutoHome * 1000).toLong(), 1000L) {
            override fun onTick(millisUntilFinished: Long) {
                val sec = (millisUntilFinished / 1000).toInt() + 1
                countdownView.text = "Auto-returning home in ${sec}s..."
            }

            override fun onFinish() {
                goHome(context)
            }
        }.start()

        val frameParams = FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.MATCH_PARENT,
            FrameLayout.LayoutParams.WRAP_CONTENT
        ).apply {
            gravity = Gravity.CENTER
        }
        root.addView(contentLayout, frameParams)

        return root
    }

    /**
     * Dismisses the interruption overlay and navigates the device directly to the Home screen.
     * Does NOT open any new tabs or external URLs.
     */
    fun goHome(context: Context) {
        try {
            // Dismiss overlay view first
            dismiss()

            // Dispatch standard Android Home Intent
            val homeIntent = Intent(Intent.ACTION_MAIN).apply {
                addCategory(Intent.CATEGORY_HOME)
                flags = Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK
            }
            context.startActivity(homeIntent)
            Log.i(TAG, "Sent HOME intent to return to home screen")

        } catch (e: Exception) {
            Log.e(TAG, "Error returning home: ${e.message}")
        }
    }

    private fun dpToPx(context: Context, dp: Int): Int {
        return (dp * context.resources.displayMetrics.density).toInt()
    }
}
