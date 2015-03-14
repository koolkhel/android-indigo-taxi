/*
    Copyright (c) 2012-2013, BogDan Vatra <bogdan@kde.org>
    Contact: http://www.qt-project.org/legal

    Commercial License Usage
    Licensees holding valid commercial Qt licenses may use this file in
    accordance with the commercial license agreement provided with the
    Software or, alternatively, in accordance with the terms contained in
    a written agreement between you and Digia.  For licensing terms and
    conditions see http://qt.digia.com/licensing.  For further information
    use the contact form at http://qt.digia.com/contact-us.

    BSD License Usage
    Alternatively, this file may be used under the BSD license as follows:
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
    THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

package ru.indigosystem.taxi.android;
import org.qtproject.qt5.android.bindings.*;
import android.os.Bundle;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.Context;
import android.provider.Settings;
import android.location.LocationManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.view.WindowManager;

import android.app.ApplicationErrorReport;
import android.media.MediaPlayer;
import java.io.*;
import android.util.Log;
import java.nio.charset.*;

public class MyQtActivity extends QtActivity
{
    public static int playAudio(String input) {
        MediaPlayer mp = new MediaPlayer();
           Log.i("tag", input);
           try {
               mp.setDataSource(input);
           } catch (IllegalArgumentException e) {
               Log.i("setDataSource", "IllegalArgumentException");
               e.printStackTrace();
           } catch (IllegalStateException e) {
               Log.i("setDataSource", "IllegalStateException");
               e.printStackTrace();
           } catch (IOException e) {
               Log.i("setDataSource", "IOException");
               e.printStackTrace();
           }
           try {
               mp.prepare();
           } catch (IllegalStateException e) {
               Log.i("prepare", "IllegalStateException");
               e.printStackTrace();
           } catch (IOException e) {
               Log.i("prepare", "IOException");
               e.printStackTrace();
           }
           mp.start();

        return 0;
    }

    public void displayPromptForEnablingGPS()
       {
           final AlertDialog.Builder builder =
               new AlertDialog.Builder(this);
           final String action = Settings.ACTION_LOCATION_SOURCE_SETTINGS;
           final String message = "Для работы Клиента Такси необходимо включить GPS. Нажмите OK для перехода к экрану настроек. Без GPS работа запрещена";

           final Activity activity = this;

           builder.setMessage(message)
               .setPositiveButton("OK",
                   new DialogInterface.OnClickListener() {
                       public void onClick(DialogInterface d, int id) {
                           activity.startActivity(new Intent(action));
                           d.dismiss();
                       }
               })
               .setNegativeButton("Cancel",
                   new DialogInterface.OnClickListener() {
                       public void onClick(DialogInterface d, int id) {
                           d.cancel();
                       }
               });
           builder.create().show();
       }

       public void displayPromptForEnabling3G()
          {
              final AlertDialog.Builder builder =
                  new AlertDialog.Builder(this);
              final String action = Settings.ACTION_WIRELESS_SETTINGS;
              final String message = "Для работы Клиента Такси необходимо включить Интернет. Нажмите OK для перехода к экрану настроек. Без Интернета работа запрещена";

              final Activity activity = this;

              builder.setMessage(message)
                  .setPositiveButton("OK",
                      new DialogInterface.OnClickListener() {
                          public void onClick(DialogInterface d, int id) {
                              activity.startActivity(new Intent(action));
                              d.dismiss();
                          }
                  })
                  .setNegativeButton("Cancel",
                      new DialogInterface.OnClickListener() {
                          public void onClick(DialogInterface d, int id) {
                              d.cancel();
                          }
                  });
              builder.create().show();
          }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final LocationManager manager = (LocationManager) getSystemService( Context.LOCATION_SERVICE );

        if (!manager.isProviderEnabled( LocationManager.GPS_PROVIDER ) ) {
            displayPromptForEnablingGPS();
        }

        ConnectivityManager conMan = ((ConnectivityManager) getSystemService(CONNECTIVITY_SERVICE));
        boolean isWifiEnabled = conMan.getNetworkInfo(ConnectivityManager.TYPE_WIFI).isAvailable();
        boolean is3GEnabled = !(conMan.getNetworkInfo(ConnectivityManager.TYPE_MOBILE).getState() == NetworkInfo.State.DISCONNECTED);
        is3GEnabled |= !"dataDisabled".equals(conMan.getNetworkInfo(ConnectivityManager.TYPE_MOBILE).getReason());

        if (!isWifiEnabled && !is3GEnabled) {
            displayPromptForEnabling3G();
        }

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

}
