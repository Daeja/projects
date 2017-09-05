package eth.fingerfixer;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.os.Environment;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.FilenameFilter;
import java.io.IOException;

import static java.lang.Boolean.FALSE;
import static java.lang.Boolean.TRUE;

// http://recipes4dev.tistory.com/43 참고

public class NoteList extends AppCompatActivity {

    String selectedNote = null; // 선택된 악보
    long backKeyPressedTime; //백버튼 클릭 시간
    boolean state = FALSE;

    ListView listview;
    ListViewAdapter adapter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        setTheme(R.style.AppTheme);

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_note_list);

        try {
            // Create file
            /*
            BufferedWriter bw = new BufferedWriter(new FileWriter(this.getFilesDir().getAbsolutePath() + "/test.eth", true));
            bw.write("4/4hgdfheidceidcg");
            bw.close();
            */

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

        backKeyPressedTime = System.currentTimeMillis();    // 백버튼 클릭 초기화

        // 툴바 생성
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        // Adapter 생성
        adapter = new ListViewAdapter();

        // 리스트뷰 참조 및 Adapter달기
        listview = (ListView) findViewById(R.id.listview);
        listview.setAdapter(adapter);

        // 아이템 추가
        for (int i = 0; i < titleList.length; i++) {
            adapter.addItem(ContextCompat.getDrawable(this, R.drawable.ic_level1), titleList[i], "ETH");
        }

        // 위에서 생성한 listview에 클릭 이벤트 핸들러 정의.
        listview.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView parent, View v, int position, long id) {
                // get item
                ListViewItem item = (ListViewItem) parent.getItemAtPosition(position);

                String titleStr = item.getTitle();
                Drawable iconDrawable = item.getLevel();

                // TODO : use item data.
                selectedNote = titleStr;
            }
        });
    }

    @Override
    public void onBackPressed() {
        //1번째 백버튼 클릭
        if (System.currentTimeMillis() > backKeyPressedTime + 2000) {
            backKeyPressedTime = System.currentTimeMillis();
            Toast.makeText(this, "한 번 더 누르시면 종료됩니다.", Toast.LENGTH_SHORT).show();
        }
        //2번째 백버튼 클릭 (종료)
        else {
            finish();
            System.exit(0);
            android.os.Process.killProcess(android.os.Process.myPid());
        }
    }

    // 툴바에 메뉴(다운로드 버튼) 인플레이트
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.toolbar_menu, menu);
        return true;
    }

    // 툴바 다운로드 버튼 이벤트 처리
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.toolbar_menu:
                Intent intent = new Intent(this, DownloadActivity.class);
                startActivity(intent);
                break;
        }
        return true;
    }

    // Play 플로팅 버튼 이벤트 처리
    public void fabPlayOnClick(View v) {
        if (selectedNote == null)
            Toast.makeText(this, "악보를 선택해 주세요.", Toast.LENGTH_SHORT).show();
        else {
            //Toast.makeText(this, selectedNote + "가 선택 되었습니다.", Toast.LENGTH_SHORT).show();
            Intent intent = new Intent(this, NoteMain.class);
            intent.putExtra("songName", selectedNote);
            startActivity(intent);
        }
    }

    // Delete 플로팅 버튼 이벤트 처리
    public void fabDeleteOnClick(View v) {
        if (selectedNote == null)
            Toast.makeText(this, "악보를 선택해 주세요.", Toast.LENGTH_SHORT).show();
        else
            deleteDialog();
    }

    // Refresh 플로팅 버튼 이벤트 처리
    public void fabRefreshOnClick(View v) {
        String[] titleList = null;

        try {
            Toast.makeText(this, "Dir Path => " + this.getFilesDir().getAbsolutePath().toString(), Toast.LENGTH_SHORT).show();
        } catch (Exception e) {
            e.printStackTrace();
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_SHORT).show();
        }

        try {
            FilenameFilter fileFilter = new FilenameFilter() {
                public boolean accept(File dir, String name) {
                    return name.endsWith("eth");
                }
            };

            File file = new File(this.getFilesDir().getAbsolutePath() + "/");

            File[] files = file.listFiles(fileFilter);

            titleList = new String[files.length];

            for (int i = 0; i < files.length; i++) {
                titleList[i] = files[i].getName();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        // Adapter 생성
        adapter = new ListViewAdapter();

        // 리스트뷰 참조 및 Adapter달기
        listview = (ListView) findViewById(R.id.listview);
        listview.setAdapter(adapter);

        // 아이템 추가
        for (int i = 0; i < titleList.length; i++) {
            adapter.addItem(ContextCompat.getDrawable(this, R.drawable.ic_level1), titleList[i], "ETH");
        }

        // 위에서 생성한 listview에 클릭 이벤트 핸들러 정의.
        listview.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView parent, View v, int position, long id) {
                // get item
                ListViewItem item = (ListViewItem) parent.getItemAtPosition(position);

                String titleStr = item.getTitle();
                Drawable iconDrawable = item.getLevel();

                // TODO : use item data.
                selectedNote = titleStr;
            }
        });
    }

    // 삭제할 때 물어보는 창
    private void deleteDialog() {
        AlertDialog.Builder alt_bld = new AlertDialog.Builder(this);
        alt_bld.setMessage("악보를 삭제하시겠습니까?").setCancelable(false)
                .setPositiveButton("네",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int id) {
                                File file = new File(getFilesDir().getAbsolutePath() + "/" + selectedNote);

                                if (file.delete()) {
                                    Toast.makeText(getBaseContext(), selectedNote + "을(를) 삭제하였습니다.", Toast.LENGTH_SHORT).show();
                                } else {
                                    Toast.makeText(getBaseContext(), selectedNote + "을(를) 삭제하지 못하였습니다.", Toast.LENGTH_SHORT).show();
                                }


                            }
                        }).setNegativeButton("아니요",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        // Action for 'NO' Button
                        dialog.cancel();
                    }
                });
        AlertDialog alert = alt_bld.create();
        alert.show();
    }

}






