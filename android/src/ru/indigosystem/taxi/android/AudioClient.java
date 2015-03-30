package ru.indigosystem.taxi.android;

import android.app.ApplicationErrorReport;
import android.media.MediaPlayer;
import java.io.*;
import android.util.Log;


public class AudioClient //Вот именно об этом активите я и говорил
{
    private static MediaPlayer mp = null;
    private static final Object lock = new Object();

    private static final String TAG = "AudioClient";

    public static int playaudio(String input) {
        boolean started = true;

        if (mp == null) {
            Log.e(TAG, "new mediaplayer");
            mp = new MediaPlayer();
            mp.setOnCompletionListener(new MediaPlayer.OnCompletionListener(){
                @Override
                public void onCompletion(MediaPlayer mediaplayer) {
                  Log.e(TAG, "playback complete");
                  mp.stop();
                  mp.release();
                  mp = null;
                  synchronized (lock) {
                      lock.notifyAll();
                  }
                }
             });
        }

        try {
            Log.e(TAG, "arming playing sound " + input);
            mp.setDataSource(input);            
            mp.prepare();
            mp.start();
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "exception while playing sound");
            started = false;
        }

        synchronized (lock) {
            try {
                if (started) {
                    Log.e(TAG, "sound started, waiting");
                    lock.wait();
                } else {
                    Log.e(TAG, "sound NOT STARTED, NOT WAITING");
                }

            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        return 0;
    }
}
