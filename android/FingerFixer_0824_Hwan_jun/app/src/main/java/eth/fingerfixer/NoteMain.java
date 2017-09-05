package eth.fingerfixer;

import android.app.ActionBar;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.animation.LinearInterpolator;
import android.widget.AdapterView;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FilenameFilter;

// http://recipes4dev.tistory.com/43 참고

public class NoteMain extends AppCompatActivity {

    boolean isPlay = false; // 현재 실행,정지 중인거 구분하는 변수
    ImageButton buttonPlay; // play 버튼 일시정지로 바꾸는데 이용
    RadioGroup radioGroup;  // 기본연주,자동연주 선택하는 라디오 그룹, onCreate에서 사용
    boolean isNormalMode = true; // 기본연주, 자동연주인거 구분하는 변수

    int nowLine = 0; // 악보가 갱신될 때 현재 어느 줄을 연주할건지

    // 몇분의 몇박자인지 변수
    int topTempo = 4, bottomTempo = 4; // 임시로 여기서 초기화
    int noteCount = 16 / bottomTempo * topTempo + 1;    // 한 마디에 들어가는 음표의 갯수 (+1하는 이유는 몇분의 몇박자도 그려줘야 해서)
    // 5선지에 그려질 음표 배열
    Note[] noteArray = new Note[noteCount]; // 1째줄 높은음 자리표
    Note[] noteArray2 = new Note[noteCount]; // 1째줄 낮은음 자리표
    Note[] noteArray3 = new Note[noteCount]; // 2째줄 높은음 자리표
    Note[] noteArray4 = new Note[noteCount]; // 2째줄 낮은음 자리표
    //음표를 담을 레이아웃
    LinearLayout linearLayout1, linearLayout2, linearLayout3, linearLayout4;

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


        ///////////////////////////////파일 읽기////////////////////////////////////
        File file;

        FileReader fr = null ;
        BufferedReader bufrd = null ;

        try {
            file = new File(this.getFilesDir().getAbsolutePath().toString() + "/airplane.eth");
            fr = new FileReader(file) ;
        } catch (Exception e) {
            e.printStackTrace();
            Toast.makeText(this, "에바야 안 나와", Toast.LENGTH_SHORT).show();
        }

        char ch ;

        try {
            // open file.
            bufrd = new BufferedReader(fr) ;

            ch = (char) bufrd.read();
            Toast.makeText(this, ch, Toast.LENGTH_SHORT).show();
            // read 1 char from file.
            //while ((ch = (char) bufrd.read()) != -1) {
            //    System.out.println("char : " + ch) ;
            //}

            // close file.
            bufrd.close() ;
            fr.close() ;
        } catch (Exception e) {
            e.printStackTrace();
            //Toast.makeText(this, "에바야 안 나와", Toast.LENGTH_SHORT).show();
        }

/*
        try {
            Toast.makeText(this, "Dir Path => " + this.getFilesDir().getAbsolutePath().toString(), Toast.LENGTH_SHORT).show();
        } catch (Exception e) {
            e.printStackTrace();
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_SHORT).show();
        }

        String[] titleList = null;

        try {
            FilenameFilter fileFilter = new FilenameFilter() {
                public boolean accept(File dir, String name) {
                    return name.endsWith("eth");
                }
            };

            File file = new File(this.getFilesDir().getAbsolutePath() + "/");
            //Context.getFilesDir().getPath()           => internal storage directory
            //getExternalStorageDirectory().getPath()   => external sdcard directory

            if (!file.exists())
                file.mkdirs();

            File[] files = file.listFiles(fileFilter);

            titleList = new String[files.length];

            for (int i = 0; i < files.length; i++) {
                titleList[i] = files[i].getName();
            }

            //Toast.makeText(getApplicationContext(), "파일 읽기 완료", Toast.LENGTH_SHORT).show();
        } catch (Exception e) {
            //Toast.makeText(getApplicationContext(), "파일 읽기 실패", Toast.LENGTH_SHORT).show();
            e.printStackTrace();
        }
*/
        // File read
        /*
        for (int i = 0; i < titleList.length; i++) {
            try {
                BufferedReader br = new BufferedReader(new FileReader(this.getFilesDir().getAbsolutePath() + "/" + titleList[i]));
                String readStr = "";
                String str = null;

                while (((str = br.readLine()) != null)) {
                    readStr += str + "\n";
                }

                br.close();

                Toast.makeText(this, titleList[i] + " => " + readStr.substring(0, readStr.length() - 1), Toast.LENGTH_SHORT).show();

            } catch (FileNotFoundException e) {
                e.printStackTrace();
                Toast.makeText(this, "File not Found", Toast.LENGTH_SHORT).show();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        */



