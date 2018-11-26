package org.angzangy.aalive;

import android.content.pm.PackageManager;
import android.support.annotation.NonNull;
import android.support.v4.app.Fragment;

public class BaseFragment extends Fragment {

    public void requestPermission(String permission, int requestCode){
        requestPermissions(new String[]{permission}, requestCode);
    }

    protected void onRequestPermissionSuccess(int requestCode, String permission){

    }

    protected void onRequestPermissionFail(int requestCode, String permission){

    }
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        int size = Math.min(permissions.length, grantResults.length);
        for(int i = 0; i < size; i++){
            if(grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                onRequestPermissionSuccess(requestCode, permissions[i]);
            }else{
                onRequestPermissionFail(requestCode, permissions[i]);
            }
        }
    }
}
