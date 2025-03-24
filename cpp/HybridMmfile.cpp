#include "HybridMmfile.hpp"
#include <jsi/jsi.h>

namespace margelo::nitro::mmfile
{

using namespace facebook;

double HybridMmfile::getSize() {
    return (double)instance->size();
}

void HybridMmfile::resize(double newSize) {
    instance->resize((size_t)newSize);
}

void HybridMmfile::clear() {
    instance->clear();
}

void HybridMmfile::append(const std::shared_ptr<ArrayBuffer>& buffer) {
    instance->append(buffer->data(), buffer->size());
}

void HybridMmfile::write(double offset, const std::shared_ptr<ArrayBuffer>& buffer) {
    instance->write((size_t)offset, buffer->data(), buffer->size());
}

double HybridMmfile::read(double offset, const std::shared_ptr<ArrayBuffer>& buffer) {
    return (double)instance->read((size_t)offset, buffer->data(), buffer->size());
}


double HybridEncryptedMmfile::getSize() {
    return (double)instance->size();
}

void HybridEncryptedMmfile::resize(double newSize) {
    instance->resize((size_t)newSize);
}

void HybridEncryptedMmfile::clear() {
    instance->clear();
}

void HybridEncryptedMmfile::append(const std::shared_ptr<ArrayBuffer>& buffer) {
    instance->append(buffer->data(), buffer->size());
}

void HybridEncryptedMmfile::write(double offset, const std::shared_ptr<ArrayBuffer>& buffer) {
    instance->write((size_t)offset, buffer->data(), buffer->size());
}

double HybridEncryptedMmfile::read(double offset, const std::shared_ptr<ArrayBuffer>& buffer) {
    return (double)instance->read((size_t)offset, buffer->data(), buffer->size());
}

} // namespace margelo::nitro::mmfile
