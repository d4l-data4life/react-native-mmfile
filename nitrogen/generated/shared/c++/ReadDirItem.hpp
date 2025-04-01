///
/// ReadDirItem.hpp
/// This file was generated by nitrogen. DO NOT MODIFY THIS FILE.
/// https://github.com/mrousavy/nitro
/// Copyright © 2025 Marc Rousavy @ Margelo
///

#pragma once

#if __has_include(<NitroModules/JSIConverter.hpp>)
#include <NitroModules/JSIConverter.hpp>
#else
#error NitroModules cannot be found! Are you sure you installed NitroModules properly?
#endif
#if __has_include(<NitroModules/NitroDefines.hpp>)
#include <NitroModules/NitroDefines.hpp>
#else
#error NitroModules cannot be found! Are you sure you installed NitroModules properly?
#endif



#include <string>

namespace margelo::nitro::mmfile {

  /**
   * A struct which can be represented as a JavaScript object (ReadDirItem).
   */
  struct ReadDirItem {
  public:
    double mtime     SWIFT_PRIVATE;
    std::string name     SWIFT_PRIVATE;
    std::string path     SWIFT_PRIVATE;
    double size     SWIFT_PRIVATE;
    bool isFile     SWIFT_PRIVATE;
    bool isDirectory     SWIFT_PRIVATE;

  public:
    ReadDirItem() = default;
    explicit ReadDirItem(double mtime, std::string name, std::string path, double size, bool isFile, bool isDirectory): mtime(mtime), name(name), path(path), size(size), isFile(isFile), isDirectory(isDirectory) {}
  };

} // namespace margelo::nitro::mmfile

namespace margelo::nitro {

  using namespace margelo::nitro::mmfile;

  // C++ ReadDirItem <> JS ReadDirItem (object)
  template <>
  struct JSIConverter<ReadDirItem> final {
    static inline ReadDirItem fromJSI(jsi::Runtime& runtime, const jsi::Value& arg) {
      jsi::Object obj = arg.asObject(runtime);
      return ReadDirItem(
        JSIConverter<double>::fromJSI(runtime, obj.getProperty(runtime, "mtime")),
        JSIConverter<std::string>::fromJSI(runtime, obj.getProperty(runtime, "name")),
        JSIConverter<std::string>::fromJSI(runtime, obj.getProperty(runtime, "path")),
        JSIConverter<double>::fromJSI(runtime, obj.getProperty(runtime, "size")),
        JSIConverter<bool>::fromJSI(runtime, obj.getProperty(runtime, "isFile")),
        JSIConverter<bool>::fromJSI(runtime, obj.getProperty(runtime, "isDirectory"))
      );
    }
    static inline jsi::Value toJSI(jsi::Runtime& runtime, const ReadDirItem& arg) {
      jsi::Object obj(runtime);
      obj.setProperty(runtime, "mtime", JSIConverter<double>::toJSI(runtime, arg.mtime));
      obj.setProperty(runtime, "name", JSIConverter<std::string>::toJSI(runtime, arg.name));
      obj.setProperty(runtime, "path", JSIConverter<std::string>::toJSI(runtime, arg.path));
      obj.setProperty(runtime, "size", JSIConverter<double>::toJSI(runtime, arg.size));
      obj.setProperty(runtime, "isFile", JSIConverter<bool>::toJSI(runtime, arg.isFile));
      obj.setProperty(runtime, "isDirectory", JSIConverter<bool>::toJSI(runtime, arg.isDirectory));
      return obj;
    }
    static inline bool canConvert(jsi::Runtime& runtime, const jsi::Value& value) {
      if (!value.isObject()) {
        return false;
      }
      jsi::Object obj = value.getObject(runtime);
      if (!JSIConverter<double>::canConvert(runtime, obj.getProperty(runtime, "mtime"))) return false;
      if (!JSIConverter<std::string>::canConvert(runtime, obj.getProperty(runtime, "name"))) return false;
      if (!JSIConverter<std::string>::canConvert(runtime, obj.getProperty(runtime, "path"))) return false;
      if (!JSIConverter<double>::canConvert(runtime, obj.getProperty(runtime, "size"))) return false;
      if (!JSIConverter<bool>::canConvert(runtime, obj.getProperty(runtime, "isFile"))) return false;
      if (!JSIConverter<bool>::canConvert(runtime, obj.getProperty(runtime, "isDirectory"))) return false;
      return true;
    }
  };

} // namespace margelo::nitro
