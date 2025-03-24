#pragma once

#include <jsi/jsi.h>

using namespace facebook;

class MMapFileModuleHostObject : public jsi::HostObject
{
public:
    MMapFileModuleHostObject(const std::string &basePath) : basePath_(basePath) {}

public:
    jsi::Value get(jsi::Runtime &, const jsi::PropNameID &name) override;
    std::vector<jsi::PropNameID> getPropertyNames(jsi::Runtime &runtime) override;

private:
    std::string basePath_;
};
