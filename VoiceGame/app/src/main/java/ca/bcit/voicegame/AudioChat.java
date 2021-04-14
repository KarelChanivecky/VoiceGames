package ca.bcit.voicegame;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.util.Log;
import android.widget.Button;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

public class AudioChat {

    private final int uid;

    private static String TAG = "AudioClient";

    // the server information
    private static final String SERVER = "xx.xx.xx.xx";
    private static final int PORT = 2001;

    // the audio recording options
    private static final int RECORDING_RATE = 10000;
    private static final int CHANNEL = AudioFormat.CHANNEL_IN_MONO;
    private static final int FORMAT = AudioFormat.ENCODING_PCM_16BIT;

    public AudioChat(int uid){
        this.uid = uid;
    }

    // the audio recorder
    private AudioRecord recorder;

    // the minimum buffer size needed for audio recording
    private static int BUFFER_SIZE = AudioRecord.getMinBufferSize(
            RECORDING_RATE, CHANNEL, FORMAT);
//    private static int BUFFER_SIZE = 12850;

    // are we currently sending audio data
    private boolean currentlySendingAudio = false;

    public void startStreamingAudio() {

        Log.i(TAG, "Starting the audio stream");
        currentlySendingAudio = true;
        startStreaming();
    }

    public void stopStreamingAudio() {

        Log.i(TAG, "Stopping the audio stream");
        currentlySendingAudio = false;
        if (recorder != null) recorder.release();
    }

    private void startStreaming() {

        Log.i(TAG, "Starting the background thread to stream the audio data");

        Thread streamThread = new Thread(new Runnable() {

            @Override
            public void run() {
                String server_name = "10.0.2.2";
                int server_port = 4445;
                String data_to_send = "testData";

                try {

                    Log.d(TAG, "Creating the datagram socket");
                    DatagramSocket socket = new DatagramSocket();

                    Log.d(TAG, "Creating the buffer of size " + BUFFER_SIZE);
                    byte[] buffer = new byte[BUFFER_SIZE];
                    byte[] buffer1 = new byte[5008];
                    for(int i = 0; i < 8; i++){
                        buffer1[i]=1;
                    }

                    Log.d(TAG, "Connecting to " + SERVER + ":" + PORT);
//                    final InetAddress serverAddress = InetAddress
//                            .getByName(SERVER);
                    final InetAddress serverAddress = InetAddress
                            .getByName(server_name);

                    Log.d(TAG, "Connected to " + SERVER + ":" + PORT);
//
                    Log.d(TAG, "Creating the reuseable DatagramPacket");
                    DatagramPacket packet;
//
                    Log.d(TAG, "Creating the AudioRecord");
                    recorder = new AudioRecord(MediaRecorder.AudioSource.MIC,
                            RECORDING_RATE, CHANNEL, FORMAT, BUFFER_SIZE * 10);

                    Log.d(TAG, "AudioRecord recording...");
                    recorder.startRecording();
                    while (currentlySendingAudio) {
                        int read = 0;

//                        while (read < 5000) {
                        // read the data into the buffer
                        read = recorder.read(buffer1, 8, buffer1.length-8);

                        // place contents of buffer into the packet
                        packet = new DatagramPacket(buffer1, read,
                                serverAddress, PORT);

                        // send the packet
                        socket.send(packet);
                    }

                    Log.d(TAG, "AudioRecord finished recording");

                } catch (Exception e) {
                    Log.e(TAG, "Exception: " + e);
                }
            }
        });

        // start the thread
        streamThread.start();
    }
}
