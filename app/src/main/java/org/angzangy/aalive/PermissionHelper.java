package org.angzangy.aalive;


import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.provider.Settings;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.widget.Toast;

/**
 * Helper class for handling dangerous permissions for Android API level >= 23 which
 * requires user consent at runtime
 */
class PermissionHelper {
  public static final int  RC_PERMISSION_REQUEST = 9222;
  public static boolean hasCameraPermission(Activity activity) {
    return ContextCompat.checkSelfPermission(activity,
            Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED;
  }
  public static boolean hasWriteStoragePermission(Activity activity) {
    return ContextCompat.checkSelfPermission(activity,
            Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;
  }
  public static void requestCameraPermission(Activity activity, boolean requestWritePermission) {

    boolean showRationale = ActivityCompat.shouldShowRequestPermissionRationale(activity,
              Manifest.permission.CAMERA) || (requestWritePermission &&
    ActivityCompat.shouldShowRequestPermissionRationale(activity,
            Manifest.permission.WRITE_EXTERNAL_STORAGE));
    if (showRationale) {
        Toast.makeText(activity,
                "Camera permission is needed to run this application", Toast.LENGTH_LONG).show();
      } else {

        // No explanation needed, we can request the permission.

      String permissions[] = requestWritePermission ? new String[]{Manifest.permission.CAMERA,
              Manifest.permission.WRITE_EXTERNAL_STORAGE}: new String[]{Manifest.permission.CAMERA};
        ActivityCompat.requestPermissions(activity,permissions,RC_PERMISSION_REQUEST);
      }
    }

  public static void requestCameraPermission(android.support.v4.app.Fragment fragment) {
    boolean showRationale = ActivityCompat.shouldShowRequestPermissionRationale(fragment.getActivity(),
            Manifest.permission.CAMERA);
    if (showRationale) {
      Toast.makeText(fragment.getActivity(),
              "Camera permission is needed to run this application", Toast.LENGTH_LONG).show();
    } else {
      fragment.requestPermissions(new String[]{Manifest.permission.CAMERA},RC_PERMISSION_REQUEST);
    }
  }

  public static void requestWriteStoragePermission(Activity activity) {
    boolean showRationale = ActivityCompat.shouldShowRequestPermissionRationale(activity,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE);
    if (showRationale) {
      Toast.makeText(activity,
              "Writing to external storage permission is needed to run this application",
              Toast.LENGTH_LONG).show();
    } else {

      // No explanation needed, we can request the permission.

      String permissions[] =  new String[]{ Manifest.permission.WRITE_EXTERNAL_STORAGE};

      ActivityCompat.requestPermissions(activity,permissions,RC_PERMISSION_REQUEST);
    }
  }

  /** Launch Application Setting to grant permission. */
  public static void launchPermissionSettings(Activity activity) {
    Intent intent = new Intent();
    intent.setAction(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
    intent.setData(Uri.fromParts("package", activity.getPackageName(), null));
    activity.startActivity(intent);
  }

}
