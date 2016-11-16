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

import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import java.util.List;

public class QmoteAdapter extends BaseAdapter {
    private List<BluetoothDevice> mQmoteList;
    private Context mContext;

    public QmoteAdapter(Context context, List<BluetoothDevice> list){
        mQmoteList = list;
        mContext = context;
    }
    @Override
    public int getCount() {
        return mQmoteList.size();
    }

    @Override
    public BluetoothDevice getItem(int position) {
        return mQmoteList.get(position);
    }

    @Override
    public long getItemId(int position) {
        return 0;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder holder = null;
        if (convertView == null) {
            convertView = LayoutInflater.from(mContext).inflate(android.R.layout.simple_list_item_2, null);
            holder = new ViewHolder();
            holder.text1 = (TextView) convertView.findViewById(android.R.id.text1);
            holder.text2 = (TextView)convertView.findViewById(android.R.id.text2);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder)convertView.getTag();
        }

        holder.text1.setText(mQmoteList.get(position).getAddress());
        if(mQmoteList.get(position).getName()!=null)
            holder.text2.setText(mQmoteList.get(position).getName());

        return convertView;
    }

    private class ViewHolder {
        TextView text1 = null;
        TextView text2 = null;
    }
}
