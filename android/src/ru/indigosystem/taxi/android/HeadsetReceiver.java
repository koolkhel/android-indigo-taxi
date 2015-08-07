package ru.indigosystem.taxi.android;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class HeadsetReceiver extends BroadcastReceiver {
	private static final String TAG = "HeadsetReceiver";
	
	@Override
	public void onReceive(Context context, Intent intent) {
		if (intent.getAction().equals(Intent.ACTION_HEADSET_PLUG)) {			
			int state = intent.getIntExtra("state", -1);
			switch (state) {
			case 0:
				NativeFunctions.onHeadsetDetach();
				Log.d(TAG, "Headset is unplugged");
				break;
			case 1:
				NativeFunctions.onHeadsetAttach();
				Log.d(TAG, "Headset is plugged");
				break;
			default:
				Log.d(TAG, "I have no idea what the headset state is");
			}
		}

	}

}