        ////////////////////////////////파일 읽기 끝////////////////////////////////////////////

        // 악보 띄우는 레이아웃
        LinearLayout linearLayoutSet = (LinearLayout) findViewById(R.id.linearlayout_note_set);
        LinearLayout.LayoutParams params = (LinearLayout.LayoutParams) linearLayoutSet.getLayoutParams();
        params.topMargin = 33;
        params.leftMargin = 110;
        linearLayoutSet.setLayoutParams(params);

        LinearLayout linearLayoutSet2 = (LinearLayout) findViewById(R.id.linearlayout_note_set2);
        LinearLayout.LayoutParams params2 = (LinearLayout.LayoutParams) linearLayoutSet2.getLayoutParams();
        params2.topMargin = 192;
        params2.leftMargin = 110;
        linearLayoutSet2.setLayoutParams(params2);
        //linearLayoutSet2.setBackground(ContextCompat.getDrawable(this, R.color.mainColor));

        LinearLayout linearLayoutSet3 = (LinearLayout) findViewById(R.id.linearlayout_note_set3);
        LinearLayout.LayoutParams params3 = (LinearLayout.LayoutParams) linearLayoutSet3.getLayoutParams();
        params3.topMargin = 33;
        params3.leftMargin = 110;
        linearLayoutSet3.setLayoutParams(params3);

        LinearLayout linearLayoutSet4 = (LinearLayout) findViewById(R.id.linearlayout_note_set4);
        LinearLayout.LayoutParams params4 = (LinearLayout.LayoutParams) linearLayoutSet4.getLayoutParams();
        params4.topMargin = 192;
        params4.leftMargin = 110;
        linearLayoutSet4.setLayoutParams(params4);


        // 음표 띄우기
        FrameLayout frameLayout = (FrameLayout) findViewById(R.id.framelayout_tempo_main); // 박자표
        FrameLayout frameLayout2 = (FrameLayout) findViewById(R.id.framelayout_tempo_main2); // 박자표
        FrameLayout frameLayout3 = (FrameLayout) findViewById(R.id.framelayout_tempo_main3); // 박자표
        FrameLayout frameLayout4 = (FrameLayout) findViewById(R.id.framelayout_tempo_main4); // 박자표
        linearLayout1 = (LinearLayout) findViewById(R.id.linearlayout_note_main1);
        //linearLayout1.setBackground(ContextCompat.getDrawable(this, R.color.mainColor));
        linearLayout2 = (LinearLayout) findViewById(R.id.linearlayout_note_main2);
        //linearLayout2.setBackground(ContextCompat.getDrawable(this, R.color.mainColor));
        linearLayout3 = (LinearLayout) findViewById(R.id.linearlayout_note_main3);
        //linearLayout1.setBackground(ContextCompat.getDrawable(this, R.color.mainColor));
        linearLayout4 = (LinearLayout) findViewById(R.id.linearlayout_note_main4);
        //linearLayout2.setBackground(ContextCompat.getDrawable(this, R.color.mainColor));

        /*for (int i = 0; i < noteCount; i++) {
            noteArray[i] = new ImageView(this);
        }*/

        // 임시로 수동으로 설정 - 원래는 위에 for문에서 해야함함
        noteArray[0] = new Note(this, frameLayout, topTempo, bottomTempo);
        noteArray[1] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');
        noteArray[2] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');
        noteArray[3] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');
        noteArray[4] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');
        noteArray[5] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');
        noteArray[6] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');
        noteArray[7] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');
        noteArray[8] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');
        noteArray[9] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');
        noteArray[10] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');
        noteArray[11] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');
        noteArray[12] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');
        noteArray[13] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');
        noteArray[14] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');
        noteArray[15] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');
        noteArray[16] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');


