package ca.bcit.voicegame;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.Socket;
import java.nio.ByteBuffer;

public class TTTutils {

    // Constants for PROTOCOL VERSIONS
    static final byte V1 = 1;

    // Constants for GAME IDs
    static final byte GAME_TTT = 1;
    static final byte GAME_RPS = 2;

    // Constants for TEAMs IN TTT
    static final byte SYM_X = 1;
    static final byte SYM_O = 2;

    // Constants for TEAMs IN RPS
    static final byte SYM = 0;

    // Constants for END OF GAME STATUS
    static final byte NOT_STARTED = 0;
    static final byte WIN = 1;
    static final byte LOSS = 2;
    static final byte TIE = 3;
    static final byte RUNNING = 4;
    static final byte OPP_LEFT = 5;

    // Constants for MOVES IN RPS
    static final byte ROCK = 1;
    static final byte PAPER = 2;
    static final byte SCISSORS = 3;

    // Constants for REQUEST MESSAGE TYPES
    static final byte CONFIRMATION = 1;
    static final byte INFORMATION = 2;
    static final byte META_ACTION = 3;
    static final byte GAME_ACTION = 4;

    // Constants for REQUEST CONTEXT VALUES
    static final byte CONFIRM_RULE_SET = 1;
    static final byte MAKE_MOVE = 1;
    static final byte QUIT_GAME = 1;

    // Constants for RESPONSE MESSAGE TYPES

        //success responses 10-19
    static final byte SUCCESS = 10;

        //update responses 20-29
    static final byte UPDATE = 20;

        //client error responses 30-39
    static final byte E_INVAL_REQ = 30;
    static final byte E_INVAL_UID = 31;
    static final byte E_INVAL_TYP = 32;
    static final byte E_INVAL_CON = 33;
    static final byte E_INVAL_PAY = 34;

        //server error responses 40-49
    static final byte E_SERVER = 40;

        //game error responses 50-59
    static final byte E_INVAL_ACT = 50;
    static final byte E_OUT_OF_TURN = 51;

    // Constants for  UPDATE RESPONSE CONTEXT VALUES
    static final byte START_GAME = 1;
    static final byte MOVE_MADE = 2;
    static final byte END_OF_GAME = 3;
    static final byte OPPONENT_DISCONNECTED = 4;

    // Server Address and Port
    static final String SERVER_ADDRESS = "10.0.2.2";
    static final int TCP_PORT = 3001;

    //ByteBuffer Capacities
    static final int IN_CAP = 20;
    static final int OUT_CAP = 20;

    static int UID = 0;
    static byte PRESENT_GAME = 0;
    static byte GAME_STATE = NOT_STARTED;

    static Socket socket;
    static InputStream input;
    static OutputStream output;

    private static boolean connectToServer(){
        try {
            final InetAddress serverAddress = InetAddress
                    .getByName(SERVER_ADDRESS);
            socket = new Socket(serverAddress, TCP_PORT);
            input = socket.getInputStream();
            output = socket.getOutputStream();
            System.out.println("connected to the server!");
            return true;
        } catch (Exception e) {
            System.err.println("connection to the server failed :|");
            e.printStackTrace();
            return false;
        }
    }

    private static boolean writeToServer(ByteBuffer msg){
        byte[] arr = msg.array();
        try {
            output.write(arr);
            System.out.println("written to output stream");
            return true;
        } catch (IOException e) {
            System.err.println("failed to write to output stream");
            e.printStackTrace();
            return false;
        }
    }

    private static ByteBuffer readFromServer(){
        ByteBuffer inBuff = ByteBuffer.allocate(IN_CAP);
        byte[] arr = new byte[IN_CAP];
        int read = 0;
        try {
            read = input.read(arr);
            System.out.println("read from input stream");
        } catch (IOException e) {
            System.err.println("failed to read from input stream");
            e.printStackTrace();
            return null;
        }
        inBuff.put(arr, 0, read);
        return inBuff;

//        System.out.println("position of buffer: " + buff.position());
//        System.out.println("capacity of buffer: " + buff.capacity());
//        System.out.println("remaining of buffer: " + buff.remaining());

    }

