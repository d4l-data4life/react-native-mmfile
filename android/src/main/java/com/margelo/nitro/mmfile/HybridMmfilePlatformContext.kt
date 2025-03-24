package com.margelo.nitro.mmfile

import android.util.Log
import androidx.annotation.Keep
import com.facebook.proguard.annotations.DoNotStrip
import com.margelo.nitro.core.AnyMap
import com.margelo.nitro.core.AnyValue
import com.margelo.nitro.core.ArrayBuffer
import com.margelo.nitro.core.Promise
import kotlinx.coroutines.delay

@Keep
@DoNotStrip
class HybridMmfilePlatformContext : HybridMmfileSpec() {
    private val TAG = "HybridMmfilePlatformContext"

    override fun getBaseDirectory(): String {
        // return "${context.filesDir.absolutePath}/rtnmmarray"
        return "rtnmmarray"
    }

    override fun getAppGroupDirectory(): String? {
        // AppGroups do not exist on Android. It's iOS only.
        return null
    }
}
