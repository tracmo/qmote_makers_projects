/*
 * Copyright (c) 2015
 * Qblinks Corporation.
 * All rights reserved.
 *
 * The information contained herein is confidential and proprietary to
 * Qblinks. Use of this information by anyone other than authorized employees
 * of Qblinks is granted only under a written non-disclosure agreement,
 * expressly prescribing the scope and manner of such use.
 *
 * @ contact TzuChing Chang
 */

package com.example.tzuchingchang.qmote_android_sample;

import android.app.Activity;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import java.math.BigInteger;
import java.nio.charset.StandardCharsets;
import java.util.Set;
import java.util.UUID;

public class MainActivity extends Activity {
    public static final String TAG = "MainActivity";

    // Member fields
    private Button mButtonConnect;
    private Button mButtonKeepAlive;
    private Button mButtonFwVersion;
    private TextView mTextView;
    private Handler mHandler;
    private Runnable mSetText;
    public String clickLabel = "Click" ;
    public String getVersion = null;

    // Initialize used device and bluetooth fields.
    private BluetoothAdapter mBluetoothAdapter=null;
    private BluetoothGatt mBluetoothGatt=null;
    private BluetoothDevice mDevice =null;
    private BluetoothGattCharacteristic mCharacterCallback = null;
    private BluetoothGattCharacteristic mCharacterButton = null;
    private BluetoothGattCharacteristic mCharacterCommand = null;

    // Set up unique UUID for Qmote.
    public static final UUID UUID_PRIMARY_SERVICE = UUID.fromString("E8008802-4143-5453-5162-6C696E6B73EC");
    public static final UUID UUID_COMMAND_CHARACTERISTIC = UUID.fromString("E8009A01-4143-5453-5162-6C696E6B73EC");
    public static final UUID UUID_CALLBACK_CHARACTERISTIC = UUID.fromString("E8009A02-4143-5453-5162-6C696E6B73EC");
    public static final UUID UUID_BUTTON_CHARACTERISTIC = UUID.fromString("E8009A03-4143-5453-5162-6C696E6B73EC");


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.v(TAG, "onCreate");
        setContentView(R.layout.main_activity);

        // Use this check to determine whether BLE is supported on the device.
        if (!getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
            mDialog("BLE is not supported");
            finish();
        }

