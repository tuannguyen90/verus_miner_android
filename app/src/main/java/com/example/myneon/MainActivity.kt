package com.example.myneon

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import com.example.myneon.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Example of a call to a native method
        binding.sampleText.text = stringFromJNI()

        updateStatusText(findViewById(R.id.sample_text))

        // startMiningbutton
        val btnStarMining = findViewById<Button>(R.id.btnStartMining)
        btnStarMining.setOnClickListener {
            startMiningFromJNI();
        }

        // updateDeviceId
        val btnUpdateDeviceId = findViewById<Button>(R.id.saveDeviceId);
        btnUpdateDeviceId.setOnClickListener {
            val editText = findViewById<EditText>(R.id.deviceId)
            val text = editText.text.toString()
            var rigId = text.toIntOrNull()
            if (rigId != null) {
                updateDeviceIdFromJNI(rigId)
            }
        }
    }

    /**
     * A native method that is implemented by the 'myneon' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String
    external fun startMiningFromJNI();
    external fun updateDeviceIdFromJNI(id: Int);

    fun updateStatusText(textView: TextView) {
        val handler = Handler(Looper.getMainLooper())
        val runnable = object  : Runnable {
            override fun run() {
                val status = stringFromJNI();
                textView.text = status
                handler.postDelayed(this, 1000)
            }
        }
        handler.post(runnable)
    }

    companion object {
        // Used to load the 'myneon' library on application startup.
        init {
            System.loadLibrary("myneon")
        }
    }
}