        noteArray2[0] = new Note(this, frameLayout2, topTempo, bottomTempo);
        noteArray2[1] = new Note(this, linearLayout2, 'M', '3', 'H', 'c', '%');
        noteArray2[2] = new Note(this, linearLayout2, 'M', '0', '0', '0', '%');
        noteArray2[3] = new Note(this, linearLayout2, 'M', '3', 'H', 'g', '%');
        noteArray2[4] = new Note(this, linearLayout2, 'M', '0', '0', '0', '%');
        noteArray2[5] = new Note(this, linearLayout2, 'M', '3', 'H', 'e', '%');
        noteArray2[6] = new Note(this, linearLayout2, 'M', '0', '0', '0', '%');
        noteArray2[7] = new Note(this, linearLayout2, 'M', '3', 'H', 'g', '%');
        noteArray2[8] = new Note(this, linearLayout2, 'M', '0', '0', '0', '%');
        noteArray2[9] = new Note(this, linearLayout2, 'M', '3', 'H', 'c', '%');
        noteArray2[10] = new Note(this, linearLayout2, 'M', '0', '0', '0', '%');
        noteArray2[11] = new Note(this, linearLayout2, 'M', '3', 'H', 'g', '%');
        noteArray2[12] = new Note(this, linearLayout2, 'M', '0', '0', '0', '%');
        noteArray2[13] = new Note(this, linearLayout2, 'M', '3', 'H', 'e', '%');
        noteArray2[14] = new Note(this, linearLayout2, 'M', '0', '0', '0', '%');
        noteArray2[15] = new Note(this, linearLayout2, 'M', '3', 'H', 'g', '%');
        noteArray2[16] = new Note(this, linearLayout2, 'M', '0', '0', '0', '%');


        noteArray3[0] = new Note(this, frameLayout3, topTempo, bottomTempo);
        noteArray3[1] = new Note(this, linearLayout3, 'M', '4', 'F', 'g', '%');
        noteArray3[2] = new Note(this, linearLayout3, 'M', '0', '0', '0', '%');
        noteArray3[3] = new Note(this, linearLayout3, 'M', '0', '0', '0', '%');
        noteArray3[4] = new Note(this, linearLayout3, 'M', '0', '0', '0', '%');
        noteArray3[5] = new Note(this, linearLayout3, 'M', '4', 'F', 'g', '%');
        noteArray3[6] = new Note(this, linearLayout3, 'M', '0', '0', '0', '%');
        noteArray3[7] = new Note(this, linearLayout3, 'M', '0', '0', '0', '%');
        noteArray3[8] = new Note(this, linearLayout3, 'M', '0', '0', '0', '%');
        noteArray3[9] = new Note(this, linearLayout3, 'M', '4', 'F', 'g', '%');
        noteArray3[10] = new Note(this, linearLayout3, 'M', '0', '0', '0', '%');
        noteArray3[11] = new Note(this, linearLayout3, 'M', '0', '0', '0', '%');
        noteArray3[12] = new Note(this, linearLayout3, 'M', '0', '0', '0', '%');
        noteArray3[13] = new Note(this, linearLayout3, 'M', '4', 'F', 'e', '%');
        noteArray3[14] = new Note(this, linearLayout3, 'M', '0', '0', '0', '%');
        noteArray3[15] = new Note(this, linearLayout3, 'M', '0', '0', '0', '%');
        noteArray3[16] = new Note(this, linearLayout3, 'M', '0', '0', '0', '%');


        noteArray4[0] = new Note(this, frameLayout4, topTempo, bottomTempo);
        noteArray4[1] = new Note(this, linearLayout4, 'M', '3', 'H', 'c', '%');
        noteArray4[2] = new Note(this, linearLayout4, 'M', '0', '0', '0', '%');
        noteArray4[3] = new Note(this, linearLayout4, 'M', '3', 'H', 'g', '%');
        noteArray4[4] = new Note(this, linearLayout4, 'M', '0', '0', '0', '%');
        noteArray4[5] = new Note(this, linearLayout4, 'M', '3', 'H', 'e', '%');
        noteArray4[6] = new Note(this, linearLayout4, 'M', '0', '0', '0', '%');
        noteArray4[7] = new Note(this, linearLayout4, 'M', '3', 'H', 'g', '%');
        noteArray4[8] = new Note(this, linearLayout4, 'M', '0', '0', '0', '%');
        noteArray4[9] = new Note(this, linearLayout4, 'M', '3', 'H', 'c', '%');
        noteArray4[10] = new Note(this, linearLayout4, 'M', '0', '0', '0', '%');
        noteArray4[11] = new Note(this, linearLayout4, 'M', '3', 'H', 'g', '%');
        noteArray4[12] = new Note(this, linearLayout4, 'M', '0', '0', '0', '%');
        noteArray4[13] = new Note(this, linearLayout4, 'M', '3', 'H', 'e', '%');
        noteArray4[14] = new Note(this, linearLayout4, 'M', '0', '0', '0', '%');
        noteArray4[15] = new Note(this, linearLayout4, 'M', '3', 'H', 'g', '%');
        noteArray4[16] = new Note(this, linearLayout4, 'M', '0', '0', '0', '%');
        /*
        Note tempo2 = new Note(this, frameLayout2, topTempo, bottomTempo);
        Note note11 = new Note(this, linearLayout2, 'E', 'b', '#');
        Note note12 = new Note(this, linearLayout2, 'F', 'a', '#');
        Note note13 = new Note(this, linearLayout2, 'G', 'g', '$');
        Note note14 = new Note(this, linearLayout2, 'H', 'b', '$');
        Note note15 = new Note(this, linearLayout2, 'I', 'a', '#');
        Note note16 = new Note(this, linearLayout2, 'J', 'g', '$');
        */

