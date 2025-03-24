package com.margelo.nitro.mmfile;

import android.util.Log;

import androidx.annotation.Nullable;

import java.util.HashMap;
import java.util.function.Supplier;

public class NitroMmfilePackage extends HybridMmfileSpec {
  @Nullable
  @Override
  public NativeModule getModule(String name, ReactApplicationContext reactContext) {
    return null;
  }

  @Override
  public ReactModuleInfoProvider getReactModuleInfoProvider() {
    return () -> {
        return new HashMap<>();
    };
  }

  static {
    NitroMmfileOnLoad.initializeNative();
  }
}