    public static int connectToGame(byte version, byte game_id){
        boolean sock_connected = connectToServer();
        if (!sock_connected) return -1;

        boolean written = writeToServer( writeGameStartMsg(version, game_id) );
        if (!written) return -1;

        PRESENT_GAME = game_id;
        boolean read_id = readUIDMsg();
        if (!read_id) return -1;

        return readGameStartMsg();
    }

    public static boolean sendMove(byte move){
        boolean written = writeToServer( writeMoveMsg(move) );
        if (!written) return false;

        return readMoveResponse();
    }

    private static boolean readUIDMsg() {
        ByteBuffer read_id_buff = readFromServer();
        if (read_id_buff == null) return false;
        int bytes_read = read_id_buff.position();
        byte server_msg_type = read_id_buff.get(0);

        if (server_msg_type >= E_INVAL_REQ) {
            System.err.println("Error occurred with server error code: " + server_msg_type);
            return false;
        }
        else if (server_msg_type >= UPDATE) {
            byte context_val = read_id_buff.get(1);
            System.err.println("Update occurred where not expected with context value: " + context_val);
            return false;
        }

        byte payload_len = read_id_buff.get(2);
        if (payload_len != 4) return false;
        UID = read_id_buff.getInt(3);

        return true;
    }

    private static int readGameStartMsg() {
        ByteBuffer read_buff = readFromServer();
        if (read_buff == null) return -1;
        int bytes_read = read_buff.position();
        byte server_msg_type = read_buff.get(0);

        if (server_msg_type >= E_INVAL_REQ) {
            System.err.println("Error occurred with server error code: " + server_msg_type);
            return -1;
        }
        byte context_val = read_buff.get(1);
        if (context_val != START_GAME) {
            System.err.println("Update occurred where not expected with context value: " + context_val);
            return -1;
        }
        byte payload_len = read_buff.get(2);
        if (PRESENT_GAME == GAME_TTT) {
            if (payload_len != 1) return -1;
            return read_buff.get(3);
        }

        return 0;
    }

    private static boolean readMoveResponse() {
        ByteBuffer read_buff = readFromServer();
        if (read_buff == null) return false;
        int bytes_read = read_buff.position();
        byte server_msg_type = read_buff.get(0);

        if (server_msg_type >= E_INVAL_REQ) {
            System.err.println("Error occurred with server error code: " + server_msg_type);
            return false;
        }
        else if (server_msg_type >= UPDATE) {
            byte context_val = read_buff.get(1);
            System.err.println("Update occurred where not expected with context value: " + context_val);
            return false;
        }

        return true;
    }

    public static int readOpponentMove() {
        ByteBuffer read_buff = readFromServer();
        if (read_buff == null) return -1;
        int bytes_read = read_buff.position();
        byte server_msg_type = read_buff.get(0);

        if (server_msg_type >= E_INVAL_REQ) {
            System.err.println("Error occurred with server error code: " + server_msg_type);
            return -1;
        }
        byte context_val = read_buff.get(1);
        byte payload_len = read_buff.get(2);

        if (context_val == MOVE_MADE) {
            if (payload_len != 1) return -1;
            return read_buff.get(3);
        }

        else if (context_val == END_OF_GAME) {
            if (payload_len != 2) return -1;
            GAME_STATE = read_buff.get(3);
            return read_buff.get(4);
        }

        else if (context_val == OPPONENT_DISCONNECTED) {
            if (payload_len != 0) return -1;
            GAME_STATE = OPP_LEFT;
            return -1;
        }

        System.err.println("Update occurred where not expected with context value: " + context_val);
        return -1;
    }

    private static ByteBuffer writeGameStartMsg(byte version, byte game_id) {
        ByteBuffer buff = ByteBuffer.allocate(9);
        buff.putInt(UID);
        buff.put(CONFIRMATION);
        buff.put(CONFIRM_RULE_SET);
        buff.put((byte) 2);
        buff.put(version);
        buff.put(game_id);

        return buff;
    }

    private static ByteBuffer writeMoveMsg(byte move) {
        ByteBuffer buff = ByteBuffer.allocate(9);
        buff.putInt(UID);
        buff.put(GAME_ACTION);
        buff.put(MAKE_MOVE);
        buff.put((byte) 1);
        buff.put(move);

        return buff;
    }

    public static void refreshValues() {
        UID = 0;
        PRESENT_GAME = 0;
        GAME_STATE = NOT_STARTED;
        socket = null;
        input = null;
        output = null;
    }
}
