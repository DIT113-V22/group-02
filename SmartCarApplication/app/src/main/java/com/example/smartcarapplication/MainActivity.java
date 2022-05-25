package com.example.smartcarapplication;

import android.graphics.Bitmap;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.NumberPicker;
import android.widget.Toast;
import android.widget.ToggleButton;

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
    private static final String MQTT_SERVER = "tcp://" + LOCALHOST + ":1883";
    private static final String SPEED_CONTROL = "/smartcar/control/speed";
    private static final String STEERING_CONTROL = "/smartcar/control/steering";
    private static final String AUTO_PARK = "/smartcar/park";
    private static final int QOS = 1;
    private static final int IMAGE_WIDTH = 320;
    private static final int IMAGE_HEIGHT = 240;

    private MqttClient mMqttClient;
    private boolean isConnected = false;
    private ImageView mCameraView;
    private NumberPicker speedSelector;
    private String[] speedOptions;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        speedSelector = findViewById(R.id.cruisecontrol);
        speedSelector.setMaxValue(13);
        speedSelector.setMinValue(0);
        speedOptions = new String[]{"0","30","40","50","60","70","80","90","100","110","120","130","140","150"};
        speedSelector.setDisplayedValues(speedOptions);
        speedSelector.setWrapSelectorWheel(false);
        mMqttClient = new MqttClient(getApplicationContext(), MQTT_SERVER, TAG);
        mCameraView = findViewById(R.id.imageView);

        connectToMqttBroker();

        final JoystickJhr joystickJhr = findViewById(R.id.joystick);

        joystickJhr.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent motionEvent) {
                joystickJhr.move(motionEvent);
                ToggleButton toggle = (ToggleButton) findViewById(R.id.btnPlay);
                if(!toggle.isChecked()){
                    setSpeed((joystickJhr.joyY()/joystickJhr.getHeight())*100, "setting speed");
                }
                setAngle((joystickJhr.joyX()/joystickJhr.getWidth())*100, "setting angle");
                return true;
            }
        });

        ToggleButton toggle = (ToggleButton) findViewById(R.id.btnPlay);
        speedSelector.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            @Override
            public void onValueChange(NumberPicker numberPicker, int oldVal, int newVal) {
                Log.i("Chosen speed", speedOptions[newVal]);
                if (Integer.parseInt(speedOptions[newVal]) > 0) {
                    toggle.setChecked(true);
                }
                else {
                    toggle.setChecked(false);
                }
                mMqttClient.publish(SPEED_CONTROL, speedOptions[newVal], 2, null);
            }
        });

        toggle.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                if(b && speedOptions[speedSelector.getValue()].equals("0")){
                    speedSelector.setValue(1);
                    mMqttClient.publish(SPEED_CONTROL, speedOptions[1], 2, null);
                }
                Log.i(TAG, "Cruise Control On");
                if(!toggle.isChecked()){
                    speedSelector.setValue(0);
                    mMqttClient.publish(SPEED_CONTROL, speedOptions[0], 2, null);
                }
            }
        });
        ToggleButton toggleCam = (ToggleButton) findViewById(R.id.switchCam);
        toggleCam.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                if (toggleCam.isChecked()) {
                    mMqttClient.unsubscribe("/smartcar/camera", null);
                    mMqttClient.subscribe("/smartcar/birdseye", 0, null);
                }
                else {
                    mMqttClient.unsubscribe("/smartcar/birdseye", null);
                    mMqttClient.subscribe("/smartcar/camera", 0, null);
                }
            }
        });
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
                    mMqttClient.subscribe("/smartcar/cruiseControl",QOS,null);
                    mMqttClient.subscribe("/smartcar/park", QOS, null);
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
                    } else if (topic.equals("/smartcar/birdseye")) {
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

    void setSpeed(double speed, String actionDescription) {
        if (!isConnected) {
            final String notConnected = "Not connected (yet)";
            Log.e(TAG, notConnected);
            Toast.makeText(getApplicationContext(), notConnected, Toast.LENGTH_SHORT).show();
            return;
        }
        Log.i(TAG, actionDescription);
        mMqttClient.publish(SPEED_CONTROL, Double.toString(speed), QOS, null);
    }

    void setAngle(double angle, String actionDescription) {
        if (!isConnected) {
            final String notConnected = "Not connected (yet)";
            Log.e(TAG, notConnected);
            Toast.makeText(getApplicationContext(), notConnected, Toast.LENGTH_SHORT).show();
            return;
        }
        //Toast.makeText(getApplicationContext(),Double.toString(angle),Toast.LENGTH_SHORT).show();
        Log.i(TAG, actionDescription);
        mMqttClient.publish(STEERING_CONTROL, Double.toString(angle), QOS, null);

    }
    public void parkTheCar(View view){
       if (!isConnected){
           final String notConnected = "Not connected (yet)";
           Log.e(TAG, notConnected);
           Toast.makeText(getApplicationContext(), notConnected, Toast.LENGTH_SHORT).show();
           return;
       }
       Log.i(TAG, "AutoPark initiated");
       mMqttClient.publish(AUTO_PARK, "Parking", 2, null);
    }
}