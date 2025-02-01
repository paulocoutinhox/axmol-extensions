package dev.axmol.app;

import android.app.Activity;
import android.app.Application;
import android.os.Bundle;

public class MainApplication extends Application implements Application.ActivityLifecycleCallbacks {
    private static MainApplication instance;
    private Activity currentActivity = null;

    public static MainApplication getInstance() {
        return instance;
    }

    public Activity getCurrentActivity() {
        return currentActivity;
    }

    @Override
    public void onCreate() {
        super.onCreate();

        instance = this;
        registerActivityLifecycleCallbacks(this);
    }

    @Override
    public void onActivityCreated(@androidx.annotation.NonNull Activity activity, Bundle savedInstanceState) {
        //
    }

    @Override
    public void onActivityStarted(@androidx.annotation.NonNull Activity activity) {
        //
    }

    @Override
    public void onActivityResumed(@androidx.annotation.NonNull Activity activity) {
        currentActivity = activity;
    }

    @Override
    public void onActivityPaused(@androidx.annotation.NonNull Activity activity) {
        //
    }

    @Override
    public void onActivityStopped(@androidx.annotation.NonNull Activity activity) {
        if (activity.equals(currentActivity)) {
            currentActivity = null;
        }
    }

    @Override
    public void onActivitySaveInstanceState(@androidx.annotation.NonNull Activity activity, @androidx.annotation.NonNull Bundle outState) {
        //
    }

    @Override
    public void onActivityDestroyed(@androidx.annotation.NonNull Activity activity) {
        //
    }
}
