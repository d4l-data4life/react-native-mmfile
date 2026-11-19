# Changelog

## 1.7.0 - 2026-11-19

### Security

- Generate the data key and IV of encrypted files with the OS random number generator (`SecRandomCopyBytes` on iOS, `getrandom(2)` with a `/dev/urandom` fallback on Android) instead of a Mersenne Twister seeded with 32 bits. The file format is unchanged; files created by earlier versions keep their keys.

### Changed

- The iOS pod now links the `Security` framework.

Earlier versions are described in the [GitHub releases](https://github.com/d4l-data4life/react-native-mmfile/releases).
