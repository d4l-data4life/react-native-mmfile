#include "HybridMmfile.hpp"
#include <jsi/jsi.h>

namespace margelo::nitro::mmfile
{

using namespace facebook;

double HybridMmfile::getSize() {
    DEBUG_LOG("getSize()");
    return (double)instance->size();
}

double HybridMmfile::getCapacity() {
    DEBUG_LOG("getCapacity()");
    return (double)instance->capacity();
}

bool HybridMmfile::getReadOnly() {
    DEBUG_LOG("getReadOnly()");
    return instance->readOnly();
}

std::string HybridMmfile::getFilePath() {
    DEBUG_LOG("getFilePath()");
    return instance->filePath();
}

bool HybridMmfile::getIsOpen() {
    DEBUG_LOG("getIsOpen()");
    return instance->isOpen();
}

void HybridMmfile::open(const std::string& path, std::optional<bool> readOnly) {
    DEBUG_LOG("open(" << path << ", readOnly=" << (readOnly ? std::to_string(*readOnly) : "nullopt") << ")");
    instance->open(path, readOnly.has_value() ? readOnly.value() : false);
}

void HybridMmfile::close() {
    DEBUG_LOG("close()");
    instance->close();
}

void HybridMmfile::resize(double newSize, std::optional<bool> strictResize) {
    DEBUG_LOG("resize(" << newSize << ", strictResize=" << (strictResize ? std::to_string(*strictResize) : "nullopt") << ")");
    instance->resize((size_t)newSize, strictResize.has_value() ? strictResize.value() : false);
}

void HybridMmfile::clear() {
    DEBUG_LOG("clear()");
    instance->clear();
}

void HybridMmfile::append(const std::shared_ptr<ArrayBuffer>& buffer, std::optional<bool> strictResize) {
    DEBUG_LOG("append(buffer, strictResize=" << (strictResize ? std::to_string(*strictResize) : "nullopt") << ")");
    instance->append(buffer->data(), buffer->size(), strictResize.has_value() ? strictResize.value() : false);
}

void HybridMmfile::write(double offset, const std::shared_ptr<ArrayBuffer>& buffer) {
    DEBUG_LOG("write(offset=" << offset << ", buffer)");
    instance->write((size_t)offset, buffer->data(), buffer->size());
}

double HybridMmfile::read(double offset, const std::shared_ptr<ArrayBuffer>& buffer) {
    DEBUG_LOG("read(offset=" << offset << ", buffer)");
    return (double)instance->read((size_t)offset, buffer->data(), buffer->size());
}




double HybridEncryptedMmfile::getSize() {
    DEBUG_LOG("[Encrypted] getSize()");
    return (double)instance->size();
}

double HybridEncryptedMmfile::getCapacity() {
    DEBUG_LOG("[Encrypted] getCapacity()");
    return (double)instance->capacity();
}

bool HybridEncryptedMmfile::getReadOnly() {
    DEBUG_LOG("[Encrypted] getReadOnly()");
    return instance->readOnly();
}

std::string HybridEncryptedMmfile::getFilePath() {
    DEBUG_LOG("[Encrypted] getFilePath()");
    return instance->filePath();
}

bool HybridEncryptedMmfile::getIsOpen() {
    DEBUG_LOG("[Encrypted] getIsOpen()");
    return instance->isOpen();
}

void HybridEncryptedMmfile::open(const std::string& path, const std::shared_ptr<ArrayBuffer>& key, std::optional<bool> readOnly) {
    DEBUG_LOG("[Encrypted] open(" << path << ", key=..., readOnly=" << (readOnly ? std::to_string(*readOnly) : "nullopt") << ")");
    instance->open(path, key->data(), readOnly.has_value() ? readOnly.value() : false);
}

void HybridEncryptedMmfile::close() {
    DEBUG_LOG("[Encrypted] close()");
    instance->close();
}

void HybridEncryptedMmfile::resize(double newSize, std::optional<bool> strictResize) {
    DEBUG_LOG("[Encrypted] resize(" << newSize << ", strictResize=" << (strictResize ? std::to_string(*strictResize) : "nullopt") << ")");
    instance->resize((size_t)newSize, strictResize.has_value() ? strictResize.value() : false);
}

void HybridEncryptedMmfile::clear() {
    DEBUG_LOG("[Encrypted] clear()");
    instance->clear();
}

void HybridEncryptedMmfile::append(const std::shared_ptr<ArrayBuffer>& buffer, std::optional<bool> readOnly) {
    DEBUG_LOG("[Encrypted] append(buffer, readOnly=" << (readOnly ? std::to_string(*readOnly) : "nullopt") << ")");
    instance->append(buffer->data(), buffer->size(), readOnly.has_value() ? readOnly.value() : false);
}

void HybridEncryptedMmfile::write(double offset, const std::shared_ptr<ArrayBuffer>& buffer) {
    DEBUG_LOG("[Encrypted] write(offset=" << offset << ", buffer)");
    instance->write((size_t)offset, buffer->data(), buffer->size());
}

double HybridEncryptedMmfile::read(double offset, const std::shared_ptr<ArrayBuffer>& buffer) {
    DEBUG_LOG("[Encrypted] read(offset=" << offset << ", buffer)");
    return (double)instance->read((size_t)offset, buffer->data(), buffer->size());
}

}
