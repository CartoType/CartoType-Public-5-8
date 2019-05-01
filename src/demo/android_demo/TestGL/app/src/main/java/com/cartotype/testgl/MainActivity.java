package com.cartotype.testgl;

import android.Manifest;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.os.Environment;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;

import com.cartotype.Framework;
import com.cartotype.NoticePosition;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class MainActivity extends AppCompatActivity
    {

    @Override
    protected void onCreate(Bundle savedInstanceState)
        {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
                        .setAction("Action", null).show();
            }
        });


            int permission = ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE);


            File map_file, font_file, style_file;

            try
                {
                map_file = getCopyOfFile(this, "data/isle_of_wight.ctm1", "isle_of_wight.ctm1");
                font_file = getCopyOfFile(this, "data/DejaVuSans.ttf", "DejaVuSans.ttf");
                style_file = getCopyOfFile(this, "data/standard.ctstyle", "standard.ctstyle");
                }
            catch (Exception e)
                {
                Log.d("CartoType","failed to open map, font or style sheet file");
                return;
                }

            String ct_dir = Environment.getExternalStorageDirectory().getPath() + "/CartoType/";
            // String ct_dir = getFilesDir().getPath() + "/CartoType/";
            // String my_file = ct_dir + "map/britain_and_ireland.ctm1";
            //String style_file = ct_dir + "style/standard.ctstyle";
            //String font_file = ct_dir + "font/DejaVuSans.ttf";

            Framework framework = new Framework(map_file.getAbsolutePath(),style_file.getAbsolutePath(),font_file.getAbsolutePath(),
                    256,256);
            //framework.setPerspective(true);
            //framework.enableLayer("terrain-height-feet",false);

            m_view = new MainView(this,framework);
            setContentView(m_view);

            // Set the scale and create a scale bar after creating the main view, which creates a MapView which sets the screen resolution.
            framework.setScale(500000);
            framework.setScaleBar(false,3,"in",NoticePosition.BottomLeft);
        }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

        private File getCopyOfFile(Context aContext, String aAssetPath, String aAssetFile) throws IOException
        {
            AssetManager am = aContext.getAssets();
            ContextWrapper wrapper = new ContextWrapper(aContext);
            File dir =  wrapper.getDir("localAssets",Context.MODE_PRIVATE);
            File f = new File(dir,aAssetFile);

            // Copy the file if it doesn't exist.
            if (!f.exists())
            {
                InputStream is = am.open(aAssetPath);
                OutputStream os = new FileOutputStream(f);
                byte[] buffer = new byte[1024];
                for (; ; )
                {
                    int length = is.read(buffer);
                    if (length <= 0)
                        break;
                    os.write(buffer, 0, length);
                }
                os.flush();
                os.close();
                is.close();
            }

            return f;
        }


    private MainView m_view;
    }
