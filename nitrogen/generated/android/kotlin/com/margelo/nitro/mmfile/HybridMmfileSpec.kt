///
/// HybridMmfileSpec.kt
/// This file was generated by nitrogen. DO NOT MODIFY THIS FILE.
/// https://github.com/mrousavy/nitro
/// Copyright © 2025 Marc Rousavy @ Margelo
///

package com.margelo.nitro.mmfile

import androidx.annotation.Keep
import com.facebook.jni.HybridData
import com.facebook.proguard.annotations.DoNotStrip
import com.margelo.nitro.core.*

/**
 * A Kotlin class representing the Mmfile HybridObject.
 * Implement this abstract class to create Kotlin-based instances of Mmfile.
 */
@DoNotStrip
@Keep
@Suppress(
  "KotlinJniMissingFunction", "unused",
  "RedundantSuppression", "RedundantUnitReturnType", "SimpleRedundantLet",
  "LocalVariableName", "PropertyName", "PrivatePropertyName", "FunctionName"
)
abstract class HybridMmfileSpec: HybridObject() {
  @DoNotStrip
  private var mHybridData: HybridData = initHybrid()

  init {
    super.updateNative(mHybridData)
  }

  override fun updateNative(hybridData: HybridData) {
    mHybridData = hybridData
    super.updateNative(hybridData)
  }

  // Properties
  @get:DoNotStrip
  @get:Keep
  abstract val size: Double

  // Methods
  @DoNotStrip
  @Keep
  abstract fun resize(newSize: Double): Unit
  
  @DoNotStrip
  @Keep
  abstract fun clear(): Unit
  
  @DoNotStrip
  @Keep
  abstract fun append(buffer: ArrayBuffer): Unit
  
  @DoNotStrip
  @Keep
  abstract fun write(offset: Double, buffer: ArrayBuffer): Unit
  
  @DoNotStrip
  @Keep
  abstract fun read(offset: Double, buffer: ArrayBuffer): Double

  private external fun initHybrid(): HybridData

  companion object {
    private const val TAG = "HybridMmfileSpec"
  }
}
