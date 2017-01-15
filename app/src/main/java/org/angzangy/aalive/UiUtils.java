package org.angzangy.aalive;
import android.content.Context;
import android.util.DisplayMetrics;
import android.view.WindowManager;

/**
 * Collection of utility functions used in this package.
 */
public abstract class UiUtils {
    private static float sPixelDensity = 1;
    private static int   sPixelHeight  = -1;
    private static int   sPixelWidth   = -1;
    private static int   sDensityDpi   = 1;
    private static int   sSettingsHeight   = -1;
    public static final int DIRECTION_LEFT = 0;
    public static final int DIRECTION_RIGHT = 1;

    private UiUtils() {}

    public static void initialize(Context context) {
        DisplayMetrics metrics = new DisplayMetrics();
        WindowManager wm = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        wm.getDefaultDisplay().getMetrics(metrics);
        sPixelDensity = metrics.density;
        sPixelHeight  = metrics.heightPixels;
        sPixelWidth   = metrics.widthPixels;
        sDensityDpi = metrics.densityDpi;
    }

    public static int dpToPixel(int dp) {
        return Math.round(sPixelDensity * dp);
    }

    public static final int screenWidth() { return sPixelWidth;}
    public static final int screenHeight(){ return sPixelHeight;}
    public static final float screenDensity(){ return sPixelDensity;}
    public static final int screenDensityDPI() {return sDensityDpi;}
    public static final int settingsHeight()   {return sSettingsHeight;}
    public static final int shutterBarHeight() {return 0 /*sShutterBarHeight*/;}
}
