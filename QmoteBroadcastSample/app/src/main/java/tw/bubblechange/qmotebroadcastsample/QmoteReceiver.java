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

import android.app.NotificationManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.media.RingtoneManager;
import android.net.Uri;
import android.support.v4.app.NotificationCompat;

public class QmoteReceiver extends BroadcastReceiver    {

    private static final String QMOTE_ACTOIN_NAME = "QmoteCommand";// filter in Manifests
    private static final String QMOTE_CLICK = "QmoteClick";
    private static final String QMOTE_ADDRESS = "QmoteAddress";

    @Override
        public void onReceive(final Context context, Intent intent) {

        if (intent == null) return;

        String click = intent.getStringExtra(QMOTE_CLICK);
        String address = intent.getStringExtra(QMOTE_ADDRESS);

        if(click!=null && address!=null){
            // Do whatever you wish to do with this action
            sendNotification(context, address + ": " + click);
        }
    }

    /**
     * Show a simple notification
     */
    private void sendNotification(Context context, String message) {

        Uri defaultSoundUri = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION);
        NotificationCompat.Builder notificationBuilder = new NotificationCompat.Builder(context)
                .setSmallIcon(R.drawable.icon)
                .setContentTitle("Qmote Click")
                .setStyle(new NotificationCompat.BigTextStyle().bigText(message))//multi line
                .setContentText(message)
                .setSound(defaultSoundUri); // ring or vibrate if no ring

        NotificationManager notificationManager =
                (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.notify(999, notificationBuilder.build());
    }
}
