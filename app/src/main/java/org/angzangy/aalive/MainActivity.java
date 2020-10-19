package org.angzangy.aalive;

import android.os.Bundle;
import android.view.Window;
import android.view.WindowManager;

public class MainActivity extends BaseActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        UiUtils.initialize(this);
        setContentView(R.layout.main_layout);
        if(savedInstanceState == null){
            getSupportFragmentManager().beginTransaction().
                    replace(R.id.content, new CameraMediaCodecFragment()).
                    commit();
        }
    }
}
