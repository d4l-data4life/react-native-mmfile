# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

### Changed

### Deprecated

### Removed

### Fixed

### Security

## [1.7.0] - 2026-11-19

### Changed

- The iOS pod links the `Security` framework

### Security

- Generate the data key and IV of encrypted files with a cryptographically secure random number generator

## [1.6.0] - 2026-07-10

### Changed

- Update Nitro Modules to v0.35
- Layout of the example app (fix the safe area, remove unused code)

## [1.5.0] - 2026-06-14

### Added

- AES-192 and AES-256: `EncryptedMmfile` accepts 16-, 24- or 32-byte keys and selects AES-128, AES-192 or AES-256 accordingly; keys of any other size are rejected. The key only wraps the per-file AES-128 data key and the payload stays AES-128-CTR, so the file format is unchanged and files written by earlier versions remain readable

## [1.4.0] - 2026-06-14

### Added

- `armeabi-v7a` (32-bit ARM) support with hardware AES (ARMv8 crypto extensions in AArch32, built with `-march=armv8-a+crypto`), e.g. for 64-bit CPUs running a 32-bit Android such as the Samsung Galaxy A13. Encrypted files stay byte-compatible across ABIs, and a CPU without the AES extensions gets an error instead of `SIGILL`

### Changed

- The minimum SDK is now 28 (Android 9), which guarantees that the `armeabi-v7a` library only runs on CPUs with the AES instructions. Apps that pin `reactNativeArchitectures` need to include `armeabi-v7a`

## [1.3.5] - 2025-10-18

### Fixed

- Opening a locked file deleted it

## [1.3.3] - 2025-09-30

### Changed

- Support 16 KB page sizes
- Upgrade to Nitro Modules 0.29.7

## [1.3.0] - 2025-06-29

### Added

- Debug logs, turned on with the environment variable `NITRO_MMFILE_DEBUG=1`

### Changed

- Opening a file that is already open fails

## [1.2.2] - 2025-06-13

### Fixed

- Header synchronisation for shrunk files

## [1.2.1] - 2025-06-13

### Fixed

- Missing `lib/index.js` and `lib/index.d.ts` files

## [1.2.0] - 2025-06-11

### Added

- `readDirSync()` and `unlinkSync()` as synchronous alternatives to `readDir()` and `unlink()`

### Fixed

- `getFileSize()` and `getEncryptedFileSize()` returning non-zero values for directories

## [1.1.1] - 2025-04-05

### Added

- Medium article in the README

### Fixed

- Repository link in `package.json`

## [1.1.0] - 2025-04-01

### Added

- `readOnly` option for opening files, `baseDirectory`, `fileExists()`, `getFileSize()`, `getEncryptedFileSize()`, `readDir()` and `unlink()`

### Changed

- The API is the `Mmfile` object (`Mmfile.openMmfile()`, `Mmfile.openEncryptedMmfile()`) instead of standalone functions

## [1.0.2] - 2025-03-30

### Changed

- Include the `docs` folder in the package

## [1.0.1] - 2025-03-30

### Fixed

- `write()` and `append()` after `clear()`

## [1.0.0] - 2025-03-29

### Added

- Initial release

[Unreleased]: https://github.com/d4l-data4life/react-native-mmfile/compare/1.7.0...HEAD
[1.7.0]: https://github.com/d4l-data4life/react-native-mmfile/compare/1.6.0...1.7.0
[1.6.0]: https://github.com/d4l-data4life/react-native-mmfile/compare/1.5.0...1.6.0
[1.5.0]: https://github.com/d4l-data4life/react-native-mmfile/compare/1.4.0...1.5.0
[1.4.0]: https://github.com/d4l-data4life/react-native-mmfile/compare/1.3.5...1.4.0
[1.3.5]: https://github.com/d4l-data4life/react-native-mmfile/compare/1.3.3...1.3.5
[1.3.3]: https://github.com/d4l-data4life/react-native-mmfile/compare/1.3.0...1.3.3
[1.3.0]: https://github.com/d4l-data4life/react-native-mmfile/compare/1.2.2...1.3.0
[1.2.2]: https://github.com/d4l-data4life/react-native-mmfile/compare/1.2.1...1.2.2
[1.2.1]: https://github.com/d4l-data4life/react-native-mmfile/compare/1.2.0...1.2.1
[1.2.0]: https://github.com/d4l-data4life/react-native-mmfile/compare/1.1.1...1.2.0
[1.1.1]: https://github.com/d4l-data4life/react-native-mmfile/compare/1.1.0...1.1.1
[1.1.0]: https://github.com/d4l-data4life/react-native-mmfile/compare/1.0.2...1.1.0
[1.0.2]: https://github.com/d4l-data4life/react-native-mmfile/compare/1.0.1...1.0.2
[1.0.1]: https://github.com/d4l-data4life/react-native-mmfile/compare/1.0.0...1.0.1
[1.0.0]: https://github.com/d4l-data4life/react-native-mmfile/releases/tag/1.0.0
