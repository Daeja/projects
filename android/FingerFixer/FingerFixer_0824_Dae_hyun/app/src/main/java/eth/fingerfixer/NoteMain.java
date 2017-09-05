package eth.fingerfixer;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

// http://recipes4dev.tistory.com/43 참고 - 유환준

public class NoteMain extends AppCompatActivity {

    boolean isPlay = false; // 현재 실행,정지 중인거 구분하는 변수
    ImageButton buttonPlay; // play 버튼 일시정지로 바꾸는데 이용
    RadioGroup radioGroup;  // 기본연주,자동연주 선택하는 라디오 그룹, onCreate에서 사용
    boolean isNormalMode = true; // 기본연주, 자동연주인거 구분하는 변수

    // Reference MusicNote class
    MusicNote musicNote;

    // Division Image
    ImageView division;
    final int move_gap = 24;
    int left_margin = 110;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_note);

        // 곡 제목 적어주기
        TextView songName = (TextView) findViewById(R.id.song_name);
        songName.setText(getIntent().getStringExtra("songName"));

        // 툴바 생성
        Toolbar noteToolbar1 = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(noteToolbar1);

        // 라디오그룹 리스너, 이벤트 처리
        radioGroup = (RadioGroup)findViewById(R.id.radioGroup);
        radioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                if(checkedId == R.id.radio_button_normal) {
                    isNormalMode = true;
                } else if (checkedId == R.id.radio_button_auto) {
                    isNormalMode = false;
                }
            }
        });

        // play 버튼 객체 연결
        buttonPlay = (ImageButton) findViewById(R.id.button_play);

        // Get id from division
        division = (ImageView) findViewById(R.id.division);

        // parameter - Song name, Context
        musicNote = new MusicNote(getIntent().getStringExtra("songName"), getApplicationContext());

        division.setX(left_margin);
        musicNote.current_location = 1;
    }

    // 툴바 백 버튼 이벤트 처리
    public void backButtonOnClick(View v) {
        finish();
    }

    // toolbar2 stop 버튼 이벤트 처리
    public void stopButtonOnClick(View v) {
        // 구분선 제자리로...
        left_margin = 110;
        division.setX(left_margin);

        Toast.makeText(this, "정지 버튼 클릭", Toast.LENGTH_SHORT).show();
        buttonPlay.setBackground(getResources().getDrawable(R.drawable.fab_play));
        isPlay = false;
    }

    ///////////////////////////////////////////////////////
    //
    //  구분선 0.125초 마다 구분선 이동
    //
    ///////////////////////////////////////////////////////
    // toolbar2 play 버튼 이벤트 처리
    public void playButtonOnClick(View v) {
        try {
            if (isPlay == false) {
                if (isNormalMode == true) {
                    Toast.makeText(this, "기본연주 모드 시작", Toast.LENGTH_SHORT).show();
                } else {
                    Toast.makeText(this, "자동연주 모드 시작", Toast.LENGTH_SHORT).show();
                }

                // Test - 버튼 누를 때마다 특정 값만큼 우로 이동
                move_division();

                buttonPlay.setBackground(getResources().getDrawable(R.drawable.button_pause));
                isPlay = true;
            } else {
                Toast.makeText(this, "일시 정지 버튼클릭", Toast.LENGTH_SHORT).show();
                buttonPlay.setBackground(getResources().getDrawable(R.drawable.fab_play));
                isPlay = false;
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    // Move right division
    public void move_division() {
        if(musicNote.current_location == musicNote.ARRNUM)
        { musicNote.current_location = 0; left_margin -= (move_gap * musicNote.ARRNUM); }

        musicNote.current_location++;

        left_margin += move_gap;
        division.setX(left_margin);
    }
}
