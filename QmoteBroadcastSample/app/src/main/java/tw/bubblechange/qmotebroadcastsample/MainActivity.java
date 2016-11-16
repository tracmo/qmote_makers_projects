/*
 * Copyright (c) 2016
 * Qblinks Incorporated ("Qblinks").
 * All rights reserved.
 *
 * The information contained herein is confidential and proprietary to
 * Qblinks. Use of this information by anyone other than authorized employees
 * of Qblinks is granted only under a written non-disclosure agreement,
 * expressly prescribing the scope and manner of such use.
 *
 * @author Tzuching Chang
 *
 */
package tw.bubblechange.qmotebroadcastsample;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ListView;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;


public class MainActivity extends Activity implements View.OnClickListener{

    private Button btnGetQmote;
    private ListView lvQmote;

    private BluetoothAdapter mBluetoothAdapter;
    private List<BluetoothDevice> mQmoteList = new ArrayList<>();
    private QmoteAdapter mQmoteAdapter;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Use this check to determine whether BLE is supported on the device.
        if (!getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
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

        btnGetQmote = (Button) findViewById(R.id.btn_get_qmote);
        lvQmote = (ListView) findViewById(R.id.lv_qmote);

        btnGetQmote.setOnClickListener(this);
        mQmoteAdapter = new QmoteAdapter(MainActivity.this, mQmoteList);
        lvQmote.setAdapter(mQmoteAdapter);

//        IntentFilter filter = new IntentFilter();
//        filter.addAction(QMOTE_ACTOIN_NAME);
//        registerReceiver(mQmoteReceiver, filter);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
//        unregisterReceiver(mQmoteReceiver);
    }


    @Override
    public void onClick(View v) {
        switch (v.getId()){
            case R.id.btn_get_qmote:
                getQmote();
                break;
        }
    }


    /**
     * Add Qmote by “Qmote APP” first.
     *
     * This function will get Qmotes in Bluetooth system.
     * Then you can expect which Qmote's action you may receive.
     */
    private void getQmote(){
        mQmoteList.clear();

        if (mBluetoothAdapter != null) {
            //Get bonded device from system
            Set<BluetoothDevice> bondDevices = mBluetoothAdapter.getBondedDevices();
            if (bondDevices.size() > 0) {

                for (BluetoothDevice bondDevice : bondDevices) {
                    if(bondDevice.getName() != null){
                        if (bondDevice.getName().length() == 10 && bondDevice.getName().contains("Qmote ")) {
                            mQmoteList.add(bondDevice);
                        }
                    }
                }

                mQmoteAdapter.notifyDataSetChanged();
            }
        }
    }


    /**
     * In this sample, we use receiver to demo actions while getting Qmote click.
     * You can also register BroadcastReceiver in Activity to make UI change as this.
     */
//    private static final String QMOTE_ACTOIN_NAME = "QmoteCommand";
//    private static final String QMOTE_CLICK = "QmoteClick";
//    private static final String QMOTE_ADDRESS = "QmoteAddress";
//
//    BroadcastReceiver mQmoteReceiver = new BroadcastReceiver(){
//        @Override
//        public void onReceive(final Context context, Intent intent) {
//
//            if (intent == null) return;
//
//            String click = intent.getStringExtra(QMOTE_CLICK);
//            String address = intent.getStringExtra(QMOTE_ADDRESS);
//
//            if(click!=null && address!=null)
//                Toast.makeText(MainActivity.this, address + ": " + click, Toast.LENGTH_SHORT).show();
//        }
//    };
}
