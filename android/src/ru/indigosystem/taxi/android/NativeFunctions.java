package ru.indigosystem.taxi.android;

public class NativeFunctions {
	public static native void onHeadsetAttach();
	
	public static native void onHeadsetDetach();
	
	public static void registerBroadcastReceiver() {
		if (MyQtActivity.s_activity != null) {
			MyQtActivity.s_activity.runOnUiThread(new HeadsetReceiverRunnable());
		}
	}
}
