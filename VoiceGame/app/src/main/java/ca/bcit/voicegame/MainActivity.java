package ca.bcit.voicegame;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.content.Intent;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

import static ca.bcit.voicegame.TTTutils.SERVER_ADDRESS;
import static ca.bcit.voicegame.TTTutils.UDP_PORT;
import static ca.bcit.voicegame.TTTutils.UID;

public class MainActivity extends AppCompatActivity {

    private static String TAG = "AudioClient";
    private static final int REQUEST_MICROPHONE = 1;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ActivityCompat.requestPermissions(this,
                new String[]{Manifest.permission.RECORD_AUDIO},
                REQUEST_MICROPHONE);

    }

    public void goToTTT(View view) {
        Intent i = new Intent(this, TttActivity.class);
        startActivity(i);
    }

    public void goToRPS(View view) {
        Intent i = new Intent(this, RpsActivity.class);
        startActivity(i);
    }
}

//package ca.bcit.mixedexample;

