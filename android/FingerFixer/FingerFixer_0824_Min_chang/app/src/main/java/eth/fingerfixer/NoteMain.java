package eth.fingerfixer;

import android.bluetooth.BluetoothAdapter;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import eth.fingerfixer.service.BTCTemplateService;
import eth.fingerfixer.utils.Constants;

// http://recipes4dev.tistory.com/43 참고 - 유환준

public class NoteMain extends AppCompatActivity {
    /**
     * bt
     */
    private BTCTemplateService mService;
    private Handler mActivityHandler = null;
    private int firstCount = 0;
    private int secondCount = 0;
    private TextView mTextBTState = null;
    private ImageView mImageView = null;
    private long tempo = 120L;
    private long time = 60000/4/tempo;


    boolean isPlay = false; // 현재 실행,정지 중인거 구분하는 변수
    ImageButton buttonPlay; // play 버튼 일시정지로 바꾸는데 이용
    RadioGroup radioGroup;  // 기본연주,자동연주 선택하는 라디오 그룹, onCreate에서 사용
    boolean isNormalMode = true; // 기본연주, 자동연주인거 구분하는 변수

    // Reference MusicNote class
    MusicNote musicNote;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_note);
        /**
         * bt
         */
        mActivityHandler = new ActivityHandler();
        bindService(new Intent(this, BTCTemplateService.class), mServiceConn, Context.BIND_AUTO_CREATE);
        mTextBTState = (TextView) findViewById(R.id.bt_state2);
        mTextBTState.setText("connected");
        mImageView = (ImageView) findViewById(R.id.status_title2);
        mImageView.setImageDrawable(getResources().getDrawable(android.R.drawable.presence_online));


        // 곡 제목 적어주기
        TextView songName = (TextView) findViewById(R.id.song_name);
        songName.setText(getIntent().getStringExtra("songName"));

        // 툴바 생성
        Toolbar noteToolbar1 = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(noteToolbar1);

        // 악보 이미지 보여주기
        ImageView noteMain = (ImageView) findViewById(R.id.note_main);
        noteMain.setImageResource(R.drawable.note_sample);

        // 라디오그룹 리스너, 이벤트 처리
        radioGroup = (RadioGroup) findViewById(R.id.radioGroup);
        radioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                if (checkedId == R.id.radio_button_normal) {
                    isNormalMode = true;
                } else if (checkedId == R.id.radio_button_auto) {
                    isNormalMode = false;
                }
            }
        });

        // play 버튼 객체 연결
        buttonPlay = (ImageButton) findViewById(R.id.button_play);

        // parameter - 곡 명, Context
        musicNote = new MusicNote(getIntent().getStringExtra("songName"), getApplicationContext());
    }

    /**
     * Service connection
     */
    private ServiceConnection mServiceConn = new ServiceConnection() {

        public void onServiceConnected(ComponentName className, IBinder binder) {
            // Log.d(TAG, "Activity - Service connected");

            mService = ((BTCTemplateService.ServiceBinder) binder).getService();
            //mServiceHandler = mService.mServiceHandler;

            initialize();
        }

        public void onServiceDisconnected(ComponentName className) {
            mService = null;
        }
    };

    /**
     * Initialization / Finalization
     */
    private void initialize() {
        //Logs.d(TAG, "# Activity - initialize()");
        mService.setupService(mActivityHandler);

        // If BT is not on, request that it be enabled.
        // RetroWatchService.setupBT() will then be called during onActivityResult

        if (!mService.isBluetoothEnabled()) {
            Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableIntent, Constants.REQUEST_ENABLE_BT);
        }

    }

    // 툴바 백 버튼 이벤트 처리
    public void backButtonOnClick(View v) {
        finish();
    }

    // toolbar2 play 버튼 이벤트 처리
    public void playButtonOnClick(View v) {
        if (isPlay == false) {
            isPlay = true;
            if (isNormalMode == true) {
                Toast.makeText(this, "기본연주 모드 시작", Toast.LENGTH_SHORT).show();
                basicPlay();
            } else {
                Toast.makeText(this, "자동연주 모드 시작", Toast.LENGTH_SHORT).show();
                //Play2(120);
                autoPlay(time);
            }
            buttonPlay.setBackground(getResources().getDrawable(R.drawable.button_pause));
        } else {
            Toast.makeText(this, "일시 정지 버튼클릭", Toast.LENGTH_SHORT).show();
            buttonPlay.setBackground(getResources().getDrawable(R.drawable.fab_play));
            isPlay = false;
        }
    }

    // toolbar2 stop 버튼 이벤트 처리
    public void stopButtonOnClick(View v) {
        Toast.makeText(this, "정지 버튼 클릭", Toast.LENGTH_SHORT).show();
        firstCount = 0;
        secondCount = 0;
        buttonPlay.setBackground(getResources().getDrawable(R.drawable.fab_play));
        isPlay = false;
    }

    private void basicPlay() {
        if (isPlay == true) {
            mService.sendMessageToRemote(musicNote.upperNote[firstCount][secondCount] + "**" + musicNote.lowerNote[firstCount][secondCount]);
            secondCount++;

            // 다음줄로 넘어갈때
            if (secondCount == musicNote.ARRNUM) {
                firstCount++;
                secondCount = 0;
            }

            // 줄이 끝나면?
            if (firstCount == musicNote.upperNote.length) {
                firstCount = 0;
                secondCount = 0;
                try {
                    isPlay = false;
                    buttonPlay.setBackground(getResources().getDrawable(R.drawable.fab_play));
                    Toast.makeText(this, "악보 끝", Toast.LENGTH_SHORT).show();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private void autoPlay(long time){
        if(isPlay==true)
            mActivityHandler.sendEmptyMessageDelayed(9999, time);
    }

    public class ActivityHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                // Receives BT state messages from service
                // and updates BT state UI
                case Constants.MESSAGE_BT_STATE_INITIALIZED:
                    mTextBTState.setText("initializing...");
                    mImageView.setImageDrawable(getResources().getDrawable(android.R.drawable.presence_invisible));
                    break;
                case Constants.MESSAGE_BT_STATE_LISTENING:
                    mTextBTState.setText("waiting...");
                    mImageView.setImageDrawable(getResources().getDrawable(android.R.drawable.presence_invisible));
                    break;
                case Constants.MESSAGE_BT_STATE_CONNECTING:
                    mTextBTState.setText("connectiong...");
                    mImageView.setImageDrawable(getResources().getDrawable(android.R.drawable.presence_away));
                    break;
                case Constants.MESSAGE_BT_STATE_CONNECTED:
                    if (mService != null) {
                        String deviceName = mService.getDeviceName();
                        if (deviceName != null) {
                            mTextBTState.setText("connected");
                            mImageView.setImageDrawable(getResources().getDrawable(android.R.drawable.presence_online));
                        }
                    }
                    break;
                case Constants.MESSAGE_BT_STATE_ERROR:
                    mTextBTState.setText("Error");
                    mImageView.setImageDrawable(getResources().getDrawable(android.R.drawable.presence_busy));
                    break;

                // BT Command status
                case Constants.MESSAGE_CMD_ERROR_NOT_CONNECTED:
                    break;

                ///////////////////////////////////////////////
                // When there's incoming packets on bluetooth
                // do the UI works like below
                ///////////////////////////////////////////////
                case Constants.MESSAGE_READ_CHAT_DATA:
                    try {
                        if (msg.obj != null) {
                            if ("YES".equals(msg.obj.toString())) {
                                basicPlay();
                            }
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    break;

                case Constants.MESSAGE_WRITE_CHAT_DATA:
                    autoPlay(time);
                    break;
                case 9999:
                    mService.sendMessageToRemote("1" + musicNote.upperNote[firstCount][secondCount] + "**" + musicNote.lowerNote[firstCount][secondCount]);
                    secondCount++;

                    // 다음줄로 넘어갈때
                    if (secondCount == musicNote.ARRNUM) {
                        firstCount++;
                        secondCount = 0;
                    }

                    // 줄이 끝나면?
                    if (firstCount == musicNote.upperNote.length) {
                        firstCount = 0;
                        secondCount = 0;

                        isPlay = false;
                        buttonPlay.setBackground(getResources().getDrawable(R.drawable.fab_play));
                        Toast.makeText(getApplicationContext(), "악보 끝", Toast.LENGTH_SHORT).show();
                    }
                    break;
                // 악보가 끝났을 때
                case 999:
                    Toast.makeText(getApplicationContext(), "악보 끝", Toast.LENGTH_LONG).show();
                    buttonPlay.setBackground(getResources().getDrawable(R.drawable.fab_play));
                    break;

                default:
                    break;
            }

            super.handleMessage(msg);
        }
    }    // End of class ActivityHandler

/*
    private class Play extends Thread{
        boolean stopflag;

        public Play(){
            stopflag = false;
        }

        public void stopPlay(){
            stopflag = true;
        }

        @Override
        public void run() {
            while(!stopflag) {
                mHandler.sendEmptyMessageDelayed(0, 60000/4*tempo);
            }
        }

        public void autoPlay(){
            mService.sendMessageToRemote("1" + musicNote.upperNote[firstCount][secondCount] + "**" + musicNote.lowerNote[firstCount][secondCount]);
            secondCount++;

            // 다음줄로 넘어갈때
            if (secondCount == musicNote.ARRNUM) {
                firstCount++;
                secondCount = 0;
            }

            // 줄이 끝나면?
            if (firstCount == musicNote.upperNote.length) {
                firstCount = 0;
                secondCount = 0;

                isPlay = false;
                stopflag = true;

                mActivityHandler.obtainMessage(999);
            }
        }

        Handler mHandler = new Handler(){
            @Override
            public void handleMessage(Message msg) {
                switch(msg.what){
                    // 자동연주
                    case 0:
                        autoPlay();
                        break;

                    default:
                        break;
                }
                super.handleMessage(msg);
            }
        };
    }*/
}

