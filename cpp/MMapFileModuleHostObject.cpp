#include "MMapFileModuleHostObject.h"
#include "MMapFileHostObject.h"
#include <jsi/jsi.h>

using namespace facebook;

fe

std::vector<jsi::PropNameID> MMapFileModuleHostObject::getPropertyNames(jsi::Runtime &runtime)
{
    return jsi::PropNameID::names(runtime, "openMMapFile", "openMMapEncryptedFile");
}

jsi::Value MMapFileModuleHostObject::get(jsi::Runtime &runtime, const jsi::PropNameID &propNameId)
{
    std::string propName = propNameId.utf8(runtime);

    if (propName == "openMMapEncryptedFile")
    {
        // MMapFile::openMMapEncryptedFile(filePath: string, encryptionKey: ArrayBuffer)
        return jsi::Function::createFromHostFunction(
            runtime, jsi::PropNameID::forAscii(runtime, propName), 2,
            [this](jsi::Runtime &runtime, const jsi::Value &thisValue, const jsi::Value *arguments, size_t count) -> jsi::Value
            {
                if (count == 2 && arguments[0].isString() && arguments[1].isObject()) [[likely]]
                {
                    jsi::Object object = arguments[1].asObject(runtime);
                    if (object.isArrayBuffer(runtime)) [[likely]]
                    {
                        jsi::ArrayBuffer key = object.getArrayBuffer(runtime);
                        if (key.size(runtime) == 16) [[likely]]
                        {
                            std::string filePath = arguments[0].getString(runtime).utf8(runtime);
                            if (filePath.empty())
                            {
                                throw jsi::JSError(runtime, "File path must not be empty");
                            }
                            // Check if the file path is relative
                            if (filePath.front() != '/')
                            {
                                filePath = basePath_ + filePath;
                            }
                            auto mmapEncryptedFile = std::make_shared<MMapEncryptedFileHostObject>(filePath, key.data(runtime));
                            return jsi::Object::createFromHostObject(runtime, mmapEncryptedFile);
                        }
                        else
                        {
                            throw jsi::JSError(runtime, "encryptionKey must be of length 16");
                        }
                    }
                }
                throw jsi::JSError(runtime,
                                   "openMMapEncryptedFile: First argument ('filePath') must be a string and second argument ('encryptionKey') must be an ArrayBuffer");
            });
    }

    if (propName == "openMMapFile")
    {
        // MMapFile::openMMapFile(filePath: string)
        return jsi::Function::createFromHostFunction(
            runtime, jsi::PropNameID::forAscii(runtime, propName), 1,
            [this](jsi::Runtime &runtime, const jsi::Value &thisValue, const jsi::Value *arguments, size_t count) -> jsi::Value
            {
                if (count == 1 && arguments[0].isString()) [[likely]]
                {
                    std::string filePath = arguments[0].getString(runtime).utf8(runtime);
                    if (filePath.empty())
                    {
                        throw jsi::JSError(runtime, "File path must not be empty");
                    }
                    // Check if the file path is relative
                    if (filePath.front() != '/')
                    {
                        filePath = basePath_ + filePath;
                    }
                    auto mmapFile = std::make_shared<MMapFileHostObject>(filePath);
                    return jsi::Object::createFromHostObject(runtime, mmapFile);            
                }
                throw jsi::JSError(runtime,
                                   "openMMapFile: First argument ('filePath') must be a non-empty string");
            });
    }


    return jsi::Value::undefined();
}
