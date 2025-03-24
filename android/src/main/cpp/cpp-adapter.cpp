#include <jni.h>
#include "NitroMmfileOnLoad.hpp"

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void*) {
  return margelo::nitro::mmfile::initialize(vm);
}