        // 라디오그룹 리스너, 이벤트 처리
        radioGroup = (RadioGroup)findViewById(R.id.radioGroup);
        radioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                if(checkedId == R.id.radio_button_normal){
                    isNormalMode = true;
                } else if (checkedId == R.id.radio_button_auto) {
                    isNormalMode = false;
                }
            }
        });

        // play 버튼 객체 연결
        buttonPlay = (ImageButton) findViewById(R.id.button_play);
    }

    // 툴바 백 버튼 이벤트 처리
    public void backButtonOnClick(View v) {
        finish();
    }

    // toolbar2 stop 버튼 이벤트 처리
    public void stopButtonOnClick(View v) {
        Toast.makeText(this, "정지 버튼 클릭", Toast.LENGTH_SHORT).show();
        buttonPlay.setBackground(getResources().getDrawable(R.drawable.fab_play));
        isPlay = false;
    }

    // toolbar2 play 버튼 이벤트 처리
    public void playButtonOnClick(View v) {
        if (isPlay == false) {
            if (isNormalMode == true) {
                Toast.makeText(this, "기본연주 모드 시작", Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(this, "자동연주 모드 시작", Toast.LENGTH_SHORT).show();
            }
            buttonPlay.setBackground(getResources().getDrawable(R.drawable.button_pause));
            isPlay = true;


            readNext();
            v.invalidate();
/*
            Thread thread = new Thread() {
                @Override
                public void run() {
                    //super.run();
                }
            };
            thread.start();
*/

        } else {
            Toast.makeText(this, "일시 정지 버튼클릭", Toast.LENGTH_SHORT).show();
            buttonPlay.setBackground(getResources().getDrawable(R.drawable.fab_play));
            isPlay = false;
        }
    }

    public void readNext() {
        nowLine++;
        int nextLine = nowLine + 1;

        linearLayout1.removeAllViews();
        linearLayout2.removeAllViews();
        linearLayout3.removeAllViews();
        linearLayout4.removeAllViews();
        /*
        for(int i = 1; i < 17; i++) {
            for(int j = 0; j < 16; j++) {
                noteArray[i].setParam(linearLayout1, upperNote[nowLine][j]);
                noteArray2[i].setParam(linearLayout2, lowerNote[nowLine][j]);
                noteArray3[i].setParam(linearLayout3, upperNote[nextLine][j]);
                noteArray4[i].setParam(linearLayout4, lowerNote[nextLine][j]);
            }
        }
        */

        noteArray[1] = new Note(this, linearLayout1, 'M', '3', 'H', 'c', '%');
        noteArray[2] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');
        noteArray[3] = new Note(this, linearLayout1, 'M', '3', 'H', 'g', '%');
        noteArray[4] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');
        noteArray[5] = new Note(this, linearLayout1, 'M', '3', 'H', 'e', '%');
        noteArray[6] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');
        noteArray[7] = new Note(this, linearLayout1, 'M', '3', 'H', 'g', '%');
        noteArray[8] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');
        noteArray[9] = new Note(this, linearLayout1, 'M', '3', 'H', 'c', '%');
        noteArray[10] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');
        noteArray[11] = new Note(this, linearLayout1, 'M', '3', 'H', 'g', '%');
        noteArray[12] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');
        noteArray[13] = new Note(this, linearLayout1, 'M', '3', 'H', 'e', '%');
        noteArray[14] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');
        noteArray[15] = new Note(this, linearLayout1, 'M', '3', 'H', 'g', '%');
        noteArray[16] = new Note(this, linearLayout1, 'M', '0', '0', '0', '%');


    }


}