/*
package eth.fingerfixer;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.Toast;

// http://recipes4dev.tistory.com/43 참고

public class NoteList extends AppCompatActivity {

    String selectedNote = null; // 선택된 악보
    long backKeyPressedTime; //백버튼 클릭 시간

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        setTheme(R.style.AppTheme);

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_note_list);

        backKeyPressedTime = System.currentTimeMillis();    // 백버튼 클릭 초기화

        // 툴바 생성
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        ListView listview ;
        ListViewAdapter adapter;

        // Adapter 생성
        adapter = new ListViewAdapter() ;

        // 리스트뷰 참조 및 Adapter달기
        listview = (ListView) findViewById(R.id.listview);
        listview.setAdapter(adapter);

        // 아이템 추가
        adapter.addItem(ContextCompat.getDrawable(this, R.drawable.ic_level1), "easy level sample music", "Song Writer") ;
        adapter.addItem(ContextCompat.getDrawable(this, R.drawable.ic_level2), "normal level sample music", "Song Writer") ;
        adapter.addItem(ContextCompat.getDrawable(this, R.drawable.ic_level3), "hard level sample music", "Song Writer") ;
        adapter.addItem(ContextCompat.getDrawable(this, R.drawable.ic_level1), "easy level sample music", "Song Writer") ;
        adapter.addItem(ContextCompat.getDrawable(this, R.drawable.ic_level2), "normal level sample music", "Song Writer") ;
        adapter.addItem(ContextCompat.getDrawable(this, R.drawable.ic_level3), "hard level sample music", "Song Writer") ;
        adapter.addItem(ContextCompat.getDrawable(this, R.drawable.ic_level1), "easy level sample music", "Song Writer") ;
        adapter.addItem(ContextCompat.getDrawable(this, R.drawable.ic_level2), "normal level sample music", "Song Writer") ;
        adapter.addItem(ContextCompat.getDrawable(this, R.drawable.ic_level3), "hard level sample music", "Song Writer") ;
        adapter.addItem(ContextCompat.getDrawable(this, R.drawable.ic_level1), "easy level sample music", "Song Writer") ;
        adapter.addItem(ContextCompat.getDrawable(this, R.drawable.ic_level2), "normal level sample music", "Song Writer") ;
        adapter.addItem(ContextCompat.getDrawable(this, R.drawable.ic_level3), "hard level sample music", "Song Writer") ;

        // 위에서 생성한 listview에 클릭 이벤트 핸들러 정의.
        listview.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView parent, View v, int position, long id) {
                // get item
                ListViewItem item = (ListViewItem) parent.getItemAtPosition(position) ;

                String titleStr = item.getTitle() ;
                Drawable iconDrawable = item.getLevel() ;

                // TODO : use item data.
                selectedNote = titleStr;
            }
        }) ;
    }

    @Override
    public void onBackPressed() {
        //1번째 백버튼 클릭
        if(System.currentTimeMillis()>backKeyPressedTime+2000){
            backKeyPressedTime = System.currentTimeMillis();
            Toast.makeText(this, "한 번 더 누르시면 종료됩니다.", Toast.LENGTH_SHORT).show();
        }
        //2번째 백버튼 클릭 (종료)
        else{
            finish();
            System.exit(0);
            android.os.Process.killProcess(android.os.Process.myPid());
        }
    }

    // 툴바에 메뉴(다운로드 버튼) 인플레이트
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.toolbar_menu, menu);
        return true;
    }

    // 툴바 다운로드 버튼 이벤트 처리
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()){
            case R.id.toolbar_menu:
                Intent intent = new Intent(this, DownloadActivity.class);
                startActivity(intent);
                break;
        }
        return true;
    }

    // Play 플로팅 버튼 이벤트 처리
    public void fabPlayOnClick(View v) {
        if (selectedNote == null)
            Toast.makeText(this, "악보를 선택해 주세요.", Toast.LENGTH_SHORT).show();
        else {
            Toast.makeText(this, selectedNote + "가 선택 되었습니다.", Toast.LENGTH_SHORT).show();
            Intent intent = new Intent(this, NoteMain.class);
            intent.putExtra("songName", selectedNote);
            startActivity(intent);
        }

    }

    // Delete 플로팅 버튼 이벤트 처리
    public void fabDeleteOnClick(View v) {
        if (selectedNote == null)
            Toast.makeText(this, "악보를 선택해 주세요.", Toast.LENGTH_SHORT).show();
        else
            deleteDialog();

    }

    // 삭제할 때 물어보는 창
    private void deleteDialog(){
        AlertDialog.Builder alt_bld = new AlertDialog.Builder(this);
        alt_bld.setMessage("악보를 삭제하시겠습니까?").setCancelable(false)
                .setPositiveButton("네",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        Toast.makeText(getBaseContext(), selectedNote + "를 삭제하였습니다.", Toast.LENGTH_SHORT).show();
                    }
                }).setNegativeButton("아니요",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        // Action for 'NO' Button
                        dialog.cancel();
                    }
                });
        AlertDialog alert = alt_bld.create();
        alert.show();
    }

}
*/