package org.angzangy.aalive;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import androidx.annotation.Nullable;

public class WelcomeActivity extends BaseActivity {
    private static int MIN_STAY_MS = 1000;
    private boolean hasInitPermission;
    private Handler handler = new Handler();
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.welcome_activity);
    }

    @Override
    protected void onStart() {
        super.onStart();
        hasInitPermission = PermissionHelper.hasCameraPermission(this)
                && PermissionHelper.hasWriteStoragePermission(this)
                && PermissionHelper.hasRecordAudioPermission(this);
        if(hasInitPermission){
            handler.postDelayed(scenceRunnable, MIN_STAY_MS);
        } else {
            PermissionHelper.requestCameraPermission(this, true);
            LogPrinter.d("AALog request init permission");
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        hasInitPermission = PermissionHelper.hasCameraPermission(this)
                && PermissionHelper.hasWriteStoragePermission(this);
        LogPrinter.d("AALog request permission result : "+hasInitPermission);
        if(!hasInitPermission){
            PermissionHelper.launchPermissionSettings(this);
            finish();
        } else {
            jumpMain();
        }
    }

    private void jumpMain(){
        Intent intent = new Intent(this, MainActivity.class);
        startActivity(intent);
        finish();
    }

    private Runnable scenceRunnable = new Runnable() {
        @Override
        public void run() {
            if(hasInitPermission){
                jumpMain();
            } else {
                handler.postDelayed(this, Math.max(500, MIN_STAY_MS / 2));
            }
        }
    };
}
