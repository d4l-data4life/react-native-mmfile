package com.margelo.nitro.mmfile

import androidx.annotation.Keep
import com.facebook.proguard.annotations.DoNotStrip
import com.margelo.nitro.NitroModules

@Keep
@DoNotStrip
class HybridMmfilePlatformContext : HybridMmfilePlatformContextSpec() {
    private val TAG = "HybridMmfilePlatformContext"

    override fun getBaseDirectory(): String {
        NitroModules.applicationContext?.let {
            return "${it.filesDir.absolutePath}/rtnmmfile"
        }
        return ""
    }

    override fun getAppGroupDirectory(): String? {
        // AppGroups do not exist on Android. It's iOS only.
        return null
    }
}
