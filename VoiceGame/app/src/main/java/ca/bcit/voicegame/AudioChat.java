package ca.bcit.voicegame;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaRecorder;
import android.util.Log;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.nio.ByteBuffer;

import static android.media.AudioFormat.CHANNEL_OUT_MONO;


public class AudioChat {

    private static String TAG = "AudioClient";

    // the server information
    private String SERVER = "xx.xx.xx.xx";
    private int PORT;
    private DatagramSocket socket;

    private int UID;

    // the audio recording options
    private static final int RECORDING_RATE = 10000;
    private static final int CHANNEL = AudioFormat.CHANNEL_IN_MONO;
    private static final int FORMAT = AudioFormat.ENCODING_PCM_16BIT;
    private static final int BUF_SIZE = AudioTrack.getMinBufferSize(RECORDING_RATE, CHANNEL_OUT_MONO, FORMAT); //Bytes

    public AudioChat(int uid, int port, String server){
        this.UID = uid;
        this.PORT = port;
        this.SERVER = server;
    }

    // the audio recorder
    private AudioRecord recorder;
    private AudioTrack track;

    // the minimum buffer size needed for audio recording
    private static int BUFFER_SIZE = AudioRecord.getMinBufferSize(
            RECORDING_RATE, CHANNEL, FORMAT);

    // are we currently sending audio data
    private boolean currentlySendingAudio = false;
    private boolean speakers = false;

    public void startStreamingAudio() {

        Log.i(TAG, "Starting the audio stream");
        currentlySendingAudio = true;
        speakers = true;
        startStreaming();
        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        startPlaying();
    }

    public void stopStreamingAudio() {

        Log.i(TAG, "Stopping the audio stream");
        currentlySendingAudio = false;
        speakers = false;
        if (recorder != null) recorder.release();
        if (track != null) track.release();
    }

    private void startStreaming() {

        Log.i(TAG, "Starting the background thread to stream the audio data");

        Thread streamThread = new Thread(new Runnable() {

            @Override
            public void run() {

                try {
                    Log.d(TAG, "Creating the datagram socket");
                    socket = new DatagramSocket();
                    int packet_order_num = 0;

                    Log.d(TAG, "Creating the buffer of size " + BUFFER_SIZE);
                    ByteBuffer buffer = ByteBuffer.allocate(5008);// BUFFER_SIZE
                    buffer.putInt(packet_order_num);
                    buffer.putInt(UID);

                    final InetAddress serverAddress = InetAddress
                            .getByName(SERVER);

                    Log.d(TAG, "Creating the reuseable DatagramPacket");
                    DatagramPacket packet;

                    Log.d(TAG, "Creating the AudioRecord");
                    recorder = new AudioRecord(MediaRecorder.AudioSource.MIC,
                            RECORDING_RATE, CHANNEL, FORMAT, BUFFER_SIZE * 10);

                    Log.d(TAG, "AudioRecord recording...");
                    recorder.startRecording();
                    while (currentlySendingAudio) {
                        int read = 0;

//                        while (read < 5000) {
                        // read the data into the buffer
                        read = recorder.read(buffer.array(), 8, buffer.array().length-8);
//                        }
                            byte[] b = buffer.array();
                        // place contents of buffer into the packet
                        packet = new DatagramPacket(buffer.array(), read,
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

    private void startPlaying() {

        Log.i(TAG, "Starting the background thread to stream the audio data");

        Thread listeningThread = new Thread(new Runnable() {

            @Override
            public void run() {

                Log.d(TAG, "Creating the AudioTrack");
                track = new AudioTrack(AudioManager.STREAM_MUSIC, RECORDING_RATE, CHANNEL_OUT_MONO,
                        AudioFormat.ENCODING_PCM_16BIT, BUF_SIZE, AudioTrack.MODE_STREAM);
                track.play();
                int trial = 1;
                int order = 0;
                try {
                    while(speakers) {

                        final byte[]         bufIn;
                        final DatagramPacket packetIn;

                        bufIn    = new byte[5008];
                        packetIn = new DatagramPacket(bufIn, bufIn.length);
                        socket.receive(packetIn);

                        byte[] b_order = packetIn.getData();
                        ByteBuffer buff = ByteBuffer.allocate(4);
                        buff.put(b_order, 0, 4);
                        if (buff.getInt(0) < order) continue;

//                        Log.i(TAG, "Packet received: " + packetIn.getLength() + ", trial: " + trial++);
                        track.write(b_order, 8, 5000);
                    }

                } catch (Exception e) {
                    Log.e(TAG, "Exception: " + e);
                }
            }
        });

        listeningThread.start();
    }


}