        // Initialize a Bluetooth adapter.
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        // Ensures Bluetooth is enabled on the device.  If Bluetooth is not currently enabled,
        // fire an intent to display a dialog asking the user to grant permission to enable it.
        if (mBluetoothAdapter == null || !mBluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, 1);
        }

        // This button is used to get system bonded device and
        // connect on GATT if this Device determine as Qmote.
        mButtonConnect = (Button) findViewById(R.id.button_connect);
        mButtonConnect.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                if (mBluetoothAdapter != null) {
                    Set<BluetoothDevice> bondDevices = mBluetoothAdapter.getBondedDevices();
                    if (bondDevices.size() > 0) {
                        for (BluetoothDevice bondDevice : bondDevices) {
                            if(bondDevice.getName() != null){
                                if (bondDevice.getName().length() == 10 && bondDevice.getName().contains("Qmote ")) {
                                    mDevice = bondDevice;

                                    // Close the old GATT before you reconnect.
                                    if(mBluetoothGatt != null) mBluetoothGatt.close();

                                    mBluetoothGatt = mDevice.connectGatt(v.getContext(), false, mGattCallback);
                                    Log.v(TAG, "bonded Qmote" + bondDevice.getAddress());

                                    return;//Get the first bonded Qmote.
                                }
                            }
                        }
                    }
                } else {
                    Log.v(TAG, "There is no bonded device.");
                }
            }
        });

        //This button is used to sent keep-alive command to Qmote.
        //0x2c, 0x02: Keep alive command
        mButtonKeepAlive = (Button) findViewById(R.id.button_keepAlive);
        mButtonKeepAlive.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Log.i(TAG, "button keepAlive");
                if (mBluetoothGatt != null && mCharacterCommand != null) {
                    mCharacterCommand.setValue(new byte[]{0x2c, 0x02});
                    mBluetoothGatt.writeCharacteristic(mCharacterCommand);
                }
            }
        });

        //This button is used to get firmware version of this Qmote.
        //0x06, 0x07: Firmware version.
        mButtonFwVersion = (Button) findViewById(R.id.button_FwVersion);
        mButtonFwVersion.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Log.i(TAG, "button FwVersion");
                if (mBluetoothGatt != null && mCharacterCommand != null) {
                    mCharacterCommand.setValue(new byte[]{0x06, 0x07});
                    mBluetoothGatt.writeCharacteristic(mCharacterCommand);
                }
            }
        });

        //Display return values form callback on UI.
        mHandler = new Handler();
        mSetText =new Runnable() {
            @Override
            public void run() {
                mHandler.postDelayed(this, 100);
                mTextView = (TextView)findViewById(R.id.click_label);
                mTextView.setText(clickLabel);
                if(getVersion != null){
                    mDialog(getVersion);
                    getVersion =null;
                }
            }
        };

    }

    protected void onResume() {
        super.onResume();
        Log.v(TAG, "onResume");
    }

    protected void onPause() {
        super.onPause();
        Log.v(TAG, "onPause");
    }
    protected void onStop(){
        super.onStop();
        Log.v(TAG, "onStop");
    }

    /**
     * Implements callback methods used in this project for GATT and
     * determinate variables then return back to UI.
     *
     * For more information of Qmote GATT specification:
     * http://qblinks.com/devkit/developers/qmote-developers
     */
    public final BluetoothGattCallback mGattCallback = new BluetoothGattCallback() {

        //Connected state of this GATT changed.
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            Log.v(TAG, "onConnectionStateChange");

            final BluetoothDevice qmote = gatt.getDevice();
            if(qmote!=null){
                switch (newState) {
                    case BluetoothGatt.STATE_CONNECTED:
                        Log.v(TAG, "Connected: " + gatt.getDevice().getAddress());
                        clickLabel = "Connected";
                        mHandler.post(mSetText);
                        gatt.discoverServices(); //Star to discover service of Qmote.
                        break;
                    case BluetoothGatt.STATE_DISCONNECTED:
                        Log.v(TAG, "Disconnected");
                        clickLabel = "Disconnected";
                        mHandler.post(mSetText);
                        break;
                    default:
                        Log.v(TAG, "Connection state waiting.");
                        break;
                }
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            super.onServicesDiscovered(gatt, status);
            Log.v(TAG, "onServiceDiscovered");

            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.v(TAG, "ServiceDiscovery success ");

                //Service and characteristics of Qmote was found.
                BluetoothGattService service = gatt.getService(UUID_PRIMARY_SERVICE);
                if (service != null) {
                    Log.v(TAG, "Get service QPS.");
                    mCharacterCallback = service.getCharacteristic(UUID_CALLBACK_CHARACTERISTIC);
                    mCharacterButton = service.getCharacteristic(UUID_BUTTON_CHARACTERISTIC);
                    mCharacterCommand = service.getCharacteristic(UUID_COMMAND_CHARACTERISTIC);

                    if (mCharacterButton != null && mCharacterCallback != null) {
                        Log.v(TAG, "Initial setting of the Qmote");
                        //Set callback notify on for getting notification from Qmote.
                        gatt.setCharacteristicNotification(mCharacterCallback, true);
                        gatt.setCharacteristicNotification(mCharacterButton, true);
                        //Enable long-click.
                        mCharacterCommand.setValue(new byte[]{0x06, 0x01});
                        mBluetoothGatt.writeCharacteristic(mCharacterCommand);
                    }
                    else{
                        Log.e(TAG, "Characteristic not found.");
                    }
                }else{
                    Log.e(TAG, "Service QPS not found.");
                }
            }else{
                Log.e(TAG, "GATT operation failed.");
            }
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            super.onCharacteristicChanged(gatt, characteristic);

            if(characteristic.getValue()!=null){
                Log.v(TAG, "onCharacteristicChanged: " + new BigInteger(1, characteristic.getValue()).toString(16));
                byte[] data = characteristic.getValue();

                if (characteristic.getUuid().equals(UUID_BUTTON_CHARACTERISTIC)) {
                    Log.v(TAG, "From Button");

                    int intData = (int) data[0];

                    if (data.length == 1 && intData != 0) { //0x0 is button release code
                        Log.v(TAG, "Button clicked");

                        /* F-code list
                         * [Release] 0x00
                         * .         0x01
                         * ..        0x02
                         * ...       0x03
                         * ....      0x04
                         * -.        0x05
                         * .-.       0x06
                         * --.       0x07
                         * -..       0x08
                         * -...      0x09
                         * --..      0x0A
                         * ---.      0x0B
                         */

                        switch (intData) {
                            case 1:
                                clickLabel = "Click .(01)";
                                mHandler.post(mSetText);
                                break;
                            case 2:
                                clickLabel = "Click ..(02)";
                                mHandler.post(mSetText);
                                break;
                            case 3:
                                clickLabel = "Click ...(03)";
                                mHandler.post(mSetText);
                                break;
                            case 4:
                                clickLabel = "Click ....(04)";
                                mHandler.post(mSetText);
                                break;
                            case 5:
                                clickLabel = "Click -.(05)";
                                mHandler.post(mSetText);
                                break;
                            case 6:
                                clickLabel = "Click .-.(06)";
                                mHandler.post(mSetText);
                                break;
                            case 7:
                                clickLabel = "Click --.(07)";
                                mHandler.post(mSetText);
                                break;
                            case 8:
                                clickLabel = "Click -..(08)";
                                mHandler.post(mSetText);
                                break;
                            case 9:
                                clickLabel = "Click -...(09)";
                                mHandler.post(mSetText);
                                break;
                            case 10:
                                clickLabel = "Click --..(0A)";
                                mHandler.post(mSetText);
                                break;
                            case 11:
                                clickLabel = "Click ---.(0B)";
                                mHandler.post(mSetText);
                                break;
                            default:
                                clickLabel = "Button event not found.";
                                mHandler.post(mSetText);
                                Log.e(TAG, "Button event not found.");
                                break;
                        }
                    }
                } else if (characteristic.getUuid().equals(UUID_CALLBACK_CHARACTERISTIC)) {
                    Log.v(TAG, "From callback");

                    if (data.length > 2 && data[0] == 0x06 && data[1] == 0x07) { //Get callback from Firmware version request.
                        Log.i(TAG, "Get Firmware");

                        byte[] versionByte= new byte[data.length-2];
                        System.arraycopy(data, 2, versionByte, 0, data.length-2);// Takeoff the the return header.
                        Log.i(TAG, new String(versionByte, StandardCharsets.UTF_8));
                        getVersion = "Version  " + new String(versionByte, StandardCharsets.UTF_8);
                        mHandler.post(mSetText);
                    }
                }

            }
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            super.onCharacteristicWrite(gatt, characteristic, status);
            //status = 0 means write success.

            if (characteristic.getUuid().equals(UUID_BUTTON_CHARACTERISTIC)) {
                Log.i(TAG, "buttonWrite: status=" + status);
            }
            else if (characteristic.getUuid().equals(UUID_COMMAND_CHARACTERISTIC)) {
                Log.i(TAG, "CommandWrite: status=" + status);
            }
        }

    };

    /** Show Dialog when method mDialog used. */
    public void mDialog(String mMessage){
        AlertDialog.Builder builder =new AlertDialog.Builder(MainActivity.this);
        builder.setMessage(mMessage)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                    }
                }).create().show();
    }


}