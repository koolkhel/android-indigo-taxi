package ru.indigosystem.taxi.android;

import android.content.Intent;
import android.content.IntentFilter;

public class HeadsetReceiverRunnable implements Runnable {

	@Override
	public void run() {
		IntentFilter filter = new IntentFilter();
		filter.addAction(Intent.ACTION_HEADSET_PLUG);
		
		MyQtActivity.s_activity.registerReceiver(new HeadsetReceiver(), filter);
	}

}
