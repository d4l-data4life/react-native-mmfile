///
/// HybridMmfileBaseSpec.hpp
/// This file was generated by nitrogen. DO NOT MODIFY THIS FILE.
/// https://github.com/mrousavy/nitro
/// Copyright © 2025 Marc Rousavy @ Margelo
///

#pragma once

#if __has_include(<NitroModules/HybridObject.hpp>)
#include <NitroModules/HybridObject.hpp>
#else
#error NitroModules cannot be found! Are you sure you installed NitroModules properly?
#endif

// Forward declaration of `ArrayBuffer` to properly resolve imports.
namespace NitroModules { class ArrayBuffer; }

#include <string>
#include <optional>
#include <NitroModules/ArrayBuffer.hpp>

namespace margelo::nitro::mmfile {

  using namespace margelo::nitro;

  /**
   * An abstract base class for `MmfileBase`
   * Inherit this class to create instances of `HybridMmfileBaseSpec` in C++.
   * You must explicitly call `HybridObject`'s constructor yourself, because it is virtual.
   * @example
   * ```cpp
   * class HybridMmfileBase: public HybridMmfileBaseSpec {
   * public:
   *   HybridMmfileBase(...): HybridObject(TAG) { ... }
   *   // ...
   * };
   * ```
   */
  class HybridMmfileBaseSpec: public virtual HybridObject {
    public:
      // Constructor
      explicit HybridMmfileBaseSpec(): HybridObject(TAG) { }

      // Destructor
      ~HybridMmfileBaseSpec() override = default;

    public:
      // Properties
      virtual double getSize() = 0;
      virtual double getCapacity() = 0;
      virtual bool getReadOnly() = 0;
      virtual std::string getFilePath() = 0;
      virtual bool getIsOpen() = 0;

    public:
      // Methods
      virtual void close() = 0;
      virtual void resize(double newSize, std::optional<bool> strictResize) = 0;
      virtual void clear() = 0;
      virtual void append(const std::shared_ptr<ArrayBuffer>& buffer, std::optional<bool> strictResize) = 0;
      virtual void write(double offset, const std::shared_ptr<ArrayBuffer>& buffer) = 0;
      virtual double read(double offset, const std::shared_ptr<ArrayBuffer>& buffer) = 0;

    protected:
      // Hybrid Setup
      void loadHybridMethods() override;

    protected:
      // Tag for logging
      static constexpr auto TAG = "MmfileBase";
  };

} // namespace margelo::nitro::mmfile
