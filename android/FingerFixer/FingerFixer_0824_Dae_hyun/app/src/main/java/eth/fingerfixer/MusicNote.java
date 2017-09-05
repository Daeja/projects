package eth.fingerfixer;

import android.content.Context;
import android.support.v7.app.AppCompatActivity;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.StringTokenizer;

/**
 * Created by hyunx on 2017-08-02-0002.
 */

public class MusicNote extends AppCompatActivity {
    ///////////////////////////////////////////////////////
    //
    //  Value about data read...
    //
    ///////////////////////////////////////////////////////
    private String title;
    private String[] staveNote;                 // 한 라인(높은 음 + 낮은 음)
    private String[][] tempoNote;               // 한 구간(높은 음 + 낮은 음)
    private final String beat_division = "||";
    private final String section_division = "~~";
    private final String area_division = "**";
    private final int minute_to_second = 60;
    private int bpm;                             // 분당 박자 수

    protected String[][] upperNote;             // 높은 음 자리표
    protected String[][] lowerNote;             // 낮은 음 자리표
    protected int temp;                            // note_Tokenize()의 count 임시 변수
    protected int current_location;
    protected static final int ARRNUM = 16;

    public int denominator;         // 분모
    public int numerator;           // 분자
    public double time;             // 한 음표당 소요되는 시간
    public Context mContext;        //getFilesDir() 필요...

    ///////////////////////////////////////////////////////
    //
    //  생성자
    //
    ///////////////////////////////////////////////////////
    public MusicNote(String title, Context mContext) {
        this.title = title;
        this.mContext = mContext;
        readFile();
    }

    ///////////////////////////////////////////////////////
    //
    //  파일 읽기
    //
    ///////////////////////////////////////////////////////
    public void readFile() {
        // read file
        try {
            String file_name = title;       // 파일명
            String readStr = "";            // 토큰화 될 String
            String temp_str = "";           // 임시 String

            // 파일 읽기 위한 변수 선언
            File file = new File(mContext.getFilesDir().getAbsolutePath(), file_name);
            FileReader fr = null;
            BufferedReader br = null;

            // 파일 존재??
            if(file.exists()) {
                fr = new FileReader(file);
                br = new BufferedReader(fr);

                // read beat
                temp_str = br.readLine();

                // divide beat token
                // e.g.) 4/4, 8.8, ....
                beat_Tokenize(temp_str);

                // reset
                temp_str = "";

                // read tempo
                // default => 120
                bpm = Integer.parseInt(br.readLine());

                // calculate time per note
                time = time_to_note(bpm, minute_to_second);

                // read music note data
                while (((temp_str = br.readLine()) != null)) {
                    readStr += temp_str;

                    // reset
                    temp_str = "";
                }

                // 토큰 분리
                note_Tokenize(readStr);
            }

            br.close();
            fr.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            Toast.makeText(this, "File not Found", Toast.LENGTH_SHORT).show();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    ///////////////////////////////////////////////////////
    //
    //  박자 토큰화
    //
    ///////////////////////////////////////////////////////
    public void beat_Tokenize(String str) {
        ///////////////////////////////////////////////////////
        //  4 -> denominator
        // ---
        //  4 -> numerator
        ///////////////////////////////////////////////////////
        StringTokenizer st = new StringTokenizer(str, "/");
        denominator = Integer.parseInt(st.nextToken());
        numerator = Integer.parseInt(st.nextToken());
    }

    ///////////////////////////////////////////////////////
    //
    //  음표당 소요 시간 체크
    //
    ///////////////////////////////////////////////////////
    public double time_to_note(int bpm, int minute_to_second) {
        double time = 0;             // 소요 시간
        int beat_per_second = 0;    // 초당 박자 수

        beat_per_second =  bpm / minute_to_second;

        time = (double) 1 / (double) beat_per_second;

        return time;
    }

    ///////////////////////////////////////////////////////
    //
    //  음표 토큰화
    //
    ///////////////////////////////////////////////////////
    public void note_Tokenize(String str) {
        // "~~"을 기반으로 토큰화 시킴
        StringTokenizer sectionToken = new StringTokenizer(str, section_division);
        // 토큰 갯수 설정
        final int count = sectionToken.countTokens();
        int arr_count;

        staveNote = new String[count];
        tempoNote = new String[count][ARRNUM];
        upperNote = new String[count][ARRNUM];
        lowerNote = new String[count][ARRNUM];

        temp = count;

        // init arr_count
        arr_count = 0;
        // 다음 토큰이 없을 때까지 잘라줌
        while(sectionToken.hasMoreTokens()) {
            staveNote[arr_count] = sectionToken.nextToken();
            arr_count++;
        }

        // "||"을 기반으로 토큰화 시킴
        for(int i = 0; i < count; i++) {
            StringTokenizer tempoToken = new StringTokenizer(staveNote[i], beat_division);
            arr_count = 0;

            while(tempoToken.hasMoreTokens()) {
                tempoNote[i][arr_count] = tempoToken.nextToken();
                arr_count++;
            }
        }

        // "**"을 기반으로 토큰화 시킴
        for(int i = 0; i < count; i++) {
            for(int j = 0; j < ARRNUM; j++) {
                StringTokenizer areaToken = new StringTokenizer(tempoNote[i][j], area_division);

                upperNote[i][j] = areaToken.nextToken();
                lowerNote[i][j] = areaToken.nextToken();
            }
        }
    }
}
