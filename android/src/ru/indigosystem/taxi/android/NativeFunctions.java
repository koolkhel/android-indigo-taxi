package ru.indigosystem.taxi.android;

public class NativeFunctions {
	public static native void onHeadsetAttach();
	
	public static native void onHeadsetDetach();
	
	public static native void onBatteryLow();
	
	public static native void onBatteryOkay();
	
	public static native void onBatteryChanged(int charging, double percent);
	
	public static native void onPowerChange(int status);
	
	public static native void onShutdown();
	
	public static native void onReboot();
	
	public static native void onAirplaneModeChanged(int isAirplane);
	
	public static native void onScreenOnOff(int on);
	
	public static void registerBroadcastReceiver() {
		if (MyQtActivity.s_activity != null) {
			MyQtActivity.s_activity.runOnUiThread(new HeadsetReceiverRunnable());
			MyQtActivity.s_activity.runOnUiThread(new SystemEventReceiverRunnable());
		}
	}
}
