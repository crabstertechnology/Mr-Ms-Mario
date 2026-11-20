package com.mrmsluna.controller.mobile_flutter

import android.app.Activity
import android.content.Context
import android.content.Intent
import android.graphics.Color
import android.graphics.Typeface
import android.graphics.drawable.GradientDrawable
import android.os.Bundle
import android.os.CountDownTimer
import android.util.TypedValue
import android.view.Gravity
import android.view.ViewGroup
import android.widget.Button
import android.widget.FrameLayout
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.TextView

class DetachBlockActivity : Activity() {

    private var countdownTimer: CountDownTimer? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val sourceInfo = intent?.getStringExtra("source_info") ?: "Restricted Content"
        val customMessage = intent?.getStringExtra("custom_message") ?: "Get back to what matters."

        val root = FrameLayout(this).apply {
            setBackgroundColor(Color.BLACK)
            layoutParams = ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
            )
        }

        val contentLayout = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER_HORIZONTAL or Gravity.CENTER_VERTICAL
            setPadding(dpToPx(28), dpToPx(48), dpToPx(28), dpToPx(48))
        }

        // 1. Icon
        val icon = ImageView(this).apply {
            setImageResource(android.R.drawable.ic_lock_lock)
            setColorFilter(Color.parseColor("#FF6D00"))
            val size = dpToPx(56)
            layoutParams = LinearLayout.LayoutParams(size, size).apply {
                bottomMargin = dpToPx(20)
            }
        }
        contentLayout.addView(icon)

        // 2. Badge
        val badge = TextView(this).apply {
            text = "SYNAPS DETACH • HABIT INTERRUPT"
            setTextColor(Color.parseColor("#FF9100"))
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 10.5f)
            typeface = Typeface.create("sans-serif-medium", Typeface.BOLD)
            gravity = Gravity.CENTER
            setPadding(dpToPx(12), dpToPx(4), dpToPx(12), dpToPx(4))
            background = GradientDrawable().apply {
                setColor(Color.parseColor("#261A0E"))
                cornerRadius = dpToPx(12).toFloat()
                setStroke(dpToPx(1), Color.parseColor("#4DFF9100"))
            }
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply {
                bottomMargin = dpToPx(24)
            }
        }
        contentLayout.addView(badge)

        // 3. Primary Message
        val title = TextView(this).apply {
            text = customMessage
            setTextColor(Color.WHITE)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 26f)
            typeface = Typeface.create("sans-serif", Typeface.BOLD)
            gravity = Gravity.CENTER
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply {
                bottomMargin = dpToPx(14)
            }
        }
        contentLayout.addView(title)

        // 4. Source detail
        val detail = TextView(this).apply {
            text = "Access to $sourceInfo is restricted."
            setTextColor(Color.parseColor("#B0BEC5"))
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 13.5f)
            gravity = Gravity.CENTER
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply {
                bottomMargin = dpToPx(8)
            }
        }
        contentLayout.addView(detail)

        // 5. Mindful Reminder
        val reminder = TextView(this).apply {
            text = "Notice the reflex. Reclaim your focus."
            setTextColor(Color.parseColor("#78909C"))
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 12f)
            gravity = Gravity.CENTER
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply {
                bottomMargin = dpToPx(36)
            }
        }
        contentLayout.addView(reminder)

        // 6. Action Button: "RETURN TO HOME SCREEN"
        val btnHome = Button(this).apply {
            text = "RETURN TO HOME SCREEN"
            setTextColor(Color.WHITE)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 13f)
            typeface = Typeface.create("sans-serif-medium", Typeface.BOLD)
            background = GradientDrawable().apply {
                setColor(Color.parseColor("#FF5722"))
                cornerRadius = dpToPx(24).toFloat()
            }
            setPadding(dpToPx(28), dpToPx(14), dpToPx(28), dpToPx(14))
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply {
                bottomMargin = dpToPx(16)
            }
            setOnClickListener {
                goHome()
            }
        }
        contentLayout.addView(btnHome)

        // 7. Auto-return countdown
        val countdownView = TextView(this).apply {
            text = "Auto-returning home in 4s..."
            setTextColor(Color.parseColor("#546E7A"))
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 11f)
            gravity = Gravity.CENTER
        }
        contentLayout.addView(countdownView)

        countdownTimer = object : CountDownTimer(4000L, 1000L) {
            override fun onTick(millisUntilFinished: Long) {
                val sec = (millisUntilFinished / 1000).toInt() + 1
                countdownView.text = "Auto-returning home in ${sec}s..."
            }
            override fun onFinish() {
                goHome()
            }
        }.start()

        val frameParams = FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.MATCH_PARENT,
            FrameLayout.LayoutParams.WRAP_CONTENT
        ).apply {
            gravity = Gravity.CENTER
        }
        root.addView(contentLayout, frameParams)

        setContentView(root)
    }

    override fun onBackPressed() {
        goHome()
    }

    override fun onDestroy() {
        countdownTimer?.cancel()
        super.onDestroy()
    }

    private fun goHome() {
        countdownTimer?.cancel()
        try {
            val homeIntent = Intent(Intent.ACTION_MAIN).apply {
                addCategory(Intent.CATEGORY_HOME)
                flags = Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK
            }
            startActivity(homeIntent)
        } catch (_: Exception) {}
        finish()
    }

    private fun dpToPx(dp: Int): Int {
        return (dp * resources.displayMetrics.density).toInt()
    }
}
