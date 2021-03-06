package com.ssb.droidsound.service;

import android.app.Notification;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.graphics.BitmapFactory;
import android.os.Binder;
import android.os.IBinder;

import com.ssb.droidsound.R;
import com.ssb.droidsound.activity.PlayerActivity;
import com.ssb.droidsound.utils.Log;

/**
 * This class protects the application from being murdered by android.
 *
 * @author alankila
 */
public class ProtectionMoneyService extends Service {
	protected static final String TAG = ProtectionMoneyService.class.getSimpleName();
	protected static final int NOTIFY_ID = 1;

	@Override
	public void onCreate() {
		super.onCreate();
		Log.i(TAG, "Preparing to participate in the Android protection money racket.");
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
		Log.i(TAG, "Stopping participation in the Android preoction money racket.");
	}

	public class LocalBinder extends Binder {
		public void setOngoingNotification(String text) {
			if (text != null) {
				Notification.Builder nb = new Notification.Builder(ProtectionMoneyService.this);
				nb.setLargeIcon(BitmapFactory.decodeResource(getResources(), R.drawable.droidsound64x64));
				nb.setContentTitle("DroidSound");
				nb.setOngoing(true);

				Log.i(TAG, "Waving white flag which says: %s", text);
				PendingIntent p = PendingIntent.getActivity(
						ProtectionMoneyService.this,
						0,
						new Intent(ProtectionMoneyService.this, PlayerActivity.class),
						0
				);
				nb.setContentInfo(text);
				nb.setContentIntent(p);
				startForeground(NOTIFY_ID, nb.getNotification());
			} else {
				Log.i(TAG, "Hiding white flag.");
				stopForeground(true);
			}
		}
	}
	private final LocalBinder binder = new LocalBinder();

	@Override
	public IBinder onBind(Intent arg0) {
		return binder;
	}
}
