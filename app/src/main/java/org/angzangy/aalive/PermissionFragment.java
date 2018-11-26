package org.angzangy.aalive;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

public class PermissionFragment extends BaseFragment{
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        boolean ret = true;
        for(int i = 0; i < grantResults.length; i++) {
            if(grantResults[i] != PackageManager.PERMISSION_GRANTED) {
                ret = false;
                break;
            }
        }
        if(ret) {
            getActivity().getSupportFragmentManager().beginTransaction().
                    replace(R.id.content, new CameraFragment()).
                    commitAllowingStateLoss();
        }
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        super.onCreateView(inflater, container, savedInstanceState);
        return inflater.inflate(R.layout.permission_layout, null);
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        requestPermissions(new String[]{ Manifest.permission.WRITE_EXTERNAL_STORAGE,
                Manifest.permission.CAMERA}, 1);
    }
}
