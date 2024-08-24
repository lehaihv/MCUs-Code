package com.example.myapplication

import android.Manifest
import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothManager
import android.bluetooth.le.ScanFilter
import android.content.Intent
import android.os.Build
import android.os.Bundle
import android.os.ParcelUuid
import android.view.View
import android.widget.Button
import android.widget.EditText
import android.widget.Toast
import androidx.activity.result.ActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.annotation.RequiresApi
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat


class MainActivity : AppCompatActivity() {


    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    @SuppressLint("SetTextI18n")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        val bluetoothManager: BluetoothManager = getSystemService(BluetoothManager::class.java)
        val bluetoothAdapter: BluetoothAdapter? = bluetoothManager.adapter
        if (bluetoothAdapter == null) {
            Toast.makeText(this@MainActivity, "Device doesn't support Bluetooth", Toast.LENGTH_SHORT).show()
        }
        // set on-click listener of button3
        findViewById<Button>(R.id.button3).setOnClickListener {
            Toast.makeText(this@MainActivity, "You clicked me.", Toast.LENGTH_SHORT).show()
            findViewById<EditText>(R.id.editTextTextMultiLine).setText("List of BLE devices nearby")
            // Check and enable BLE
            if (bluetoothAdapter?.isEnabled == false) {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                    blueToothPermissionLauncher.launch(Manifest.permission.BLUETOOTH_CONNECT) //SCAN: Turn on; CONNECT: Turn on and find devices
                }
            }
            else {
                Toast.makeText(this, "Already on", Toast.LENGTH_LONG).show()
                // findViewById<ImageView>(R.id.bluetoothIv).setImageResource(R.drawable.ic_bluetooth_on)
            }
            startScanning()

        }
    }

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    private val blueToothPermissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestPermission() ){ isGranted ->
        // Handle Permission granted/rejected
        if (isGranted) {
            // Permission is granted
            val bluetoothManager: BluetoothManager = getSystemService(BluetoothManager::class.java)
            val bluetoothAdapter: BluetoothAdapter? = bluetoothManager.adapter
            //Toast.makeText(this, "User allow", Toast.LENGTH_LONG).show()
            // var btPermission = true
            if(bluetoothAdapter?.isEnabled == false){
                val enableBtIntent = Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE)
                activityResultLauncher.launch(enableBtIntent)
            }
        } else {
            // Permission is denied
            // Toast.makeText(this, "User not allow", Toast.LENGTH_LONG).show()
            // var btPermission = false
        }

    }
    private val activityResultLauncher = registerForActivityResult(
        ActivityResultContracts.StartActivityForResult()) { result: ActivityResult ->
        // Handle Permission granted/rejected
        if (result.resultCode == RESULT_OK) { //if user allow to turn on bluetooth
            // findViewById<ImageView>(R.id.bluetoothIv).setImageResource(R.drawable.ic_bluetooth_on) //Set BLE image on
            Toast.makeText(this, "User allow to turn on Bluetooth", Toast.LENGTH_LONG).show()
        }

        else {
            // Permission is denied
            Toast.makeText(this, "User not allow to turn on Bluetooth", Toast.LENGTH_LONG).show()
        }
    }

    val filter = ScanFilter.Builder().setServiceUuid(
        ParcelUuid.fromString(ENVIRONMENTAL_SERVICE_UUID.toString())
    ).build()

    private fun startScanning() {
        Toast.makeText(this, "Start scanning", Toast.LENGTH_LONG).show()
        

    }


}