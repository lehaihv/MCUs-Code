package com.example.ble_esp32

import android.Manifest
import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.widget.Button
import android.widget.ImageView
import android.widget.TextView
import android.widget.Toast
import androidx.activity.result.ActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.annotation.RequiresApi
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat


class MainActivity : AppCompatActivity() {

    private var btPermission = false

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    @SuppressLint("SetTextI18n", "MissingPermission")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Register for broadcasts when a device is discovered.
        val filter = IntentFilter(BluetoothDevice.ACTION_FOUND)
        registerReceiver(receiver, filter)

        //bluetooth adapter
        val bluetoothManager: BluetoothManager = getSystemService(BluetoothManager::class.java)
        val bluetoothAdapter: BluetoothAdapter? = bluetoothManager.adapter
        //check if bluetooth is available or not
        if (bluetoothAdapter == null) {
            findViewById<TextView>(R.id.bluetoothStatusTv).setText("Bluetooth is not available")
                .toString() //Set text of TextView
        } else {
            findViewById<TextView>(R.id.bluetoothStatusTv).setText("Bluetooth is available")
                .toString() //Set text of TextView
        }

        //set image according to bluetooth status (on/off)
        if (bluetoothAdapter != null) {
            if (bluetoothAdapter.isEnabled) {
                findViewById<ImageView>(R.id.bluetoothIv).setImageResource(R.drawable.ic_bluetooth_on) //Set BLE image on
            } else {
                findViewById<ImageView>(R.id.bluetoothIv).setImageResource(R.drawable.ic_bluetooth_off) //Set BLE image off
            }
        }

        //turn on bluetooth
        findViewById<Button>(R.id.turnOnBtn).setOnClickListener {
            if (bluetoothAdapter?.isEnabled == false) {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                    blueToothPermissionLauncher.launch(Manifest.permission.BLUETOOTH_CONNECT) //SCAN: Turn on; CONNECT: Turn on and find devices
                }
            }
            else {
                Toast.makeText(this, "Already on", Toast.LENGTH_LONG).show()
                findViewById<ImageView>(R.id.bluetoothIv).setImageResource(R.drawable.ic_bluetooth_on)
            }
        }

        //turn off bluetooth
        findViewById<Button>(R.id.turnOffBtn).setOnClickListener {
            if (bluetoothAdapter?.isEnabled == false) {
                Toast.makeText(this, "Already off", Toast.LENGTH_LONG).show()
                findViewById<ImageView>(R.id.bluetoothIv).setImageResource(R.drawable.ic_bluetooth_off) //Set BLE image off
            }
            else {
                //bluetoothAdapter.disable()
                findViewById<ImageView>(R.id.bluetoothIv).setImageResource(R.drawable.ic_bluetooth_off) //Set BLE image off
            }

        }

        //discoverable bluetooth
        findViewById<Button>(R.id.discoverableBtn).setOnClickListener {
            if (bluetoothAdapter?.isEnabled == true) {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                    //blueToothPermissionLauncher.launch(Manifest.permission.BLUETOOTH_CONNECT) //SCAN: Turn on; CONNECT: Turn on and find devices
                    Toast.makeText(this, "Looking for nearby devices", Toast.LENGTH_LONG).show()

                }
            }
            else {
                Toast.makeText(this, "Turn on Bluetooth first", Toast.LENGTH_LONG).show()
            }

        }

        //get list of paired devices
        findViewById<Button>(R.id.pairedBtn).setOnClickListener {
            if (bluetoothAdapter?.isEnabled == true) {
                findViewById<TextView>(R.id.pairedTv).setText("Paired Devices").toString() //Set text of TextView
                //get list of devices
                var bondedDevices = bluetoothAdapter.bondedDevices
                for (device in bondedDevices) {
                    val deviceName = device.name
                    //val deviceAddress = device
                    findViewById<TextView>(R.id.pairedTv).append("\nDevice: $deviceName , $device")
                }
            } else{
                Toast.makeText(this, "Turn on Bluetooth first", Toast.LENGTH_LONG).show()
            }
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
            btPermission = true
            if(bluetoothAdapter?.isEnabled == false){
                val enableBtIntent = Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE)
                activityResultLauncher.launch(enableBtIntent)
            }
        } else {
            // Permission is denied
            //Toast.makeText(this, "User not allow", Toast.LENGTH_LONG).show()
            btPermission = false
        }

    }
    private val activityResultLauncher = registerForActivityResult(
            ActivityResultContracts.StartActivityForResult()) { result: ActivityResult ->
            // Handle Permission granted/rejected
            if (result.resultCode == RESULT_OK) { //if user allow to turn on bluetooth
                findViewById<ImageView>(R.id.bluetoothIv).setImageResource(R.drawable.ic_bluetooth_on) //Set BLE image on
                Toast.makeText(this, "User allow to turn on Bluetooth", Toast.LENGTH_LONG).show()
                }

            else {
                // Permission is denied
                Toast.makeText(this, "User not allow to turn on Bluetooth", Toast.LENGTH_LONG).show()
            }
        }
    // Create a BroadcastReceiver for ACTION_FOUND.
    private val receiver = object : BroadcastReceiver() {

        @SuppressLint("MissingPermission", "SetTextI18n")
        override fun onReceive(context: Context, intent: Intent) {
            when(intent.action) {
                BluetoothDevice.ACTION_FOUND -> {
                    // Discovery has found a device. Get the BluetoothDevice
                    // object and its info from the Intent.
                    val device: BluetoothDevice = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE)!!
                    val deviceName = device.name
                    val deviceHardwareAddress = device.address // MAC address
                    findViewById<TextView>(R.id.pairedTv).setText("Paired Devices").toString()
                    findViewById<TextView>(R.id.pairedTv).append("\nDevice: $deviceName , $deviceHardwareAddress , $device")

                }
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()


        // Don't forget to unregister the ACTION_FOUND receiver.
        unregisterReceiver(receiver)
    }

}