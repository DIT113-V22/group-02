package com.example.smartcarapplication;

import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.example.joystickjhr.JoystickJhr;

import org.eclipse.paho.client.mqttv3.IMqttActionListener;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.IMqttToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttMessage;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "SmartcarMqttController";
    private static final String EXTERNAL_MQTT_BROKER = "192.168.0.10";
    private static final String LOCALHOST = "10.0.2.2";
    private static final String MQTT_SERVER = "tcp://" + EXTERNAL_MQTT_BROKER + ":1883";
    private static final String SPEED_CONTROL = "/smartcar/control/speed";
    private static final String STEERING_CONTROL = "/smartcar/control/steering";
    private static final String AUTO_PARK = "/smartcar/park";
    private static final String RETRIEVE = "/smartcar/retrieve";
    private static final int MOVEMENT_SPEED = 50;
    private static final int IDLE_SPEED = 0;
    private static final int STRAIGHT_ANGLE = 0;
    private static final int STEERING_ANGLE = 50;
    private static final int QOS = 1;
    private static final int IMAGE_WIDTH = 320;
    private static final int IMAGE_HEIGHT = 240;

    private MqttClient mMqttClient;
    private boolean isConnected = false;
    private ImageView mCameraView;
    private boolean isParked = false;
    private JoystickJhr joystickJhr;
    private Button parkingButton;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mMqttClient = new MqttClient(getApplicationContext(), MQTT_SERVER, TAG);
        parkingButton = (Button) findViewById(R.id.park);
        joystickJhr = findViewById(R.id.joystick);
        joystickJhr.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent motionEvent) {
                joystickJhr.move(motionEvent);
                joystickJhr.joyY();
                joystickJhr.angle();
                joystickJhr.distancia();

                int dir = joystickJhr.getDireccion();

                if (dir == joystickJhr.stick_up()) {
                    setAngle(STRAIGHT_ANGLE, "Setting angle straight");
                    setSpeed(MOVEMENT_SPEED, "Moving forward");
                } else if (dir == joystickJhr.stick_down()) {
                    setAngle(STRAIGHT_ANGLE, "Setting angle straight");
                    setSpeed(-MOVEMENT_SPEED, "Moving backwards");
                } else if (dir == joystickJhr.stick_upRight()) {
                    setAngle(STEERING_ANGLE, "Setting angel up right");
                    setSpeed(MOVEMENT_SPEED, "Moving down right");
                } else if (dir == joystickJhr.stick_upLeft()) {
                    setAngle(-STEERING_ANGLE, "Setting angel up left");
                    setSpeed(MOVEMENT_SPEED, "Moving down left");
                } else if (dir == joystickJhr.stick_downRight()) {
                    setAngle(STEERING_ANGLE, "Setting angel down right");
                    setSpeed(-MOVEMENT_SPEED, "Moving down right");
                } else if (dir == joystickJhr.stick_downLeft()) {
                    setAngle(-STEERING_ANGLE, "Setting angel down left");
                    setSpeed(-MOVEMENT_SPEED, "Moving down left");
                } else if (dir == 0) {
                    setAngle(STRAIGHT_ANGLE, "Setting angle straight");
                    setSpeed(IDLE_SPEED, "Stopping smartcar");
                }
                return true;
            }
        });
        //mCameraView = findViewById(R.id.imageView);

        connectToMqttBroker();
    }

    @Override
    protected void onResume() {
        super.onResume();
        connectToMqttBroker();
    }

    @Override
    protected void onPause() {
        super.onPause();
        mMqttClient.disconnect(new IMqttActionListener() {
            @Override
            public void onSuccess(IMqttToken asyncActionToken) {
                Log.i(TAG, "Disconnected from broker");
            }

            @Override
            public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
            }
        });
    }

    private void connectToMqttBroker() {
        if (!isConnected) {
            mMqttClient.connect(TAG, "", new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    isConnected = true;
                    final String successfulConnection = "Connected to MQTT broker";
                    Log.i(TAG, successfulConnection);
                    Toast.makeText(getApplicationContext(), successfulConnection, Toast.LENGTH_SHORT).show();
                    mMqttClient.subscribe("/smartcar/ultrasound/front", QOS, null);
                    mMqttClient.subscribe("/smartcar/camera", QOS, null);
                    mMqttClient.subscribe("/smartcar/parking/#", QOS, null);
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    final String failedConnection = "Failed to connect to MQTT broker";
                    Log.e(TAG, failedConnection);
                    Toast.makeText(getApplicationContext(), failedConnection, Toast.LENGTH_SHORT).show();
                }
            }, new MqttCallback() {
                @Override
                public void connectionLost(Throwable cause) {
                    isConnected = false;

                    final String connectionLost = "Connection to MQTT broker lost";
                    Log.w(TAG, connectionLost);
                    Toast.makeText(getApplicationContext(), connectionLost, Toast.LENGTH_SHORT).show();
                }

                @Override
                public void messageArrived(String topic, MqttMessage message) throws Exception {
                    if (topic.equals("/smartcar/camera")) {
                        final Bitmap bm = Bitmap.createBitmap(IMAGE_WIDTH, IMAGE_HEIGHT, Bitmap.Config.ARGB_8888);

                        final byte[] payload = message.getPayload();
                        final int[] colors = new int[IMAGE_WIDTH * IMAGE_HEIGHT];
                        for (int ci = 0; ci < colors.length; ++ci) {
                            final int r = payload[3 * ci] & 0xFF;
                            final int g = payload[3 * ci + 1] & 0xFF;
                            final int b = payload[3 * ci + 2] & 0xFF;
                            colors[ci] = Color.rgb(r, g, b);
                        }
                        bm.setPixels(colors, 0, IMAGE_WIDTH, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);
                        mCameraView.setImageBitmap(bm);
                    } else if (topic.equals("/smartcar/parking/isParking")  || topic.equals("/smartcar/parking/isRetrieving")){
                        joystickJhr.setEnabled(false);
                        joystickJhr.setColorFirst(Color.parseColor("#8d8d8d"));
                        joystickJhr.setColorSecond(Color.parseColor("#bdbdbd"));
                        parkingButton.setEnabled(false);
                        parkingButton.setBackgroundColor(Color.parseColor("#bdbdbd"));
                        parkingButton.setTextColor(Color.parseColor("#8d8d8d"));
                    } else if (topic.equals("/smartcar/parking/hasParked") || topic.equals("/smartcar/parking/hasRetrieved")){
                        isParked = !isParked;
                        parkingButton.setEnabled(true);
                        joystickJhr.setEnabled(true);
                        joystickJhr.setColorFirst(Color.parseColor("#DDE8E8"));
                        joystickJhr.setColorSecond(Color.parseColor("#111E6C"));
                        parkingButton.setBackgroundColor(Color.parseColor("#0D1E70"));
                        parkingButton.setTextColor(Color.parseColor("#ffffff"));
                        if(parkingButton.getText().equals("P")){
                            parkingButton.setText("R");
                        } else {
                            parkingButton.setText("P");
                        }
                    } else {
                        Log.i(TAG, "[MQTT] Topic: " + topic + " | Message: " + message.toString());
                    }
                }

                @Override
                public void deliveryComplete(IMqttDeliveryToken token) {
                    Log.d(TAG, "Message delivered");
                }
            });
        }
    }

    void setSpeed(float speed, String actionDescription) {
        if (!isConnected) {
            final String notConnected = "Not connected (yet)";
            Log.e(TAG, notConnected);
            Toast.makeText(getApplicationContext(), notConnected, Toast.LENGTH_SHORT).show();
            return;
        }
        Log.i(TAG, actionDescription);
        mMqttClient.publish(SPEED_CONTROL, Float.toString(speed), QOS, null);
    }

    void setAngle(float angle, String actionDescription) {
        if (!isConnected) {
            final String notConnected = "Not connected (yet)";
            Log.e(TAG, notConnected);
            Toast.makeText(getApplicationContext(), notConnected, Toast.LENGTH_SHORT).show();
            return;
        }
        Toast.makeText(getApplicationContext(),Float.toString(angle),Toast.LENGTH_SHORT).show();
        Log.i(TAG, actionDescription);
        mMqttClient.publish(STEERING_CONTROL, Float.toString(angle), QOS, null);

    }

    public void togglePark(View view){
        if(isParked){
            mMqttClient.publish(RETRIEVE, "Retrieving", 2, null);
        } else {
            mMqttClient.publish(AUTO_PARK, "Parking", 2, null);
        }
    }
}