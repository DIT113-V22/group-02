package com.example.smartcarapplication;

import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.NumberPicker;
import android.widget.TextView;
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
    private static final String EXTERNAL_MQTT_BROKER = "192.168.187.128";
    private static final String LOCALHOST = "10.0.2.2";
    private static final String MQTT_SERVER = "tcp://" + EXTERNAL_MQTT_BROKER + ":1883";
    private static final String SPEED_CONTROL = "/smartcar/control/speed";
    private static final String STEERING_CONTROL = "/smartcar/control/steering";
    private static final String AUTO_PARK = "/smartcar/parking/park";
    private static final String RETRIEVE = "/smartcar/parking/retrieve";
    private static final int QOS = 1;
    private static final int IMAGE_WIDTH = 320;
    private static final int IMAGE_HEIGHT = 240;

    private MqttClient mMqttClient;
    private boolean isConnected = false;
    private ImageView mCameraView;
    private boolean isParked = false;
    private JoystickJhr joystickJhr;
    private Button parkingButton;

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
        parkingButton = (Button) findViewById(R.id.park);
        joystickJhr = findViewById(R.id.joystick);
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
                    mMqttClient.unsubscribe("/smartcar/camera/front", null);
                    mMqttClient.subscribe("/smartcar/camera/birdseye", 0, null);
                }
                else {
                    mMqttClient.unsubscribe("/smartcar/camera/birdseye", null);
                    mMqttClient.subscribe("/smartcar/camera/front", 0, null);
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
                    mMqttClient.subscribe("/smartcar/info/#", QOS, null);
                    mMqttClient.subscribe("/smartcar/camera/front", QOS, null);
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
                    if (topic.equals("/smartcar/camera/front") || topic.equals("/smartcar/camera/birdseye")) {
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
                    } else if (topic.equals("/smartcar/info/speed")) {
                        final String speed = message.toString();
                        TextView view = (TextView) findViewById(R.id.speedlog);
                        view.setText(String.format("%s km/h", speed));
                        // display distance in meters on mobile device if message "/smartcar/ultrasound/front"
                    } else if (topic.equals("/smartcar/info/ultrasound/front")) {
                            final String distance = message.toString();
                            TextView view = (TextView) findViewById(R.id.distance);
                            view.setText(String.format("%s m", distance));
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

    public void togglePark(View view){
        if(isParked){
            mMqttClient.publish(RETRIEVE, "Retrieving", 2, null);
        } else {
            mMqttClient.publish(AUTO_PARK, "Parking", 2, null);
        }
    }
}